/*
 * Copyright 2010-2021 OpenXcom Developers.
 *
 * This file is part of OpenXcom.
 *
 * OpenXcom is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenXcom is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenXcom.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "DiplomacyFaction.h"
#include "SoldierPool.h"
#include <algorithm>
#include "../fmath.h"
#include "../Engine/Game.h"
#include "../Engine/RNG.h"
#include "../Engine/Logger.h"
#include "../Mod/Mod.h"
#include "../Mod/RuleMissionScript.h"
#include "../Mod/RuleDiplomacyFaction.h"
#include "../Mod/RuleDiplomacyFactionEvent.h"
#include "../Mod/RuleItem.h"
#include "../Mod/RuleResearch.h"
#include "../Mod/RuleSoldier.h"
#include "../Savegame/SavedGame.h"
#include "../Savegame/ItemContainer.h"
#include "../Savegame/FactionalResearch.h"
#include "../FTA/MasterMind.h"

namespace OpenXcom
{

DiplomacyFaction::DiplomacyFaction(const Mod* mod, const std::string& name):
	_mod(mod), _rule(nullptr), _reputationScore(0), _reputationLvL(0), _power(0), _vigilance(0), _helpTreatyTimer(0), _funds(0),
	_discovered(false), _thisMonthDiscovered(false), _repLvlChanged(false)
{
	_rule = _mod->getDiplomacyFaction(name);
	_items = new ItemContainer();
	_staffPool = new SoldierPool();
}

DiplomacyFaction::~DiplomacyFaction()
{
	for (auto& i : _research)
	{
		delete i;
	}
	delete _items;
	delete _staffPool;
}

/**
 * Loads the Diplomacy Faction from YAML.
 * @param node The YAML node containing the data.
 */
void DiplomacyFaction::load(const YAML::YamlNodeReader &reader, SavedGame *save)
{
	reader.tryRead("reputationScore", _reputationScore);
	reader.tryRead("reputationLvL", _reputationLvL);
	reader.tryRead("reputationName", _reputationName);
	reader.tryRead("repLvlChanged", _repLvlChanged);
	reader.tryRead("funds", _funds);
	reader.tryRead("power", _power);
	reader.tryRead("vigilance", _vigilance);
	reader.tryRead("helpTreatyTimer", _helpTreatyTimer);
	reader.tryRead("discovered", _discovered);
	reader.tryRead("thisMonthDiscovered", _thisMonthDiscovered);
	reader.tryRead("treaties", _treaties);
	reader.tryRead("unlockedResearches", _unlockedResearches);
	_items->load(reader["items"], _mod);
	for (const auto& researchReader : reader["research"].children())
	{
		std::string name;
		researchReader.tryRead("name", name);
		if (_mod->getResearch(name))
		{
			FactionalResearch* r = new FactionalResearch(_mod->getResearch(name), this);
			r->load(researchReader, save, _mod);
			_research.push_back(r);
		}
		else
		{
			Log(LOG_ERROR) << "Failed to load research " << name;
		}
	}

	_staffPool->load(reader["soldierPool"], save, _mod);

	reader.tryRead("dailyRepScore", _dailyRepScore);
}

/**
 * Saves the Diplomacy Faction to YAML.
 * @return YAML node.
 */
void DiplomacyFaction::save(YAML::YamlNodeWriter writer) const
{
	writer.setAsMap();
	writer.write("name", _rule->getName());
	writer.write("reputationScore", _reputationScore);
	writer.write("reputationLvL", _reputationLvL);
	writer.write("reputationName", _reputationName);
	if (_repLvlChanged)
	{
		writer.write("repLvlChanged", _repLvlChanged);
	}
	writer.write("funds", _funds);
	writer.write("power", _power);
	writer.write("vigilance", _vigilance);
	writer.write("helpTreatyTimer", _helpTreatyTimer);
	if (_discovered)
	{
		writer.write("discovered", _discovered);
	}
	if (_thisMonthDiscovered)
	{
		writer.write("thisMonthDiscovered", _thisMonthDiscovered);
	}
	writer.write("treaties", _treaties);
	writer.write("unlockedResearches", _unlockedResearches);
	_items->save(writer["items"]);
	_staffPool->save(writer["soldierPool"], _mod);

	writer.write("research", _research,
		[&](YAML::YamlNodeWriter& vectorWriter, FactionalResearch* r)
		{
			r->save(vectorWriter.write(), _mod);
		}
	);
	writer.write("dailyRepScore", _dailyRepScore);
}

/**
 * Updates reputation score based on incoming value and handle simple reaction to it.
 * @param change value that should be added to reputation score.
 */
void DiplomacyFaction::updateReputationScore(int change)
{
	if (change == 0)
		return;

	_reputationScore += change;
	_dailyRepScore.push_back(change);
}

/**
 * Removes research projet's name to a faction's list of unlocked researches.
 * @param research name of research project.
 */
void DiplomacyFaction::disableResearch(const std::string& research)
{
	bool erased = false;
	std::vector<std::string>::iterator r = std::find(_unlockedResearches.begin(), _unlockedResearches.end(), research);
	if (r != _unlockedResearches.end())
	{
		_unlockedResearches.erase(r);
		erased = true;
	}
	if (!erased) { Log(LOG_ERROR) << "Research project  named " << research << " was not deleted from <unlockedResearches> list!"; }
}


void DiplomacyFaction::addItem(const RuleItem* item, int qty)
{
	if (qty > 0)
	{
		_items->addItem(item, qty);
	}

}

void DiplomacyFaction::removeItem(const RuleItem* item, int qty)
{
	if (qty > 0)
	{
		_items->removeItem(item, qty);
	}
}

/**
 * Main handler of Faction logic.
 * @param engine - game engine
 * @param period - timestep to determine think process
 */
void DiplomacyFaction::think(Game& engine, ThinkPeriod period)
{
	SavedGame& save = *engine.getSavedGame();
	MasterMind& mind = *engine.getMasterMind();
	_commandsToProcess.clear();
	_availableMissionScripts.clear();
	_eventsToProcess.clear();
	_avoidRepeatVars.clear();

	//let's process out daily duty
	if (period == TIMESTEP_DAILY || period == TIMESTEP_10_DAYS)
	{
		//check if we discover our faction
		if (!_discovered && save.isResearched(_mod->getResearch(_rule->getDiscoverResearch())))
		{
			setDiscovered(true);
			setThisMonthDiscovered(true);
			//spawn celebration event if faction wants it
			if (!_rule->getDiscoverEvent().empty())
			{
				save.spawnEvent(engine.getMod()->getEvent(_rule->getDiscoverEvent()));
			}
			// update reputation level for just discovered fraction
			mind.updateReputationLvl(this, false);
			// and if it turns friendly at start we sign help treaty by default
			if (_reputationLvL > 0)
			{
				_treaties.push_back("STR_HELP_TREATY");
			}
		}
		//check if we need to react reputation score change
		if (!_dailyRepScore.empty())
		{
			processDailyReputation(engine);
		}

		//it's management time!
		if (period == TIMESTEP_10_DAYS)
		{
			processFactionalEvents(engine);
			Log(LOG_INFO) << "Managing faction:  " << _rule->getName() << " has funds: " << _funds << " and power: " << _power << "."; //#CLEARLOGS
			managePower();
			handleRestock();
			handleSelling();
			manageStaff(engine);
			Log(LOG_INFO) << ">>> Faction management finished:  " << _rule->getName() << " has funds: " << _funds << " and power: " << _power << "."; //#CLEARLOGS
		}
		//handleResearch(engine); //#FINNIKTODO

		if (_discovered)
		{
			//process treaty logic
			if (std::find(_treaties.begin(), _treaties.end(), "STR_HELP_TREATY") != _treaties.end())
			{
				//load treaty scripts from rules to generate missions and geoscape events
				if (_helpTreatyTimer > 0)
				{
					_helpTreatyTimer -= 1;
				}
				else
				{
					_commandsToProcess = _rule->getHelpTreatyMissions();
					_helpTreatyTimer = _rule->getHelpTreatyGap();
				}
				_eventsToProcess = _rule->getHelpTreatyEventScripts();
			}

			//generate missions and events
			factionMissionGenerator(engine);

			if (!_rule->getUsualEventScripts().empty())
			{
				for (auto& s : _rule->getUsualEventScripts())
				{
					_eventsToProcess.push_back(s);
				}
			}
			mind.eventScriptProcessor(_eventsToProcess, SCRIPT_FACTIONAL);
		}
	}
}

/**
 * Handle daily reputation change and immediate reaction to it.
 * @param engine game engine.
 */
void DiplomacyFaction::processDailyReputation(Game& engine)
{
	const Mod& mod = *engine.getMod();
	SavedGame& save = *engine.getSavedGame();
	MasterMind& mind = *engine.getMasterMind();
	int dailyReputation = 0;
	int breakLevel = mod.getReputationBreakthroughValue();
	std::vector<std::string> events;
	bool needUpdate = false;

	for (auto& n : _dailyRepScore)
	{
		dailyReputation += n;
	}
	_dailyRepScore.clear();

	if (dailyReputation * 1.2 > breakLevel)
	{
		events = _rule->getHappyEvents();
	}
	else if (dailyReputation < breakLevel * (-1))
	{
		events = _rule->getAngryEvents();
		_treaties.clear(); //we break all friendly-like treaties in this case
	}

	if (needUpdate)
	{
		_repLvlChanged = mind.updateReputationLvl(this, false);;
		if (!events.empty())
		{
			save.spawnEvent(engine.getMod()->getEvent(events.at(RNG::generate(0, events.size() - 1))));
		}
	}
}

void DiplomacyFaction::processFactionalEvents(Game& engine)
{
	const Mod& mod = *engine.getMod();
	SavedGame& save = *engine.getSavedGame();

	auto &list = this->getRules()->getFactionalEvents();

	for (auto &i : list)
	{
		auto rule = mod.getDiplomacyFactionEvent(i);
		if (rule != nullptr)
		{
			if (RNG::percent(rule->getExecutionOdds()))
			{
				auto month = save.getMonthsPassed();
				if (rule->getFirstMonth() <= month &&
					(rule->getLastMonth() >= month || rule->getLastMonth() == -1) &&
					rule->getMinScore() <= save.getCurrentScore(month) &&
					rule->getMaxScore() >= save.getCurrentScore(month) &&
					rule->getMinDifficulty() <= save.getDifficulty() &&
					rule->getMaxDifficulty() >= save.getDifficulty() &&
					rule->getMinPower() <= _power &&
					rule->getMaxPower() >= _power &&
					rule->getMinFunds() <= _funds &&
					rule->getMaxFunds() >= _funds)
				{
					// level two condition check: make sure we meet any player's research requirements, if any.
					bool triggerHappy = true;
					for (std::map<std::string, bool>::const_iterator j = rule->getPlayerResearchTriggers().begin(); triggerHappy && j != rule->getPlayerResearchTriggers().end(); ++j)
					{
						triggerHappy = (save.isResearched(j->first) == j->second);
					}
					if (triggerHappy)
					{
						// factional research requirments
						for (auto& triggerFactionsResearch : rule->getFactionResearchTriggers())
						{
							triggerHappy = (isResearched(triggerFactionsResearch.first) == triggerFactionsResearch.second);
						}
					}
					if (triggerHappy)
					{
						// item requirements
						for (auto& triggerItem : rule->getItemTriggers())
						{
							triggerHappy = ((_items->getItem(triggerItem.first) > 0) == triggerItem.second);
						}
					}

					//ok, we are happy with this factional event, let's finally process its effects!
					if (triggerHappy)
					{
						Log(LOG_INFO) << "Faction:  " << _rule->getName() << " got event: " << rule->getType(); //#CLEARLOGS
						_power += rule->getPowerChange();
						_funds += rule->getFundsChange();
						_vigilance += rule->getVigilanceChange();

						if (!rule->getDiscoveredResearches().empty())
						{
							for (auto r : rule->getDiscoveredResearches())
							{
								auto ruleResearch = mod.getResearch(r);
								if (ruleResearch != nullptr && !isResearched(ruleResearch))
								{
									unlockResearch(ruleResearch->getName());

									if (!ruleResearch->getGetOneFree().empty())
									{
										for (auto g : ruleResearch->getGetOneFree())
										{
											unlockResearch(g->getName());
										}
									}

									if (!ruleResearch->getSpawnedItem().empty())
									{
										auto ruleSpawnedItem = mod.getItem(ruleResearch->getSpawnedItem());
										if (ruleSpawnedItem != nullptr)
										{
											_items->addItem(ruleSpawnedItem, 1);
										}
									}

									if (!ruleResearch->getDisabled().empty())
									{
										for (auto d : ruleResearch->getDisabled())
										{
											disableResearch(d->getName());
										}
									}
								}
							}
						}

						for (auto& item : rule->getItemsToAdd())
						{
							auto ruleItem = mod.getItem(item.first);
							if (ruleItem)
							{
								int val = item.second;
								if (val > 0)
								{
									addItem(ruleItem, val);
								}
								else if (val < 0)
								{
									if (_items->getItem(ruleItem) >= val)
									{
										removeItem(ruleItem, val);
									}
								}
							}
						}

						for (auto &s : rule->getStaffToAdd())
						{
							auto ruleSoldier = mod.getSoldier(s.first);
							if (ruleSoldier)
							{
								for (int j = 0; j < s.second; ++j)
								{
									_staffPool->createSoldier(ruleSoldier, &mod, &save);
								}
							}
						}
					}
				}
			}
		}
	}
}

/**
 * Handle initial processing of factional missions and stores success cases.
 * @param engine game engine.
 */
void DiplomacyFaction::factionMissionGenerator(Game& engine)
{
	if (!_commandsToProcess.empty())
	{
		const Mod& mod = *engine.getMod();
		SavedGame& save = *engine.getSavedGame();
		if (RNG::percent(_rule->getGenMissionFrequency()))
		{
			for (auto &name : _commandsToProcess)
			{
				auto ruleScript = mod.getMissionScript(name);
				auto month = save.getMonthsPassed();
				int loyalty = save.getLoyalty();
				// lets process mission script with own way, first things first
				if (RNG::percent(ruleScript->getExecutionOdds())
					&& ruleScript->getFirstMonth() <= month &&
					(ruleScript->getLastMonth() >= month || ruleScript->getLastMonth() == -1) &&
					(ruleScript->getMinScore() <= save.getCurrentScore(month)) &&
					(ruleScript->getMaxScore() >= save.getCurrentScore(month)) &&
					(ruleScript->getMinLoyalty() <= loyalty) &&
					(ruleScript->getMaxLoyalty() >= loyalty) &&
					(ruleScript->getMinFunds() <= save.getFunds()) &&
					(ruleScript->getMaxFunds() >= save.getFunds()) &&
					ruleScript->getMinDifficulty() <= save.getDifficulty())
				{
					// don't forget about FTA-specific stuff
					if (save.getMissionScriptGapped(ruleScript->getType()))
					{
						//nope, we don't want such mission
						continue;
					}
					// level two condition check: make sure we meet any research requirements, if any.
					bool triggerHappy = true;
					bool avoidRepeat = false;
					if (ruleScript->getRepeatAvoidance() > 0 && !ruleScript->getVarName().empty())
					{
						if (std::find(_avoidRepeatVars.begin(), _avoidRepeatVars.end(), ruleScript->getVarName()) != _avoidRepeatVars.end())
						{
							triggerHappy = false;
						}
						else
						{
							avoidRepeat = true;
						}
					}

					for (std::map<std::string, bool>::const_iterator j = ruleScript->getResearchTriggers().begin(); triggerHappy && j != ruleScript->getResearchTriggers().end(); ++j)
					{
						triggerHappy = (save.isResearched(j->first) == j->second);
					}
					if (triggerHappy)
					{
						// item requirements
						for (auto& triggerItem : ruleScript->getItemTriggers())
						{
							triggerHappy = (save.isItemObtained(triggerItem.first, &mod) == triggerItem.second);
						}
					}
					if (triggerHappy)
					{
						// facility requirements
						for (auto& triggerFacility : ruleScript->getFacilityTriggers())
						{
							triggerHappy = (save.isFacilityBuilt(triggerFacility.first) == triggerFacility.second);
						}
					}
					// levels one and two passed: insert this command into the array.
					if (triggerHappy)
					{
						_availableMissionScripts.push_back(ruleScript);
						if (avoidRepeat)
						{
							_avoidRepeatVars.push_back(ruleScript->getVarName());
						}
					}
				}
			}
		}
	}
}

/**
 * Handle purchasing of Faction's items, based on current situation.
 */
void DiplomacyFaction::handleRestock()
{
	for (const auto& [wishListItem, weight] : _rule->getWishList())
	{
		RuleItem* ruleItem = _mod->getItem(wishListItem);
		int cost = ruleItem->getBuyCost();
		if (cost <= 0 || !ruleItem)
		{
			// we don't have cost for the item, skip that trash!
			continue;
		}

		// first, we see if we can handle this item
		if (ruleItem->getBuyRequirements().empty() || isResearched(ruleItem->getBuyRequirements()))
		{
			// calculate wanted amount purchase things
			int64_t toBuy = round(weight * _power / 100) - getItems()->getItem(ruleItem);
			// now we can purchase things
			if (toBuy > 0 && _funds > 0)
			{
				Log(LOG_INFO) << "Buyng: " << toBuy << "of item:  " << ruleItem->getName() << " that has weight: " << weight << " and current stock is: " << getItems()->getItem(ruleItem); //#CLEARLOGS
				_items->addItem(ruleItem, (int)toBuy);
				_funds -= toBuy * cost;
			}
		}
	}
}

/**
 * Handle managing of Faction's staff and non-item equipment.
 * @param mod rulesets to get constant data.
 */
void DiplomacyFaction::handleSelling()
{
	std::map<const RuleItem*, std::pair<int, int>> sellList;

	for (auto [item, value] : *_items->getContents())
	{
		int cost = item->getBuyCost(); //yes, faction can sell items with purchase cost
		if (!cost)
		{
			cost = item->getSellCost(); //well, in case we can't buy it, this would be ok too =)
		}
		if (!cost)
		{
			cost = 1; // extra safe if something bad would come to faction stash, it would cost at least $1
		}

		double wishWeight = 0;
		for (auto& [wishItem, weight] : _rule->getWishList())
		{
			if (wishItem == item->getType())
			{
				wishWeight = weight;
				break;
			}
		}
		// calculate desired amount of that item

		int toSell = value - round(wishWeight * _power / 100);
		if (toSell > 0)
		{
			sellList.insert(std::make_pair(item, std::make_pair(toSell, cost)));
		}
	}

	if (!sellList.empty())
	{
		for (auto [sellItem, values] : sellList)
		{
			Log(LOG_INFO) << "Selling: " << values.first << " of item: " << sellItem->getName(); //#CLEARLOGS
			removeItem(sellItem, values.first);
			int64_t dFunds = values.first;
			dFunds *= values.second; // sorry for that, was too lasy to make a structure
			_funds += dFunds;
		}
	}
}

/**
 * Handle managing of Faction's staff and non-item equipment.
 * @param mod rulesets to get constant data.
 */
void DiplomacyFaction::manageStaff(Game& engine)
{
	std::map<const RuleSoldier*, int> buyList;

	// handle soldiers
	for (auto &weights : _rule->getStaffWeights())
	{
		const RuleSoldier* soldierRule = _mod->getSoldier(weights.first);

		if (soldierRule->getBuyCost() != 0 && isResearched(soldierRule->getRequirements()))
		{
			int soldiersInPool = 0;
			for (auto hiredSoldier: _staffPool->getSoldiers())
			{
				if (hiredSoldier->getRules() == soldierRule)
				{
					soldiersInPool++;
				}
			}
			int64_t amount = round(weights.second * _power / 100) - soldiersInPool;

			if (amount > 0)
			{
				buyList.insert(std::make_pair(soldierRule, (int)amount));
			}
		}
	}

	for (auto [rule, amount] : buyList)
	{
		if (RNG::percent(70 - (engine.getSavedGame()->getDifficultyCoefficient()) * 10))
		{
			int nationality = engine.getSavedGame()->selectSoldierNationalityByLocation(engine.getMod(), rule, nullptr);
			_staffPool->addSoldier(engine.getMod()->genSoldier(engine.getSavedGame(), rule, nationality));
		}

	}
}

/**
 * Handle balancing of Faction's power.
 * @param month
 * @param baseCost
 */
void DiplomacyFaction::managePower()
{

	int powerHungry = _rule->getPowerHungry();
	int64_t reqFunds = _power;
	reqFunds *= powerHungry;
	if (reqFunds > _funds)
	{
		//we lose some power!
		int powerLost = round((reqFunds - _funds) / powerHungry);
		if (powerLost > 0)
		{
			_power -= powerLost;
			int64_t gainedFunds = powerLost;
			gainedFunds *= powerHungry;
			Log(LOG_INFO) << "reqFunds:  " << reqFunds << " > funds: " << _funds << ", power loss is: " << powerLost << " and funds gain is: " << gainedFunds; //#CLEARLOGS
			_funds += gainedFunds;
		}
	}
	else if (reqFunds < _funds && _vigilance > 0)
	{
		int64_t spareFunds = _funds - reqFunds;
		int64_t vigilanceCost = _vigilance;
		vigilanceCost *= powerHungry;
		if (spareFunds >= vigilanceCost)
		{
			Log(LOG_INFO) << "spareFunds:  " << spareFunds << " > vigilanceCost: " << vigilanceCost << ", power gain equal vigilance: " << _vigilance; //#CLEARLOGS
			_power += _vigilance;
			_funds -= vigilanceCost;
			_vigilance = 0;
		}
		else
		{
			int powerGain = floor(spareFunds / powerHungry);
			if (powerGain > 0)
			{
				Log(LOG_INFO) << "powerGain:  " << powerGain << " > 0 due to having spare funds: " << spareFunds << " and vigilance: " << _vigilance << ", increasing power"; //#CLEARLOGS
				_power += powerGain;
				_vigilance -= powerGain;
				vigilanceCost = powerGain;
				vigilanceCost *= powerHungry;
				_funds -= vigilanceCost;

			}
		}

	}
}

/**
 * Handle managing of Faction's staff and non-item equipment.
 * @param engine
 */
void DiplomacyFaction::handleResearch(Game& engine) //#FINNIKTODO - refactor with new soldier-based scientists logic
{
	SavedGame& save = *engine.getSavedGame();
	if (!_research.empty())
	{
		for (FactionalResearch* project : _research)
		{
			if (project->step())
			{
				//project finished
				const RuleResearch* bonus = 0;
				const RuleResearch* research = project->getRules();

				// core research first
				unlockResearch(research->getName());
				Log(LOG_INFO) << "Faction:  " << _rule->getName() << " finished research : " << project->getName(); //#CLEARLOGS

				// spawn item
				RuleItem* spawnedItem = _mod->getItem(research->getSpawnedItem());
				if (spawnedItem)
				{
					addItem(spawnedItem, 1);
				}

				// spawn event
				auto researchEvent = research->getSpawnedEvent();
				if (!researchEvent.empty())
				{
					save.spawnEvent(engine.getMod()->getEvent(researchEvent));
				}

				// process getOneFree
				if ((bonus = save.selectGetOneFree(research)))
				{
					unlockResearch(bonus->getName());
				}

				//clear research project
				/*_staff->addItem("STR_SCIENTIST", (*p)->getScientists());*/

				Collections::deleteIf(_research, 1,
					[&](FactionalResearch* r)
					{
						return r == project;
					}
				);
			}
			else // project still in development
			{
				//std::vector<Soldier*> scientists;
				//
				//// handle low funds case
				//if (_funds < reqFunds / 4)
				//{
				//	if ((*p)->getScientists() > 1) // we can keep lone guy doing his stuff
				//	{
				//		Log(LOG_INFO) << "Faction:  " << _rule->getName() << " has too low funds: " << _funds << " < required / 4 = " << reqFunds / 4
				//			<< " and project: " << (*p)->getName() << " was reduced in funding!"; //#CLEARLOGS
				//		int qty = ceil((*p)->getScientists() / 2);
				//		//_staff->addItem("STR_SCIENTIST", qty);
				//		(*p)->setScientists((*p)->getScientists() - qty);
				//	}
				//}

				//else if (_staff->getItem("STR_SCIENTIST") > 0 && _funds > reqFunds / 2 && RNG::percent(40)) // we choose to rise funding on this project
				//{
				//	int qty = floor(RNG::generate(0, _staff->getItem("STR_SCIENTIST") / 4));
				//	_staff->removeItem("STR_SCIENTIST", qty);
				//	(*p)->setScientists((*p)->getScientists() + qty);
				//}
			}
		}
	}

	//// probably we can have some free researches unlocked
	////for (auto i = _mod->getResearchList().begin(); i != _mod->getResearchList().end(); ++i)
	////{
	////	RuleResearch* rRule = _mod->getResearch((*i));

	////	if (rRule->getCost() <= 0 && !rRule->getDependencies().empty() && isResearched(rRule->getDependencies()))
	////	{
	////		unlockResearch(rRule->getName());
	////	}
	////}

	// let's count how many research project teams we can potentially have
	//size_t totalScientists = 0; //static_cast<size_t>(_staff->getItem("STR_SCIENTIST"));
	//if (hasResearch)
	//{
	//	for (auto y : _research)
	//	{
	//		totalScientists += (*y).getScientists();
	//	}
	//}

	/*size_t projectSpots = floor(totalScientists / 10);
	if (totalScientists <= 10 && RNG::percent(totalScientists * 10))
	{
		projectSpots = 1;
	}*/

	//// now we should choose to start a new project
	//if (_staff->getItem("STR_SCIENTIST") > 0 && _funds > reqFunds && projectSpots >= (_research.size() + 1) && RNG::percent(100)) // if we have scientists, money and process was not canceled because of internal reasons
	//{
	//	std::vector<std::pair<int, RuleResearch*>> researchList;

	//	for (auto i = _mod->getResearchList().begin(); i != _mod->getResearchList().end(); ++i)
	//	{
	//		if (isResearched(*i))
	//		{
	//			continue; // we already know what is that!
	//		}
	//		RuleResearch* rRule = _mod->getResearch((*i));

	//		if (!rRule->getDependencies().empty() && isResearched(rRule->getDependencies()))// this one effectively splits xcom and factional research trees
	//		{

	//			if (rRule->getCost() == 0)
	//			{
	//				continue; // meh, too easy
	//			}

	//			if (rRule->needItem())
	//			{
	//				if (_items->getItem(_mod->getItem((*i))) <= 0 || _secretItems->getItem(_mod->getItem((*i))) <= 0)
	//				{
	//					continue; //sadly, we dont have required item
	//				}
	//			}

	//			//extra one to make sure we are not already researching it
	//			if (hasResearch)
	//			{
	//				bool ongoing = false;
	//				for (auto l = _research.begin(); l != _research.end(); ++l)
	//				{
	//					if ((*l)->getName() == rRule->getName())
	//					{
	//						ongoing = true;
	//						break; // ah, we are already researching this one!
	//					}
	//				}
	//				if (ongoing)
	//				{
	//					continue; // ok, we do not want to start same research again
	//				}
	//			}
	//			// this one looks fine, let's remember it
	//			int priority = rRule->getPoints() / rRule->getCost();
	//			researchList.push_back(std::make_pair(priority, rRule));
	//			Log(LOG_INFO) << "Faction:  " << _rule->getName() << " has potential research : " << rRule->getName() << ", processing! It has priority: " << priority; //#CLEARLOGS
	//		}
	//	}

	//	if (!researchList.empty())
	//	{
	//		// now we should pick the most sweet project to start
	//		std::sort(researchList.begin(), researchList.end());

	//		RuleResearch* choice = researchList.front().second; // our potential choice
	//		int priority = researchList.front().first;
	//		bool promising = true;
	//		int64_t factionCost = choice->getCost();
	//		//counts FTA is loaded, so we turn hours to days and say, that faction's research is 10 times slower, than player's
	//		factionCost = reqFunds * 2 / 3 - factionCost * 10 / 2400 * _rule->getScienceBaseCost();
	//		Log(LOG_INFO) << "factionCost for research project " << _rule->getName() << " is: " << factionCost
	//			<< " based on reqFunds: " << reqFunds << ", initial cost: " << choice->getCost() << " and science base cost: " << _rule->getScienceBaseCost(); //#CLEARLOGS

	//		if (_funds > factionCost) // looks like we would manage to deal with this one.
	//		{
	//			// now let's if this research is more valuable than already processed researches
	//			if (hasResearch)
	//			{
	//				for (auto k = _research.begin(); k != _research.end(); ++k)
	//				{
	//					if ((*k)->getPriority() >= priority)
	//					{
	//						promising = false;
	//						break; //ongoing research is better
	//					}
	//					else if ((*k)->getPriority() * 2 < priority)
	//					{
	//						// now we are talking, looks like we are wasting time here, let's reduce funding of this crap!
	//						if ((*k)->getScientists() > 1) // we can keep lone guy doing his stuff
	//						{
	//							int qty = ceil((*k)->getScientists() / 2);
	//							_staff->addItem("STR_SCIENTIST", qty);
	//							(*k)->setScientists((*k)->getScientists() - qty);
	//						}
	//					}
	//				}
	//				if (!promising)
	//				{
	//					return; // we should not start new researches as we have a lot more things to do right now
	//				}
	//			}

	//			// finally, we can start a new research project
	//			FactionalResearch* newResearch = new FactionalResearch(choice, this);
	//			_research.push_back(newResearch);
	//			int randomCost = (double)factionCost / 4;
	//			newResearch->setTimeLeft((int)factionCost + RNG::generate(-randomCost, randomCost));
	//			newResearch->setPriority(priority);
	//			int qty = _staff->getItem("STR_SCIENTIST");
	//			qty = RNG::generate(qty * 0.5, qty * 0.9); // FINNIKTODO: think more about how many scientists faction should assign on a new project
	//			newResearch->setScientists(qty);
	//			_staff->removeItem("STR_SCIENTIST", qty);
	//			_funds -= factionCost / 10;

	//			Log(LOG_INFO) << "Faction:  " << _rule->getName() << " has chosen research : " << choice->getName()
	//				<< " with priority: " << priority; //#CLEARLOGS
	//		}
	//	}
	//}
}

bool DiplomacyFaction::isResearched(const std::string& name) const
{
	if (std::find(_unlockedResearches.begin(), _unlockedResearches.end(), name) != _unlockedResearches.end())
	{
		return true;
	}
	return false;
}

bool DiplomacyFaction::isResearched(const RuleResearch* rule) const
{
	if (std::find(_unlockedResearches.begin(), _unlockedResearches.end(), rule->getName()) != _unlockedResearches.end())
	{
		return true;
	}
	return false;
}

bool DiplomacyFaction::isResearched(const std::vector<std::string>& names) const
{
	if (names.empty())
		return true;

	for (const std::string& r : names)
	{
		if (isResearched(r))
		{
			return true;
		}
	}

	return false;
}

bool DiplomacyFaction::isResearched(const std::vector<const RuleResearch*>& rules) const
{
	if (rules.empty())
		return true;

	for (auto r : rules)
	{
		if (isResearched(r))
		{
			return true;
		}
	}

	return false;
}

}

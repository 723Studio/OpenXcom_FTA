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
#include <assert.h>
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
#include "../Mod/RuleCraft.h"
#include "../Savegame/SavedGame.h"
#include "../Savegame/ItemContainer.h"
#include "../Savegame/FactionalContainer.h"
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
	_secretItems = new ItemContainer();
	_staff = new FactionalContainer();
}

DiplomacyFaction::~DiplomacyFaction()
{
	for (std::vector<FactionalResearch*>::iterator i = _research.begin(); i != _research.end(); ++i)
	{
		delete* i;
	}
	delete _items;
	delete _secretItems;
	delete _staff;
}

/**
 * Loads the Diplomacy Faction from YAML.
 * @param node The YAML node containing the data.
 */
void DiplomacyFaction::load(const YAML::Node &node)
{
	_reputationScore = node["reputationScore"].as<int>(_reputationScore);
	_reputationLvL = node["reputationLvL"].as<int>(_reputationLvL);
	_reputationName = node["reputationName"].as<std::string>(_reputationName);
	_repLvlChanged = node["repLvlChanged"].as<bool>(_repLvlChanged);
	_funds = node["funds"].as<int>(_funds);
	_power = node["power"].as<int>(_power);
	_vigilance = node["vigilance"].as<int>(_vigilance);
	_helpTreatyTimer = node["helpTreatyTimer"].as<int>(_helpTreatyTimer);
	_discovered = node["discovered"].as<bool>(_discovered);
	_thisMonthDiscovered = node["thisMonthDiscovered"].as<bool>(_thisMonthDiscovered);
	_treaties = node["treaties"].as<std::vector<std::string>>(_treaties);
	_unlockedResearches = node["unlockedResearches"].as<std::vector<std::string>>(_unlockedResearches);
	_items->load(node["items"]);
	_staff->load(node["staff"]);
	for (YAML::const_iterator i = node["research"].begin(); i != node["research"].end(); ++i)
	{
		std::string name = (*i)["name"].as<std::string>();
		if (_mod->getResearch(name))
		{
			FactionalResearch* r = new FactionalResearch(_mod->getResearch(name), this);
			r->load(*i);
			_research.push_back(r);
		}
		else
		{
			Log(LOG_ERROR) << "Failed to load research " << name;
		}
	}
	_dailyRepScore = node["dailyRepScore"].as<std::vector<int>>(_dailyRepScore);
}

/**
 * Saves the Diplomacy Faction to YAML.
 * @return YAML node.
 */ 
YAML::Node DiplomacyFaction::save() const
{
	YAML::Node node;
	node["name"] = _rule->getName();
	node["reputationScore"] = _reputationScore;
	node["reputationLvL"] = _reputationLvL;
	node["reputationName"] = _reputationName;
	if (_repLvlChanged)
	{
		node["repLvlChanged"] = _repLvlChanged;
	}
	node["funds"] = _funds;
	node["power"] = _power;
	node["vigilance"] = _vigilance;
	node["helpTreatyTimer"] = _helpTreatyTimer;
	if (_discovered)
	{
		node["discovered"] = _discovered;
	}
	if (_thisMonthDiscovered)
	{
		node["thisMonthDiscovered"] = _thisMonthDiscovered;
	}
	node["treaties"] = _treaties;
	node["unlockedResearches"] = _unlockedResearches;
	node["items"] = _items->save();
	node["staff"] = _staff->save();
	for (std::vector<FactionalResearch*>::const_iterator i = _research.begin(); i != _research.end(); ++i)
	{
		node["research"].push_back((*i)->save());
	}
	node["dailyRepScore"] = _dailyRepScore;
	return node;
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
		bool goToSecret = false;
		if (_secretItems->getItem(item) <= 0 && item->isAlienArtifact())
		{
			auto it = std::find(_mod->getResearchList().begin(), _mod->getResearchList().end(), item->getName());
			if (it != _mod->getResearchList().end())
			{
				if (!isResearched(*it))
				{
					goToSecret = true;
				}
			}
		}
		if (goToSecret)
		{
			_secretItems->addItem(item, 1);
			qty -= 1;
		}
		if (qty > 0)
		{
			_items->addItem(item, qty);
		}
	}

}

void DiplomacyFaction::removeItem(const RuleItem* item, int qty)
{
	_items->removeItem(item, qty);
}

/**
 * Main handler of Faction logic.
 * @param Game game engine.
 * @param ThinkPeriod - timestep to determine think process
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

		//ok, what would happen today?
		processFactionalEvents(engine);

		//it's management time!
		handleSelling(*engine.getMod());
		if (period == TIMESTEP_10_DAYS)
		{
			Log(LOG_INFO) << "Handling restock, Faction:  " << _rule->getName() << " has funds: " << _funds << " and power: " << _power << "."; //#CLEARLOGS
			handleRestock();
		}
		//manageStaff(); #FINNIKTODO uncomment with alpha 2
		int64_t reqFunds = managePower(save.getMonthsPassed(), _mod->getDefaultFactionPowerCost());
		handleResearch(engine, reqFunds);

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
 * Handle daily reputation change and immidiate reaction to it.
 * @param Game game engine.
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
						if (!triggerHappy)
							continue;
					}
					if (triggerHappy)
					{
						// factional research requirments
						for (auto& triggerFactionsResearch : rule->getFactionResearchTriggers())
						{
							triggerHappy = (isResearched(triggerFactionsResearch.first) == triggerFactionsResearch.second);
							if (!triggerHappy)
								continue;
						}
					}
					if (triggerHappy)
					{
						// item requirements
						for (auto& triggerItem : rule->getItemTriggers())
						{
							triggerHappy = ((_items->getItem(triggerItem.first) > 0) == triggerItem.second);
							if (!triggerHappy)
								continue;
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

						if (!rule->getItemsToAdd().empty())
						{
							for (auto &item : rule->getItemsToAdd())
							{
								auto ruleItem = mod.getItem(item.first);
								if (ruleItem != nullptr)
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
						}
					}
				}
			}
		}
	}
}

/**
 * Handle initial processing of factional missions and stores success cases.
 * @param Game game engine.
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
						//nope, we dont want such mission
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
						if (!triggerHappy)
							continue;
					}
					if (triggerHappy)
					{
						// item requirements
						for (auto& triggerItem : ruleScript->getItemTriggers())
						{
							triggerHappy = (save.isItemObtained(triggerItem.first) == triggerItem.second);
							if (!triggerHappy)
								continue;
						}
					}
					if (triggerHappy)
					{
						// facility requirements
						for (auto& triggerFacility : ruleScript->getFacilityTriggers())
						{
							triggerHappy = (save.isFacilityBuilt(triggerFacility.first) == triggerFacility.second);
							if (!triggerHappy)
								continue;
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
 * Handle purshaising of Faction's items, based on current situation.
 * @param mod rulesets to get constant data.
 */
void DiplomacyFaction::handleRestock()
{
	for (auto it = _rule->getWishList().begin(); it != _rule->getWishList().end(); ++it)
	{
		RuleItem* ruleItem = _mod->getItem((*it).first);
		int64_t cost = ruleItem->getBuyCost();
		if (cost <= 0)
		{
			// we don't have cost for the item, skip that trash!
			continue;
		}

		// first, we see if we can handle this item
		if (isResearched(ruleItem->getRequirements()) && isResearched(ruleItem->getBuyRequirements()))
		{
			// calculate wanted ammount purchase things
			int64_t toBuy = round((*it).second / cost * _power * _rule->getStockMod() / 1000);
			toBuy -= _items->getItem(ruleItem);
			// now we can purchase things
			if (toBuy > 0 && _funds > 0)
			{
				_items->addItem(ruleItem, (int)toBuy);
				_funds -= toBuy * cost;
				Log(LOG_INFO) << "> We buying items " << ruleItem->getType() << ": " << toBuy << ", and now it stocks: " << _items->getItem(ruleItem); //#CLEARLOGS
			}
		}
	}
	Log(LOG_INFO) << ">>>> Restock finished, Faction:  " << _rule->getName() << " has funds: " << _funds << "."; //#CLEARLOGS
}

/**
 * Handle managing of Faction's staff and non-item equipment.
 * @param mod rulesets to get constant data.
 */
void DiplomacyFaction::handleSelling(Mod& mod)
{
	std::map<RuleItem*, std::pair<int, int>> sellList;

	for (auto it = _items->getContents()->begin(); it != _items->getContents()->end(); ++it)
	{
		RuleItem* ruleItem = _mod->getItem((*it).first);
		int64_t cost = ruleItem->getBuyCost(); //yes, faction can sell items with purchase cost, or balancing resources would go crazy.
		if (!cost)
		{
			cost = ruleItem->getSellCost(); //well, in case we can't buy it, this would be ok too =)
		}
		if (!cost)
		{
			cost = 1; // extra safe if something bad would come to faction stash, it would cost at least $1
		}

		int64_t wishWeight = 0;
		for (auto k = _rule->getWishList().begin(); k != _rule->getWishList().end(); ++k)
		{
			if (k->first == (*it).first)
			{
				wishWeight = k->second;
				break;
			}
		}
		// calculate desired ammount of that item
		int64_t toSell = round((wishWeight / cost) * _power * _rule->getStockMod() / 1000);
		toSell = _items->getItem(ruleItem) - toSell;
		if (toSell > 0)
		{
			sellList.insert(std::make_pair(ruleItem, std::make_pair(toSell, cost)));
		}
	}

	if (!sellList.empty())
	{
		for (auto i = sellList.begin(); i != sellList.end(); ++i)
		{
			removeItem((*i).first, (*i).second.first);
			int64_t dFunds = (*i).second.first;
			dFunds *= (*i).second.second; // sorry for that, was too lasy to make a structure
			_funds += dFunds;
			Log(LOG_INFO) << "> We selling items " << (*i).first->getType() << ": " << (*i).second.first; //#CLEARLOGS
		}
	}

	Log(LOG_INFO) << ">>>> Sale finished, Faction:  " << _rule->getName() << " has funds: " << _funds; //#CLEARLOGS
}

/**
 * Handle managing of Faction's staff and non-item equipment.
 * @param mod rulesets to get constant data.
 */
void DiplomacyFaction::manageStaff()
{
	std::map<std::string, double> buyList;

	// handle soldiers
	const std::vector<std::string>& soldiers = _mod->getSoldiersList();
	for (std::vector<std::string>::const_iterator i = soldiers.begin(); i != soldiers.end(); ++i)
	{

		const RuleSoldier* rule = _mod->getSoldier(*i);
		
		if (rule->getBuyCost() != 0 && isResearched(rule->getRequirements()))
		{
			buyList.insert(std::make_pair((*i), rule->getBuyCost()));
		}
	}
	
	// handle STR_SCIENTIST
	if (isResearched("STR_SCIENTIST"))
	{
		buyList.insert(std::make_pair("STR_SCIENTIST", _mod->getHireScientistCost()));
	}

	// handle STR_ENGINEER
	if (isResearched("STR_ENGINEER"))
	{
		buyList.insert(std::make_pair("STR_ENGINEER", _mod->getHireEngineerCost()));
	}

	// handle crafts
	const std::vector<std::string>& crafts = _mod->getCraftsList();
	for (std::vector<std::string>::const_iterator i = crafts.begin(); i != crafts.end(); ++i)
	{

		RuleCraft* rule = _mod->getCraft(*i);

		if (rule->getBuyCost() != 0 && isResearched(rule->getRequirements()))
		{
			buyList.insert(std::make_pair((*i), rule->getBuyCost()));
		}
	}

	for (auto j = buyList.begin(); j != buyList.end(); ++j)
	{
		int weight = 0;
		for (auto k = _rule->getStaffWeights().begin(); k != _rule->getStaffWeights().end(); ++k)
		{
			if (k->first == (*j).first)
			{

				weight = k->second;
				break;
			}
		}

		if (weight)
		{
			
			int64_t toBuy = _power;
			toBuy *= weight;
			toBuy = floor(toBuy / 10000) - _staff->getItem((*j).first);

			if (toBuy)
			{
				int64_t cost = (*j).second;
				if (toBuy > 0 || _funds > 0)
				{
					_staff->addItem((*j).first, (int)toBuy);
					_funds -= toBuy * cost;
					Log(LOG_DEBUG) << "Faction:  " << _rule->getName() << " is buying staff " << (*j).first << ": " << toBuy << " because of coef: " << floor(_power / weight); //#CLEARLOGS
				}
				//else
				//{
				//	_staff->removeItem((*j).first, toBuy);
				//	_funds += toBuy * cost; //yes, faction can sell items with purchase cost, or balancing resources would go crazy.
				//	Log(LOG_DEBUG) << "Faction:  " << _rule->getName() << " is selling staff " << (*j).first << ": " << toBuy << " because of coef: " << floor(_power / weight); //#CLEARLOGS
				//}
			}
		}

	}
}

/**
 * Handle balancing of Faction's power.
 * @param mod rulesets to get constant data.
 */
int64_t DiplomacyFaction::managePower(int64_t month, int64_t baseCost) //#FINNIKCHECK
{
	int powerHungry = _rule->getPowerHungry();
	month += 1;
	double temp = month;
	temp /= month + 14;
	temp *= powerHungry;
	temp += (powerHungry / 2.33);
	int64_t reqFunds = _power * round(temp);
	temp = month * 0.017;
	int64_t powerCost = baseCost + (baseCost * round(temp));
	int dPower = 0;

	if (_funds < 0)
	{
		if (_funds < -reqFunds / 1.3)
		{
			dPower = static_cast<int>((-_funds + (reqFunds / 1.3)) / powerCost * 0.9);
			if (dPower > _power)
			{
				dPower = _power;
			}
			_power -= dPower;
			Log(LOG_INFO) << "Faction:  " << _rule->getName() << " updating power to: " << _power << " with dPower value: " << dPower << " because its funds: " << _funds << " are below limit: " << reqFunds; //#CLEARLOGS
			_funds += static_cast<int>(dPower * powerCost * 0.9);
		}
		else
		{
			return reqFunds;
		}
	}

	if (_vigilance > 0)
	{
		if (_funds > reqFunds)
		{
			int cost = static_cast<int>((_funds - reqFunds) / powerCost);
			dPower = std::min(cost, _vigilance);
			_power += dPower;
			_funds -= dPower * powerCost;
			Log(LOG_INFO) << "Faction:  " << _rule->getName() << " updating power to: " << _power << " with dPower value: " << dPower << " because it has vigilance: " << _vigilance << " and funds are above reqFunds value: " << reqFunds; //#CLEARLOGS
		}
		else
		{
			Log(LOG_INFO) << "Faction:  " << _rule->getName() << " has vigilance value : " << _vigilance << ", but can't update power because funds value: " << _funds << " below reqFunds: " << reqFunds; //#CLEARLOGS
		}
	}

	return reqFunds;
}

/**
 * Handle managing of Faction's staff and non-item equipment.
 * @param mod rulesets to get constant data.
 */
void DiplomacyFaction::handleResearch(Game& engine, int64_t reqFunds) //#FINNIKTODO - rafactor with new soldier-based scientists logic
{
	SavedGame& save = *engine.getSavedGame();

	bool hasResearch = !_research.empty();

	if (hasResearch)
	{
		for (auto p = _research.begin(); p != _research.end(); ++p)
		{
			if ((*p)->step())
			{
				//project finished
				const RuleResearch* bonus = 0;
				const RuleResearch* research = (*p)->getRules();

				// core research first
				unlockResearch(research->getName());
				Log(LOG_INFO) << "Faction:  " << _rule->getName() << " finished research : " << (*p)->getName(); //#CLEARLOGS

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
				_staff->addItem("STR_SCIENTIST", (*p)->getScientists());

				Collections::deleteIf(_research, 1,
					[&](FactionalResearch* r)
					{
						return r == (*p);
					}
				);
			}
			else // project still in development
			{
				// handle low funds case
				if (_funds < reqFunds / 4)
				{
					if ((*p)->getScientists() > 1) // we can keep lone guy doing his stuff
					{
						Log(LOG_INFO) << "Faction:  " << _rule->getName() << " has too low funds: " << _funds << " < required / 4 = " << reqFunds / 4
							<< " and project: " << (*p)->getName() << " was reduced in funding!"; //#CLEARLOGS
						int qty = ceil((*p)->getScientists() / 2);
						_staff->addItem("STR_SCIENTIST", qty);
						(*p)->setScientists((*p)->getScientists() - qty);
					}
				}
				else if (_staff->getItem("STR_SCIENTIST") > 0 && _funds > reqFunds / 2 && RNG::percent(40)) // we choose to rise funding on this project
				{
					int qty = floor(RNG::generate(0, _staff->getItem("STR_SCIENTIST") / 4));
					_staff->removeItem("STR_SCIENTIST", qty);
					(*p)->setScientists((*p)->getScientists() + qty);
				}
			}
		}
	}

	// probably we can have some free researches unlocked
	//for (auto i = _mod->getResearchList().begin(); i != _mod->getResearchList().end(); ++i)
	//{
	//	RuleResearch* rRule = _mod->getResearch((*i));

	//	if (rRule->getCost() <= 0 && !rRule->getDependencies().empty() && isResearched(rRule->getDependencies()))
	//	{
	//		unlockResearch(rRule->getName());
	//	}
	//}

	// let's count how many research project teams we can potentially have
	size_t totalScientists = static_cast<size_t>(_staff->getItem("STR_SCIENTIST"));
	if (hasResearch)
	{
		for (auto y : _research)
		{
			totalScientists += (*y).getScientists();
		}
	}

	size_t projectSpots = floor(totalScientists / 10);
	if (totalScientists <= 10 && RNG::percent(totalScientists * 10))
	{
		projectSpots = 1;
	}

	// now we should choose to start a new project
	if (_staff->getItem("STR_SCIENTIST") > 0 && _funds > reqFunds && projectSpots >= (_research.size() + 1) && RNG::percent(100)) // if we have scientists, money and process was not canceled because of internal reasons
	{
		std::vector<std::pair<int, RuleResearch*>> researchList;

		for (auto i = _mod->getResearchList().begin(); i != _mod->getResearchList().end(); ++i)
		{
			if (isResearched(*i))
			{
				continue; // we already know what is that!
			}
			RuleResearch* rRule = _mod->getResearch((*i));

			if (!rRule->getDependencies().empty() && isResearched(rRule->getDependencies()))// this one effectively splits xcom and factional research trees
			{

				if (rRule->getCost() == 0)
				{
					continue; // meh, too easy
				}

				if (rRule->needItem())
				{
					if (_items->getItem(_mod->getItem((*i))) <= 0 || _secretItems->getItem(_mod->getItem((*i))) <= 0)
					{
						continue; //sadly, we dont have required item
					}
				}

				//extra one to make sure we are not already researching it
				if (hasResearch)
				{
					bool ongoing = false;
					for (auto l = _research.begin(); l != _research.end(); ++l)
					{
						if ((*l)->getName() == rRule->getName())
						{
							ongoing = true;
							break; // ah, we are already researching this one!
						}
					}
					if (ongoing)
					{
						continue; // ok, we do not want to start same research again
					}
				}
				// this one looks fine, let's remember it
				int priority = rRule->getPoints() / rRule->getCost();
				researchList.push_back(std::make_pair(priority, rRule));
				Log(LOG_INFO) << "Faction:  " << _rule->getName() << " has potential research : " << rRule->getName() << ", processing! It has priority: " << priority; //#CLEARLOGS
			}
		}

		if (!researchList.empty())
		{
			// now we should pick the most sweet project to start
			std::sort(researchList.begin(), researchList.end());

			RuleResearch* choice = researchList.front().second; // our potential choice
			int priority = researchList.front().first;
			bool promising = true;
			int64_t factionCost = choice->getCost();
			//counts FTA is loaded, so we turn hours to days and say, that faction's research is 10 times slower, than player's
			factionCost = reqFunds * 2 / 3 - factionCost * 10 / 2400 * _rule->getScienceBaseCost();
			Log(LOG_INFO) << "factionCost for research project " << _rule->getName() << " is: " << factionCost
				<< " based on reqFunds: " << reqFunds << ", initial cost: " << choice->getCost() << " and science base cost: " << _rule->getScienceBaseCost(); //#CLEARLOGS

			if (_funds > factionCost) // looks like we would manage to deal with this one.
			{
				// now let's if this research is more valuable than already processed researches
				if (hasResearch)
				{
					for (auto k = _research.begin(); k != _research.end(); ++k)
					{
						if ((*k)->getPriority() >= priority)
						{
							promising = false;
							break; //ongoing research is better
						}
						else if ((*k)->getPriority() * 2 < priority)
						{
							// now we are talking, looks like we are wasting time here, let's reduce funding of this crap!
							if ((*k)->getScientists() > 1) // we can keep lone guy doing his stuff
							{
								int qty = ceil((*k)->getScientists() / 2);
								_staff->addItem("STR_SCIENTIST", qty);
								(*k)->setScientists((*k)->getScientists() - qty);
							}
						}
					}
					if (!promising)
					{
						return; // we should not start new researches as we have a lot more things to do right now
					}
				}

				// finally, we can start a new research project
				FactionalResearch* newResearch = new FactionalResearch(choice, this);
				_research.push_back(newResearch);
				int randomCost = (double)factionCost / 4;
				newResearch->setTimeLeft((int)factionCost + RNG::generate(-randomCost, randomCost));
				newResearch->setPriority(priority);
				int qty = _staff->getItem("STR_SCIENTIST");
				qty = RNG::generate(qty * 0.5, qty * 0.9); // FINNIKTODO: think more about how many scientists faction should assign on a new project
				newResearch->setScientists(qty);
				_staff->removeItem("STR_SCIENTIST", qty);
				_funds -= factionCost / 10;

				Log(LOG_INFO) << "Faction:  " << _rule->getName() << " has chosen research : " << choice->getName()
					<< " with priority: " << priority; //#CLEARLOGS
			}
		}
	}
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

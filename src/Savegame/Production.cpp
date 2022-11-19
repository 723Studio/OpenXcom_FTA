/*
 * Copyright 2010-2016 OpenXcom Developers.
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
#include "Production.h"
#include <algorithm>
#include "../Engine/Collections.h"
#include "../Mod/RuleManufacture.h"
#include "../Mod/RuleSoldier.h"
#include "Base.h"
#include "SavedGame.h"
#include "Transfer.h"
#include "ItemContainer.h"
#include "Soldier.h"
#include "Craft.h"
#include "BaseFacility.h"
#include "../Mod/Mod.h"
#include "../Mod/RuleItem.h"
#include "../Mod/RuleCraft.h"
#include "../Engine/Language.h"
#include "../Engine/RNG.h"
#include <climits>
#include "BaseFacility.h"

namespace OpenXcom
{
Production::Production(const RuleManufacture * rules, int amount) : _rules(rules), _amount(amount), _infinite(false), _timeSpent(0), _engineers(0), _sell(false)
{
	_efficiency = 100;
}

bool Production::haveEnoughMoneyForOneMoreUnit(SavedGame * g) const
{
	return _rules->haveEnoughMoneyForOneMoreUnit(g->getFunds());
}

bool Production::haveEnoughLivingSpaceForOneMoreUnit(Base * b)
{
	if (_rules->getSpawnedPersonType() != "")
	{
		// Note: if the production is running then the space we need is already counted by getUsedQuarters
		if (b->getAvailableQuarters() < b->getUsedQuarters())
		{
			return false;
		}
	}
	return true;
}

bool Production::haveEnoughMaterialsForOneMoreUnit(Base * b, const Mod *m) const
{
	for (auto& i : _rules->getRequiredItems())
	{
		if (b->getStorageItems()->getItem(i.first) < i.second)
			return false;
	}
	for (auto& i : _rules->getRequiredCrafts())
	{
		if (b->getCraftCountForProduction(i.first) < i.second)
			return false;
	}
	return true;
}

std::vector<Soldier*> Production::getAssignedSoldiers(Base* b)
{
	std::vector<Soldier*> assignedEngineers;
	for (auto s : *b->getSoldiers())
	{
		if (s->getProductionProject() == this)
		{
			assignedEngineers.push_back(s);
		}
	}
	return assignedEngineers;
}

int Production::getProgress(Base* b, SavedGame* g, const Mod* m, int loyaltyRating, bool prediction)
{
	if (!m->isFTAGame())
	{
		return _engineers;
	}
	else
	{
		int progress = 0;
		std::vector<Soldier*> assignedEngineers = getAssignedSoldiers(b);
		if (assignedEngineers.size() > 0)
		{
			double effort = 0;
			auto projStats = _rules->getStats();
			int factor = m->getEngineerTrainingFactor();
			int summEfficiency = 0;
			for (auto s : assignedEngineers)
			{
				auto stats = s->getStatsWithAllBonuses();
				auto caps = s->getRules()->getStatCaps();
				unsigned int statsN = 0;
				double soldierEffort = 0, statEffort = 0;
				Log(LOG_DEBUG) << "Engeneer " << s->getName() << " is calculating his/her effort for the manufacturing project";
				if (projStats.weaponry > 0)
				{
					statEffort = stats->weaponry;
					soldierEffort += statEffort / projStats.weaponry;
					if (!prediction && stats->weaponry < caps.weaponry && RNG::generate(0, caps.weaponry) > stats->weaponry && RNG::percent(factor))
						s->getEngineerExperience()->weaponry++;
					statsN++;
				}
				if (projStats.explosives > 0)
				{
					statEffort = stats->explosives;
					soldierEffort += statEffort / projStats.explosives;
					if (!prediction && stats->explosives < caps.explosives && RNG::generate(0, caps.explosives) > stats->explosives && RNG::percent(factor))
						s->getEngineerExperience()->explosives++;
					statsN++;
				}
				if (projStats.microelectronics > 0)
				{
					statEffort = stats->microelectronics;
					soldierEffort += statEffort / projStats.microelectronics;
					if (!prediction && stats->microelectronics < caps.microelectronics && RNG::generate(0, caps.microelectronics) > stats->microelectronics && RNG::percent(factor))
						s->getEngineerExperience()->microelectronics++;
					statsN++;
				}
				if (projStats.metallurgy > 0)
				{
					statEffort = stats->metallurgy;
					soldierEffort += statEffort / projStats.metallurgy;
					if (!prediction && stats->metallurgy < caps.metallurgy && RNG::generate(0, caps.metallurgy) > stats->metallurgy && RNG::percent(factor))
						s->getEngineerExperience()->metallurgy++;
					statsN++;
				}
				if (projStats.processing > 0)
				{
					statEffort = stats->processing;
					soldierEffort += statEffort / projStats.processing;
					if (!prediction && stats->processing < caps.processing && RNG::generate(0, caps.processing) > stats->processing && RNG::percent(factor))
						s->getEngineerExperience()->processing++;
					statsN++;
				}
				if (projStats.hacking > 0)
				{
					statEffort = stats->hacking;
					soldierEffort += statEffort / projStats.hacking;
					if (!prediction && stats->hacking < caps.hacking && RNG::generate(0, caps.hacking) > stats->hacking && RNG::percent(factor))
						s->getEngineerExperience()->hacking++;
					statsN++;
				}

				if (projStats.construction > 0)
				{
					statEffort = stats->construction;
					soldierEffort += statEffort / projStats.construction;
					if (!prediction && stats->construction < caps.construction && RNG::generate(0, caps.construction) > stats->construction && RNG::percent(factor))
						s->getEngineerExperience()->construction++;
					statsN++;
				}

				if (projStats.alienTech > 0)
				{
					statEffort = stats->alienTech;
					soldierEffort += statEffort / projStats.alienTech;
					if (!prediction && stats->alienTech < caps.alienTech && RNG::generate(0, caps.alienTech) > stats->alienTech && RNG::percent(factor))
						s->getEngineerExperience()->alienTech++;
					statsN++;
				}

				if (projStats.reverseEngineering > 0)
				{
					statEffort = stats->reverseEngineering;
					soldierEffort += statEffort / projStats.reverseEngineering;
					if (!prediction && stats->reverseEngineering < caps.reverseEngineering && RNG::generate(0, caps.reverseEngineering) > stats->reverseEngineering && RNG::percent(factor))
						s->getEngineerExperience()->reverseEngineering++;
					statsN++;
				}

				Log(LOG_DEBUG) << "Raw soldierEffort equals: " << soldierEffort;
				int diligence = stats->diligence;
				double deliganceFactor = 0.5;
				if (diligence > 10 && !_facility)
					deliganceFactor = -0.5 + 0.434 * std::log(std::fabs(diligence));

				soldierEffort *= deliganceFactor;
				Log(LOG_DEBUG) << "soldierEffort with diligence bonus: " << soldierEffort;
				if (statsN > 0)
					soldierEffort /= statsN;
				Log(LOG_DEBUG) << "Final soldierEffort value: " << soldierEffort;
				effort += soldierEffort;
				Log(LOG_DEBUG) << "Project effort now has value: " << effort;
				summEfficiency += stats->efficiency;
			}
			_efficiency = summEfficiency / assignedEngineers.size();
			
			if (assignedEngineers.size() > 1 && !_facility)
				effort *= (100 - 19 * log(assignedEngineers.size())) / 100;
			Log(LOG_DEBUG) << "Progress after correction for size: " << effort;
			effort *= (double)loyaltyRating;
			progress = static_cast<int>(effort);
			Log(LOG_DEBUG) << " >>> Total hourly progress for manufacturing project " << _rules->getName() << ": " << progress;
		}
		else
		{
			Log(LOG_DEBUG) << " >>> No assigned engineers for project: " << _rules->getName();
			_efficiency = 100;
		}
		return progress;
	}
}

productionProgress_e Production::step(Base * b, SavedGame * g, const Mod *m, Language *lang, int rating)
{
	int done = getAmountProduced();
	int progress = getProgress(b, g, m, rating);
	_timeSpent += progress;

	if (_facility)
	{
		int timeLeft = _rules->getManufactureTime() - _timeSpent;
		_facility->setBuildTime((timeLeft + progress - 1) / progress);
	}

	if (done < getAmountProduced())
	{
		int produced;
		if (!getInfiniteAmount())
		{
			produced = std::min(getAmountProduced(), _amount) - done; // std::min is required because we don't want to overproduce
		}
		else
		{
			produced = getAmountProduced() - done;
		}
		int count = 0;
		do
		{
			auto ruleCraft = _rules->getProducedCraft();
			if (ruleCraft)
			{
				Craft *craft = new Craft(ruleCraft, b, g->getId(ruleCraft->getType()));
				craft->initFixedWeapons(m);
				craft->setStatus("STR_REFUELLING");
				b->getCrafts()->push_back(craft);
			}
			else
			{
				for (auto& i : _rules->getProducedItems())
				{
					if (getSellItems())
					{
						int64_t adjustedSellValue = i.first->getSellCost();
						adjustedSellValue = adjustedSellValue * i.second * g->getSellPriceCoefficient() / 100;
						g->setFunds(g->getFunds() + adjustedSellValue);
					}
					else
					{
						b->getStorageItems()->addItem(i.first->getType(), i.second);
						if (!_rules->getRandomProducedItems().empty())
						{
							_randomProductionInfo[i.first->getType()] += i.second;
						}
						if (i.first->getBattleType() == BT_NONE)
						{
							for (std::vector<Craft*>::iterator c = b->getCrafts()->begin(); c != b->getCrafts()->end(); ++c)
							{
								(*c)->reuseItem(i.first);
							}
						}
					}
				}
			}
			// Random manufacture
			if (!_rules->getRandomProducedItems().empty())
			{
				int totalWeight = 0;
				for (auto& itemSet : _rules->getRandomProducedItems())
				{
					totalWeight += itemSet.first;
				}
				// RNG
				int roll = RNG::generate(1, totalWeight);
				int runningTotal = 0;
				for (auto& itemSet : _rules->getRandomProducedItems())
				{
					runningTotal += itemSet.first;
					if (runningTotal >= roll)
					{
						for (auto& i : itemSet.second)
						{
							b->getStorageItems()->addItem(i.first->getType(), i.second);
							_randomProductionInfo[i.first->getType()] += i.second;
							if (i.first->getBattleType() == BT_NONE)
							{
								for (std::vector<Craft*>::iterator c = b->getCrafts()->begin(); c != b->getCrafts()->end(); ++c)
								{
									(*c)->reuseItem(i.first);
								}
							}
						}
						// break outer loop
						break;
					}
				}
			}
			// Spawn persons (soldiers, engineers, scientists, ...)
			const std::string &spawnedPersonType = _rules->getSpawnedPersonType();
			if (spawnedPersonType != "")
			{
				if (spawnedPersonType == "STR_SCIENTIST")
				{
					Transfer *t = new Transfer(24);
					t->setScientists(1);
					b->getTransfers()->push_back(t);
				}
				else if (spawnedPersonType == "STR_ENGINEER")
				{
					Transfer *t = new Transfer(24);
					t->setEngineers(1);
					b->getTransfers()->push_back(t);
				}
				else
				{
					const RuleSoldier *rule = m->getSoldier(spawnedPersonType);
					if (rule != 0)
					{
						Transfer *t = new Transfer(24);
						int nationality = g->selectSoldierNationalityByLocation(m, rule, b);
						Soldier *s = m->genSoldier(g, rule, nationality);
						s->load(_rules->getSpawnedSoldierTemplate(), m, g, m->getScriptGlobal(), true); // load from soldier template
						if (_rules->getSpawnedPersonName() != "")
						{
							s->setName(lang->getString(_rules->getSpawnedPersonName()));
						}
						else
						{
							s->genName();
						}
						t->setSoldier(s);
						b->getTransfers()->push_back(t);
					}
				}
			}
			count++;
			if (count < produced)
			{
				// We need to ensure that player has enough cash/item to produce a new unit
				if (!haveEnoughMoneyForOneMoreUnit(g)) return PROGRESS_NOT_ENOUGH_MONEY;
				if (!haveEnoughMaterialsForOneMoreUnit(b, m)) return PROGRESS_NOT_ENOUGH_MATERIALS;
				startItem(b, g, m);
			}
		}
		while (count < produced);
	}
	if (getAmountProduced() >= _amount && !getInfiniteAmount()) return PROGRESS_COMPLETE;
	if (done < getAmountProduced())
	{
		// We need to ensure that player has enough cash/item to produce a new unit
		if (!haveEnoughMoneyForOneMoreUnit(g)) return PROGRESS_NOT_ENOUGH_MONEY;
		if (!haveEnoughLivingSpaceForOneMoreUnit(b)) return PROGRESS_NOT_ENOUGH_LIVING_SPACE;
		if (!haveEnoughMaterialsForOneMoreUnit(b, m)) return PROGRESS_NOT_ENOUGH_MATERIALS;
		startItem(b, g, m);
	}
	return PROGRESS_NOT_COMPLETE;
}

int Production::getAmountProduced() const
{
	if (_rules->getManufactureTime() > 0)
		return _timeSpent / _rules->getManufactureTime();
	else
		return _amount;
}

const RuleManufacture * Production::getRules() const
{
	return _rules;
}

void Production::startItem(Base * b, SavedGame * g, const Mod *m) const
{
	g->setFunds(g->getFunds() - ((_rules->getManufactureCost() * 100) / _efficiency));
	for (auto& i : _rules->getRequiredItems())
	{
		b->getStorageItems()->removeItem(i.first, i.second);
	}
	for (auto& i : _rules->getRequiredCrafts())
	{
		// Find suitable craft
		for (std::vector<Craft*>::iterator c = b->getCrafts()->begin(); c != b->getCrafts()->end(); ++c)
		{
			if ((*c)->getRules() == i.first)
			{
				Craft *craft = *c;
				b->removeCraft(craft, true);
				delete craft;
				break;
			}
		}
	}
}

void Production::refundItem(Base * b, SavedGame * g, const Mod *m) const
{
	g->setFunds(g->getFunds() + _rules->getManufactureCost());
	for (auto& iter : _rules->getRequiredItems())
	{
		b->getStorageItems()->addItem(iter.first->getType(), iter.second);
	}
	//for (auto& it : _rules->getRequiredCrafts())
	//{
	//	// not supported
	//}
}

YAML::Node Production::save() const
{
	YAML::Node node;
	if (_facility)
	{
		node["item"] = _facility->getRules()->getType();
	}
	else
	{
		node["item"] = getRules()->getName();
	}
	node["assigned"] = getAssignedEngineers();
	node["spent"] = getTimeSpent();
	node["amount"] = getAmountTotal();
	node["infinite"] = getInfiniteAmount();
	if (getSellItems())
		node["sell"] = getSellItems();
	if (!_rules->getRandomProducedItems().empty())
	{
		node["randomProductionInfo"] = _randomProductionInfo;
	}

	return node;
}

void Production::load(const YAML::Node &node)
{
	setAssignedEngineers(node["assigned"].as<int>(getAssignedEngineers()));
	setTimeSpent(node["spent"].as<int>(getTimeSpent()));
	setAmountTotal(node["amount"].as<int>(getAmountTotal()));
	setInfiniteAmount(node["infinite"].as<bool>(getInfiniteAmount()));
	setSellItems(node["sell"].as<bool>(getSellItems()));
	if (!_rules->getRandomProducedItems().empty())
	{
		_randomProductionInfo = node["randomProductionInfo"].as< std::map<std::string, int> >(_randomProductionInfo);
	}
	// backwards compatibility
	if (getAmountTotal() == INT_MAX)
	{
		setAmountTotal(999);
		setInfiniteAmount(true);
		setSellItems(true);
	}
}

}

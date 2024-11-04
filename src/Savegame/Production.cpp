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
#include "../Mod/Mod.h"
#include "../Mod/RuleItem.h"
#include "../Mod/RuleCraft.h"
#include "../Engine/Language.h"
#include "../Engine/RNG.h"
#include <climits>

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
	for (const auto& i : _rules->getRequiredItems())
	{
		if (b->getStorageItems()->getItem(i.first) < i.second)
			return false;
	}
	for (const auto& i : _rules->getRequiredCrafts())
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
		if (!assignedEngineers.empty())
		{
			double effort = 0;
			auto projStats = _rules->getStats();
			int trainingFactor = m->getEngineerTrainingFactor();
			double speedFactor = (double)m->getEngineeringSpeedFactor() / 100;
			int summEfficiency = 0;
			for (auto s : assignedEngineers)
			{
				auto stats = s->getCurrentStats();
				auto caps = s->getRules()->getStatCaps();
				unsigned int statsN = 0;
				double soldierEffort = 0, statEffort = 0;
				if (projStats.weaponry > 0)
				{
					statEffort = stats->weaponry;
					soldierEffort += statEffort / projStats.weaponry;
					if (!prediction && stats->weaponry < caps.weaponry && RNG::generate(0, caps.weaponry) > stats->weaponry && RNG::percent(trainingFactor))
						s->getEngineerExperience()->weaponry++;
					statsN++;
				}
				if (projStats.explosives > 0)
				{
					statEffort = stats->explosives;
					soldierEffort += statEffort / projStats.explosives;
					if (!prediction && stats->explosives < caps.explosives && RNG::generate(0, caps.explosives) > stats->explosives && RNG::percent(trainingFactor))
						s->getEngineerExperience()->explosives++;
					statsN++;
				}
				if (projStats.microelectronics > 0)
				{
					statEffort = stats->microelectronics;
					soldierEffort += statEffort / projStats.microelectronics;
					if (!prediction && stats->microelectronics < caps.microelectronics && RNG::generate(0, caps.microelectronics) > stats->microelectronics && RNG::percent(trainingFactor))
						s->getEngineerExperience()->microelectronics++;
					statsN++;
				}
				if (projStats.metallurgy > 0)
				{
					statEffort = stats->metallurgy;
					soldierEffort += statEffort / projStats.metallurgy;
					if (!prediction && stats->metallurgy < caps.metallurgy && RNG::generate(0, caps.metallurgy) > stats->metallurgy && RNG::percent(trainingFactor))
						s->getEngineerExperience()->metallurgy++;
					statsN++;
				}
				if (projStats.processing > 0)
				{
					statEffort = stats->processing;
					soldierEffort += statEffort / projStats.processing;
					if (!prediction && stats->processing < caps.processing && RNG::generate(0, caps.processing) > stats->processing && RNG::percent(trainingFactor))
						s->getEngineerExperience()->processing++;
					statsN++;
				}
				if (projStats.hacking > 0)
				{
					statEffort = stats->hacking;
					soldierEffort += statEffort / projStats.hacking;
					if (!prediction && stats->hacking < caps.hacking && RNG::generate(0, caps.hacking) > stats->hacking && RNG::percent(trainingFactor))
						s->getEngineerExperience()->hacking++;
					statsN++;
				}

				if (projStats.robotics > 0)
				{
					statEffort = stats->robotics;
					soldierEffort += statEffort / projStats.robotics;
					if (!prediction && stats->robotics < caps.robotics && RNG::generate(0, caps.robotics) > stats->robotics && RNG::percent(trainingFactor))
						s->getEngineerExperience()->robotics++;
					statsN++;
				}

				if (projStats.alienTech > 0)
				{
					statEffort = stats->alienTech;
					soldierEffort += statEffort / projStats.alienTech;
					if (!prediction && stats->alienTech < caps.alienTech && RNG::generate(0, caps.alienTech) > stats->alienTech && RNG::percent(trainingFactor))
						s->getEngineerExperience()->alienTech++;
					statsN++;
				}

				if (projStats.reverseEngineering > 0)
				{
					statEffort = stats->reverseEngineering;
					soldierEffort += statEffort / projStats.reverseEngineering;
					if (!prediction && stats->reverseEngineering < caps.reverseEngineering && RNG::generate(0, caps.reverseEngineering) > stats->reverseEngineering && RNG::percent(trainingFactor))
						s->getEngineerExperience()->reverseEngineering++;
					statsN++;
				}

				int diligence = stats->diligence;
				double diligenceFactor = 0.5;
				if (diligence > 10)
					diligenceFactor = -0.5 + 0.434 * std::log(std::fabs(diligence));

				soldierEffort *= diligenceFactor;
				if (statsN > 0)
					soldierEffort /= statsN;
				effort += soldierEffort;
				summEfficiency += stats->efficiency;
			}
			_efficiency = summEfficiency / assignedEngineers.size();

			effort *= loyaltyRating; //not normalizing by 100 to fit small hourly values into integer later
			effort *= speedFactor;
			progress = static_cast<int>(effort);
		}
		else
		{
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
			auto* ruleCraft = _rules->getProducedCraft();
			if (ruleCraft)
			{
				Craft *craft = new Craft(ruleCraft, b, g->getId(ruleCraft->getType()));
				craft->initFixedWeapons(m);
				craft->checkup();
				b->getCrafts()->push_back(craft);
			}
			else
			{
				for (const auto& i : _rules->getProducedItems())
				{
					if (getSellItems())
					{
						int64_t adjustedSellValue = i.first->getSellCostAdjusted(b, g);
						adjustedSellValue *= i.second;
						g->setFunds(g->getFunds() + adjustedSellValue);
					}
					else
					{
						b->getStorageItems()->addItem(i.first, i.second);
						if (!_rules->getRandomProducedItems().empty())
						{
							_randomProductionInfo[i.first->getType()] += i.second;
						}
						if (i.first->getBattleType() == BT_NONE)
						{
							for (auto* c : *b->getCrafts())
							{
								c->reuseItem(i.first);
							}
						}
					}
				}
			}
			// Random manufacture
			if (!_rules->getRandomProducedItems().empty())
			{
				int totalWeight = 0;
				for (const auto& itemSet : _rules->getRandomProducedItems())
				{
					totalWeight += itemSet.first;
				}
				// RNG
				int roll = RNG::generate(1, totalWeight);
				int runningTotal = 0;
				for (const auto& itemSet : _rules->getRandomProducedItems())
				{
					runningTotal += itemSet.first;
					if (runningTotal >= roll)
					{
						for (const auto& i : itemSet.second)
						{
							b->getStorageItems()->addItem(i.first, i.second);
							_randomProductionInfo[i.first->getType()] += i.second;
							if (i.first->getBattleType() == BT_NONE)
							{
								for (auto* c : *b->getCrafts())
								{
									c->reuseItem(i.first);
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

						if (g->isFtAGame())
						{
							b->getSoldiers()->push_back(s);
						}
						else
						{
							Transfer* t = new Transfer(24);
							t->setSoldier(s);
							b->getTransfers()->push_back(t);
						}
					}
				}
			}
			if (_rules->getPoints() != 0)
			{
				// yes, negative points are allowed too
				g->addResearchScore(_rules->getPoints());
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
	for (const auto& i : _rules->getRequiredItems())
	{
		b->getStorageItems()->removeItem(i.first, i.second);
	}
	for (const auto& i : _rules->getRequiredCrafts())
	{
		// Find suitable craft
		for (auto* c : *b->getCrafts())
		{
			if (c->getRules() == i.first)
			{
				Craft *craft = c;
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
	for (const auto& pair : _rules->getRequiredItems())
	{
		b->getStorageItems()->addItem(pair.first, pair.second);
	}
	//for (const auto& pair : _rules->getRequiredCrafts())
	//{
	//	// not supported
	//}
}

YAML::Node Production::save() const
{
	YAML::Node node;
	node["item"] = getRules()->getName();
	node["assigned"] = getAssignedEngineers();
	node["spent"] = getTimeSpent();
	node["amount"] = getAmountTotal();
	node["infinite"] = getInfiniteAmount();
	node["efficiency"] = getEfficiency();
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
	setEfficiency(node["efficiency"].as<bool>(getEfficiency()));
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

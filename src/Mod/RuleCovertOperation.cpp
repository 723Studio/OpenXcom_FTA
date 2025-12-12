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
#include "RuleCovertOperation.h"
#include "../Engine/RNG.h"
#include "../Engine/Exception.h"
#include "Mod.h"

namespace OpenXcom
{

/**
* Creates a blank ruleset for a certain
* type of RuleCovertOperation.
* @param type String defining the type.
*/
RuleCovertOperation::RuleCovertOperation(const std::string& name) : _name(name), _soldiersMin(1), _soldiersMax(1), _optionalSoldierEffect(0),
	_itemSpaceEffect(10), _armorEffect(20),
	_itemSpaceLimit(-1), _baseChances(50), _costs(0), _successScore(0), _failureScore(0),
	_successLoyalty(0), _failureLoyalty(0), _successFunds(0), _failureFunds(0), _danger(0), _trapChance(0),
	_progressEventChance(0), _concealedItemsBonus(20), _bonusItemsEffect(5), _repeatProgressEvent(false), _allowAllEquipment(false),
	_removeRequiredItemsOnSuccess(true), _removeRequiredItemsOnFailure(false), _listOrder(0) 
{
}

RuleCovertOperation::~RuleCovertOperation()
{
}

/**
* Loads the craft from a YAML file.
* @param node YAML node.
* @param mod Mod for the CovertOperation.
* @param modIndex A value that offsets the sounds and sprite values to avoid conflicts.
* @param listOrder The list weight for this CovertOperation.
*/
void RuleCovertOperation::load(const YAML::YamlNodeReader& node, Mod* mod, int listOrder)
{
	const auto& reader = node.useIndex();
	if (const YAML::YamlNodeReader& parent = reader["refNode"])
	{
		load(reader["refNode"], mod, listOrder);
	}
	reader.tryRead("name", _name);
	reader.tryRead("description", _description);
	reader.tryRead("categories", _categories);
	reader.tryRead("successBackground", _successBackground);
	reader.tryRead("failureBackground", _failureBackground);
	reader.tryRead("successDescription", _successDescription);
	reader.tryRead("failureDescription", _failureDescription);
	reader.tryRead("successMusic", _successMusic);
	reader.tryRead("failureMusic", _failureMusic);
	reader.tryRead("successEvent", _successEvent);
	reader.tryRead("failureEvent", _failureEvent);
	if (reader["progressEvent"])
	{
		_progressEvent.load(reader["progressEvent"]);
	}
	reader.tryRead("repeatProgressEvent", _repeatProgressEvent);
	reader.tryRead("requires", _requires);
	reader.tryRead("allowedRoles", _allowedRoles);
	mod->loadBaseFunction(_name, _requiresBaseFunc, reader["requiresBaseFunc"]);
	reader.tryRead("soldiersMin", _soldiersMin);
	if (_soldiersMin < 1)
	{
		throw Exception("Error in loading operation '" + _name + "'! It must have at least 1 soldier!");
	}
	reader.tryRead("soldiersMax", _soldiersMax);
	if (_soldiersMax == 0)
	{
		_soldiersMax = _soldiersMin;
	}
	else if (_soldiersMax < _soldiersMin)
	{
		throw Exception("Error in loading operation '" + _name + "'! soldiersMax < _soldiersMin!");
	}
	reader.tryRead("optionalSoldierEffect", _optionalSoldierEffect);
	reader.tryRead("baseChances", _baseChances);
	reader.tryRead("costs", _costs);
	if (_costs < 0)
	{
		throw Exception("Error in loading operation '" + _name + "'! Costs is less than 0, this is not allowed.");
	}
	reader.tryRead("progressEventChance", _progressEventChance);
	reader.tryRead("trapChance", _trapChance);
	reader.tryRead("danger", _danger);
	reader.tryRead("successScore", _successScore);
	reader.tryRead("failureScore", _failureScore);
	reader.tryRead("successLoyalty", _successLoyalty);
	reader.tryRead("failureLoyalty", _failureLoyalty);
	reader.tryRead("successFunds", _successFunds);
	reader.tryRead("failureFunds", _failureFunds);
	reader.tryRead("successItemNames", _successItemNames);
	reader.tryRead("failureItemNames", _failureItemNames);
	reader.tryRead("addSoldiers", _addSoldiersStr);
	reader.tryRead("successResearchList", _successResearchList);
	reader.tryRead("failureResearchList", _failureResearchList);
	if (reader["successMissions"])
	{
		_successMissions.load(reader["successMissions"]);
	}
	if (reader["failureMissions"])
	{
		_failureMissions.load(reader["failureMissions"]);
	}
	if (reader["instantTrapDeployment"])
	{
		_instantTrapDeployment.load(reader["instantTrapDeployment"]);
	}
	if (reader["instantSuccessDeployment"])
	{
		_instantSuccessDeployment.load(reader["instantSuccessDeployment"]);
	}
	reader.tryRead("requiredReputationLvl", _requiredReputationLvl);
	reader.tryRead("successReputationScore", _successReputationScore);
	reader.tryRead("failureReputationScore", _failureReputationScore);
	reader.tryRead("itemSpaceLimit", _itemSpaceLimit);
	reader.tryRead("itemSpaceEffect", _itemSpaceEffect);
	reader.tryRead("requiredItems", _requiredItemsStr);
	reader.tryRead("bonusItems", _bonusItemsStr);
	reader.tryRead("bonusItemsEffect", _bonusItemsEffect);
	reader.tryRead("allowAllEquipment", _allowAllEquipment);
	reader.tryRead("removeRequiredItemsOnSuccess", _removeRequiredItemsOnSuccess);
	reader.tryRead("removeRequiredItemsOnFailure", _removeRequiredItemsOnFailure);
	reader.tryRead("concealedItemsBonus", _concealedItemsBonus);
	reader.tryRead("allowedArmor", _allowedArmor);
	reader.tryRead("armorEffect", _armorEffect);
	reader.tryRead("soldierTypeEffectiveness", _soldierTypeEffectiveness);
	reader.tryRead("specialRule", _specialRule);
	if (!_listOrder)
	{
		_listOrder = listOrder;
	}
}

/**
 * Cross link with other rules
 */
void RuleCovertOperation::afterLoad(const Mod* mod)
{
	if (!_successEvent.empty() && !mod->getEvent(_successEvent))
	{
		throw Exception("Cover operation named: '" + this->getName() + "' has broken link in successEvent: '" + _successEvent +"' is not found!");
	}
	if (!_failureEvent.empty() && !mod->getEvent(_failureEvent))
	{
		throw Exception("Cover operation named: '" + this->getName() + "' has broken link in failureEvent: '" + _failureEvent + "' is not found!");
	}
	if (!_addSoldiersStr.empty())
	{
		for (auto &addSoldier: _addSoldiersStr)
		{
			const RuleSoldier* soldier = mod->getSoldier(addSoldier.first);
			if (soldier != 0)
			{
				if (addSoldier.second < 1)
				{
					throw Exception("Cover operation named: '" + this->getName() + " has invalid addSoldiers property - second value of the map must be greater than 0.");
				}
				_addSoldiers.insert_or_assign(soldier, addSoldier.second);
			}
			else
			{
				throw Exception("Cover operation named: '" + this->getName() + " has invalid addSoldiers property - no rule for soldier: '" + addSoldier.first + "'");
			}
			
		}
	}
	if (!_requiredItemsStr.empty())
	{
		for (auto& item : _requiredItemsStr)
		{
			const RuleItem* rule = mod->getItem(item.first);
			if (rule)
			{
				if (item.second < 1)
				{
					throw Exception("Cover operation named: '" + this->getName() + " has invalid requiredItems property - second value of the map must be greater than 0.");
				}
				_requiredItems.insert_or_assign(rule, item.second);
			}
			else
			{
				throw Exception("Cover operation named: '" + this->getName() + " has invalid requiredItems property - no rule for item: '" + item.first + "'");
			}
		}
	}
	if (!_bonusItemsStr.empty())
	{
		for (auto& item : _bonusItemsStr)
		{
			const RuleItem* rule = mod->getItem(item.first);
			if (rule)
			{
				if (item.second < 1)
				{
					throw Exception("Cover operation named: '" + this->getName() + " has invalid bonusItems property - second value of the map must be greater than 0.");
				}
				_bonusItems.insert_or_assign(rule, item.second);
			}
			else
			{
				throw Exception("Cover operation named: '" + this->getName() + " has invalid bonusItems property - no rule for item: '" + item.first + "'");
			}
		}
	}

	for (auto& itemSet : _successItemNames)
	{
		std::map<const RuleItem*, int> tmp;
		for (auto& i : itemSet.second)
		{
			auto item = mod->getItem(i.first, true);
			if (!item)
			{
				throw Exception("Cover operation named: '" + this->getName() + " has invalid successItemNames property - no rule for item: '" + i.first + "'");
			}
			else
			{
				tmp[item] = i.second;
			}
		}
		_successItems.push_back(std::make_pair(itemSet.first, tmp));
	}

	for (auto& itemSet : _failureItemNames)
	{
		std::map<const RuleItem*, int> tmp;
		for (auto& i : itemSet.second)
		{
			auto item = mod->getItem(i.first, true);
			if (!item)
			{
				throw Exception("Cover operation named: '" + this->getName() + " has invalid failureItemNames property - no rule for item: '" + i.first + "'");
			}
			else
			{
				tmp[item] = i.second;
			}
		}
		_failureItems.push_back(std::make_pair(itemSet.first, tmp));
	}

	//remove not needed data
	Collections::removeAll(_addSoldiersStr);
	Collections::removeAll(_requiredItemsStr);
	Collections::removeAll(_bonusItemsStr);
	Collections::removeAll(_successItemNames);
	Collections::removeAll(_failureItemNames);
}

std::string RuleCovertOperation::chooseGenSuccessMissionType() const
{
	return _successMissions.choose();
}

std::string RuleCovertOperation::chooseGenFailureMissionType() const
{
	return _failureMissions.choose();
}

std::string RuleCovertOperation::chooseGenInstantTrapDeploymentType() const
{
	return _instantTrapDeployment.choose();
}

std::string RuleCovertOperation::chooseGenInstantSuccessDeploymentType() const
{
	return _instantSuccessDeployment.choose();
}

std::string RuleCovertOperation::chooseProgressEvent() const
{
	return _progressEvent.choose();
}

}


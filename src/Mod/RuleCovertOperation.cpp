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
#include <algorithm>
#include "RuleCovertOperation.h"
#include "../Engine/RNG.h"
#include "../fmath.h"
#include "../Engine/Exception.h"
#include "Mod.h"

namespace OpenXcom
{

/**
* Creates a blank ruleset for a certain
* type of RuleCovertOperation.
* @param type String defining the type.
*/
RuleCovertOperation::RuleCovertOperation(const std::string& name) : _name(name), _soldierSlots(1), _optionalSoldierSlots(0),
																	_scientistSlots(0), _engineerSlots(0), _optionalSoldierEffect(15), _scientistEffect(10), _engineerEffect(10),
																	_baseChances(50), _costs(0), _itemSpaceLimit(-1), _itemSpaceEffect(10), _danger(0), _trapChance(0), _armorEffect(20),
																	_successScore(0), _failureScore(0), _progressEventChance(0), _repeatProgressEvent(false),
																	_successFunds(0), _failureFunds(0), _successMusic("GMMARS"), _failureMusic("GMLOSE"),
																	_listOrder(0) 
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
void RuleCovertOperation::load(const YAML::Node& node, Mod* mod, int listOrder)
{
	if (const YAML::Node& parent = node["refNode"])
	{
		load(parent, mod, listOrder);
	}
	_name = node["name"].as<std::string>(_name);
	_description = node["description"].as<std::string>(_description);
	_successBackground = node["successBackground"].as<std::string>(_successBackground);
	_failureBackground = node["failureBackground"].as<std::string>(_failureBackground);
	_successDescription = node["successDescription"].as<std::string>(_successDescription);
	_failureDescription = node["failureDescription"].as<std::string>(_failureDescription);
	_successMusic = node["successMusic"].as<std::string>(_successMusic);
	_failureMusic = node["failureMusic"].as<std::string>(_failureMusic);
	_successEvent = node["successEvent"].as<std::string>(_successEvent);
	_failureEvent = node["failureEvent"].as<std::string>(_failureEvent);
	_progressEvent = node["progressEvent"].as<std::string>(_progressEvent);
	_repeatProgressEvent = node["repeatProgressEvent"].as<bool>(_repeatProgressEvent);
	_requires = node["requires"].as<std::vector<std::string>>(_requires);
	mod->loadBaseFunction(_name, _requiresBaseFunc, node["requiresBaseFunc"]);
	_soldierSlots = node["soldierSlots"].as<int>(_soldierSlots);
	if (_soldierSlots < 1)
	{
		throw Exception("Error in loading operation '" + _name + "'! It must have at least 1 soldier.");
	}
	_optionalSoldierSlots = node["optionalSoldierSlots"].as<int>(_optionalSoldierSlots);
	_scientistSlots = node["scientistSlots"].as<int>(_scientistSlots);
	_engineerSlots = node["engineerSlots"].as<int>(_engineerSlots);
	_optionalSoldierEffect = node["optionalSoldierEffect"].as<int>(_optionalSoldierEffect);
	_scientistEffect = node["scientistEffect"].as<int>(_scientistEffect);
	_engineerEffect = node["engineerEffect"].as<int>(_engineerEffect);
	_baseChances = node["baseChances"].as<int>(_baseChances);
	_costs = node["costs"].as<int>(_costs);
	_progressEventChance = node["progressEventChance"].as<int>(_progressEventChance);
	_trapChance = node["trapChance"].as<int>(_trapChance);
	_danger = node["danger"].as<int>(_danger);
	_successScore = node["successScore"].as<int>(_successScore);
	_failureScore = node["failureScore"].as<int>(_failureScore);
	_successFunds = node["successFunds"].as<int>(_successFunds);
	_failureFunds = node["failureFunds"].as<int>(_failureFunds);
	_successEveryItemList = node["successEveryItemList"].as<std::map<std::string, int> >(_successEveryItemList);
	_failureEveryItemList = node["failureEveryItemList"].as<std::map<std::string, int> >(_failureEveryItemList);
	if (node["successWeightedItemList"])
	{
		_successWeightedItemList.load(node["successWeightedItemList"]);
	}
	if (node["failureWeightedItemList"])
	{
		_failureWeightedItemList.load(node["failureWeightedItemList"]);
	}
	_successResearchList = node["successResearchList"].as<std::vector<std::string> >(_successResearchList);
	_failureResearchList = node["failureResearchList"].as<std::vector<std::string> >(_failureResearchList);
	if (node["successMissions"])
	{
		_successMissions.load(node["successMissions"]);
	}
	if (node["failureMissions"])
	{
		_failureMissions.load(node["failureMissions"]);
	}
	if (node["instantTrapDeployment"])
	{
		_instantTrapDeployment.load(node["instantTrapDeployment"]);
	}
	if (node["instantSuccessDeployment"])
	{
		_instantSuccessDeployment.load(node["instantSuccessDeployment"]);
	}
	_requiredReputationLvl = node["requiredReputationLvl"].as<std::map<std::string, int>>(_requiredReputationLvl);
	_successReputationScore = node["successReputationScore"].as<std::map<std::string, int>>(_successReputationScore);
	_failureReputationScore = node["failureReputationScore"].as<std::map<std::string, int>>(_failureReputationScore);
	_itemSpaceLimit = node["itemSpaceLimit"].as<double>(_itemSpaceLimit);
	_itemSpaceEffect = node["itemSpaceEffect"].as<double>(_itemSpaceEffect);
	_requiredItems = node["requiredItems"].as<std::map<std::string, int>>(_requiredItems);
	_allowedArmor = node["allowedArmor"].as<std::vector<std::string>>(_allowedArmor);
	_armorEffect = node["armorEffect"].as<int>(_armorEffect);
	_soldierTypeEffectiveness = node["soldierTypeEffectiveness"].as<std::map<std::string, int>>(_soldierTypeEffectiveness);
	_specialRule = node["specialRule"].as<std::string>(_specialRule);
	
	if (!_listOrder)
	{
		_listOrder = listOrder;
	}
}

std::string RuleCovertOperation::chooseGenSuccessMissionType() const
{
	return _successMissions.choose();
}

std::string RuleCovertOperation::chooseGenFailureMissionType() const
{
	return _failureMissions.choose();
}

std::string RuleCovertOperation::chooseGenInstantTrapDeploumentType() const
{
	return _instantTrapDeployment.choose();
}

std::string RuleCovertOperation::chooseGenInstantSuccessDeploumentType() const
{
	return _instantSuccessDeployment.choose();
}


}


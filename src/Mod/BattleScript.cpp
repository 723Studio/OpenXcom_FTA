/*
 * Copyright 2010-2020 OpenXcom Developers.
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

#include "BattleScript.h"
#include <yaml-cpp/yaml.h>
#include "../Engine/RNG.h"
#include "../Engine/Exception.h"
#include "../Engine/Logger.h"
#include "../Mod/RuleTerrain.h"


namespace OpenXcom
{

BattleScript::BattleScript() :
	_type(BSC_UNDEFINED), 
	_executionChances(100), _executions(1), _cumulativeFrequency(0), _label(0), _startTurn(0), _endTurn(-1), _unitSide(1),
	_minLevel(0), _maxLevel(0), _packSize(1), _randomPackSize(false), _minDifficulty(0), _maxDifficulty(4)
{
}

BattleScript::~BattleScript()
{
}

/**
* Loads a map script command from YAML.
* @param node the YAML node from which to read.
*/
void BattleScript::load(const YAML::Node& node)
{

	std::string command;
	if (const YAML::Node& map = node["type"])
	{
		command = map.as<std::string>("");
		if (command == "spawnItem")
			_type = BSC_SPAWN_ITEM;
		else if (command == "spawnUnit")
			_type = BSC_SPAWN_UNIT;
		else if (command == "showMessage")
			_type = BSC_SHOW_MESSAGE;
		else if (command == "addBlock")
			_type = BSC_ADDBLOCK;
		else
		{
			throw Exception("Unknown battlescritp command: " + command);
		}
	}
	else
	{
		throw Exception("Missing command type.");
	}

	if (const YAML::Node& map = node["conditionals"])
	{
		if (map.Type() == YAML::NodeType::Sequence)
		{
			_conditionals = map.as<std::vector<int> >(_conditionals);
		}
		else
		{
			_conditionals.push_back(map.as<int>(0));
		}
	}

	_executionChances = node["executionChances"].as<int>(_executionChances);
	_executions = node["executions"].as<int>(_executions);
	_label = std::abs(node["label"].as<int>(_label));
	_itemSet = node["itemSet"].as<std::vector<std::string>>(_itemSet);
	_unitSet = node["unitSet"].as<std::vector<std::string>>(_unitSet);
	_spawnBlocks = node["spawnBlocks"].as< std::vector<std::string> >(_spawnBlocks);
	_groups = node["groups"].as< std::vector<int> >(_groups);
	_packSize = node["packSize"].as<int>(_packSize);
	_randomPackSize = node["randomPackSize"].as<bool>(_randomPackSize);
	_minLevel = node["minLevel"].as<int>(_minLevel);
	_maxLevel = node["maxLevel"].as<int>(_maxLevel);
	_unitSide = node["unitSide"].as<int>(_unitSide);
	_startTurn = node["startTurn"].as<int>(_startTurn);
	_endTurn = node["endTurn"].as<int>(_endTurn);
	_minDifficulty = node["minDifficulty"].as<int>(_minDifficulty);
	_maxDifficulty = node["maxDifficulty"].as<int>(_maxDifficulty);
	if (const YAML::Node& messages = node["messages"])
	{
		for (YAML::const_iterator i = messages.begin(); i != messages.end(); ++i)
		{
			_message[i->first.as<int>()].load(i->second);
		}
	}

}

}

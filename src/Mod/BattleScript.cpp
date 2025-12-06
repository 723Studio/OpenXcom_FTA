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
#include "../Engine/Yaml.h"
#include "../Engine/RNG.h"
#include "../Engine/Exception.h"
#include "../Engine/Logger.h"
#include <climits>

namespace OpenXcom
{

BattleScript::BattleScript() :
	_type(BSC_UNDEFINED), 
	_executionChances(100), _executions(1), _maxRuns(-1), _label(0), _startTurn(0), _endTurn(-1), _unitSide(1),
	_packSize(1), _minLevel(0), _maxLevel(0), _randomPackSize(false),
	_minDifficulty(0), _maxDifficulty(4), _minAlarmLevel(0), _maxAlarmLevel(INT_MAX), _variable("battleScriptVariable")
{
}

BattleScript::~BattleScript()
{
}

/**
* Loads a map script command from YAML.
* @param node the YAML node from which to read.
*/
void BattleScript::load(const YAML::YamlNodeReader& reader)
{

	std::string command;
		if (reader["type"])
	{
			reader.tryRead("type", command);
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

	if (reader["conditionals"])
	{
		   std::vector<int> tmp = _conditionals;
		   reader.tryRead("conditionals", tmp);
		   _conditionals = tmp;
	}

	reader.tryRead("executionChances", _executionChances);
	reader.tryRead("executions", _executions);
	reader.tryRead("maxRuns", _maxRuns);
	reader.tryRead("variable", _variable);
	reader.tryRead("label", _label);
	_label = std::abs(_label);
	reader.tryRead("itemSet", _itemSet);
	reader.tryRead("unitSet", _unitSet);
	reader.tryRead("spawnBlocks", _spawnBlocks);
	reader.tryRead("groups", _groups);
	reader.tryRead("packSize", _packSize);
	reader.tryRead("randomPackSize", _randomPackSize);
	reader.tryRead("minLevel", _minLevel);
	reader.tryRead("maxLevel", _maxLevel);
	reader.tryRead("unitSide", _unitSide);
	reader.tryRead("startTurn", _startTurn);
	reader.tryRead("endTurn", _endTurn);
	reader.tryRead("minDifficulty", _minDifficulty);
	reader.tryRead("maxDifficulty", _maxDifficulty);
	reader.tryRead("minAlarmLevel", _minAlarmLevel);
	reader.tryRead("maxAlarmLevel", _maxAlarmLevel);
	reader.tryRead("spawnNodeRanks", _spawnNodeRanks);
	if (reader["messages"])
	{
		for (const auto& child : reader["messages"].children())
		{
			int key;
			child.tryReadKey(key);
			_message[key].load(child);
		}
	}

}

}

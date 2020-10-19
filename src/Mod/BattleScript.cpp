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
	_type(BSC_UNDEFINED), _canBeSkipped(true),
	_executionChances(100), _executions(1), _cumulativeFrequency(0), _label(0), _startTurn(0), _endTurn(-1), _unitSide(1),
	_minLevel(0), _maxLevel(0), _packSize(1), _randomPackSize(false), _minDifficulty(0), _maxDifficulty(4)
{
}

BattleScript::~BattleScript()
{
	for (std::vector<SDL_Rect*>::iterator i = _rects.begin(); i != _rects.end(); ++i)
	{
		delete* i;
	}
	//delete _tunnelData;
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

	if (const YAML::Node& map = node["rects"])
	{
		for (YAML::const_iterator i = map.begin(); i != map.end(); ++i)
		{
			SDL_Rect* rect = new SDL_Rect();
			rect->x = (*i)[0].as<int>();
			rect->y = (*i)[1].as<int>();
			rect->w = (*i)[2].as<int>();
			rect->h = (*i)[3].as<int>();
			_rects.push_back(rect);
		}
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
	/*if (const YAML::Node& map = node["size"])
	{
		if (map.Type() == YAML::NodeType::Sequence)
		{
			int* sizes[3] = { &_sizeX, &_sizeY, &_sizeZ };
			int entry = 0;
			for (YAML::const_iterator i = map.begin(); i != map.end(); ++i)
			{
				*sizes[entry] = (*i).as<int>(1);
				entry++;
				if (entry == 3)
				{
					break;
				}
			}
		}
		else
		{
			_sizeX = map.as<int>(_sizeX);
			_sizeY = _sizeX;
		}
	}*/

	// map block analisys not done
	if (const YAML::Node& map = node["groups"])
	{
		_groups.clear();
		if (map.Type() == YAML::NodeType::Sequence)
		{
			for (YAML::const_iterator i = map.begin(); i != map.end(); ++i)
			{
				_groups.push_back((*i).as<int>(0));
			}
		}
		else
		{
			_groups.push_back(map.as<int>(0));
		}
	}
	size_t selectionSize = _groups.size();
	if (const YAML::Node& map = node["blocks"])
	{
		_groups.clear();
		if (map.Type() == YAML::NodeType::Sequence)
		{
			for (YAML::const_iterator i = map.begin(); i != map.end(); ++i)
			{
				_blocks.push_back((*i).as<int>(0));
			}
		}
		else
		{
			_blocks.push_back(map.as<int>(0));
		}
		selectionSize = _blocks.size();
	}

	_frequencies.resize(selectionSize, 1);
	_maxUses.resize(selectionSize, -1);

	_canBeSkipped = node["canBeSkipped"].as<bool>(_canBeSkipped);
	_executionChances = node["executionChances"].as<int>(_executionChances);
	_executions = node["executions"].as<int>(_executions);
	// take no chances, don't accept negative values here.
	_label = std::abs(node["label"].as<int>(_label));

	_itemSet = node["itemSet"].as<std::vector<std::string>>(_itemSet);
	_unitSet = node["unitSet"].as<std::vector<std::string>>(_unitSet);
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

/**
* Initializes all the various scratch values and such for the command.
*/
void BattleScript::init()
{
	_cumulativeFrequency = 0;
	_blocksTemp.clear();
	_groupsTemp.clear();
	_frequenciesTemp.clear();
	_maxUsesTemp.clear();

	for (std::vector<int>::const_iterator i = _frequencies.begin(); i != _frequencies.end(); ++i)
	{
		_cumulativeFrequency += *i;
	}
	_blocksTemp = _blocks;
	_groupsTemp = _groups;
	_frequenciesTemp = _frequencies;
	_maxUsesTemp = _maxUses;
}

/**
* Gets a random group number from the array, accounting for frequencies and max uses.
* If no groups or blocks are defined, this command will return the default" group,
* If all the max uses are used up, it will return "undefined".
* @return Group number.
*/
int BattleScript::getGroupNumber()
{
	if (!_groups.size())
	{
		return MT_DEFAULT;
	}
	if (_cumulativeFrequency > 0)
	{
		int pick = RNG::generate(0, _cumulativeFrequency - 1);
		for (size_t i = 0; i != _groupsTemp.size(); ++i)
		{
			if (pick < _frequenciesTemp.at(i))
			{
				int retVal = _groupsTemp.at(i);

				if (_maxUsesTemp.at(i) > 0)
				{
					if (--_maxUsesTemp.at(i) == 0)
					{
						_groupsTemp.erase(_groupsTemp.begin() + i);
						_cumulativeFrequency -= _frequenciesTemp.at(i);
						_frequenciesTemp.erase(_frequenciesTemp.begin() + i);
						_maxUsesTemp.erase(_maxUsesTemp.begin() + i);
					}
				}
				return retVal;
			}
			pick -= _frequenciesTemp.at(i);
		}
	}
	return MT_UNDEFINED;
}

/**
* Gets a random block number from the array, accounting for frequencies and max uses.
* If no blocks are defined, it will use a group instead.
* @return Block number.
*/
int BattleScript::getBlockNumber()
{
	if (_cumulativeFrequency > 0)
	{
		int pick = RNG::generate(0, _cumulativeFrequency - 1);
		for (size_t i = 0; i != _blocksTemp.size(); ++i)
		{
			if (pick < _frequenciesTemp.at(i))
			{
				int retVal = _blocksTemp.at(i);

				if (_maxUsesTemp.at(i) > 0)
				{
					if (--_maxUsesTemp.at(i) == 0)
					{
						_blocksTemp.erase(_blocksTemp.begin() + i);
						_cumulativeFrequency -= _frequenciesTemp.at(i);
						_frequenciesTemp.erase(_frequenciesTemp.begin() + i);
						_maxUsesTemp.erase(_maxUsesTemp.begin() + i);
					}
				}
				return retVal;
			}
			pick -= _frequenciesTemp.at(i);
		}
	}
	return MT_UNDEFINED;
}

}

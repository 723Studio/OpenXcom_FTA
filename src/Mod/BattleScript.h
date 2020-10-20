#pragma once
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
#include <vector>
#include <string>
#include <yaml-cpp/yaml.h>
#include <SDL_video.h>
#include "MapBlock.h"
#include "../Engine/Logger.h"

namespace OpenXcom
{

/**
* Definition of custom Battlsescape message.
*/
struct BattleMessage
{
	std::string title;
	std::vector<std::string> description;
	std::string answer;

	/// Loads stats from YAML.
	void load(const YAML::Node& node)
	{
		title = node["title"].as<std::string>(title);
		description = node["description"].as<std::vector<std::string>>(description);
		answer = node["answer"].as<std::string>(answer);
	}

};

/**
* Definition of one custom player answer to Geoscape Event.
*/
enum BattleScriptCommand { BSC_UNDEFINED = -1, BSC_SPAWN_ITEM, BSC_SPAWN_UNIT, BSC_SHOW_MESSAGE, BSC_ADDBLOCK};

class MapBlock;
class RuleTerrain;

class BattleScript
{
private:
	BattleScriptCommand _type;
	bool _canBeSkipped;
	std::vector<SDL_Rect*> _rects;
	std::vector<int> _groups, _blocks, _frequencies, _maxUses, _conditionals;
	std::vector<int> _groupsTemp, _blocksTemp, _frequenciesTemp, _maxUsesTemp;
	int _executionChances, _executions, _cumulativeFrequency, _label;
	
	std::vector<std::string> _itemSet, _unitSet;
	std::map<int, BattleMessage> _message;
	int _startTurn, _endTurn, _unitSide, _packSize, _minLevel, _maxLevel, _minDifficulty, _maxDifficulty;
	bool _randomPackSize;

	/// Randomly generate a group from within the array.
	int getGroupNumber();
	/// Randomly generate a block number from within the array.
	int getBlockNumber();
public:
	BattleScript();
	~BattleScript();
	/// Loads information from a ruleset file.
	void load(const YAML::Node& node);
	/// Initializes all the variables and junk for a mapscript command.
	void init();
	/// Gets what type of command this is.
	BattleScriptCommand getType() const { return _type; };
	/// Can this command be skipped if unsuccessful?
	bool canBeSkipped() const { return _canBeSkipped; };
	/// Gets the rects, describing the areas this command applies to.
	const std::vector<SDL_Rect*>* getRects() const { return &_rects; };
	/// Get the chances of this command executing.
	int getChancesOfExecution() const { return _executionChances; };
	/// Gets the label for this command.
	int getLabel() const { return _label; };
	/// Gets how many times this command repeats (1 repeat means 2 executions)
	int getExecutions() const { return _executions; };
	/// Gets what conditions apply to this command.
	const std::vector<int>* getConditionals() const { return &_conditionals; };

	/// Gets the groups vector for iteration.
	const std::vector<int>* getGroups() const { return &_groups; };
	/// Gets the blocks vector for iteration.
	const std::vector<int>* getBlocks() const { return &_blocks; };

	/// Gets the turn number that would start script execution.
	int getStartTurn() const { return _startTurn; };
	/// Gets the turn number that would end script execution.
	int getEndTurn() const { return _endTurn; };
	/// Gets the item set for command.
	const std::vector<std::string> getItemSet() const { return _itemSet; };
	/// Gets the unit set for command.
	const std::vector<std::string> getUnitSet() const { return _unitSet; };
	/// Gets the side for unit, that would be spawned with command.
	int getSide() const { return _unitSide; };
	/// Gets pack size for units, that would be spawned with command.
	int getPackSize() const { return _packSize; };
	/// Gets if pack size should be randomised with command.
	bool getRandomPackSize() const { return _randomPackSize; };
	/// Gets the min Z level for command.
	int getMinLevel() const { return _minLevel; };
	/// Gets the max Z level for command.
	int getMaxLevel() const { return _maxLevel; };
	/// Gets the min difficulty level for command.
	int getMinDifficulty() const { return _minDifficulty; };
	/// Gets the max difficulty level for command.
	int getMaxDifficulty() const { return _maxDifficulty; };

};

}

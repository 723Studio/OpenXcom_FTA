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
	std::string title, content, answer, background;

	/// Loads stats from YAML.
	void load(const YAML::Node& node)
	{
		title = node["title"].as<std::string>(title);
		content = node["content"].as<std::string>(content);
		answer = node["answer"].as<std::string>(answer);
		background = node["background"].as<std::string>(background);
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
	std::vector<int> _groups, _conditionals;
	int _executionChances, _executions, _maxRuns, _label;
	std::vector<std::string> _spawnBlocks;
	std::vector<std::string> _itemSet, _unitSet;
	std::map<int, BattleMessage> _message;
	int _startTurn, _endTurn, _unitSide, _packSize, _minLevel, _maxLevel;
	bool _randomPackSize;
	int _minDifficulty, _maxDifficulty, _minAlarmLevel, _maxAlarmLevel;
	std::vector<int> _spawnNodeRanks;
	std::string _variable;
public:
	BattleScript();
	~BattleScript();
	/// Loads information from a ruleset file.
	void load(const YAML::Node& node);
	/// Gets what type of command this is.
	BattleScriptCommand getType() const { return _type; };
	/// Get the chances of this command executing.
	int getChancesOfExecution() const { return _executionChances; };
	/// Gets the label for this command.
	int getLabel() const { return _label; };
	/// Gets how many times this command repeats in one turn.
	int getExecutions() const { return _executions; };
	/// Gets how many times this command repeats in battlescape game session (controlled with variable).
	int getMaxRuns() const { return _maxRuns; };
	/// Gets what conditions apply to this command.
	const std::vector<int>* getConditionals() const { return &_conditionals; };
	/// Gets the groups vector for iteration.
	const std::vector<int>* getGroups() const { return &_groups; };
	/// Gets the blocks vector for iteration.
	std::vector<std::string> getSpawnBlocks() const { return  _spawnBlocks; };
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
	/// Gets the min required alarm level to run this command.
	int getMinAlarm() const { return _minAlarmLevel; };
	/// Gets the max alarm level for this command to run.
	int getMaxAlarm() const { return _maxAlarmLevel; };
	/// Gets what conditions apply to this command.
	const std::vector<int> *getSpawnNodeRanks() const { return &_spawnNodeRanks; };
	/// Gets the list of battle messages for command.
	std::map<int, BattleMessage> getBattleMessages() const { return _message; };
	/// Gets the script variable name.
	std::string getVariableName() const { return _variable; };
};

}

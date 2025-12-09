#pragma once
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
#include <string>
#include <vector>
#include <map>
#include "../Engine/Yaml.h"

namespace OpenXcom
{

class RuleDiplomacyFactionEvent
{
private:
	std::string _type;
	int _firstMonth, _lastMonth, _executionOdds, _minDifficulty, _maxDifficulty;
	int _minPlayerScore, _maxPlayerScore;
	int _minPower, _maxPower, _powerChange;
	int _vigilanceChange;
	int64_t _minFunds, _maxFunds, _fundsChange;
	std::map<std::string, bool> _playerResearchTriggers, _factionResearchTriggers;
	std::map<std::string, bool> _itemTriggers;
	std::map<std::string, int> _itemsToAdd;
	std::map<std::string, int> _staffToAdd;
	std::vector<std::string> _discoveredResearches;
public:
	/// Creates a blank RuleEventScript.
	RuleDiplomacyFactionEvent(const std::string& type);
	/// Cleans up the event script ruleset.
	~RuleDiplomacyFactionEvent();
	/// Loads an event script from YAML.
	void load(const YAML::YamlNodeReader& reader);
	/// Gets the name of the script command.
	const std::string& getType() const { return _type; }
	/// Gets the first month this command will run.
	int getFirstMonth() const { return _firstMonth; }
	/// Gets the last month this command will run.
	int getLastMonth() const { return _lastMonth; }
	/// Gets the odds of this command executing.
	int getExecutionOdds() const { return _executionOdds; }
	/// Gets the minimum difficulty for this command to run.
	int getMinDifficulty() const { return _minDifficulty; }
	/// Gets the maximum difficulty for this command to run.
	int getMaxDifficulty() const { return _maxDifficulty; }
	/// Gets the minimum score (from last month) for this command to run.
	int getMinScore() const { return _minPlayerScore; }
	/// Gets the maximum score (from last month) for this command to run.
	int getMaxScore() const { return _maxPlayerScore; }
	/// Gets the minimum factional power for this command to run.
	int getMinPower() const { return _minPower; }
	/// Gets the maximum factional power for this command to run.
	int getMaxPower() const { return _maxPower; }
	/// Gets the minimum factional funds for this command to run.
	int64_t getMinFunds() const { return _minFunds; }
	/// Gets the maximum factional funds for this command to run.
	int64_t getMaxFunds() const { return _maxFunds; }
	/// Gets the triggers for the player's researches that may apply to this command.
	const std::map<std::string, bool>& getPlayerResearchTriggers() const { return _playerResearchTriggers; }
	/// Gets the triggers for the player's researches that may apply to this command.
	const std::map<std::string, bool>& getFactionResearchTriggers() const { return _factionResearchTriggers; }
	/// Gets the item triggers that may apply to this command.
	const std::map<std::string, bool>& getItemTriggers() const { return _itemTriggers; }

	/// Gets the value for factional power change.
	int getPowerChange() const { return _powerChange; }
	/// Gets the value for factional funds change.
	int64_t getFundsChange() const { return _fundsChange; }
	/// Gets the value for factional vigilance change.
	int getVigilanceChange() const { return _vigilanceChange; }
	/// Gets the list of items to add to factional storage.
	const std::map<std::string, int>& getItemsToAdd() const { return _itemsToAdd; }
	/// Gets the list of staff, that would join faction.
	const std::map<std::string, int>& getStaffToAdd() const { return _staffToAdd; }
	/// Gets the list of discovered researches.
	const std::vector<std::string>& getDiscoveredResearches() const { return _discoveredResearches; }
};

}

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
#include "RuleDiplomacyFactionEvent.h"
#include <climits>

namespace OpenXcom
{

/**
* RuleDiplomacyFactionEvent: the (optional) rules for generating Factional events, to simulate ongoing processes for it's AI.
* Each element is independent, and Faction in its daily think() process will go through all allowed for this faction rules to check if something is allowed to happen.
*/
RuleDiplomacyFactionEvent::RuleDiplomacyFactionEvent(const std::string& type) :
	_type(type), _firstMonth(0), _lastMonth(-1), _executionOdds(100), _minDifficulty(0), _maxDifficulty(4),
	_minPlayerScore(INT_MIN), _maxPlayerScore(INT_MAX), _minPower(INT_MIN), _maxPower(INT_MAX), _powerChange(0), _vigilanceChange(0),
	_minFunds(INT64_MIN), _maxFunds(INT64_MAX), _fundsChange(0)
{
}

/**
* Cleans up the faction event ruleset.
*/
RuleDiplomacyFactionEvent::~RuleDiplomacyFactionEvent()
{
}

/**
* Loads an event script from YAML.
* @param reader YAML reader.
*/
void RuleDiplomacyFactionEvent::load(const YAML::YamlNodeReader& node)
{
	const auto& reader = node.useIndex();
	if (const YAML::YamlNodeReader& parent = reader["refNode"])
	{
		load(reader["refNode"]);
	}
	reader.tryRead("type", _type);
	reader.tryRead("firstMonth", _firstMonth);
	reader.tryRead("lastMonth", _lastMonth);
	reader.tryRead("executionOdds", _executionOdds);
	reader.tryRead("minDifficulty", _minDifficulty);
	reader.tryRead("maxDifficulty", _maxDifficulty);
	reader.tryRead("minPlayerScore", _minPlayerScore);
	reader.tryRead("maxPlayerScore", _maxPlayerScore);
	reader.tryRead("minPower", _minPower);
	reader.tryRead("maxPower", _maxPower);
	reader.tryRead("powerChange", _powerChange);
	reader.tryRead("vigilanceChange", _vigilanceChange);
	reader.tryRead("minFunds", _minFunds);
	reader.tryRead("maxFunds", _maxFunds);
	reader.tryRead("fundsChange", _fundsChange);
	reader.tryRead("playerResearchTriggers", _playerResearchTriggers);
	reader.tryRead("factionResearchTriggers", _factionResearchTriggers);
	reader.tryRead("itemTriggers", _itemTriggers);
	reader.tryRead("itemsToAdd", _itemsToAdd);
	reader.tryRead("staffToAdd", _staffToAdd);
	reader.tryRead("discoveredResearches", _discoveredResearches);
}
}

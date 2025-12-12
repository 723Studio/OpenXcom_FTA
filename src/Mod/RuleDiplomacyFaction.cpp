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
#include "RuleDiplomacyFaction.h"
#include "../Engine/RNG.h"
#include "../fmath.h"

namespace OpenXcom
{

RuleDiplomacyFaction::RuleDiplomacyFaction(const std::string &name) :
	_name(name), _description("NONE"), _background("BACK13.SCR"), _cardBackground("BACK13.SCR"),
	_genMissionFrequency(0), _helpTreatyGap(0),
	_sellPriceFactor(0), _buyPriceFactor(0), _repPriceFactor(0), _powerHungry(10000), _scienceBaseCost(2000),
	_startingReputation(0), _startingFunds(0), _startingPower(0)
{
}

void RuleDiplomacyFaction::load(const YAML::YamlNodeReader& node)
{
	const auto& reader = node.useIndex();
	if (const YAML::YamlNodeReader& parent = reader["refNode"])
	{
		load(reader["refNode"]);
	}
	reader.tryRead("name", _name);
	reader.tryRead("description", _description);
	reader.tryRead("background", _background);
	reader.tryRead("cardBackground", _cardBackground);
	reader.tryRead("discoverResearch", _discoverResearch);
	reader.tryRead("discoverEvent", _discoverEvent);
	reader.tryRead("helpTreatyMissions", _helpTreatyMissions);
	reader.tryRead("helpTreatyEventScripts", _helpTreatyEventScripts);
	reader.tryRead("genMissionFreq", _genMissionFrequency);
	reader.tryRead("helpTreatyGap", _helpTreatyGap);
	reader.tryRead("usualEventsScripts", _usualEventsScripts);
	reader.tryRead("happyEvents", _happyEvents);
	reader.tryRead("angryEvents", _angryEvents);
	reader.tryRead("factionalEvents", _factionalEvents);
	reader.tryRead("sellPriceFactor", _sellPriceFactor);
	reader.tryRead("buyPriceFactor", _buyPriceFactor);
	reader.tryRead("repPriceFactor", _repPriceFactor);
	reader.tryRead("wishList", _wishList);
	reader.tryRead("staffWeights", _staffWeights);
	reader.tryRead("powerHungry", _powerHungry);
	reader.tryRead("scienceBaseCost", _scienceBaseCost);
	reader.tryRead("startingReputation", _startingReputation);
	reader.tryRead("startingFunds", _startingFunds);
	reader.tryRead("startingPower", _startingPower);
	reader.tryRead("startingItems", _startingItems);
	reader.tryRead("startingStaff", _startingStaff);
	reader.tryRead("startingResearches", _startingResearches);
}

}

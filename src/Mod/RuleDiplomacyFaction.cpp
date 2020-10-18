/*
 * Copyright 2010-2019 OpenXcom Developers.
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
			_startingReputation(0), _genMissionFrequency(0), _genEventFrequency(0)
{
}

/**
 * Loads the event definition from YAML.
 * @param node YAML node.
 */
void RuleDiplomacyFaction::load(const YAML::Node &node)
{
	if (const YAML::Node &parent = node["refNode"])
	{
		load(parent);
	}
	_name = node["name"].as<std::string>(_name);
	_description = node["description"].as<std::string>(_description);
	_background = node["background"].as<std::string>(_background);
	_cardBackground = node["cardBackground"].as<std::string>(_cardBackground); 
	_sellingSet = node["sellingSet"].as<std::map<std::string, int>>(_sellingSet);
	_discoverResearch = node["discoverResearch"].as<std::string>(_discoverResearch);
	_discoverEvent = node["discoverEvent"].as<std::string>(_discoverEvent);
	_startingReputation = node["startingReputation"].as<int>(_startingReputation);
	if (node["helpTreatyMissions"])
	{
		_helpTreatyMissions.load(node["helpTreatyMissions"]);
	}
	if (node["helpTreatyEvents"])
	{
		_helpTreatyEvents.load(node["helpTreatyEvents"]);
	}
	_genMissionFrequency = node["genMissionFreq"].as<int>(_genMissionFrequency);
	_genEventFrequency = node["genEventFreq"].as<int>(_genEventFrequency);
}
std::string RuleDiplomacyFaction::chooseGenMissionScriptType() const
{
	return _helpTreatyMissions.choose();
}

std::string RuleDiplomacyFaction::chooseGenEventScriptType() const
{
	return _helpTreatyEvents.choose();
}

}

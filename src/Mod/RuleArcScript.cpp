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
#include "RuleArcScript.h"
#include <climits>

namespace OpenXcom
{

/**
 * RuleArcScript: the (optional) rules for the high-level game progression.
 * Each script element is independent, and the saved game will probe the list of these each month to determine what's going to happen.
 * Arc scripts are executed just before the mission scripts and unlock research topics that can then be used by mission scripts.
 */
RuleArcScript::RuleArcScript(const std::string& type) :
	_type(type), _firstMonth(0), _lastMonth(-1), _executionOdds(100), _maxArcs(-1), _minDifficulty(0), _maxDifficulty(4),
	_minScore(INT_MIN), _maxScore(INT_MAX), _minLoyalty(INT_MIN), _maxLoyalty(INT_MAX), _minFunds(INT64_MIN), _maxFunds(INT64_MAX)
{
}

/**
 *
 */
RuleArcScript::~RuleArcScript()
{
}

/**
 * Loads an arcScript from a YML file.
 * @param node the node within the file we're reading.
 */
void RuleArcScript::load(const YAML::Node& node)
{
	if (const YAML::Node & parent = node["refNode"])
	{
		load(parent);
	}
	_type = node["type"].as<std::string>(_type);
	_sequentialArcs = node["sequentialArcs"].as<std::vector<std::string> >(_sequentialArcs);
	if (node["randomArcs"])
	{
		_randomArcs.load(node["randomArcs"]);
	}
	_firstMonth = node["firstMonth"].as<int>(_firstMonth);
	_lastMonth = node["lastMonth"].as<int>(_lastMonth);
	_executionOdds = node["executionOdds"].as<int>(_executionOdds);
	_maxArcs = node["maxArcs"].as<int>(_maxArcs);
	_minDifficulty = node["minDifficulty"].as<int>(_minDifficulty);
	_maxDifficulty = node["maxDifficulty"].as<int>(_maxDifficulty);
	_minScore = node["minScore"].as<int>(_minScore);
	_maxScore = node["maxScore"].as<int>(_maxScore);
	_minLoyalty = node["minLoyalty"].as<int>(_minLoyalty);
	_maxLoyalty = node["maxLoyalty"].as<int>(_maxLoyalty);
	_minFunds = node["minFunds"].as<int64_t>(_minFunds);
	_maxFunds = node["maxFunds"].as<int64_t>(_maxFunds);
	_researchTriggers = node["researchTriggers"].as<std::map<std::string, bool> >(_researchTriggers);
	_itemTriggers = node["itemTriggers"].as<std::map<std::string, bool> >(_itemTriggers);
	_facilityTriggers = node["facilityTriggers"].as<std::map<std::string, bool> >(_facilityTriggers);
}

}

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
#include "FactionalResearch.h"

#include "SoldierPool.h"

#include <assert.h>
#include <algorithm>
#include "../fmath.h"
#include "../Engine/RNG.h"
#include "../Engine/Logger.h"
#include "../Mod/RuleResearch.h"
#include "../Mod/RuleDiplomacyFaction.h"
#include "../Savegame/DiplomacyFaction.h"


namespace OpenXcom
{

/**
* Initializes an FactionalContainer with no contents.
*/
FactionalResearch::FactionalResearch(const RuleResearch* rule, DiplomacyFaction* faction) :
_rule(rule), _faction(faction), _priority(0), _timeLeft(0), _scientists(0)
{
}

FactionalResearch::~FactionalResearch()
{
}

/**
* Loads the Diplomacy Faction from YAML.
* @param node The YAML node containing the data.
*/
void FactionalResearch::load(const YAML::Node& node, SavedGame* save, const Mod* mod)
{
	_scientists->load(node, save, mod);
	_timeLeft = node["timeLeft"].as<int>(_timeLeft);
}

/**
* Saves the Factional Research to YAML.
* @return YAML node.
*/
YAML::Node FactionalResearch::save(const Mod* mod) const
{
	YAML::Node node;
	node["name"] = _rule->getName();
	node["scientists"] = _scientists->save(mod);
	node["timeLeft"] = _timeLeft;

	return node;
}

/**
* Process ongoing research project, decreasing timer.
* @return true if project is over.
*/
bool FactionalResearch::step()
{
	int64_t effort = _scientists->getSoldiers().size(); // #FINNIKTODO - more precise calculation required?
	_timeLeft -= effort;
	int64_t cost = _faction->getRules()->getScienceBaseCost() * effort;
	_faction->setFunds(_faction->getFunds() - cost);
	return (_timeLeft <= 0);
}

const std::string& FactionalResearch::getName()
{
	return _rule->getName();
}

}

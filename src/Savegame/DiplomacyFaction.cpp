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
#include "DiplomacyFaction.h"
#include <assert.h>
#include "../Mod/RuleDiplomacyFaction.h"
#include "../Savegame/SavedGame.h"
#include "../Engine/Game.h"
#include "../Geoscape/GeoscapeEventState.h"
#include "../Savegame/GeoscapeEvent.h"
#include "../Mod/RuleEvent.h"
#include "../Mod/RuleMissionScript.h"
#include "../Savegame/AlienStrategy.h"
#include "../fmath.h"
#include "../Engine/RNG.h"
#include "../Engine/Logger.h"


namespace OpenXcom
{

DiplomacyFaction::DiplomacyFaction(const RuleDiplomacyFaction &rule) : _rule(rule), _reputation(0), _reputationLvL(0), _discovered(false)
{
	// Empty by design.
}

DiplomacyFaction::~DiplomacyFaction()
{
	// Empty by design.
}

/**
 * Loads the event from YAML.
 * @param node The YAML node containing the data.
 */
void DiplomacyFaction::load(const YAML::Node &node)
{
	_reputation = node["reputation"].as<int>(_reputation);
	_discovered = node["discovered"].as<bool>(_discovered);
	_treaties = node["treaties"].as<std::vector<std::string>>(_treaties);
}

/**
 * Saves the event to YAML.
 * @return YAML node.
 */ 
YAML::Node DiplomacyFaction::save() const
{
	YAML::Node node;
	node["name"] = _rule.getName();
	node["reputation"] = _reputation;
	if (_discovered)
	{
		node["discovered"] = _discovered;
	}
	node["treaties"] = _treaties;
	return node;
}

/**
 * Sets current player's reputation in this Faction.
 * @param new reputation value.
 */
void DiplomacyFaction::setReputation(int reputation)
{
	_reputation = reputation;
}

int DiplomacyFaction::getReputationLevel()
{
	return _reputationLvL;
}

std::string DiplomacyFaction::getReputationName()
{
	int repLvl = getReputationLevel();
	if (repLvl == 3)
	{
		return "STR_ALLY";
	}
	else if (repLvl == 2)
	{
		return "STR_HONORED";
	}
	else if (repLvl == 1)
	{
		return "STR_FRIENDLY";
	}
	else if (repLvl == 0)
	{
		return "STR_NEUTRAL";
	}
	else if (repLvl == -1)
	{
		return "STR_UNFRIENDLY";
	}
	else if (repLvl == -2)
	{
		return "STR_HOSTILE";
	}
	else if (repLvl == -3)
	{
		return "STR_HATED";
	}
	else
	{
		return "STR_ERROR";
	}

}

void DiplomacyFaction::updateReputationLevel()
{
	if (_reputation >= 75)
	{
		_reputationLvL = 3;
	}
	else if (_reputation >= 50 && _reputation < 75)
	{
		_reputationLvL = 2;
	}
	else if (_reputation >= 25 && _reputation < 50)
	{
		_reputationLvL = 1;
	}
	else if (_reputation >= -25 && _reputation < 25)
	{
		_reputationLvL = 0;
	}
	else if (_reputation <= -50 && _reputation < 25)
	{
		_reputationLvL = -1;
	}
	else if (_reputation <= -50 && _reputation < 25)
	{
		_reputationLvL = -2;
	}
	else if (_reputation <= -50 && _reputation < 25)
	{
		_reputationLvL = -3;
	}
	else { _reputationLvL = 0; }
}


/**
 * Sets status of the Faction to the player, was it discovered or not.
 * @param isDiscovered status.
 */
void DiplomacyFaction::setDiscovered(bool status)
{
	_discovered = status;
}
/**
 * Handle Faction logic..
 * @param Game game engine.
 * @param ThinkPeriond - timestep to determine think process
 */
bool DiplomacyFaction::think(Game& engine, ThinkPeriond period)
{
	const Mod& mod = *engine.getMod();
	SavedGame& game = *engine.getSavedGame();
	//lets process out daily duty
	if (period == TIMESTEP_DAILY)
	{
		//check if we discover our faction
		if (game.isResearched(mod.getResearch(_rule.getDiscoverResearch())) && !_discovered)
		{
			setDiscovered(true);
			//spawn celebration event if faction wants it
			if (!_rule.getDiscoverEvent().empty())
			{
				RuleEvent* eventRules = mod.getEvent(_rule.getDiscoverEvent());
				GeoscapeEvent* newEvent = new GeoscapeEvent(*eventRules);
				int minutes = (eventRules->getTimer() + (RNG::generate(0, eventRules->getTimerRandom()))) / 30 * 30;
				if (minutes < 30) minutes = 30; // just in case
				newEvent->setSpawnCountdown(minutes);
				game.getGeoscapeEvents().push_back(newEvent);
				// remember that it has been generated
				game.addGeneratedEvent(eventRules);
			}
			// update reputation level for just discovered fraction
			updateReputationLevel();
			// and if it turns friendly at start we sign help treaty by default
			if (_reputationLvL > 0)
			{
				_treaties.push_back("STR_HELP_TREATY");
			}
		}
		// let's see 
		if (std::find(_treaties.begin(), _treaties.end(), "STR_HELP_TREATY") != _treaties.end())
		{
			bool generate = factionMissionGenerator(engine);
			return generate;
		}
	}
	return false;
}
bool DiplomacyFaction::factionMissionGenerator(Game& engine)
{
	const Mod& mod = *engine.getMod();
	SavedGame& game = *engine.getSavedGame();
	if (RNG::percent(_rule.getGenMissionFrequency()))
	{
		const RuleMissionScript& ruleScript = *mod.getMissionScript(_rule.chooseGenMissionScriptType());
		auto month = game.getMonthsPassed();
		// lets process mission script with own way, first things first
		if (ruleScript.getFirstMonth() <= month &&
			(ruleScript.getLastMonth() >= month || ruleScript.getLastMonth() == -1) &&
			// make sure we haven't hit our run limit, if we have one
			(ruleScript.getMaxRuns() == -1 ||	ruleScript.getMaxRuns() > game.getAlienStrategy().getMissionsRun(ruleScript.getVarName())) &&
			// and make sure we satisfy the difficulty restrictions
			(month < 1 || ruleScript.getMinScore() <= game.getCurrentScore(month)) &&
			(month < 1 || ruleScript.getMaxScore() >= game.getCurrentScore(month)) &&
			(month < 1 || ruleScript.getMinFunds() <= game.getFunds()) &&
			(month < 1 || ruleScript.getMaxFunds() >= game.getFunds()) &&
			ruleScript.getMinDifficulty() <= game.getDifficulty())
		{
			// level two condition check: make sure we meet any research requirements, if any.
			bool triggerHappy = true;
			for (std::map<std::string, bool>::const_iterator j = ruleScript.getResearchTriggers().begin(); triggerHappy && j != ruleScript.getResearchTriggers().end(); ++j)
			{
				triggerHappy = (game.isResearched(j->first) == j->second);
				if (!triggerHappy)
					return false;
			}
			if (triggerHappy)
			{
				// item requirements
				for (auto& triggerItem : ruleScript.getItemTriggers())
				{
					triggerHappy = (game.isItemObtained(triggerItem.first) == triggerItem.second);
					if (!triggerHappy)
						return false;
				}
			}
			if (triggerHappy)
			{
				// facility requirements
				for (auto& triggerFacility : ruleScript.getFacilityTriggers())
				{
					triggerHappy = (game.isFacilityBuilt(triggerFacility.first) == triggerFacility.second);
					if (!triggerHappy)
						return false;
				}
			}
			// levels one and two passed: insert this command into the array.
			if (triggerHappy)
			{
				_generatedCommandType = ruleScript.getType();
				return true;
			}
		}
	}
	return false;
}



}

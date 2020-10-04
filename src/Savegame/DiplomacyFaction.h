#pragma once
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
#include <string>
#include <yaml-cpp/yaml.h>

namespace OpenXcom
{
/**
* Define what time frame should think logic process.
*/
enum ThinkPeriod { TIMESTEP_DAILY, TIMESTEP_MONTHLY };

/**
* Define arrey of treaty names.
*/
enum TreatyName { HELP_TREATY, RESEARCH_TREATY };

class RuleDiplomacyFaction;
class Game;
class SavedGame;
class Mod;
class GeoscapeEvent;
class RuleEvent;
class RuleMissionScript;


/**
 * Represents a custom Diplomacy Faction, spawned at the game start and waiting to be discovered.
 */
class DiplomacyFaction
{
private:
	const RuleDiplomacyFaction &_rule;
	int _reputation, _reputationLvL;
	bool _discovered;
	std::vector<std::string> _treaties;
	std::string _generatedCommandType;
public:
	/// Creates a blank Diplomacy Faction.
	DiplomacyFaction(const RuleDiplomacyFaction &rule);
	/// Cleans up the Faction info.
	~DiplomacyFaction();
	/// Loads the Faction from YAML.
	void load(const YAML::Node &node);
	/// Saves the Faction to YAML.
	YAML::Node save() const;
	/// Gets the Faction's ruleset.
	const RuleDiplomacyFaction &getRules() const { return _rule; };
	/// Gets current player's reputation in this Faction.
	int getReputation() const { return _reputation; }
	/// Sets current player's reputation in this Faction.
	void setReputation(int reputation);
	/// Gets corrent reputation level.
	int getReputationLevel();
	/// Gets corrent reputation level name.
	std::string getReputationName();
	/// Update reputation level based on current reputation of the faction.
	void updateReputationLevel();
	/// Is this Faction was discovered?
	bool isDiscovered() const { return _discovered; }
	/// Sets Faction's discovered status.
	void setDiscovered(bool status);
	/// Handle Faction logic.
	bool think(Game& engine, ThinkPeriod = TIMESTEP_DAILY);
	/// Generates mission for the faction base on current situation in the game.
	bool factionMissionGenerator(Game& engine);
	/// Generates event from the faction base on current situation in the game.
	bool factionEventGenerator(Game& engine);
	/// Get mission script command that pass all checks to generate alien mission for that faction.
	std::string getCommandType() const { return _generatedCommandType; };
};
}

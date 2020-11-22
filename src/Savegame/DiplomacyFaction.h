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
	int _reputationScore, _reputationLvL;
	bool _discovered, _thisMonthDiscovered;
	std::vector<std::string> _treaties;
	std::string _reputationName, _generatedCommandType;
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
	int getReputationScore() const { return _reputationScore; }
	/// Sets current player's reputation in this Faction.
	void setReputationScore(int reputation) { _reputationScore = reputation; };
	/// Gets corrent reputation level.
	int getReputationLevel() const { return _reputationScore;};
	/// Gets corrent reputation level name.
	std::string getReputationName() const { return _reputationName; };
	/// Sets new reputation level of the faction.
	void setReputationLevel(int level) { _reputationLvL = level; };
	/// Sets new reputation level of the faction.
	void setReputationName(std::string reputationName ) { _reputationName = reputationName; };
	/// Is this Faction was discovered?
	bool isDiscovered() const { return _discovered; }
	/// Sets Faction's discovered status.
	void setDiscovered(bool status) { _discovered = status; };
	/// Was this Faction discovered this month?
	bool isThisMonthDiscovered() const { return _thisMonthDiscovered; }
	/// Sets Faction's this month discovered status.
	void setThisMonthDiscovered(bool status) { _thisMonthDiscovered = status; };
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

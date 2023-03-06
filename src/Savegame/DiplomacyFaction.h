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
enum ThinkPeriod { TIMESTEP_DAILY, TIMESTEP_10_DAYS, TIMESTEP_MONTHLY };

/**
* Define array of treaty names.
*/
enum TreatyName { HELP_TREATY, RESEARCH_TREATY };

class RuleDiplomacyFaction;
class Game;
class SavedGame;
class Mod;
class MasterMind;
class GeoscapeEvent;
class RuleEvent;
class RuleMissionScript;
class RuleResearch;
class RuleSoldier;
class RuleCraft;
class RuleItem;
class ItemContainer;
class FactionalContainer;
class FactionalResearch;
class RuleDiplomacyFactionEvent;

/**
 * Represents a custom Diplomacy Faction, spawned at the game start and waiting to be discovered.
 */
class DiplomacyFaction
{
private:
	const Mod* _mod;
	const RuleDiplomacyFaction* _rule;
	int _reputationScore, _reputationLvL, _power, _vigilance, _helpTreatyTimer;
	std::vector<std::string> _avoidRepeatVars;
	int64_t _funds;
	std::vector<int> _dailyRepScore;
	bool _discovered, _thisMonthDiscovered, _repLvlChanged;
	std::vector<std::string> _treaties;
	std::string _reputationName;
	std::vector<std::string> _commandsToProcess, _eventsToProcess;
	std::vector<RuleMissionScript*> _availableMissionScripts;
	std::vector<std::string> _unlockedResearches;
	ItemContainer* _items, *_secretItems;
	FactionalContainer* _staff;
	std::vector<FactionalResearch*> _research;

	/// Handle daily reputation change and immidiate reaction to it.
	void processDailyReputation(Game& engine);
	/// Handle factional specific events.
	void processFactionalEvents(Game& engine);
	/// Generates mission for the faction base on current situation in the game.
	void factionMissionGenerator(Game& engine);
	/// Handle purshaising of Faction's items, based on current situation.
	void handleRestock();
	/// Handle selling of faction's items, if they don't need them anymore.
	void handleSelling(Mod& mod);
	/// Handle managing of Faction's staff and non-item equipment.
	void manageStaff();
	/// Process Faction's power management and returns required funds for further use.
	int64_t managePower(int64_t month, int64_t baseCost);
	/// Handle researching.
	void handleResearch(Game& engine, int64_t reqFunds);
	/// Get if research article is unlocked by faction.
	bool isResearched(const std::string& name) const;
	bool isResearched(const RuleResearch* rule) const;
	bool isResearched(const std::vector<std::string>& names) const;
	bool isResearched(const std::vector<const RuleResearch*>& rules) const;

public:
	/// Creates a blank Diplomacy Faction.
	DiplomacyFaction(const Mod* mod, const std::string& name);
	/// Cleans up the Faction info.
	~DiplomacyFaction();
	/// Loads the Faction from YAML.
	void load(const YAML::Node &node);
	/// Saves the Faction to YAML.
	YAML::Node save() const;
	/// Gets the Faction's ruleset.
	const RuleDiplomacyFaction* getRules() const { return _rule; };
	/// Gets current player's reputation in this Faction.
	int getReputationScore() const { return _reputationScore; }
	/// Sets current player's reputation in this Faction.
	void setReputationScore(int reputation) { _reputationScore = reputation; };
	/// Updates reputation score based on incoming value and handle simple reaction to it.
	void updateReputationScore(int change);
	/// Gets corrent reputation level.
	int getReputationLevel() const { return _reputationLvL;};
	/// Gets corrent reputation level name.
	std::string getReputationName() const { return _reputationName; };
	/// Sets new reputation level of the faction.
	void setReputationLevel(int level) { _reputationLvL = level; };
	/// Sets new reputation level of the faction.
	void setReputationName(const std::string& reputationName) { _reputationName = reputationName; };
	/// Adds research projet's name to a faction's list of unlocked researches.
	void unlockResearch(const std::string& research) { _unlockedResearches.push_back(research); };
	/// Removes research projet's name to a faction's list of unlocked researches.
	void disableResearch(const std::string& research);
	/// Gets the faction power value.
	int getPower() const { return _power; };
	/// Sets a new power value for the faction.
	void setPower(int power) { _power = power; };
	/// Gets the faction's funds.
	int getFunds() const { return _funds; };
	/// Sets a new value for the faction's funds.
	void setFunds(int funds) { _funds = funds; };
	/// Is this Faction was discovered?
	bool isDiscovered() const { return _discovered; }
	/// Sets Faction's discovered status.
	void setDiscovered(bool status) { _discovered = status; };
	/// Was this Faction discovered this month?
	bool isThisMonthDiscovered() const { return _thisMonthDiscovered; }
	/// Sets Faction's this month discovered status.
	void setThisMonthDiscovered(bool status) { _thisMonthDiscovered = status; };
	/// Was this Faction discovered this month?
	bool isThisMonthRepLvlChanged() const { return _repLvlChanged; }
	/// Sets Faction's this month discovered status.
	void setThisMonthRepLvlChanged(bool status) { _repLvlChanged = status; };
	/// Get mission script commands that pass all checks to generate alien mission for that faction.
	const std::vector<RuleMissionScript*>& getAvalibleMissionScripts() const { return _availableMissionScripts; };

	/// Public manipulators to factional stores.
	ItemContainer* getPublicItems() const { return _items; };
	void addItem(const RuleItem* item, int qty = 1);
	void removeItem(const RuleItem* item, int qty = 1);


	/// Gets Faction's personell and non-item properties.
	FactionalContainer* getStaffContainer() { return _staff; };

	/// The main handler of Faction logic.
	void think(Game& engine, ThinkPeriod = TIMESTEP_DAILY);
};

}

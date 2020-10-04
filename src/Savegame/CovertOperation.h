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
//#include "../Engine/State.h"

namespace OpenXcom
{

class RuleCovertOperation;
class Game;
class SavedGame;
class Globe;
class Mod;
class RuleAlienMission;
class DiplomacyFaction;
class Soldier;
class Base;
class ItemContainer;
class CovertOperationResults;

/**
* Represents a Covert Operation.
*/
class CovertOperation
{
private:
	const RuleCovertOperation* _rule;
	Base* _base;
	int _spent, _cost;
	int _successChance, _assignedScientists, _assignedEngineers;
	ItemContainer* _items;
	CovertOperationResults* _results;
	bool _inBattlescape, _hasBattlescapeResolve, _over, _hasPsi, _progressEventSpawned;
	std::string _generatedMission, _researchName;
public:
	/// Creates a blank Covert Operation.
	CovertOperation(const RuleCovertOperation* rule, Base* base, int cost = 0, int chances = 0);
	/// Cleans up the Covert Operation info.
	~CovertOperation();
	/// Loads the Covert Operation from YAML.
	void load(const YAML::Node& node);
	/// Saves the Covert Operation to YAML.
	YAML::Node save() const;
	/// Gets the operation's ruleset.
	const RuleCovertOperation* getRules() const { return _rule; };
	/// Gets the operation's base.
	Base* getBase() const { return _base; };
	/// Sets the operation's base.
	void setBase(Base* base);
	/// Gets operation name
	std::string getOperationName();
	/// Gets spent effort for this operation.
	int getSpent() const { return _spent; };
	/// Sets spent effort for this operation.
	void setSpent(int spent) { _spent = spent; };
	/// Gets cost for this operation.
	int getCost() const { return _cost; };
	/// Sets cost for this operation.
	void setCost(int cost) { _cost = cost; };

	/// Gets success chance for this operation.
	int getSuccessChance() const { return _successChance; };
	/// Sets success chance for this operation.
	void setSuccessChance(int successChance) { _successChance = successChance; };
	/// Gets assigned scientists for this operation.
	int getAssignedScientists() const { return _assignedScientists; };
	/// Sets assigned scientists for this operation.
	void setAssignedScientists(int assignedScientists) { _assignedScientists = assignedScientists; };
	/// Gets assigned Engineers for this operation.
	int getAssignedEngineers() const { return _assignedEngineers; };
	/// Sets assigned Engineers for this operation.
	void setAssignedEngineers(int assignedEngineers) { _assignedEngineers = assignedEngineers; };
	/// Gets if operation has psionic agents capable to use their abilities.
	int getIsPsi() const { return _hasPsi; };
	/// Sets if operation has psionic agents capable to use their abilities.
	void setIsPsi(int hasPsi) { _hasPsi = hasPsi; };
	/// Gets name of the discovered research
	std::string getDiscoveredResearch() { return _researchName; }

	/// Gets operation's items.
	ItemContainer* getItems() { return _items; };
	/// Gets operation's results.
	CovertOperationResults* getResults() { return _results; }
	/// Gets operation's soldiers
	std::vector<Soldier*> getSoldiers();

	/// Gets approximate odds of operation success for UI.
	std::string getOddsName();
	/// Get approximate time before operation results.
	std::string getTimeLeftName();

	/// Is this covert operation over?
	bool isOver() const { return _over; }
	/// Is this covert operation is running in battlescape?
	bool isInBattlescape() const { return _inBattlescape; }
	/// Sets if this operation is running in battlescape
	void setInBattlescape(bool inbattle) { _inBattlescape = inbattle; }
	/// Handle covert operation logic.
	bool think(Game& engine, const Globe& globe);
	/// Process soldier's actions results in covert operation
	void backgroundSimulation(Game& engine, bool operationResult, bool criticalFail, int woundOdds, int deathOdds);
	/// Converts the number of experience to the stat increase.
	int improveStat(int exp) const;
	/// Takes care to finish covert operation.
	void finishOperation();
};

struct UnitStats;

class CovertOperationResults
{
private:
	std::string _operationName;
	std::string _finishDate;
	bool _result;
	int _score, _funds;
	std::map<std::string, int> _bountyItems, _reputation, _soldierDamage;
	std::string _specialMessage;
	std::vector<std::pair<std::string, UnitStats*>> _soldierStats;
public:
	/// Create base Covert Operation results
	CovertOperationResults(const std::string& operationName, bool result, std::string finishDate) :
		_operationName(operationName), _result(result), _finishDate(finishDate), _score(0), _funds(0) {};
	/// Manipulate score
	void addScore(int score) { _score += score; }
	int getScore() { return _score; }
	/// Manipulate funds
	void addFunds(int funds) { _funds += funds; }
	int getFunds() { return _funds; }
	/// Manipulate items
	void addItem(const std::string& name, int value) { _bountyItems[name] = value; }
	std::map<std::string, int> getItems() { return _bountyItems; }
	/// Manipulate reputation
	void addReputation(const std::string& name, int value) { _reputation[name] = value; }
	std::map<std::string, int> getReputation() { return _reputation; }
	/// Manipulate soldier damage information
	void addSoldierDamage(const std::string& name, int value) { _soldierDamage[name] = value; }
	std::map<std::string, int> getSoldierDamage() { return _soldierDamage; }
	/// Manipulate special story messages
	void setSpecialMessage(const std::string& message) { _specialMessage = message; }
	std::string getSpecialMessage() { return _specialMessage; }
	/// Handlers for soldier stat improvement
	void addSoldierImprovement(std::string soldier, UnitStats* improvement)
	{ _soldierStats.push_back(std::pair<std::string, UnitStats*>(soldier, improvement)); };
	std::vector<std::pair<std::string, UnitStats*>> getSoldierImprovement() const { return _soldierStats; }
};

}

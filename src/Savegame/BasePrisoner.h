#pragma once
/*
 * Copyright 2010-2016 OpenXcom Developers.
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
#include <utility>
#include <vector>
#include <string>
#include "Soldier.h"
#include "BattleUnit.h"

namespace OpenXcom
{

class Mod;
class RulePrisoner;
class Armor;
class Soldier;
class BattleUnit;
enum UnitFaction : int;
enum PrisonerState: int {
	PRISONER_STATE_NONE = 0,
	PRISONER_STATE_CONTAINING = 1,
	PRISONER_STATE_INTERROGATION = 2,
	PRISONER_STATE_TORTURE = 3,
	PRISONER_STATE_REQRUITING = 4 };

class BasePrisoner
{
private:
	std::string _id, _type;
	const RulePrisoner* _rules = nullptr;
	PrisonerState _state;
	int	_soldierId, _health;
	UnitFaction _faction = FACTION_HOSTILE;
	std::string _name;
	UnitStats _stats;
	int _intelligence, _aggression, _morale, _cooperation;
	Armor *_armor = nullptr;
	Soldier* _geoscapeSoldier = nullptr;
	std::vector<SoldierRole> _roles;
	const Mod* _mod;

	void loadRoles(const std::vector<int>& r);
public:
	/// Creates a BasePrisoner.
	BasePrisoner(const Mod *mod, const std::string &type, std::string id);
	/// Loads the unit from YAML.
	void load(const YAML::Node &node, const Mod *mod);
	/// Saves the unit to YAML.
	YAML::Node save() const;

	//getters and setters
	const std::string& getId() const { return _id; }
	void setName(const std::string& name) { _name = name; }
	std::string getName() const { return _name; }
	std::string getNameAndId();

	void setPrisonerState(PrisonerState state) { _state = state; }
	PrisonerState getPrisonerState() const { return _state; }

	int getSoldierId() const { return _soldierId; }

	const RulePrisoner* getRules() const { return _rules; }

	void setGeoscapeSoldier(Soldier* soldier)
	{
		_geoscapeSoldier = soldier;
		_soldierId = _geoscapeSoldier->getId();
	};
	Soldier* getGeoscapeSoldier() const { return _geoscapeSoldier; }

	void setArmor(Armor* armor) { _armor = armor; }
	const Armor* getArmor() const { return _armor; }

	void setRoles(std::vector<SoldierRole> roles) { _roles = std::move(roles); }
	std::vector<SoldierRole> getRoles() const { return _roles; }

	void setFaction(UnitFaction faction) { _faction = faction; }
	UnitFaction getFaction() const { return _faction; }

	void setStats(const UnitStats* stats) { _stats = *stats; }
	UnitStats* getStats() { return &_stats; }

	void setHealth(int health) { _health = health; }
	int getHealth() const { return _health; }

	void setIntelligence(int intel) { _intelligence = intel; }
	int getIntelligence() const { return _intelligence; }

	void setAggression(int aggro) { _aggression = aggro; }
	int getAggression() const { return _aggression; }
	
	void setMorale(int morale) { _morale = morale; }
	int getMorale() const { return _morale; }

	void setCooperation(int cooperation) { _cooperation = cooperation; }
	int getCooperation() const { return _cooperation; }
	

};

}

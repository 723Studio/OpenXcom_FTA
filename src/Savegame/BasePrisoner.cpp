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
#include "BasePrisoner.h"
#include <sstream>
#include <algorithm>
#include "../Engine/Exception.h"
#include "../Mod/Mod.h"
#include "../Mod/Armor.h"
#include "../Mod/RulePrisoner.h"


namespace OpenXcom
{
/**
 * Initializes a BattleUnit from a Soldier
 * @param soldier Pointer to the Soldier.
 * @param depth the depth of the battlefield (used to determine movement type in case of MT_FLOAT).
 */
BasePrisoner::BasePrisoner(const Mod* mod, std::string type, std::string id) : _mod(mod), _type(type), _id(id), _soldierId(-1),
_health(1), _intelligence(0), _aggression(0), _morale(100), _cooperation(0)
{
	_rules = mod->getPrisonerRules(type);
}

BasePrisoner::~BasePrisoner()
{

}

///**
// * Loads the unit from a YAML file.
// * @param node YAML node.
// */
void BasePrisoner::load(const YAML::Node& node, const Mod* mod)
{
	std::vector<SoldierRole> _roles;

	_soldierId = node["soldierId"].as<int>(_soldierId);
	_health = node["health"].as<int>(_health);
	_faction = (UnitFaction)node["faction"].as<int>(_faction);
	_stats = node["stats"].as<UnitStats>(_stats);
	_intelligence = node["intelligence"].as<int>(_intelligence);
	_aggression = node["aggression"].as<int>(_aggression);
	_morale = node["morale"].as<int>(_morale);
	_cooperation = node["cooperation"].as<int>(_cooperation);
	if (node["armor"])
	{
		std::string armor = node["armor"].as<std::string>();
		_armor = mod->getArmor(armor);
	}
	//in case
	if (!_armor)
	{
		_armor = mod->getArmor(mod->getArmorsList().at(0));
		Log(LOG_ERROR) << "Base Prisoner named: " << _name << " fails to load correct ruleset for armor, default armor type: " << _armor->getType() << " was assigned. Please, report this case!";
	}

	if (node["roles"])
		loadRoles(node["roles"].as<std::vector<int> >());

}

///**
// * Saves the soldier to a YAML file.
// * @return YAML node.
// */
YAML::Node BasePrisoner::save() const
{
	YAML::Node node;

	node["id"] = _id;
	node["type"] = _type;
	if (_geoscapeSoldier)
	{
		node["soldierId"] = _geoscapeSoldier->getId();
	}
	else
	{
		node["soldierId"] = -1;
	}
	node["health"] = _health;
	node["faction"] = (int)_faction;
	node["stats"] = _stats;
	node["intelligence"] = _intelligence;
	node["aggression"] = _aggression;
	node["morale"] = _morale;
	node["cooperation"] = _cooperation;
	node["armor"] = _armor->getType();
	//node["roles"] = _roles;

	return node;
}

void BasePrisoner::loadRoles(const std::vector<int>& r)
{
	_roles.clear();
	for (auto i : r)
	{
		SoldierRole role = static_cast<SoldierRole>(i);
		if (_roles.empty() || std::find(_roles.begin(), _roles.end(), role) == _roles.end())
		{
			_roles.push_back(role);
		}
	}
}

}

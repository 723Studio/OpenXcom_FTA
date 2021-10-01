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

#include "BattleObject.h"
#include "Tile.h"
#include "SavedGame.h"
#include "SavedBattleGame.h"
#include "../Mod/Mod.h"
#include "../Mod/RuleObject.h"
#include "../Engine/Surface.h"
#include "../Engine/SurfaceSet.h"
#include "../Engine/RNG.h"
#include "../fmath.h"

namespace OpenXcom
{
/**
* Initializes BattleObject of the specified type.
* @param rules Pointer to ruleset.
* @param id The id of the object.
*/
BattleObject::BattleObject(const RuleObject* rules) : _rules(rules), _tile(0), _hackingDefence(0), _wasHacked(false)
{
	if (_rules)
	{
		_hackingDefence = rules->getHackingDefence();
	}
}

/**
*
*/
BattleObject::~BattleObject()
{
}

/**
* Loads the BattleObject from a YAML file.
* @param node YAML node.
* @param mod Mod for the item.
*/
void BattleObject::load(const YAML::Node& node, Mod* mod)
{
	_hackingDefence = node["hackingDefence"].as<int>(_hackingDefence);
	_wasHacked = node["wasHacked"].as<bool>(_wasHacked);
	_position = node["position"].as<Position>(_position);
}

/**
* Saves the BattleObject to a YAML file.
* @return YAML node.
*/
YAML::Node BattleObject::save() const
{
	YAML::Node node;
	node["id"] = _id;
	node["type"] = _rules->getType();
	node["hackingDefence"] = _hackingDefence;
	node["wasHacked"] = _wasHacked;
	if (_tile)
		node["position"] = _tile->getPosition();

	return node;
}

/**
 * Create new battle object from rules for tile;
 */
BattleObject* SavedBattleGame::createObjectForTile(const RuleObject* rule, Tile* tile)
{
	BattleObject* object = new BattleObject(rule);
	if (tile)
	{
		tile->setBattleObject(object);
	}
	return object;
}

}

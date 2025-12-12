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
#include "../Engine/RNG.h"
#include "../Engine/Game.h"

namespace OpenXcom
{
/**
* Initializes BattleObject of the specified type.
* @param rules Pointer to ruleset.
* @param id The id of the object.
*/
BattleObject::BattleObject(const RuleObject* rules) : _rules(rules), _tile(0), _hackingDefence(0), _failedAttempts(0), _wasUsed(false)
{
	_hackingDefence = rules->getHackingDefence(); // needs as it can be increased later with failed hacking attempts
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
void BattleObject::load(const YAML::YamlNodeReader& reader, Mod* mod)
{
	reader.tryRead("hackingDefence", _hackingDefence);
	reader.tryRead("failedAttempts", _failedAttempts);
	reader.tryRead("wasUsed", _wasUsed);
	reader.tryRead("position", _position);
}

/**
* Saves the BattleObject to a YAML file.
* @return YAML node.
*/
void BattleObject::save(YAML::YamlNodeWriter writer) const
{
	writer.setAsMap();
	writer.write("type", _rules->getName());
	writer.write("hackingDefence", _hackingDefence);
	writer.write("failedAttempts", _failedAttempts);
	writer.write("wasUsed", _wasUsed);
	if (_tile)
		writer.write("position", _tile->getPosition());
}

void BattleObject::setTile(Tile *tile)
{
	_tile = tile;
}

/**
 * Process battleobject value updates on hacking
 * @param result - result of hacking, true if success
 */
void BattleObject::hackingPostProcess(bool result, Game* game)
{
	setHackingDefence(ceil(getHackingDefence() * RNG::generate(0.3, 0.7)));
	if (result)
	{
		for (auto e : _rules->getSpawnedEvents())
		{
			game->getSavedGame()->spawnEvent(game->getMod()->getEvent(e));
		}
		game->getSavedGame()->spawnEvent(game->getMod()->getEvent(_rules->getWeightedEvent(game->getSavedGame()->getMonthsPassed())));

		if (_rules->isMissionObjective())
		{
			game->getSavedGame()->getSavedBattle()->setHackingObjectiveGained(true);
		}
	}
	else
	{
		_failedAttempts++;
	}
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
		object->setTile(tile);
		object->setPosition(tile->getPosition());
		return object;
	}
	else
	{
		delete object;
		return 0;
	}
}
}

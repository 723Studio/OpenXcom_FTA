#pragma once
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
#include "../Engine/Yaml.h"
#include "../Mod/RuleObject.h"
#include "../Battlescape/Position.h"

namespace OpenXcom
{
class Game;
class RuleObject;
class Tile;
class Mod;
class SurfaceSet;
class Surface;
class SavedBattleGame;


/**
* Represents a single BattleObject in the battlescape.
* Contains battle-related info about an object like the position, hacking defence, etc.
* @sa RuleObject
*/
class BattleObject
{
private:
	const RuleObject* _rules;
	Tile* _tile;
	int _hackingDefence;
	int _failedAttempts;
	bool _wasUsed;
	Position _position;

public:
	/// Creates a BattleObject of the specified type.
	BattleObject(const RuleObject* rules);
	/// Cleans up the item.
	~BattleObject();
	/// Loads the item from YAML.
	void load(const YAML::YamlNodeReader& reader, Mod* mod);
	/// Saves the item to YAML.
	void save(YAML::YamlNodeWriter writer) const;
	/// Gets the item's ruleset.
	const RuleObject* getRules() const { return _rules; }

	/// Gets the item's tile.
	Tile* getTile() const { return _tile; }
	/// Sets the tile.
	void setTile(Tile* tile);

	/// Gets a flag if the object was used.
	void setWasUsed(bool wasHacked) { _wasUsed = wasHacked; }
	/// Checks a flag if the object was used.
	bool wasUsed() const { return _wasUsed; }
	/// Gets the objects's hacking defence value.
	int getHackingDefence() const { return _hackingDefence; }
	/// Sets the objects's hacking defence value.
	void setHackingDefence(int hackingDefence) { _hackingDefence = hackingDefence; }
	///returns a tile radius of alterations caused by hacking

	/// Checks if this object can be hacked
	bool canBeHacked() const { return !_wasUsed && _hackingDefence != 0; }

	void hackingPostProcess(bool result, Game* game);

	Position getPosition() { return _position; }

	void setPosition(Position pos) { _position = pos; }
};

}

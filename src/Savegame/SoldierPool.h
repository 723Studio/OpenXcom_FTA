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
#include "../Engine/Yaml.h"

namespace OpenXcom
{
class Mod;
class SavedGame;
class Soldier;
class RuleSoldier;
enum SoldierRole : int;

/**
 * Represents the items contained by a certain entity,
 * like base stores, craft equipment, etc.
 * Handles all necessary item management tasks.
 */
class SoldierPool
{
private:
	std::vector<Soldier*> _pool;
public:
	/// Creates an empty item container.
	SoldierPool();
	/// Cleans up the item container.
	~SoldierPool();
	/// Loads the item container from YAML.
	void load(const YAML::YamlNodeReader& reader, SavedGame* save, const Mod* mod);
	/// Saves the item container to YAML.
	void save(YAML::YamlNodeWriter writer, const Mod* mod) const;
	/// Creates a new generated soldier for the pool.
	void createSoldier(const RuleSoldier* rule, const Mod* mod, SavedGame* save);
	/// Adds an item to the container.
	void addSoldier(Soldier* soldier);
	/// Removes an item from the container.
	void removeSoldier(Soldier* soldier);
	/// Gets an item in the container.
	std::vector<Soldier*> getSoldiers() const;
	// Gets soldiers by role
	std::vector<Soldier*> getSoldiers(SoldierRole role) const;

};

}

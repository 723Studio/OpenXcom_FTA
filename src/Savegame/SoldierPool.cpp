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
#include "SoldierPool.h"
#include "../Mod/Mod.h"
#include "Soldier.h"
#include "../Savegame/SavedGame.h"

namespace OpenXcom
{
/**
 * Initializes a soldier pool with no contents.
 */
SoldierPool::SoldierPool()
{
}

/**
 *
 */
SoldierPool::~SoldierPool()
{
}

/**
 * Loads the item container from a YAML file.
 * @param node YAML node.
 */
void SoldierPool::load(const YAML::YamlNodeReader &reader, SavedGame* save, const Mod* mod)
{
	for (const auto& child : reader["soldiers"].children())
	{
		std::string type = mod->getSoldiersList().front();
		child.tryRead("type", type);
		if (mod->getSoldier(type))
		{
			Soldier* s = new Soldier(mod->getSoldier(type), nullptr, 0);
			s->load(child, mod, save, mod->getScriptGlobal());
			s->clearBaseDuty();
			_pool.push_back(s);
		}
	}
}

/**
 * Saves the item container to a YAML file.
 * @return YAML node.
 */
void SoldierPool::save(YAML::YamlNodeWriter writer, const Mod* mod) const
{
	writer.setAsMap();
	writer.write("soldiers", _pool,
		[&](YAML::YamlNodeWriter& vectorWriter, Soldier* s)
		{
			s->save(vectorWriter.write(), mod->getScriptGlobal());
		}
	);
}

void SoldierPool::createSoldier(const RuleSoldier* rule, const Mod* mod, SavedGame* save)
{
	int nationality = save->selectSoldierNationalityByLocation(mod, rule, nullptr); //diplomacy factions are purely international
	Soldier* soldier = mod->genSoldier(save, rule, nationality);
	addSoldier(soldier);
}

void SoldierPool::addSoldier(Soldier* soldier)
{
	_pool.push_back(soldier);
}

void SoldierPool::removeSoldier(Soldier* soldier)
{
	for (auto it = _pool.begin(); it != _pool.end(); ++it)
	{
		if ((*it) == soldier)
		{
			_pool.erase(it);
			break;
		}
	}
}

std::vector<Soldier*> SoldierPool::getSoldiers() const
{
	return _pool;
}

std::vector<Soldier*> SoldierPool::getSoldiers(SoldierRole role) const
{
	std::vector<Soldier*> filteredPool;
	for (auto s : _pool)
	{
		if (s->getRoleRank(role) > 0)
		{
			filteredPool.push_back(s);
		}
	}
	return filteredPool;
}

}

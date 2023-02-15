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
void SoldierPool::load(const YAML::Node &node, SavedGame* save, const Mod* mod)
{
	for (YAML::const_iterator i = node["soldiers"].begin(); i != node["soldiers"].end(); ++i)
	{
		std::string type = (*i)["type"].as<std::string>(mod->getSoldiersList().front());
		if (mod->getSoldier(type))
		{
			Soldier* s = new Soldier(mod->getSoldier(type), nullptr, 0);
			s->load(*i, mod, save, mod->getScriptGlobal());
			s->clearBaseDuty();
			_pool.push_back(s);
		}
	}
}

/**
 * Saves the item container to a YAML file.
 * @return YAML node.
 */
YAML::Node SoldierPool::save(const Mod* mod) const
{
	YAML::Node node;
	for (std::vector<Soldier*>::const_iterator i = _pool.begin(); i != _pool.end(); ++i)
	{
		node["soldiers"].push_back((*i)->save(mod->getScriptGlobal()));
	}
	
	return node;
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

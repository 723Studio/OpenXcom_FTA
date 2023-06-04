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
#include "MCDPatch.h"
#include "MapDataSet.h"
#include "MapData.h"

namespace OpenXcom
{

/**
 * Initializes an MCD Patch.
 */
MCDPatch::MCDPatch()
{
}

/**
 *
 */
MCDPatch::~MCDPatch()
{
}

/**
 * Loads the MCD Patch from a YAML file.
 * TODO: fill this out with more data.
 * @param node YAML node.
 */
void MCDPatch::load(const YAML::Node &node)
{
	YAML::Node data = node["data"];
	for (YAML::const_iterator i = data.begin(); i != data.end(); ++i)
	{
		size_t MCDIndex = (*i)["MCDIndex"].as<size_t>();
		if ((*i)["bigWall"])
		{
			int bigWall = (*i)["bigWall"].as<int>();
			_bigWalls.push_back(std::make_pair(MCDIndex, bigWall));
		}
		if ((*i)["TUWalk"])
		{
			int TUWalk = (*i)["TUWalk"].as<int>();
			_TUWalks.push_back(std::make_pair(MCDIndex, TUWalk));
		}
		if ((*i)["TUFly"])
		{
			int TUFly = (*i)["TUFly"].as<int>();
			_TUFlys.push_back(std::make_pair(MCDIndex, TUFly));
		}
		if ((*i)["TUSlide"])
		{
			int TUSlide = (*i)["TUSlide"].as<int>();
			_TUSlides.push_back(std::make_pair(MCDIndex, TUSlide));
		}
		if ((*i)["deathTile"])
		{
			int deathTile = (*i)["deathTile"].as<int>();
			_deathTiles.push_back(std::make_pair(MCDIndex, deathTile));
		}
		if ((*i)["terrainHeight"])
		{
			int terrainHeight = (*i)["terrainHeight"].as<int>();
			_terrainHeight.push_back(std::make_pair(MCDIndex, terrainHeight));
		}
		if ((*i)["specialType"])
		{
			int specialType = (*i)["specialType"].as<int>();
			_specialTypes.push_back(std::make_pair(MCDIndex, specialType));
		}
		if ((*i)["explosive"])
		{
			int explosive = (*i)["explosive"].as<int>();
			_explosives.push_back(std::make_pair(MCDIndex, explosive));
		}
		if ((*i)["armor"])
		{
			int armor = (*i)["armor"].as<int>();
			_armors.push_back(std::make_pair(MCDIndex, armor));
		}
		if ((*i)["flammability"])
		{
			int flammability = (*i)["flammability"].as<int>();
			_flammabilities.push_back(std::make_pair(MCDIndex, flammability));
		}
		if ((*i)["fuel"])
		{
			int fuel = (*i)["fuel"].as<int>();
			_fuels.push_back(std::make_pair(MCDIndex, fuel));
		}
		if ((*i)["footstepSound"])
		{
			int footstepSound = (*i)["footstepSound"].as<int>();
			_footstepSounds.push_back(std::make_pair(MCDIndex, footstepSound));
		}
		if ((*i)["HEBlock"])
		{
			int HEBlock = (*i)["HEBlock"].as<int>();
			_HEBlocks.push_back(std::make_pair(MCDIndex, HEBlock));
		}
		if ((*i)["noFloor"])
		{
			bool noFloor = (*i)["noFloor"].as<bool>();
			_noFloors.push_back(std::make_pair(MCDIndex, noFloor));
		}
		if ((*i)["LOFTS"])
		{
			std::vector<int> lofts = (*i)["LOFTS"].as< std::vector<int> >();
			_LOFTS.push_back(std::make_pair(MCDIndex, lofts));
		}
		if ((*i)["stopLOS"])
		{
			bool stopLOS = (*i)["stopLOS"].as<bool>();
			_stopLOSses.push_back(std::make_pair(MCDIndex, stopLOS));
		}
		if ((*i)["objectType"])
		{
			int objectType = (*i)["objectType"].as<int>();
			_objectTypes.push_back(std::make_pair(MCDIndex, objectType));
		}
	}
}

/**
 * Applies an MCD patch to a mapDataSet.
 * @param dataSet The MapDataSet we want to modify.
 */
void MCDPatch::modifyData(MapDataSet *dataSet) const
{
	for (const auto& pair : _bigWalls)
	{
		dataSet->getObject(pair.first)->setBigWall(pair.second);
	}
	for (const auto& pair : _TUWalks)
	{
		dataSet->getObject(pair.first)->setTUWalk(pair.second);
	}
	for (const auto& pair : _TUFlys)
	{
		dataSet->getObject(pair.first)->setTUFly(pair.second);
	}
	for (const auto& pair : _TUSlides)
	{
		dataSet->getObject(pair.first)->setTUSlide(pair.second);
	}
	for (const auto& pair : _deathTiles)
	{
		dataSet->getObject(pair.first)->setDieMCD(pair.second);
	}
	for (const auto& pair : _terrainHeight)
	{
		dataSet->getObject(pair.first)->setTerrainLevel(pair.second);
	}
	for (const auto& pair : _specialTypes)
	{
		dataSet->getObject(pair.first)->setSpecialType(pair.second, dataSet->getObject(pair.first)->getObjectType());
	}
	for (const auto& pair : _explosives)
	{
		dataSet->getObject(pair.first)->setExplosive(pair.second);
	}
	for (const auto& pair : _armors)
	{
		dataSet->getObject(pair.first)->setArmor(pair.second);
	}
	for (const auto& pair : _flammabilities)
	{
		dataSet->getObject(pair.first)->setFlammable(pair.second);
	}
	for (const auto& pair : _fuels)
	{
		dataSet->getObject(pair.first)->setFuel(pair.second);
	}
	for (const auto& pair : _HEBlocks)
	{
		dataSet->getObject(pair.first)->setHEBlock(pair.second);
	}
	for (const auto& pair : _footstepSounds)
	{
		dataSet->getObject(pair.first)->setFootstepSound(pair.second);
	}
	for (const auto& pair : _objectTypes)
	{
		dataSet->getObject(pair.first)->setObjectType((TilePart)pair.second);
	}
	for (const auto& pair : _noFloors)
	{
		dataSet->getObject(pair.first)->setNoFloor(pair.second);
	}
	for (const auto& pair : _stopLOSses)
	{
		dataSet->getObject(pair.first)->setStopLOS(pair.second);
	}
	for (const auto& pair : _LOFTS)
	{
		int layer = 0;
		for (int loft : pair.second)
		{
			dataSet->getObject(pair.first)->setLoftID(loft, layer);
			++layer;
		}
	}
}

}

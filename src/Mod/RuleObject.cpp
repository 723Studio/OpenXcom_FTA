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
#include "RuleObject.h"
#include "../Engine/RNG.h"

namespace OpenXcom
{

RuleObject::RuleObject(const std::string& type)
	: _type(type), _useType(BATTLE_OBJECT_HACKING_TERMINAL), _hackingDefence(0), _samplingDefence(0), _alterationMCDNumber(0), _alterationMCDRadius(0), _isMissionObjective(false)
{
}

RuleObject::~RuleObject()
{
	for (std::vector<std::pair<size_t, WeightedOptions*> >::iterator i = _eventWeights.begin(); i != _eventWeights.end(); ++i)
	{
		delete i->second;
	}
}

/**
	* Loads the event definition from YAML.
	* @param node YAML node.
	*/
void RuleObject::load(const YAML::YamlNodeReader& node)
{
	const auto& reader = node.useIndex();
	if (const YAML::YamlNodeReader& parent = reader["refNode"])
	{
		load(reader["refNode"]);
	}
	reader.tryRead("type", _type);
	reader.tryRead("useType", _useType);
	reader.tryRead("hackingDefence", _hackingDefence);
	reader.tryRead("samplingDefence", _samplingDefence);
	reader.tryRead("isMissionObjective", _isMissionObjective);
	reader.tryRead("spawnedEvents", _spawnedEvents);
	reader.tryRead("spawnedItem", _spawnedItem);
	reader.tryRead("alterationMCDNumber", _alterationMCDNumber);
	reader.tryRead("alterationMCDRadius", _alterationMCDRadius);
	if (reader["eventWeights"])
	{
		for (const auto& child : reader["eventWeights"].children())
		{
			WeightedOptions* nw = new WeightedOptions();
			nw->load(child);
			size_t key = 0;
			child.tryReadKey(key);
			_eventWeights.push_back(std::make_pair(key, nw));
		}
	}
}

std::string RuleObject::getWeightedEvent(const size_t monthsPassed) const
{
	if (_eventWeights.empty())
		return std::string();

	std::vector<std::pair<size_t, WeightedOptions*> >::const_reverse_iterator rw;
	rw = _eventWeights.rbegin();
	while (monthsPassed < rw->first)
		++rw;
	return rw->second->choose();
}

}

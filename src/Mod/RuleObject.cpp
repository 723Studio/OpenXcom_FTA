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
#include "../fmath.h"

namespace OpenXcom
{

RuleObject::RuleObject(const std::string& type) : _type(type), _hackingDefence(0), _alterationMCDNumber(0), _alterationMCDRadius(0)
{
}

/**
	* Loads the event definition from YAML.
	* @param node YAML node.
	*/
void RuleObject::load(const YAML::Node& node)
{
	if (const YAML::Node& parent = node["refNode"])
	{
		load(parent);
	}
	_type = node["type"].as<std::string>(_type);
	_hackingDefence = node["hackingDefence"].as<int>(_hackingDefence);

	_alterationMCDNumber= node["alterationMCDNumber"].as<int>(_alterationMCDNumber);
	_alterationMCDRadius= node["alterationMCDRadius"].as<int>(_alterationMCDRadius);
}

}

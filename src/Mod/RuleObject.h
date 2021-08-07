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
#include <string>
#include <map>
#include <vector>
#include <yaml-cpp/yaml.h>

namespace OpenXcom
{

/**
	* Represents a rules that are used to create BattleObject on battlescape.
	*/
class RuleObject
{
private:
	std::string _type;
	int _hackingDefence;

public:
	/// Creates a blank RuleObject.
	RuleObject(const std::string& name);
	/// Cleans up the RuleObject ruleset.
	~RuleObject() = default;
	/// Loads the RuleObject definition from YAML.
	void load(const YAML::Node& node);
	/// Gets the RuleObject's type.
	const std::string& getType() const { return _type; }
	/// Gets the RuleObject's hacking defence.
	int getHackingDefence() const { return _hackingDefence; };





};
}

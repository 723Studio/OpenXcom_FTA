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
#include <vector>
#include "../Engine/Yaml.h"
#include "../Savegame/WeightedOptions.h"

namespace OpenXcom
{
enum BattleObjectType : int
{
	BATTLE_OBJECT_HACKING_TERMINAL = 0,
	BATTLE_OBJECT_BIOLOGY_SAMPLING = 1,
	BATTLE_OBJECT_ANOMALY_SAMPLING = 2
};
/**
* Represents a rules that are used to create BattleObject on battlescape.
*/
class RuleObject
{
private:
	std::string _type;
	int _hackingDefence, _samplingDefence;
	int _alterationMCDNumber, _alterationMCDRadius;
	bool _isMissionObjective;
	BattleObjectType _useType;
	std::vector<std::string> _spawnedEvents;
	std::string _spawnedItem;
	std::vector<std::pair<size_t, WeightedOptions*> > _eventWeights;
public:
	/// Creates a blank RuleObject.
	RuleObject(const std::string& name);
	/// Cleans up the RuleObject ruleset.
	~RuleObject();
	/// Loads the RuleObject definition from YAML.
	void load(const YAML::YamlNodeReader& reader);
	/// Gets the RuleObject's name.
	const std::string& getName() const { return _type; }
	/// Gets the RuleObject's type.
	BattleObjectType getUseType() const { return _useType; }
	/// Gets the RuleObject's hacking defence.
	int getHackingDefence() const { return _hackingDefence; }
	/// Gets the RuleObject's hacking defence.
	int getSamplingDefence() const { return _samplingDefence; }
	/// Gets the MCD number for a tile to be altered (to ALT_MCD) once hacking performed.
	int getAlterationMCDNumber() const { return _alterationMCDNumber; }
	/// Gets the radius where hacking MCD alteration will check tiles.
	int getAlterationMCDRadius() const { return _alterationMCDRadius; }
	/// Gets if the BattleObject is a mission objective.
	bool isMissionObjective() const { return _isMissionObjective; }
	/// Gets the list of spawned event to choose once succesful BattleObject iteration.
	std::vector<std::string> getSpawnedEvents() const { return _spawnedEvents; }
	/// Generates an event based on the month.
	std::string getWeightedEvent(const size_t monthsPassed) const;
	/// Gets the item name to spawn succesful BattleObject iteration.
	std::string getSpawnedItem() const { return _spawnedItem; }
};
}

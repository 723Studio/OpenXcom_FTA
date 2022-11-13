#pragma once
/*
 * Copyright 2010-2022 OpenXcom Developers.
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
#include <yaml-cpp/yaml.h>
#include "RuleBaseFacilityFunctions.h"
#include "RuleResearch.h"
#include "../Mod/Unit.h"
#include "../Mod/Mod.h"

namespace OpenXcom
{

class Mod;
class RuleResearch;
class RuleIntelStage;

enum IntelProjectSpecialRule { INTEL_NONE = 0, INTEL_UFO_TRACKING = 1, INTEL_COVERT_OPERATIONS = 2, INTEL_DEPLOYMENT_HINTS = 3 };
/**
 * Represents rules for an intel project.
 */
class RuleIntelProject
{
 private:
	std::string _name, _description, _requiredResearchName;
	int _cost, _costIncrease;
	IntelProjectSpecialRule _specialRule;
	UnitStats _stats;
	const RuleResearch* _requiredResearch = nullptr;
	std::vector<RuleIntelStage*> _stages;
	int _listOrder;
public:
	RuleIntelProject(const std::string &name);

	/// Loads the research from YAML.
	void load(const YAML::Node& node, Mod* mod, int listOrder);
	/// Cross link with other rules.
	void afterLoad(const Mod* mod);
	/// Gets the intel project name.
	const std::string& getName() const { return _name; };
	/// Gets the intel project description.
	const std::string& getDescription() const { return _description; };
	/// Gets the base cost for making a roll for this project.
	int getCost() const { return _cost; }
	/// Gets the cost increase value to calculate next roll cost for this project.
	int getCostIncrease() const { return _costIncrease; }
	/// Gets the required tech for this project.
	const RuleResearch* getRequiredResearch() const { return _requiredResearch; }
	/// Gets the special rule for the intel project.
	const IntelProjectSpecialRule getSpecialRule() const { return _specialRule; }
	/// Get pointer to this project's stats.
	UnitStats getStats() const { return _stats; }
	/// Gets the list weight for this intel project.
	int getListOrder() const { return _listOrder; }
	/// Gets the list of the stages for this intel prject.
	std::vector<RuleIntelStage*> getStages() const { return _stages; }
};

/**
* Definition of the stage to Intel Project.
*/
class RuleIntelStage
{
private:
	std::string _stageName;
	int _odds, _requireRolls, _availableRolls;
	const RuleResearch* _requiredResearch = nullptr;
	const RuleResearch* _disabledByResearch = nullptr;
	RuleBaseFacilityFunctions _requiresBaseFunc;
	std::string _spawnMission, _requiredResearchName, _disabledByResearchName;
	std::vector<std::string> _eventScripts;
	bool _finalStage;
public:
	RuleIntelStage();
	void load(const YAML::Node& node, Mod* mod);
	void afterLoad(const Mod* mod);
	const std::string& getName() const { return _stageName; }
	const int getOdds() const { return _odds; }
	const int getRequireRolls() const { return _requireRolls; }
	const int getAvailableRolls() const { return _availableRolls; }
	const RuleResearch* getRequiredResearch() const { return _requiredResearch; }
	const RuleResearch* getDisabledByResearch() const { return _disabledByResearch; }
	RuleBaseFacilityFunctions getRequireBaseFunc() const { return _requiresBaseFunc; }
	const std::string& getSpawnedMission() const { return _spawnMission; }
	const std::vector<std::string>& getEventScripts() const { return _eventScripts; }
	const bool isFinalStage() const { return _finalStage; }
};

}

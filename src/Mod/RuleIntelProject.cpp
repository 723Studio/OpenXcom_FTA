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
#include <algorithm>
#include "RuleIntelProject.h"
#include "../Engine/Exception.h"
#include "../Engine/Collections.h"
#include "Mod.h"

namespace OpenXcom
{

RuleIntelProject::RuleIntelProject(const std::string &name) : _name(name), _cost(100), _costIncrease(0), _specialRule(INTEL_NONE), _listOrder(0)
{
}

/**
 * Loads the intel project from a YAML file.
 * @param node YAML node.
 * @param listOrder The list weight for this project.
 */
void RuleIntelProject::load(const YAML::Node &node, Mod* mod, int listOrder)
{
	if (const YAML::Node &parent = node["refNode"])
	{
		load(parent, mod, listOrder);
	}

	_name = node["name"].as<std::string>(_name);
	_description = node["description"].as<std::string>(_description);
	_cost = node["cost"].as<int>(_cost);
	_costIncrease = node["costIncrease"].as<int>(_costIncrease);
	_requiredResearchName = node["requiredResearch"].as<std::string>(_requiredResearchName);
	_specialRule = (IntelProjectSpecialRule)node["specialRule"].as<int>(_specialRule);

	if (const YAML::Node& stages = node["stages"])
	{
		for (YAML::const_iterator i = stages.begin(); i != stages.end(); ++i)
		{
			RuleIntelStage* stage = new RuleIntelStage();
			stage->load(*i, mod);
			_stages.push_back(stage);
		}
	}
	if (_stages.empty())
	{
		throw Exception("No stages defined for intelligence project " + _name);
	}
	_stats.merge(node["stats"].as<UnitStats>(_stats));
	_listOrder = node["listOrder"].as<int>(_listOrder);
	if (!_listOrder)
	{
		_listOrder = listOrder;
	}
}

/**
 * Cross link with other Rules.
 */
void RuleIntelProject::afterLoad(const Mod* mod)
{
	mod->linkRule(_requiredResearch, _requiredResearchName);
	Collections::removeAll(_requiredResearchName);

	for (auto i : _stages)
	{
		i->afterLoad(mod);
	}
}

/**
* Creates a stage to Intel Project.
*/
RuleIntelStage::RuleIntelStage() : _odds(100), _requireRolls(0), _availableRolls(1), _finalStage(false)
{ /*Empty by Design*/ };
	

/// Loads stats from YAML.
void RuleIntelStage::load(const YAML::Node& node, Mod* mod)
{
	_stageName = node["stageName"].as<std::string>(_stageName);
	_odds = node["odds"].as<int>(_odds);
	_requireRolls = node["requireRolls"].as<int>(_requireRolls);
	_availableRolls = node["availableRolls"].as<int>(_availableRolls);
	_eventScripts = node["eventScripts"].as<std::vector<std::string>>(_eventScripts);
	_spawnMission = node["spawnMission"].as<std::string>(_spawnMission);
	_requiredResearchName = node["requiredResearch"].as<std::string>(_requiredResearchName);
	_disabledByResearchName = node["disabledByResearch"].as<std::string>(_disabledByResearchName);
	mod->loadBaseFunction(_stageName, _requiresBaseFunc, node["requiresBaseFunc"]);
	_finalStage = node["finalStage"].as<bool>(_finalStage);
}

void RuleIntelStage::afterLoad(const Mod* mod)
{
	mod->linkRule(_requiredResearch, _requiredResearchName);
	mod->linkRule(_disabledByResearch, _disabledByResearchName);

	//remove not needed data
	Collections::removeAll(_requiredResearchName);
	Collections::removeAll(_disabledByResearchName);
}


}

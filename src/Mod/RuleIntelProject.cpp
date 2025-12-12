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
/**
 * Creates a new intel project with the given name.
 * @param name The name of the intel project.
 * @param listOrder The list weight for this project.
 */
RuleIntelProject::RuleIntelProject(const std::string &name, int listOrder) : _name(name), _cost(100), _costIncrease(0), _specialRule(INTEL_NONE), _listOrder(listOrder)
{
}

/**
 * Loads the intel project from a YAML file.
 * @param node YAML node.
 * @param listOrder The list weight for this project.
 */
void RuleIntelProject::load(const YAML::YamlNodeReader& node, Mod* mod)
{
	const auto& reader = node.useIndex();
	if (const YAML::YamlNodeReader& parent = reader["refNode"])
	{
		load(reader["refNode"], mod);
	}

	reader.tryRead("name", _name);
	reader.tryRead("description", _description);
	reader.tryRead("cost", _cost);
	reader.tryRead("costIncrease", _costIncrease);
	reader.tryRead("requiredResearch", _requiredResearchName);
	int specialRule = static_cast<int>(_specialRule);
	reader.tryRead("specialRule", specialRule);
	_specialRule = static_cast<IntelProjectSpecialRule>(specialRule);

	if (reader["stages"])
	{
		for (const auto& child : reader["stages"].children())
		{
			RuleIntelStage* stage = new RuleIntelStage();
			stage->load(child, mod);
			_stages.push_back(stage);
		}
	}
	if (_stages.empty())
	{
		throw Exception("No stages defined for intelligence project " + _name);
	}
	reader.tryRead("stats", _stats);
	reader.tryRead("listOrder", _listOrder);
}

/**
 * Cross link with other Rules.
 */
void RuleIntelProject::afterLoad(const Mod* mod)
{
	mod->linkRule(_requiredResearch, _requiredResearchName);
	Collections::removeAll(_requiredResearchName);

	if (mod->isFTAGame() && _stats.empty())
	{
		throw Exception("Stats are not defined for intelligence project, it is required for FTA game");
	}

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
void RuleIntelStage::load(const YAML::YamlNodeReader& reader, Mod* mod)
{
	reader.tryRead("stageName", _stageName);
	reader.tryRead("odds", _odds);
	reader.tryRead("requireRolls", _requireRolls);
	//reader.tryRead("availableRolls", _availableRolls);
	reader.tryRead("eventScripts", _eventScripts);
	reader.tryRead("spawnMission", _spawnMission);
	reader.tryRead("requiredResearch", _requiredResearchName);
	reader.tryRead("disabledByResearch", _disabledByResearchName);
	//mod->loadBaseFunction(_stageName, _requiresBaseFunc, reader["requiresBaseFunc"]);
	reader.tryRead("finalStage", _finalStage);
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

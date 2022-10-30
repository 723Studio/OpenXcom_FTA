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

#include "RulePrisoner.h"
#include "Mod.h"
#include "../Engine/Exception.h"

namespace OpenXcom
{

PrisonerInterrogationRules::PrisonerInterrogationRules() : _baseResistance(100), _diesAfter(false)
{
}

PrisonerInterrogationRules::~PrisonerInterrogationRules()
{
}

void PrisonerInterrogationRules::load(const YAML::Node& node)
{
	_requiredResearchName = node["requiredResearch"].as<std::string>(_requiredResearchName);
	_unlockResearchNames = node["unlockResearches"].as<std::vector<std::string>>(_unlockResearchNames);
	_baseResistance = node["baseResistance"].as<int>(_baseResistance);
	_diesAfter = node["diesAfter"].as<bool>(_diesAfter);
}

void PrisonerInterrogationRules::afterLoad(const Mod* mod)
{
	mod->linkRule(_requiredResearch, _requiredResearchName);
	for (auto ruleName : _unlockResearchNames)
	{
		const RuleResearch* rule = nullptr;
		mod->linkRule(rule, ruleName);
		_unlockResearches.push_back(rule);
	}
	
	//remove not needed data
	Collections::removeAll(_requiredResearchName);
	Collections::removeAll(_unlockResearchNames);
}

PrisonerRecruitingRules::PrisonerRecruitingRules() : _difficulty(100)
{
}

PrisonerRecruitingRules::~PrisonerRecruitingRules()
{
}

void PrisonerRecruitingRules::load(const YAML::Node& node)
{
	_requiredResearchName = node["requiredResearch"].as<std::string>(_requiredResearchName);
	_spawnedSoldierRuleName = node["spawnedSoldierRule"].as<std::string>(_spawnedSoldierRuleName);
	_difficulty = node["difficulty"].as<int>(_difficulty);
	_spawnEvent = node["spawnEvent"].as<std::string>(_spawnEvent);
}

void PrisonerRecruitingRules::afterLoad(const Mod* mod)
{
	mod->linkRule(_requiredResearch, _requiredResearchName);
	mod->linkRule(_spawnedSoldierRule, _spawnedSoldierRuleName);

	//remove not needed data
	Collections::removeAll(_requiredResearchName);
	Collections::removeAll(_spawnedSoldierRuleName);
}

PrisonerTortureRules::PrisonerTortureRules() : _difficulty(100), _loyaltyChange(0), _moraleChange(-5), _cooperationChange(-30), _eventChance(100)
{
}

PrisonerTortureRules::~PrisonerTortureRules()
{
}

void PrisonerTortureRules::load(const YAML::Node& node)
{
	_difficulty = node["difficulty"].as<int>(_difficulty);
	_loyaltyChange = node["loyaltyChange"].as<int>(_loyaltyChange);
	_moraleChange = node["moraleChange"].as<int>(_moraleChange);
	_cooperationChange = node["cooperationChange"].as<int>(_cooperationChange);
	_eventChance = node["eventChance"].as<int>(_eventChance);
	_spawnEvent = node["spawnEvent"].as<std::string>(_spawnEvent);
}

PrisonerContainingRules::PrisonerContainingRules() : _funds(-100), _cooperationChange(0)
{
}

PrisonerContainingRules::~PrisonerContainingRules()
{
}

void PrisonerContainingRules::load(const YAML::Node& node)
{
	_requiredResearchName = node["requiredResearch"].as<std::string>(_requiredResearchName);
	_funds = node["funds"].as<int>(_funds);
	_cooperationChange = node["cooperationChange"].as<int>(_cooperationChange);
}

void PrisonerContainingRules::afterLoad(const Mod* mod)
{
	mod->linkRule(_requiredResearch, _requiredResearchName);

	//remove not needed data
	Collections::removeAll(_requiredResearchName);
}

/**
 * Creates a blank RulePrisoner.
 * @param type String defining the type.
 */
RulePrisoner::RulePrisoner(const std::string &type) : _type(type), _startingCooperation(-300), _damageOverTime(0)
{

}

RulePrisoner::~RulePrisoner()
{
}

/**
 * Loads the event definition from YAML.
 * @param node YAML node.
 */
void RulePrisoner::load(const YAML::Node &node)
{
	if (const YAML::Node &parent = node["refNode"])
	{
		load(parent);
	}

	_startingCooperation = node["startingCooperation"].as<int>(_startingCooperation);
	_damageOverTime = node["damageOverTime"].as<int>(_damageOverTime);
	
	if (const YAML::Node& yml = node["interrogation"])
	{
		PrisonerInterrogationRules *rules = new PrisonerInterrogationRules();
		rules->load(yml);
		_interrogationRules = rules;
	}
	if (const YAML::Node& yml = node["recruiting"])
	{
		PrisonerRecruitingRules* rules = new PrisonerRecruitingRules();
		rules->load(yml);
		_recruitingRules = rules;
	}
	if (const YAML::Node& yml = node["torture"])
	{
		PrisonerTortureRules* rules = new PrisonerTortureRules();
		rules->load(yml);
		_tortureRules = rules;
	}
	if (const YAML::Node& yml = node["contain"])
	{
		PrisonerContainingRules* rules = new PrisonerContainingRules();
		rules->load(yml);
		_containingRules = rules;
	}
}

void RulePrisoner::afterLoad(const Mod* mod)
{
	_interrogationRules->afterLoad(mod);
	_recruitingRules->afterLoad(mod);
	_containingRules->afterLoad(mod);
}

}

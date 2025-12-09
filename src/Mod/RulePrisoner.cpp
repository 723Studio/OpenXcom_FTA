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

namespace OpenXcom
{

PrisonerInterrogationRules::PrisonerInterrogationRules() : _baseResistance(100), _diesAfter(false)
{
}

PrisonerInterrogationRules::~PrisonerInterrogationRules()
{
}

void PrisonerInterrogationRules::load(const YAML::YamlNodeReader& reader)
{
	reader.tryRead("requiredResearch", _requiredResearchName);
	reader.tryRead("unlockResearches", _unlockResearchNames);
	reader.tryRead("baseResistance", _baseResistance);
	reader.tryRead("diesAfter", _diesAfter);
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

PrisonerRecruitingRules::PrisonerRecruitingRules() : _difficulty(100), _eventChance(0)
{
}

PrisonerRecruitingRules::~PrisonerRecruitingRules()
{
}

void PrisonerRecruitingRules::load(const YAML::YamlNodeReader& reader)
{
	reader.tryRead("requiredResearch", _requiredResearchName);
	reader.tryRead("spawnedSoldierRule", _spawnedSoldierRuleName);
	reader.tryRead("difficulty", _difficulty);
	reader.tryRead("eventChance", _eventChance);
	reader.tryRead("spawnEvents", _spawnEvents);
}

void PrisonerRecruitingRules::afterLoad(const Mod* mod)
{
	mod->linkRule(_requiredResearch, _requiredResearchName);

	//remove not needed data
	Collections::removeAll(_requiredResearchName);
	Collections::removeAll(_spawnedSoldierRuleName);
}

PrisonerTortureRules::PrisonerTortureRules() : _difficulty(100), _loyaltyChange(0), _moraleChange(-5), _cooperationChange(-30),
	_eventChance(100), _isMultipleEventsPossible(false)
{
}

PrisonerTortureRules::~PrisonerTortureRules()
{
	//for (std::vector<std::pair<size_t, WeightedOptions*> >::iterator i = _eventWeights.begin(); i != _eventWeights.end(); ++i)
	//{
	//	delete i->second;
	//}
}

void PrisonerTortureRules::load(const YAML::YamlNodeReader& reader)
{
	reader.tryRead("difficulty", _difficulty);
	reader.tryRead("loyaltyChange", _loyaltyChange);
	reader.tryRead("moraleChange", _moraleChange);
	reader.tryRead("cooperationChange", _cooperationChange);
	reader.tryRead("eventChance", _eventChance);
	reader.tryRead("isMultipleEventsPossible", _isMultipleEventsPossible);
	reader.tryRead("spawnEvents", _spawnEvents);
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

std::string PrisonerTortureRules::getWeightedEvent(const size_t monthsPassed) const
{
	if (_eventWeights.empty())
		return std::string();

	std::vector<std::pair<size_t, WeightedOptions*> >::const_reverse_iterator rw;
	rw = _eventWeights.rbegin();
	while (monthsPassed < rw->first)
		++rw;
	return rw->second->choose();
}

PrisonerContainingRules::PrisonerContainingRules() : _funds(-100), _cooperationChange(0)
{
}

PrisonerContainingRules::~PrisonerContainingRules()
{
}

void PrisonerContainingRules::load(const YAML::YamlNodeReader& reader)
{
	reader.tryRead("requiredResearch", _requiredResearchName);
	reader.tryRead("funds", _funds);
	reader.tryRead("cooperationChange", _cooperationChange);
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
 * @param reader YAML reader.
 */
void RulePrisoner::load(const YAML::YamlNodeReader& node)
{
	const auto& reader = node.useIndex();
	if (const YAML::YamlNodeReader& parent = reader["refNode"])
	{
		load(reader["refNode"]);
	}

	reader.tryRead("startingCooperation", _startingCooperation);
	reader.tryRead("damageOverTime", _damageOverTime);
	
	if (reader["interrogation"])
	{
		PrisonerInterrogationRules *rules = new PrisonerInterrogationRules();
		rules->load(reader["interrogation"]);
		_interrogationRules = rules;
	}
	if (reader["recruiting"])
	{
		PrisonerRecruitingRules* rules = new PrisonerRecruitingRules();
		rules->load(reader["recruiting"]);
		_recruitingRules = rules;
	}
	if (reader["torture"])
	{
		PrisonerTortureRules* rules = new PrisonerTortureRules();
		rules->load(reader["torture"]);
		_tortureRules = rules;
	}
	if (reader["contain"])
	{
		PrisonerContainingRules* rules = new PrisonerContainingRules();
		rules->load(reader["contain"]);
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

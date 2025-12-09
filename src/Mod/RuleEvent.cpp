/*
 * Copyright 2010-2019 OpenXcom Developers.
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
#include "RuleEvent.h"
#include "Mod.h"
#include "../Engine/Exception.h"

namespace OpenXcom
{

RuleEvent::RuleEvent(const std::string &name) :
	_name(name), _background("BACK13.SCR"), _alignBottom(false),
	_city(false), _points(0), _funds(0), _loyalty(0), _spawnedPersons(0), _timer(30), _timerRandom(0), _invert(false)
{
}

/**
 * Loads the event definition from YAML.
 * @param node YAML node.
 */
void RuleEvent::load(const YAML::YamlNodeReader& node, Mod* mod)
{
	const auto& reader = node.useIndex();
	if (const auto& parent = reader["refNode"])
	{
		load(parent, mod);
	}

	reader.tryRead("description", _description);
	reader.tryRead("alignBottom", _alignBottom);
	reader.tryRead("background", _background);
	reader.tryRead("music", _music);
	reader.tryRead("cutscene", _cutscene);
	reader.tryRead("regionList", _regionList);
	reader.tryRead("city", _city);
	reader.tryRead("points", _points);
	reader.tryRead("funds", _funds);
	reader.tryRead("loyalty", _loyalty);
	reader.tryRead("spawnedCraftType", _spawnedCraftType);
	reader.tryRead("spawnedPersons", _spawnedPersons);
	reader.tryRead("spawnedPersonType", _spawnedPersonType);
	reader.tryRead("spawnedPersonName", _spawnedPersonName);
	reader.tryRead("removedCovertOperationsList", _removedCovertOperationsList);
	if (reader["spawnedSoldier"])
	{
		_spawnedSoldier = reader["spawnedSoldier"].emitDescendants(YAML::YamlRootNodeReader(_spawnedSoldier, "(spawned soldier template)"));
	}
	reader.tryRead("reputationScore", _reputationScore);
	reader.tryRead("everyMultiItemList", _everyMultiItemList);
	reader.tryRead("everyItemList", _everyItemList);
	reader.tryRead("randomItemList", _randomItemList);
	reader.tryRead("randomMultiItemList", _randomMultiItemList);
	if (reader["weightedItemList"])
	{
		_weightedItemList.load(reader["weightedItemList"]);
	}
	reader.tryRead("researchList", _researchNames);
	reader.tryRead("adhocMissionScriptTags", _adhocMissionScriptTags);
	reader.tryRead("interruptResearch", _interruptResearch);
	mod->loadUnorderedNames(_name, _decreaseCounter, reader["decreaseCounter"]);
	mod->loadUnorderedNames(_name, _increaseCounter, reader["increaseCounter"]);
	reader.tryRead("counterValue", _counterValue);
	reader.tryRead("timer", _timer);
	reader.tryRead("timerRandom", _timerRandom);
	reader.tryRead("invert", _invert);

	if (reader["customAnswers"])
	{
		const auto& customAnswers = reader["customAnswers"];
		if (customAnswers.childrenCount() > 4)
		{
			throw Exception("Geoscape Event named: '" + this->getName() + "' has more than 4 custom answers, this is not allowed!");
		}
		if (customAnswers.childrenCount() < 2)
		{
			throw Exception("Geoscape Event named: '" + this->getName() + "' has less than 2 custom answers, this is not allowed!");
		}
		for (const auto& child : customAnswers.children())
		{
			int answerIndex = child.readKey<int>();
			_answers[answerIndex].load(child);
		}
	}

	reader.tryRead("everyMultiSoldierList", _everyMultiSoldierList);
	reader.tryRead("randomMultiSoldierList", _randomMultiSoldierList);
}

/**
 * Cross link with other Rules.
 */
void RuleEvent::afterLoad(const Mod* mod)
{
	mod->linkRule(_research, _researchNames);
}

}

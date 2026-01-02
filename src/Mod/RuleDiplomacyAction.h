#pragma once
/*
 * Copyright 2010-2026 OpenXcom Developers.
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
#include <optional>
#include "../Engine/Yaml.h"

namespace OpenXcom
{

class Mod;
class RuleAlienMission;
class RuleDiplomacyFaction;
class RuleEventScript;
class RuleItem;
class RuleResearch;

class RuleDiplomacyAction
{
public:
	enum SpecialNavigation
	{
		NAV_NONE,
		NAV_EXIT,
		NAV_HIRE_SOLDIERS
	};

	struct Conditions
	{
		std::map<const RuleDiplomacyFaction*, int> reputationLevelMin;
		std::map<const RuleDiplomacyFaction*, int> reputationLevelMax;
		std::optional<int> loyaltyMin;
		std::optional<int> loyaltyMax;
		std::optional<int64_t> fundsMin;
		std::optional<int64_t> fundsMax;
		std::map<const RuleResearch*, bool> researchTriggers;
		std::map<const RuleItem*, bool> itemTriggers;
		std::map<std::string, bool> treaties;
		std::optional<int> timerCooldown;
	};

	struct Effects
	{
		std::string responseText;
		std::map<const RuleDiplomacyFaction*, int> reputationChanges;
		const RuleEventScript* eventScript = nullptr;
		std::optional<int64_t> funds;
		std::optional<int> loyalty;
		const RuleAlienMission* spawnMission = nullptr;
		std::map<std::string, bool> changeTreaty;
		SpecialNavigation specialNavigation = NAV_NONE;
	};

private:
	std::string _type;
	int _listOrder = 0;
	std::string _factionName;
	const RuleDiplomacyFaction* _faction = nullptr;
	std::string _text;
	std::string _tooltip;
	Conditions _conditions;
	Effects _effects;

	std::map<std::string, int> _reputationLevelMinNames;
	typedef std::map<std::string, int> StringIntMap;
	StringIntMap _reputationLevelMaxNames;
	std::map<std::string, bool> _researchTriggerNames;
	std::map<std::string, bool> _itemTriggerNames;
	StringIntMap _reputationChangeNames;
	std::string _eventScriptName;
	std::string _spawnMissionName;

public:
	RuleDiplomacyAction(const std::string& type);
	~RuleDiplomacyAction() = default;

	void load(const YAML::YamlNodeReader& reader);
	void afterLoad(const Mod* mod);

	const std::string& getType() const { return _type; }
	int getListOrder() const { return _listOrder; }
	const RuleDiplomacyFaction* getFactionRules() const { return _faction; }
	const std::string& getEventScriptName() const { return _eventScriptName; }
	const std::string& getSpawnMissionName() const { return _spawnMissionName; }
	const std::string& getText() const { return _text; }
	const std::string& getTooltip() const { return _tooltip; }
	const Conditions& getConditions() const { return _conditions; }
	const Effects& getEffects() const { return _effects; }
};

}

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
#include "RuleDiplomacyAction.h"
#include "Mod.h"


namespace OpenXcom
{

namespace
{
	static void readFactionLevelMap(const YAML::YamlNodeReader& node, std::map<std::string, int>& out)
	{
		if (!node)
		{
			return;
		}

		if (node.isMap())
		{
			for (const auto& child : node.children())
			{
				std::string key;
				int value = 0;
				if (child.tryReadKey(key) && child.tryReadVal(value))
				{
					out[key] = value;
				}
			}
			return;
		}

		if (node.isSeq())
		{
			for (const auto& entry : node.children())
			{
				if (!entry.isMap())
				{
					continue;
				}
				for (const auto& child : entry.children())
				{
					std::string key;
					int value = 0;
					if (child.tryReadKey(key) && child.tryReadVal(value))
					{
						out[key] = value;
					}
				}
			}
		}
	}

	static RuleDiplomacyAction::SpecialNavigation parseNavigation(const std::string& s)
	{
		if (s == "EXIT")
		{
			return RuleDiplomacyAction::NAV_EXIT;
		}
		if (s == "HIRE_SOLDIERS")
		{
			return RuleDiplomacyAction::NAV_HIRE_SOLDIERS;
		}
		return RuleDiplomacyAction::NAV_NONE;
	}

} // namespace


RuleDiplomacyAction::RuleDiplomacyAction(const std::string& type)
	: _type(type)
{
}

void RuleDiplomacyAction::load(const YAML::YamlNodeReader& node)
{
	const auto& reader = node.useIndex();
	if (const YAML::YamlNodeReader& parent = reader["refNode"])
	{
		load(reader["refNode"]);
	}

	reader.tryRead("type", _type);
	reader.tryRead("listOrder", _listOrder);
	reader.tryRead("faction", _factionName);
	reader.tryRead("text", _text);
	reader.tryRead("tooltip", _tooltip);

	if (const YAML::YamlNodeReader& conditions = reader["conditions"])
	{
		const auto& c = conditions.useIndex();

		readFactionLevelMap(c["reputationLevelMin"], _reputationLevelMinNames);
		readFactionLevelMap(c["reputationLevelMax"], _reputationLevelMaxNames);

		int loyaltyMin = 0;
		if (c.tryRead("loyaltyMin", loyaltyMin))
		{
			_conditions.loyaltyMin = loyaltyMin;
		}
		int loyaltyMax = 0;
		if (c.tryRead("loyaltyMax", loyaltyMax))
		{
			_conditions.loyaltyMax = loyaltyMax;
		}
		int64_t fundsMin = 0;
		if (c.tryRead("fundsMin", fundsMin))
		{
			_conditions.fundsMin = fundsMin;
		}
		int64_t fundsMax = 0;
		if (c.tryRead("fundsMax", fundsMax))
		{
			_conditions.fundsMax = fundsMax;
		}

		c.tryRead("researchTriggers", _researchTriggerNames);
		c.tryRead("itemTriggers", _itemTriggerNames);
		c.tryRead("treaties", _conditions.treaties);

		int timerCooldown = 0;
		if (c.tryRead("timerCooldown", timerCooldown))
		{
			_conditions.timerCooldown = timerCooldown;
		}
	}

	if (const YAML::YamlNodeReader& effects = reader["effects"])
	{
		const auto& e = effects.useIndex();
		e.tryRead("responseText", _effects.responseText);
		e.tryRead("reputationChanges", _reputationChangeNames);
		e.tryRead("eventScript", _eventScriptName);

		int64_t funds = 0;
		if (e.tryRead("funds", funds))
		{
			_effects.funds = funds;
		}

		int loyalty = 0;
		if (e.tryRead("loyalty", loyalty))
		{
			_effects.loyalty = loyalty;
		}

		e.tryRead("spawnMission", _spawnMissionName);
		e.tryRead("changeTreaty", _effects.changeTreaty);

		std::string nav;
		if (e.tryRead("specialNavigation", nav))
		{
			_effects.specialNavigation = parseNavigation(nav);
		}
	}
}

void RuleDiplomacyAction::afterLoad(const Mod* mod)
{
	if (!mod)
	{
		return;
	}

	_faction = mod->getDiplomacyFaction(_factionName, true);

	_conditions.reputationLevelMin.clear();
	_conditions.reputationLevelMax.clear();
	_conditions.researchTriggers.clear();
	_conditions.itemTriggers.clear();
	_effects.reputationChanges.clear();
	_effects.eventScript = nullptr;
	_effects.spawnMission = nullptr;

	for (const auto& [factionName, lvl] : _reputationLevelMinNames)
	{
		_conditions.reputationLevelMin[mod->getDiplomacyFaction(factionName, true)] = lvl;
	}
	for (const auto& [factionName, lvl] : _reputationLevelMaxNames)
	{
		_conditions.reputationLevelMax[mod->getDiplomacyFaction(factionName, true)] = lvl;
	}
	for (const auto& [researchName, mustHave] : _researchTriggerNames)
	{
		_conditions.researchTriggers[mod->getResearch(researchName, true)] = mustHave;
	}
	for (const auto& [itemName, mustHave] : _itemTriggerNames)
	{
		_conditions.itemTriggers[mod->getItem(itemName, true)] = mustHave;
	}

	for (const auto& [factionName, delta] : _reputationChangeNames)
	{
		_effects.reputationChanges[mod->getDiplomacyFaction(factionName, true)] = delta;
	}

	if (!Mod::isEmptyRuleName(_eventScriptName))
	{
		_effects.eventScript = mod->getEventScript(_eventScriptName, true);
	}
	if (!Mod::isEmptyRuleName(_spawnMissionName))
	{
		_effects.spawnMission = mod->getAlienMission(_spawnMissionName, true);
	}
}

}

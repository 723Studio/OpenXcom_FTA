/*
 * Copyright 2010-2016 OpenXcom Developers.
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
#include "BasePrisoner.h"
#include "Base.h"
#include "SavedGame.h"
#include "../Mod/Mod.h"
#include "../Mod/Armor.h"
#include "../Mod/RulePrisoner.h"
#include "../Engine/Game.h"
#include "../Engine/RNG.h"
#include "../FTA/MasterMind.h"
#include "../Geoscape/PrisonReportState.h"
#include <sstream>
#include <algorithm>

namespace OpenXcom
{
/**
 * Initializes a BattleUnit from a Soldier
 * @param mod Mod
 * @param type prisoner type
 * @param id prisoner id
 */
BasePrisoner::BasePrisoner(const RulePrisoner* rule, Base* base, const std::string &type, std::string id) :
	_rule(rule),_id(std::move(id)), _type(type), _state(PRISONER_STATE_NONE), _soldierId(-1),
	_health(1), _intelligence(0), _aggression(0), _morale(100), _cooperation(0), _interrogationProgress(0), _recruitingProgress(0), _base(base)
{
}

void BasePrisoner::loadRoles(const std::vector<int>& r)
{
	_roles.clear();
	for (auto i : r)
	{
		SoldierRole role = static_cast<SoldierRole>(i);
		if (_roles.empty() || std::find(_roles.begin(), _roles.end(), role) == _roles.end())
		{
			_roles.push_back(role);
		}
	}
}

///**
// * Loads the unit from a YAML file.
// * @param node YAML node.
// */
void BasePrisoner::load(const YAML::Node& node, const Mod* mod)
{
	_name = node["name"].as<std::string>(_name);
	_soldierId = node["soldierId"].as<int>(_soldierId);
	_state = (PrisonerState)node["state"].as<int>(_state);
	if (node["roles"])
		loadRoles(node["roles"].as<std::vector<int> >());
	_health = node["health"].as<int>(_health);
	_faction = (UnitFaction)node["faction"].as<int>(_faction);
	_stats = node["stats"].as<UnitStats>(_stats);
	_intelligence = node["intelligence"].as<int>(_intelligence);
	_aggression = node["aggression"].as<int>(_aggression);
	_morale = node["morale"].as<int>(_morale);
	_cooperation = node["cooperation"].as<int>(_cooperation);
	_spawnedTortureEvent = node["spawnedTortureEvent"].as<bool>(_spawnedTortureEvent);
	_interrogationProgress = node["interrogationProgress"].as<int>(_interrogationProgress);
	_recruitingProgress = node["recruitingProgress"].as<int>(_recruitingProgress);
	if (node["armor"])
	{
		std::string armor = node["armor"].as<std::string>();
		_armor = mod->getArmor(armor);
	}
	//in case
	if (!_armor)
	{
		_armor = mod->getArmor(mod->getArmorsList().at(0));
		Log(LOG_ERROR) << "Base Prisoner named: " << _name << " fails to load correct ruleset for armor, default armor type: " << _armor->getType() << " was assigned. Please, report this case!";
	}


}

///**
// * Saves the soldier to a YAML file.
// * @return YAML node.
// */
YAML::Node BasePrisoner::save() const
{
	YAML::Node node;

	node["id"] = _id;
	node["type"] = _type;
	node["name"] = _name;
	node["state"] = (int)_state;
	{
		std::vector<int> roles;
		for (auto r : _roles)
		{
			roles.push_back(r);
		}
		node["roles"] = roles;
	}
	if (_geoscapeSoldier)
	{
		node["soldierId"] = _geoscapeSoldier->getId();
	}
	else
	{
		node["soldierId"] = -1;
	}
	if (_spawnedTortureEvent)
		node["spawnedTortureEvent"] = _spawnedTortureEvent;
	node["health"] = _health;
	node["faction"] = (int)_faction;
	node["stats"] = _stats;
	node["intelligence"] = _intelligence;
	node["aggression"] = _aggression;
	node["morale"] = _morale;
	node["cooperation"] = _cooperation;
	node["interrogationProgress"] = _interrogationProgress;
	node["recruitingProgress"] = _recruitingProgress;
	node["armor"] = _armor->getType();

	return node;
}

void BasePrisoner::setMorale(int morale)
{
	if (morale < 1)
		morale = 1;
	else if (morale > 100)
		morale = 100;

	_morale = morale;
	
}

/**
 * Geoscape logic 
 * @param engine - game pointer
 */
bool BasePrisoner::think(Game &engine)
{
	const Mod& mod = *engine.getMod();
	SavedGame& save = *engine.getSavedGame();
	bool result = false;

	//populate data
	PrisonerState prisonerState = getPrisonerState();
	_agents.clear();
	for (auto s: *_base->getSoldiers())
	{
		if (s->getActivePrisoner() == this)
			_agents.push_back(s);
	}

	//first, let's process physical conditions first
	if (!save.isResearched(_rule->getContainingRules().getReuiredResearch()))
	{
		setHealth(getHealth() - RNG::generate(0, _rule->getDamageOverTime()));
	}

	if (getHealth() <= 0) //prisoner dies
	{
		die();
		engine.pushState(new PrisonReportState(this, _base));
	}
	else
	{
		//process different stats
		if (prisonerState == PRISONER_STATE_INTERROGATION)
		{
			auto rules = _rule->getInterrogationRules();
			int breakpoint = rules.getBaseResistance() + getMorale() / 2 + getAggression() * 5 + getIntelligence() * 5;
			int progress = 0;
			double effort = 0;
			int factor = mod.getIntelTrainingFactor();
			for (auto s : _agents)
			{
				auto stats = s->getStatsWithAllBonuses();
				auto caps = s->getRules()->getStatCaps();
				double soldierEffort = 0, statEffort = 0;
				int interrogationCoef = 10;
				int charismaCoef = 20;
				int deceptionCoef = 40;
				int psiCoef = 5;
				statEffort = stats->interrogation;
				soldierEffort += (statEffort / interrogationCoef);
				if (stats->interrogation < caps.interrogation
					&& RNG::generate(0, caps.interrogation) > stats->interrogation
					&& RNG::percent(factor))
				{
					s->getIntelExperience()->interrogation++;
				}

				statEffort = stats->charisma;
				soldierEffort += (statEffort / charismaCoef);
				if (stats->charisma < caps.charisma
					&& RNG::generate(0, caps.charisma) > stats->charisma
					&& RNG::percent(factor))
				{
					s->getIntelExperience()->charisma++;
				}

				statEffort = stats->deception;
				soldierEffort += (statEffort / deceptionCoef);
				if (stats->deception < caps.deception
					&& RNG::generate(0, caps.deception) > stats->deception
					&& RNG::percent(factor))
				{
					s->getIntelExperience()->deception++;
				}

				//extra handle for psi
				if (stats->psiSkill > 0)
				{
					statEffort = stats->psiSkill;
					statEffort += stats->psiStrength;
					if (RNG::percent(factor / 2))
					{
						s->getIntelExperience()->psiSkill++;
					}
				}
				else
				{
					statEffort = stats->psiStrength;
					soldierEffort += (statEffort / psiCoef);
				}

				soldierEffort /= 4;

				double insightBonus = RNG::generate(0, stats->insight);
				soldierEffort += insightBonus / 20;

				effort += soldierEffort;
			}
			// If one woman can carry a baby in nine months, nine women can't do it in a month...
			if (_agents.size() > 1)
			{
				effort *= (100 - (25 * log(_agents.size()))) / 100;
			}

			effort *= (double)engine.getMasterMind()->getLoyaltyPerformanceBonus() / 100;
			progress = static_cast<int>(effort);
			_interrogationProgress += progress;
			if (_interrogationProgress >= breakpoint)
			{
				result = true;
				_interrogationProgress = 0;
				// give research if any
				std::string researchName = "";
				std::string bonusResearchName = "";
				if (!rules.getUnlockedResearches().empty())
				{

					std::vector<const RuleResearch*> possibilities;

					engine.getMasterMind()->helpResearchDiscovery(rules.getUnlockedResearches(), possibilities, _base, researchName, bonusResearchName);

					bool removeAgents = false;
					if (rules.isDiesAfterInterrogation()) //prisoner dies
					{
						removeAgents = true;
						_base->removePrisoner(this);
					}
					else if (possibilities.empty()) //there is no point interrogating further
					{
						removeAgents = true;
						if (save.isResearched(_rule->getContainingRules().getReuiredResearch()))
						{
							setPrisonerState(PRISONER_STATE_CONTAINING);
						}
						else
						{
							setPrisonerState(PRISONER_STATE_NONE);
						}
					}

					if (removeAgents)
					{
						for (auto s : _agents)
						{
							s->setActivePrisoner(0);
						}
					}
				}
				RuleResearch* research;
				RuleResearch* bonus;
				if (!researchName.empty())
					research = mod.getResearch(researchName);
				else
					research = nullptr;

				if (!bonusResearchName.empty())
					bonus = mod.getResearch(bonusResearchName);
				else
					bonus = nullptr;

				engine.pushState(new PrisonReportState(research, bonus, this, _base));
			}
		}
		else if (prisonerState == PRISONER_STATE_TORTURE)
		{
			auto rules = _rule->getTortureRules();
			// let's calculate power of our team
			int psionics = 0, torturePower = 0;
			for (auto agent: _agents)
			{
				torturePower += agent->getStatsWithAllBonuses()->bravery;
				if (agent->getStatsWithAllBonuses()->psiSkill > 0)
				{
					if (agent->getStatsWithAllBonuses()->psiStrength > 50)
						psionics += 2;
					else
						psionics++;
				}
			}
			
			torturePower *= psionics + 1;
			if (torturePower > 0)
			{
				if (RNG::percent(-10 * save.getDifficultyCoefficient() + 80))
				{
					int difficultyRoll = RNG::generate(rules.getDifficulty() / 2, rules.getDifficulty() * 2);
					//calculate and apply torture effects
					
					int maxDmg = 4 + floor(save.getDifficultyCoefficient() / 2);
					int loyaty = rules.getLoyalty() * (1 + floor(save.getDifficultyCoefficient() / 2));
					int moraleDmg = rules.getMorale();
					int eventChance = rules.getEventChance();
					if (difficultyRoll > torturePower * 2) // min torture
					{
						moraleDmg = 0;
						maxDmg = ceil(maxDmg / 2);
						loyaty = ceil(loyaty / 5);
						eventChance = ceil(eventChance / 3);
					}
					else if (difficultyRoll > torturePower)
					{
						maxDmg = ceil(maxDmg / 2);
						moraleDmg = ceil(moraleDmg / 3);
						loyaty = ceil(loyaty / 4);
						eventChance = ceil(eventChance / 2);
					}
					setHealth(getHealth() - RNG::generate(0, maxDmg));
					setMorale(getMorale() - moraleDmg);
					setCooperation(getCooperation() - rules.getCooperation());
					engine.getMasterMind()->updateLoyalty(loyaty);

					if (_spawnedTortureEvent && !rules.isMultipleEventsPossible())
					{
						//no event, sorry.
					}
					else if (RNG::percent(eventChance))
					{
						auto events = rules.getSpawnedEvents();
						events.push_back(rules.getWeightedEvent(save.getMonthsPassed()));
						if (!events.empty())
						{
							save.spawnEvent(events, &mod);
						}
					}
				}
			}
		}
		else if (prisonerState == PRISONER_STATE_REQRUITING)
		{
			auto rules = _rule->getRecruitingRules();
			int breakpoint = rules.getDifficulty() - getCooperation() + (100 - getMorale());
			int progress = 0;
			double effort = 0;
			int factor = mod.getIntelTrainingFactor();
			for (auto s : _agents)
			{
				double soldierEffort = 0, statEffort = 0;
				auto stats = s->getStatsWithAllBonuses();
				auto caps = s->getRules()->getStatCaps();

				statEffort = stats->charisma;
				soldierEffort += (statEffort);
				if (stats->charisma < caps.charisma
					&& RNG::generate(0, caps.charisma) > stats->charisma
					&& RNG::percent(factor))
				{
					s->getIntelExperience()->charisma++;
				}

				statEffort = stats->deception;
				soldierEffort += (statEffort);
				if (stats->deception < caps.deception
					&& RNG::generate(0, caps.deception) > stats->deception
					&& RNG::percent(factor))
				{
					s->getIntelExperience()->deception++;
				}

				soldierEffort /= 2;
				effort += soldierEffort;
			}

			if (_agents.size() > 1)
			{
				effort *= (100 - (25 * log(_agents.size()))) / 100;
			}

			effort *= (double)engine.getMasterMind()->getLoyaltyPerformanceBonus() / 100;
			progress = static_cast<int>(effort);
			_recruitingProgress += progress;
			if (_recruitingProgress >= breakpoint)
			{
				result = true;
				_recruitingProgress = 0;
				auto events = rules.getSpawnedEvents();
				if (!events.empty() && RNG::percent(rules.getEventChance()))
				{
					save.spawnEvent(events, &mod);
				}

				const RuleSoldier *soldierRule = mod.getSoldier(rules.getSpawnedSoldier());
				
				if (_geoscapeSoldier != nullptr)
				{
					_base->getSoldiers()->push_back(_geoscapeSoldier);
					_geoscapeSoldier->setImprisoned(false);
					engine.pushState(new PrisonReportState(_geoscapeSoldier, this, _base));
				}
				else if (soldierRule != nullptr) // we now create a new soldier from prisoner
				{
					Soldier* soldier = new Soldier(soldierRule, _armor, save.getId("STR_SOLDIER"));
					soldier->setBothStats(&_stats);
					for (auto r : _roles)
					{
						soldier->addRole(r);
					}
					_base->getSoldiers()->push_back(soldier);
					engine.pushState(new PrisonReportState(soldier, this, _base));
				}

				die();
			}
		}
		else if (prisonerState == PRISONER_STATE_CONTAINING)
		{
			auto rules = _rule->getInterrogationRules();
			int effort = 0;
			for (auto s : _agents)
			{
				int statEffort = 0;
				auto stats = s->getStatsWithAllBonuses();

				statEffort = stats->charisma;

				if (stats->psiSkill > 0)
				{
					statEffort += ceil(stats->psiSkill * 0.5);
				}

				effort += statEffort;
			}

			int moraleRegen = RNG::generate(-5, effort / 10 + 3);
			int hpRegen = RNG::generate(0, 2);
			if (effort > 20)
			{
				hpRegen += RNG::generate(0, 1);
			}
			int coopChange = RNG::generate(-2, effort / 10 + 1);
			setMorale(getMorale() + moraleRegen);
			setHealth(getHealth() + hpRegen);
			setCooperation(getCooperation() + coopChange);
		}

		//almost done...
		if (_interrogationProgress > 0 && prisonerState != PRISONER_STATE_INTERROGATION)
		{
			_interrogationProgress -= floor(_interrogationProgress * 0.1);
		}

		if (_recruitingProgress > 0 && prisonerState != PRISONER_STATE_REQRUITING)
		{
			_recruitingProgress -= floor(_recruitingProgress * 0.15);
		}
	}
	return result;
}

void BasePrisoner::die()
{
	for (auto s : _agents)
	{
		s->setActivePrisoner(0);
	}
	_base->removePrisoner(this);
	_interrogationProgress = 0;
	_recruitingProgress = 0;
}

std::string BasePrisoner::getNameAndId()
{
	std::ostringstream nameId;
	nameId << getName();
	nameId << " / ";
	nameId << getId();
	return nameId.str();
}

}

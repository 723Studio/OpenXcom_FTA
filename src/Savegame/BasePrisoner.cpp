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

///**
// * Loads the unit from a YAML file.
// * @param node YAML node.
// */
void BasePrisoner::load(const YAML::Node& node, const Mod* mod)
{
	_name = node["name"].as<std::string>(_name);
	_soldierId = node["soldierId"].as<int>(_soldierId);
	_state = (PrisonerState)node["state"].as<int>(_state);
	_health = node["health"].as<int>(_health);
	_faction = (UnitFaction)node["faction"].as<int>(_faction);
	_stats = node["stats"].as<UnitStats>(_stats);
	_intelligence = node["intelligence"].as<int>(_intelligence);
	_aggression = node["aggression"].as<int>(_aggression);
	_morale = node["morale"].as<int>(_morale);
	_cooperation = node["cooperation"].as<int>(_cooperation);
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
	if (_geoscapeSoldier)
	{
		node["soldierId"] = _geoscapeSoldier->getId();
	}
	else
	{
		node["soldierId"] = -1;
	}
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
void BasePrisoner::think(Game& engine)
{
	const Mod& mod = *engine.getMod();
	SavedGame& save = *engine.getSavedGame();

	//populate data
	PrisonerState prisonerState = getPrisonerState();
	_agents.clear();
	for (auto s: *_base->getSoldiers())
	{
		if (s->getActivePrisoner() == this)
			_agents.push_back(s);
	}

	Log(LOG_DEBUG) << "Processing prisoner: " << this->getNameAndId();
	Log(LOG_DEBUG) << "start cycle health: " << getHealth();
	//first, let's process physical conditions first
	if (!save.isResearched(_rule->getContainingRules().getReuiredResearch()))
	{
		setHealth(getHealth() - RNG::generate(0, _rule->getDamageOverTime()));
	}

	Log(LOG_DEBUG) << "end cycle health: " << getHealth();
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
			Log(LOG_DEBUG) << "Interrogating... Breakpoint value: " << breakpoint;
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
				Log(LOG_DEBUG) << "Agent " << s->getName() << " is calculating his/her effort for the interrogation";

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
				Log(LOG_DEBUG) << "soldierEffort value: " << effort << ", total effort: " << effort;
			}
			// If one woman can carry a baby in nine months, nine women can't do it in a month...
			if (_agents.size() > 1)
			{
				effort *= (100 - (25 * log(_agents.size()))) / 100;
			}

			effort *= (double)engine.getMasterMind()->getLoyaltyPerformanceBonus() / 100;
			progress = static_cast<int>(effort);
			Log(LOG_DEBUG) << " >>> Total daily progress for prisoner " << getNameAndId() << ": " << progress;

			_interrogationProgress += progress;
			if (_interrogationProgress >= breakpoint)
			{
				_interrogationProgress = 0;
				// give research if any
				if (!rules.getUnlockedResearches().empty())
				{
					std::string researchName = "";
					std::string bonusResearchName = "";
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

					engine.pushState(new PrisonReportState(mod.getResearch(researchName), mod.getResearch(bonusResearchName), this, _base));
				}
			}
		}
		else if (prisonerState == PRISONER_STATE_TORTURE)
		{
			auto rules = _rule->getTortureRules();
			Log(LOG_DEBUG) << "Torturing... ";
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
			Log(LOG_DEBUG) << "torturePower: " << torturePower;

			if (torturePower > 0)
			{
				if (RNG::percent(-10 * save.getDifficultyCoefficient() + 80))
				{
					int difficultyRoll = RNG::generate(rules.getDifficulty() / 2, rules.getDifficulty() * 2);
					Log(LOG_DEBUG) << "difficultyRoll: " << torturePower;
					Log(LOG_DEBUG) << "application torture effects, initial stats:";
					Log(LOG_DEBUG) << "health: " << getHealth() << ", morale: " << getMorale() << " , cooperation: " << getCooperation();

					//calculate and apply torture effects
					
					int maxDmg = 2 + floor(save.getDifficultyCoefficient() / 2);
					int loyaty = rules.getLoyalty() * (1 + floor(save.getDifficultyCoefficient() / 2));
					int moraleDmg = rules.getMorale(), eventChance = 0;
					if (difficultyRoll > torturePower * 2) // min torture
					{
						moraleDmg = 0;
						maxDmg = ceil(maxDmg / 3);
						loyaty = ceil(loyaty / 3);
					}
					else if (difficultyRoll > torturePower)
					{
						maxDmg = ceil(maxDmg / 2);
						moraleDmg = ceil(moraleDmg / 3);
						loyaty = ceil(loyaty / 2);
					}
					else // max torture
					{
						eventChance = rules.getEventChance();
					}

					setHealth(getHealth() - RNG::generate(0, maxDmg));
					setMorale(getMorale() - moraleDmg);
					setCooperation(getCooperation() - rules.getCooperation());
					engine.getMasterMind()->updateLoyalty(loyaty);

					auto events = rules.getSpawnedEvents();
					if (!events.empty() && RNG::percent(eventChance))
					{
						save.spawnEvent(events, &mod);
					}

					Log(LOG_DEBUG) << "new stats:";
					Log(LOG_DEBUG) << "morale: " << getMorale() << " , cooperation: " << getCooperation();
				}
				
			}
		}
		else if (prisonerState == PRISONER_STATE_REQRUITING)
		{
			auto rules = _rule->getRecruitingRules();
			int breakpoint = rules.getDifficulty() - getCooperation() + (100 - getMorale());
			Log(LOG_DEBUG) << "Recruiting... Breakpoint value: " << breakpoint;
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
				Log(LOG_DEBUG) << "Effort value: " << effort;
				effort += soldierEffort;
			}

			if (_agents.size() > 1)
			{
				effort *= (100 - (25 * log(_agents.size()))) / 100;
			}

			effort *= (double)engine.getMasterMind()->getLoyaltyPerformanceBonus() / 100;
			progress = static_cast<int>(effort);
			Log(LOG_DEBUG) << " >>> Total daily progress for prisoner " << getNameAndId() << ": " << progress;

			_recruitingProgress += progress;
			if (_recruitingProgress >= breakpoint)
			{
				_recruitingProgress = 0;
				auto events = rules.getSpawnedEvents();
				if (!events.empty() && RNG::percent(rules.getEventChance()))
				{
					save.spawnEvent(events, &mod);
				}

				RuleSoldier *soldierRule = mod.getSoldier(rules.getSpawnedSoldier());
				
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
					_base->getSoldiers()->push_back(soldier);
					engine.pushState(new PrisonReportState(soldier, this, _base));
				}

				die();
			}
		}
		else if (prisonerState == PRISONER_STATE_CONTAINING)
		{
			Log(LOG_DEBUG) << ">>> Containing prisoner: " << getNameAndId();
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

			Log(LOG_DEBUG) << "application contain effects, initial stats:";
			Log(LOG_DEBUG) << "health: "<< getHealth() <<", morale: " << getMorale() << " , cooperation: " << getCooperation();

			setMorale(getMorale() + moraleRegen);
			setHealth(getHealth() + hpRegen);
			setCooperation(getCooperation() + coopChange);

			Log(LOG_DEBUG) << "new stats:";
			Log(LOG_DEBUG) << "health: " << getHealth() << ", morale: " << getMorale() << " , cooperation: " << getCooperation();
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

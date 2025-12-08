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
#include "../Engine/Language.h"
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
	_rule(rule),_id(std::move(id)), _type(type), _state(PRISONER_STATE_NONE),
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
void BasePrisoner::load(const YAML::YamlNodeReader& reader, const Mod* mod)
{
	reader.tryRead("name", _name);
	reader.tryRead("type", _type);
	reader.tryRead("state", _state);
	if (reader["roles"])
	{
		std::vector<int> roles;
		reader.tryRead("roles", roles);
		loadRoles(roles);
	}
	reader.tryRead("health", _health);
	reader.tryRead("faction", _faction);
	reader.tryRead("stats", _stats);
	reader.tryRead("intelligence", _intelligence);
	reader.tryRead("aggression", _aggression);
	reader.tryRead("morale", _morale);
	reader.tryRead("cooperation", _cooperation);
	reader.tryRead("spawnedTortureEvent", _spawnedTortureEvent);
	reader.tryRead("interrogationProgress", _interrogationProgress);
	reader.tryRead("recruitingProgress", _recruitingProgress);
	reader.tryRead("interrogationDone", _interrogationDone);
	if (reader["armor"].isValid())
	{
		std::string armor;
		reader.tryRead("armor", armor);
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
void BasePrisoner::save(YAML::YamlNodeWriter writer) const
{
	writer.setAsMap();
	writer.write("id", _id);
	writer.write("type", _type);
	writer.write("name", _name);
	writer.write("state", (int)_state);
	{
		std::vector<int> roles;
		for (auto r : _roles)
		{
			roles.push_back(r);
		}
		writer.write("roles", roles);
	}
	if (_spawnedTortureEvent)
		writer.write("spawnedTortureEvent", _spawnedTortureEvent);
	writer.write("health", _health);
	writer.write("faction", (int)_faction);
	writer.write("stats", _stats);
	writer.write("intelligence", _intelligence);
	writer.write("aggression", _aggression);
	writer.write("morale", _morale);
	writer.write("cooperation", _cooperation);
	writer.write("interrogationProgress", _interrogationProgress);
	writer.write("recruitingProgress", _recruitingProgress);
	writer.write("interrogationDone", _interrogationDone);
	writer.write("armor", _armor->getType());
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
 * @param promotedSoldiers - a pointer to a vector to return in geoscape for promotion screen
 */
bool BasePrisoner::think(Game &engine, std::vector<Soldier*>& promotedSoldiers)
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
		promoteAgents(promotedSoldiers);
		die();
		engine.pushState(new PrisonReportState(this, _base));
	}
	else
	{
		//process different stats
		double speedFactor = (double)engine.getMod()->getPrisonerActionsSpeedFactor() / 100;
		int trainingFactor = mod.getIntelTrainingFactor();
		double loyaltyFactor = (double)engine.getMasterMind()->getLoyaltyPerformanceBonus() / 100;

		if (prisonerState == PRISONER_STATE_INTERROGATION)
		{
			auto rules = _rule->getInterrogationRules();
			int breakpoint = rules.getBaseResistance() + getMorale() / 4 + getAggression() * 2 + getIntelligence() * 2;
			int progress = 0;
			double effort = 0;
			for (auto s : _agents)
			{
				if (s->getCraft() != nullptr && s->getCraft()->getStatus() == "STR_OUT")
				{
					continue;
				}

				auto stats = s->getCurrentStats();
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
					&& RNG::percent(trainingFactor))
				{
					s->getIntelExperience()->interrogation++;
				}

				statEffort = stats->charisma;
				soldierEffort += (statEffort / charismaCoef);
				if (stats->charisma < caps.charisma
					&& RNG::generate(0, caps.charisma) > stats->charisma
					&& RNG::percent(trainingFactor))
				{
					s->getIntelExperience()->charisma++;
				}

				statEffort = stats->deception;
				soldierEffort += (statEffort / deceptionCoef);
				if (stats->deception < caps.deception
					&& RNG::generate(0, caps.deception) > stats->deception
					&& RNG::percent(trainingFactor))
				{
					s->getIntelExperience()->deception++;
				}

				double insightBonus = RNG::generate(0, stats->insight);
				soldierEffort += insightBonus / 100;

				//extra handle for psi
				int statsN = 4;
				if (stats->psiSkill > 0)
				{
					statsN++;
					statEffort = stats->psiSkill;
					soldierEffort += (statEffort / psiCoef);

					if (stats->psiSkill < caps.psiSkill
						&& RNG::generate(0, caps.psiSkill) > stats->psiSkill
						&& RNG::percent(trainingFactor / 2))
					{
						s->getIntelExperience()->psiSkill++;
					}
				}

				soldierEffort /= statsN;
				effort += soldierEffort;
			}
			// If one woman can carry a baby in nine months, nine women can't do it in a month...
			if (_agents.size() > 1)
			{
				effort *= (100 - (25 * log(_agents.size()))) / 100;
			}
			effort *= loyaltyFactor;
			effort *= speedFactor;
			progress = static_cast<int>(effort);
			_interrogationProgress += progress;
			if (_interrogationProgress >= breakpoint)
			{
				result = true;
				promoteAgents(promotedSoldiers);
				_interrogationProgress = 0;
				// give research if any
				std::string researchName;
				std::string bonusResearchName;
				if (!rules.getUnlockedResearches().empty())
				{
					std::vector<const RuleResearch*> possibilities;

					engine.getMasterMind()->helpResearchDiscovery(rules.getUnlockedResearches(), possibilities, _base, researchName, bonusResearchName);

					if (rules.isDiesAfterInterrogation()) //prisoner dies
					{
						die();
					}
					else if (possibilities.empty()) //there is no point interrogating further
					{
						if (save.isResearched(_rule->getContainingRules().getReuiredResearch()))
						{
							setPrisonerState(PRISONER_STATE_CONTAINING);
						}
						else
						{
							setPrisonerState(PRISONER_STATE_NONE);
						}
					}
				}

				_interrogationDone = true;
				for (auto s : _agents)
				{
					s->clearBaseDuty();
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
				torturePower += agent->getCurrentStats()->bravery;
				if (agent->getCurrentStats()->psiSkill > 0)
				{
					if (agent->getCurrentStats()->psiStrength > 50)
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
					int tortureDifficulty = RNG::generate(rules.getDifficulty() / 2, rules.getDifficulty() * 2);
					//calculate and apply torture effects
					int maxDmg = 4 + floor(save.getDifficultyCoefficient() / 2);
					int loyaty = rules.getLoyalty() * (1 + floor(save.getDifficultyCoefficient() / 2));
					int moraleDmg = rules.getMorale();
					int eventChance = rules.getEventChance();
					if (tortureDifficulty > torturePower * 2) // min torture
					{
						moraleDmg = 0;
						maxDmg = ceil(maxDmg / 2);
						loyaty = ceil(loyaty / 5);
						eventChance = ceil(eventChance / 3);
					}
					else if (tortureDifficulty > torturePower)
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

			for (auto s : _agents)
			{
				double soldierEffort = 0, statEffort = 0;
				auto stats = s->getCurrentStats();
				auto caps = s->getRules()->getStatCaps();

				statEffort = stats->charisma;
				soldierEffort += (statEffort);
				if (stats->charisma < caps.charisma
					&& RNG::generate(0, caps.charisma) > stats->charisma
					&& RNG::percent(trainingFactor))
				{
					s->getIntelExperience()->charisma++;
				}

				statEffort = stats->deception;
				soldierEffort += (statEffort);
				if (stats->deception < caps.deception
					&& RNG::generate(0, caps.deception) > stats->deception
					&& RNG::percent(trainingFactor))
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

			effort *= loyaltyFactor;
			effort *= speedFactor;
			progress = static_cast<int>(effort);
			_recruitingProgress += progress;
			if (_recruitingProgress >= breakpoint)
			{
				result = true;
				promoteAgents(promotedSoldiers);
				_recruitingProgress = 0;
				auto events = rules.getSpawnedEvents();
				if (!events.empty() && RNG::percent(rules.getEventChance()))
				{
					save.spawnEvent(events, &mod);
				}

				const RuleSoldier *soldierRule = mod.getSoldier(rules.getSpawnedSoldier());
				if (soldierRule != nullptr) // we now create a new soldier from prisoner
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
	for (auto &s : _agents)
	{
		s->clearBaseDuty();
	}
	_base->removePrisoner(this);
	_interrogationProgress = 0;
	_recruitingProgress = 0;
}

void BasePrisoner::promoteAgents(std::vector<Soldier*>& promotedSoldiers)
{
	for (auto* s : _agents)
	{
		s->improvePrimaryStats(s->getIntelExperience(), ROLE_AGENT);
		s->clearIntelExperience();
		if (s->rolePromoteSoldier(ROLE_AGENT))
		{
			promotedSoldiers.push_back(s);
		}
	}
}

std::string BasePrisoner::getNameAndId(Language* lang)
{
	std::ostringstream nameId;
	nameId << lang->getString(getName());
	nameId << " / ";
	nameId << getId();
	return nameId.str();
}

}

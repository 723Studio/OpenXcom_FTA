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
#include "IntelProject.h"
#include "../Mod/RuleIntelProject.h"
#include "../Engine/Game.h"
#include "../FTA/MasterMind.h"
#include "../Savegame/SavedGame.h"
#include "../Savegame/Base.h"
#include "../Savegame/Soldier.h"

namespace OpenXcom
{

const float PROGRESS_LIMIT_POOR = 0.1f;
const float PROGRESS_LIMIT_AVERAGE = 0.2f;
const float PROGRESS_LIMIT_GOOD = 0.3f;
const float PROGRESS_LIMIT_GREAT = 0.4f;
const float PROGRESS_LIMIT_SUPERIOR = 0.5f;

IntelProject::IntelProject(const RuleIntelProject* rule, Base *base, int cost) :
	_rules(rule), _base(base), _active(true), _spent(0), _rolls(0), _cost(cost)
{
}

int IntelProject::getStepProgress(std::map<Soldier*, int>& assignedAgents, Mod* mod, int rating, std::string& description, bool estimate)
{
	int progress = 0;
	double effort = 0, soldierEffort = 0, statEffort = 0;
	auto projStats = _rules->getStats();
	int factor = mod->getIntelTrainingFactor();
	for (auto s : assignedAgents)
	{
		auto stats = s.first->getStatsWithAllBonuses();
		auto caps = s.first->getRules()->getStatCaps();
		unsigned int statsN = 0;

		if (projStats.data > 0)
		{
			statEffort = stats->data;
			soldierEffort += (statEffort / projStats.data);
			if (!estimate && stats->data < caps.data && RNG::generate(0, caps.data) > stats->data && RNG::percent(factor * (projStats.data / 100)) && RNG::percent(s.second))
				s.first->getIntelExperience()->data++;
			statsN++;
		}
		if (projStats.computers > 0)
		{
			statEffort = stats->computers;
			soldierEffort += (statEffort / projStats.computers);
			if (!estimate && stats->computers < caps.computers && RNG::generate(0, caps.computers) > stats->computers && RNG::percent(factor) && RNG::percent(s.second))
				s.first->getIntelExperience()->computers++;
			statsN++;
		}
		if (projStats.xenolinguistics > 0)
		{
			statEffort = stats->xenolinguistics;
			soldierEffort += (statEffort / projStats.xenolinguistics);
			if (!estimate && stats->xenolinguistics < caps.xenolinguistics && RNG::generate(0, caps.xenolinguistics) > stats->xenolinguistics && RNG::percent(factor) && RNG::percent(s.second))
				s.first->getIntelExperience()->xenolinguistics++;
			statsN++;
		}
		if (projStats.hacking > 0)
		{
			statEffort = stats->hacking;
			soldierEffort += (statEffort / projStats.hacking);
			if (!estimate && stats->hacking < caps.hacking && RNG::generate(0, caps.hacking) > stats->hacking && RNG::percent(factor) && RNG::percent(s.second))
				s.first->getIntelExperience()->hacking++;
			statsN++;
		}
		if (projStats.alienTech > 0)
		{
			statEffort = stats->alienTech;
			soldierEffort += (statEffort / projStats.alienTech);
			if (!estimate && stats->alienTech < caps.alienTech && RNG::generate(0, caps.alienTech) > stats->alienTech && RNG::percent(factor) && RNG::percent(s.second))
				s.first->getIntelExperience()->alienTech++;
			statsN++;
		}
		if (projStats.investigation > 0)
		{
			statEffort = stats->investigation;
			soldierEffort += (statEffort / projStats.investigation);
			if (!estimate && stats->investigation < caps.investigation && RNG::generate(0, caps.investigation) > stats->investigation && RNG::percent(factor) && RNG::percent(s.second))
				s.first->getIntelExperience()->investigation++;
			statsN++;
		}

		if (statsN > 0)
		{
			soldierEffort /= statsN;
		}
		Log(LOG_INFO) << "Adjusted effort value: " << effort;

		double insightBonus = RNG::generate(0, stats->insight / 2);
		if (estimate)
		{
			insightBonus = stats->insight;
			insightBonus /= 4; //just take average roll
		}
		soldierEffort += insightBonus / 10;
		effort += soldierEffort;
		soldierEffort = 0;
	}
	// If one woman can carry a baby in nine months, nine women can't do it in a month...
	if (assignedAgents.size() > 1)
	{
		effort *= (100 - (19 * log(assignedAgents.size()))) / 100;
	}
	effort *= rating;
	effort /= 100;
	//gets total effort to daily project progress
	progress = static_cast<int>(ceil(effort * 24));
	Log(LOG_INFO) << " >>> Total daily progress for the intel project " << _rules->getName() << ": " << progress;
	description = getState(progress);

	return progress;
}

/**
 * Called every day to compute time spent on this IntelProject
 * @return true if the ResearchProject is finished
 */
bool IntelProject::roll(Game *game, const Globe& globe, int progress, bool &finalRoll)
{
	SavedGame* save = game->getSavedGame();

	_spent += progress;
	finalRoll = false;
	bool specialRule = _rules->getSpecialRule() != INTEL_NONE;
	_active = progress > 0 && specialRule;
	
	if (_spent >= _cost)
	{
		_spent = 0; //clear progress of the project, preparing it for the next stage roll.
		_rolls++;

		std::vector<const RuleIntelStage*> rolledStages;
		for (auto stage : getAvailableStages(save))
		{
			if (RNG::percent(stage->getOdds()))
			{
				rolledStages.push_back(stage); //populate list of stages
			}
		}

		if (!rolledStages.empty())
		{
			auto pickedStage = rolledStages.at(RNG::generate(0, rolledStages.size())); // only one stage processed at a time
			//run all event scripts for chosen stage
			if (!pickedStage->getEventScripts().empty())
			{
				game->getMasterMind()->eventScriptProcessor(pickedStage->getEventScripts(), SCRIPT_XCOM);
			}

			//and create alien mission if any
			if (!pickedStage->getSpawnedMission().empty())
			{
				game->getMasterMind()->spawnAlienMission(pickedStage->getSpawnedMission(), globe, _base);
			}

			//update data if the project reaches its final stage and counted as completed.
			if (pickedStage->isFinalStage())
			{
				_active = false;
				finalRoll = true;
			}

			//update rolled stages information
			auto it = _stageRolls.find(pickedStage->getName());
			if (it != _stageRolls.end())
			{
				it->second++;
			}
			else
			{
				_stageRolls.insert(std::make_pair(pickedStage->getName(), 1));
			}
			
			return true; //we finis stage rolling, this would tell the game to prepare data for the next one
		}
	}
	return false;
}

const std::vector<const RuleIntelStage*> IntelProject::getAvailableStages(SavedGame* save)
{
	std::vector<const RuleIntelStage*> availableStages;

	for (auto stage : _rules->getStages())
	{
		bool triggerHappy = false;
		auto it = _stageRolls.find(stage->getName());
		if (it == _stageRolls.end())
		{
			triggerHappy = true; //case we have not rolled this stage before.
		}
		else if (stage->isFinalStage())
		{
			availableStages.clear(); //we already done with this project, abort the search with empty result.
			break;
		}
		else if (it->second < stage->getAvailableRolls())
		{
			triggerHappy = true; //case we don't have enough rolls for this stage yet.
		}

		if (triggerHappy) // Check for researches.
		{
			if (save->isResearched(stage->getRequiredResearch()) && !save->isResearched(stage->getDisabledByResearch()))
			{
				triggerHappy = true;
			}
		}

		if (triggerHappy) // Check for required buildings/functions in the given base
		{
			if ((~_base->getProvidedBaseFunc({}) & stage->getRequireBaseFunc()).any())
			{
				continue; //we don't have required facility, go to the next stage.
			}
		}

		if (triggerHappy)
		{
			availableStages.push_back(stage);
		}
	}

	return availableStages;
}

const std::string IntelProject::getName() const
{
	return _rules->getName();
}


/**
 * Loads the research project from a YAML file.
 * @param node YAML node.
 */
void IntelProject::load(const YAML::Node& node)
{
	_stageRolls = node["stageRolls"].as<std::map<std::string, int>>(_stageRolls);
	_active = node["active"].as<int>(_active);
	_rolls = node["rolls"].as<int>(_rolls);
	_spent = node["spent"].as<int>(_spent);
	_cost = node["cost"].as<int>(_cost);
}

/**
 * Saves the research project to a YAML file.
 * @return YAML node.
 */
YAML::Node IntelProject::save() const
{
	YAML::Node node;
	node["project"] = getRules()->getName();
	if (!_stageRolls.empty())
	{
		node["stageRolls"] = _stageRolls;
	}
	if (_active)
	{
		node["active"] = _active;
	}
	node["rolls"] = _rolls;
	node["spent"] = _spent;
	node["cost"] = _cost;
	return node;
}

/**
 * Return a string describing project progress.
 * @return a string describing project progress.
 */
std::string IntelProject::getState(int progress) const
{
	std::string result;
	if (progress <= 0)
	{
		result = "STR_NONE";
	}
	else
	{
		float rating = static_cast<float>(progress);
		rating /= static_cast<float>(_cost + (_rolls * getRules()->getCostIncrease()));
		if (rating <= PROGRESS_LIMIT_POOR)
		{
			result = "STR_POOR";
		}
		else if (rating <= PROGRESS_LIMIT_AVERAGE)
		{
			result = "STR_AVERAGE";
		}
		else if (rating <= PROGRESS_LIMIT_GOOD)
		{
			result = "STR_GOOD";
		}
		else if (rating <= PROGRESS_LIMIT_GREAT)
		{
			result = "STR_GREAT";
		}
		else if (rating <= PROGRESS_LIMIT_SUPERIOR)
		{
			result = "STR_SUPERIOR";
		}
		else
		{
			result = "STR_EXCELLENT";
		}
	}
	
	return result;
}

}

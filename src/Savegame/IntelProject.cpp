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

constexpr float PROGRESS_LIMIT_POOR = 0.1f;
constexpr float PROGRESS_LIMIT_AVERAGE = 0.2f;
constexpr float PROGRESS_LIMIT_GOOD = 0.3f;
constexpr float PROGRESS_LIMIT_GREAT = 0.4f;
constexpr float PROGRESS_LIMIT_SUPERIOR = 0.5f;

IntelProject::IntelProject(const RuleIntelProject* rule, Base *base, int cost) :
	_rules(rule), _base(base), _active(true), _spent(0), _rolls(0), _cost(cost)
{
}

int IntelProject::getStepProgress(std::map<Soldier*, int>& assignedAgents, Mod* mod, int rating, std::string& description, bool estimate)
{
	int progress = 0;
	if (assignedAgents.empty())
	{
		description = getState(progress);
		return 0;
	}

	double effort = 0;
	auto projStats = _rules->getStats();
	int trainingFactor = mod->getIntelTrainingFactor();
	double speedFactor = (double)mod->getIntelSpeedFactor() / 100;

	for (auto s : assignedAgents)
	{
		Soldier* agent = s.first;
		if (agent->getCraft() && agent->getCraft()->getStatus() == "STR_OUT")
		{
			continue;
		}

		auto stats = agent->getCurrentStats();
		auto caps = agent->getRules()->getStatCaps();
		unsigned int statsN = 0;
		double  soldierEffort = 0, statEffort = 0;
		if (projStats.data > 0)
		{
			statEffort = stats->data;
			soldierEffort += (statEffort / projStats.data);
			if (!estimate && stats->data < caps.data && RNG::generate(0, caps.data) > stats->data && RNG::percent(trainingFactor) && RNG::percent(s.second))
				agent->getIntelExperience()->data++;
			statsN++;
		}
		if (projStats.computers > 0)
		{
			statEffort = stats->computers;
			soldierEffort += (statEffort / projStats.computers);
			if (!estimate && stats->computers < caps.computers && RNG::generate(0, caps.computers) > stats->computers && RNG::percent(trainingFactor) && RNG::percent(s.second))
				agent->getIntelExperience()->computers++;
			statsN++;
		}
		if (projStats.xenolinguistics > 0)
		{
			statEffort = stats->xenolinguistics;
			soldierEffort += (statEffort / projStats.xenolinguistics);
			if (!estimate && stats->xenolinguistics < caps.xenolinguistics && RNG::generate(0, caps.xenolinguistics) > stats->xenolinguistics && RNG::percent(trainingFactor) && RNG::percent(s.second))
				agent->getIntelExperience()->xenolinguistics++;
			statsN++;
		}
		if (projStats.hacking > 0)
		{
			statEffort = stats->hacking;
			soldierEffort += (statEffort / projStats.hacking);
			if (!estimate && stats->hacking < caps.hacking && RNG::generate(0, caps.hacking) > stats->hacking && RNG::percent(trainingFactor) && RNG::percent(s.second))
				agent->getIntelExperience()->hacking++;
			statsN++;
		}
		if (projStats.alienTech > 0)
		{
			statEffort = stats->alienTech;
			soldierEffort += (statEffort / projStats.alienTech);
			if (!estimate && stats->alienTech < caps.alienTech && RNG::generate(0, caps.alienTech) > stats->alienTech && RNG::percent(trainingFactor) && RNG::percent(s.second))
				agent->getIntelExperience()->alienTech++;
			statsN++;
		}
		if (projStats.investigation > 0)
		{
			statEffort = stats->investigation;
			soldierEffort += (statEffort / projStats.investigation);
			if (!estimate && stats->investigation < caps.investigation && RNG::generate(0, caps.investigation) > stats->investigation && RNG::percent(trainingFactor) && RNG::percent(s.second))
				agent->getIntelExperience()->investigation++;
			statsN++;
		}

		double insightBonus = RNG::generate(0, stats->insight / 2);
		if (estimate)
		{
			insightBonus = stats->insight;
			insightBonus /= 4; //just take average roll
		}
		insightBonus /= 10;
		soldierEffort += insightBonus;
		soldierEffort /= statsN + 1;
		effort += soldierEffort;
	}
	// If one woman can carry a baby in nine months, nine women can't do it in a month...
	if (assignedAgents.size() > 1)
	{
		effort *= (100 - (19 * log(assignedAgents.size()))) / 100;
	}
	effort *= (double)rating / 100;
	effort *= speedFactor * 24;

	//gets total effort to daily project progress
	progress = static_cast<int>(ceil(effort));

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

	if (_spent > (_rolls * getRules()->getCostIncrease()))
	{
		_spent = 0; //clear progress of the project, preparing it for the next stage roll.
		_rolls++;
		Log(LOG_INFO) << ">> Progress made with intelligence project: " << _rules->getName();
		std::vector<const RuleIntelStage*> rolledStages;
		for (auto stage : getAvailableStages(save))
		{
			if (RNG::percent(stage->getOdds()
				&& stage->getRequireRolls() <= _rolls
				&& save->isResearched(stage->getRequiredResearch()))
				&& !save->isResearched(stage->getDisabledByResearch()))
			{
				Log(LOG_INFO) << " - stage named: " << stage->getName() << " was added to the pool.";
				rolledStages.push_back(stage); //populate list of stages
			}
		}

		if (!rolledStages.empty())
		{
			auto pickedStage = rolledStages.at(RNG::generate(0, rolledStages.size())); // only one stage processed at a time
			//run all event scripts for chosen stage
			Log(LOG_INFO) << "Stage picked for being processed: " << pickedStage->getName();
			if (!pickedStage->getEventScripts().empty())
			{
				std::ostringstream sd;
				for (auto& s : pickedStage->getEventScripts())
				{
					sd << s << " ";
				}
				Log(LOG_INFO) << " - processing eventScript: " << sd.str();
				game->getMasterMind()->eventScriptProcessor(pickedStage->getEventScripts(), OTHER_SCRIPT);
			}

			//and create alien mission if any
			if (!pickedStage->getSpawnedMission().empty())
			{
				Log(LOG_INFO) << " - spawning the mission: " << pickedStage->getSpawnedMission();
				game->getMasterMind()->spawnAlienMission(pickedStage->getSpawnedMission(), globe, _base);
			}

			//update data if the project reaches its final stage and counted as completed.
			if (pickedStage->isFinalStage())
			{
				Log(LOG_INFO) << " - it's a final roll for the project!";
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

			return true; //we finish stage rolling, this would tell the game to prepare data for the next one
		}
	}
	return false;
}

std::vector<const RuleIntelStage*> IntelProject::getAvailableStages(SavedGame* save)
{
	std::vector<const RuleIntelStage*> availableStages;

	for (auto stage : _rules->getStages())
	{
		bool triggerHappy = false;
		auto it = _stageRolls.find(stage->getName());
		if (it == _stageRolls.end() //case we have not rolled this stage before.
			|| it->second < stage->getAvailableRolls()) //case we don't have enough rolls for this stage yet.
		{
			triggerHappy = true;
		}
		else if (stage->isFinalStage()) //we already done with this project, abort the search with empty result.
		{
			availableStages.clear();
			break;
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

std::string IntelProject::getName() const
{
	return _rules->getName();
}


/**
 * Loads the research project from a YAML file.
 * @param node YAML node.
 */
void IntelProject::load(const YAML::YamlNodeReader& reader)
{
	reader.tryRead("stageRolls", _stageRolls);
	reader.tryRead("active", _active);
	reader.tryRead("rolls", _rolls);
	reader.tryRead("spent", _spent);
	reader.tryRead("cost", _cost);
}

/**
 * Saves the research project to a YAML file.
 * @return YAML node.
 */
void IntelProject::save(YAML::YamlNodeWriter writer) const
{
	writer.setAsMap();
	writer.write("name", getRules()->getName());
	if (!_stageRolls.empty())
	{
		writer.write("stageRolls", _stageRolls);
	}
	if (_active)
	{
		writer.write("active", _active);
	}
	writer.write("rolls", _rolls);
	writer.write("spent", _spent);
	writer.write("cost", _cost);
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

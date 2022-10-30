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
#include "ResearchProject.h"
#include "../Mod/RuleResearch.h"
#include "../Mod/Mod.h"
#include "../Savegame/Soldier.h"

namespace OpenXcom
{
const float PROGRESS_LIMIT_UNKNOWN = 0.333f;
const float PROGRESS_LIMIT_POOR = 0.07f;
const float PROGRESS_LIMIT_AVERAGE = 0.13f;
const float PROGRESS_LIMIT_GOOD = 0.25f;

ResearchProject::ResearchProject(const RuleResearch * p, int c) : _project(p), _assigned(0), _spent(0), _cost(c)
{
}

/**
 * Called every day (every hour in FtA) to compute time spent on this ResearchProject
 * @return true if the ResearchProject is finished
 */
bool ResearchProject::step(int progress)
{
	_spent += progress;
	return isFinished();
}

int ResearchProject::getStepProgress(std::map<Soldier*, int>& assignedScientists, Mod* mod, int rating)
{
	int progress = 0;
	double effort = 0, soldierEffort = 0, statEffort = 0;
	auto projStats = _project->getStats();
	int factor = mod->getResearchTrainingFactor();
	for (auto s : assignedScientists)
	{
		auto stats = s.first->getStatsWithAllBonuses();
		auto caps = s.first->getRules()->getStatCaps();
		unsigned int statsN = 0;

		if (projStats.physics > 0)
		{
			statEffort = stats->physics;
			soldierEffort += (statEffort / projStats.physics);
			if (stats->physics < caps.physics
				&& RNG::generate(0, caps.physics) > stats->physics
				&& RNG::percent(factor * (projStats.physics / 100))
				&& RNG::percent(s.second))
			{
				s.first->getResearchExperience()->physics++;
			}
			statsN++;
		}
		if (projStats.chemistry > 0)
		{
			statEffort = stats->chemistry;
			soldierEffort += (statEffort / projStats.chemistry);
			if (stats->chemistry < caps.chemistry
				&& RNG::generate(0, caps.chemistry) > stats->chemistry
				&& RNG::percent(factor)
				&& RNG::percent(s.second))
			{
				s.first->getResearchExperience()->chemistry++;
			}
			statsN++;
		}
		if (projStats.biology > 0)
		{
			statEffort = stats->biology;
			soldierEffort += (statEffort / projStats.biology);
			if (stats->biology < caps.biology
				&& RNG::generate(0, caps.biology) > stats->biology
				&& RNG::percent(factor)
				&& RNG::percent(s.second))
			{
				s.first->getResearchExperience()->biology++;
			}
			statsN++;
		}
		if (projStats.data > 0)
		{
			statEffort = stats->data;
			soldierEffort += (statEffort / projStats.data);
			if (stats->data < caps.data
				&& RNG::generate(0, caps.data) > stats->data
				&& RNG::percent(factor)
				&& RNG::percent(s.second))
			{
				s.first->getResearchExperience()->data++;
			}
			statsN++;
		}
		if (projStats.computers > 0)
		{
			statEffort = stats->computers;
			soldierEffort += (statEffort / projStats.computers);
			if (stats->computers < caps.computers
				&& RNG::generate(0, caps.computers) > stats->computers
				&& RNG::percent(factor)
				&& RNG::percent(s.second))
			{
				s.first->getResearchExperience()->computers++;
			}
			statsN++;
		}
		if (projStats.tactics > 0)
		{
			statEffort = stats->tactics;
			soldierEffort += (statEffort / projStats.tactics);
			if (stats->tactics < caps.tactics
				&& RNG::generate(0, caps.tactics) > stats->tactics
				&& RNG::percent(factor)
				&& RNG::percent(s.second))
			{
				s.first->getResearchExperience()->tactics++;
			}
			statsN++;
		}
		if (projStats.materials > 0)
		{
			statEffort = stats->materials;
			soldierEffort += (statEffort / projStats.materials);
			if (stats->materials < caps.materials
				&& RNG::generate(0, caps.materials) > stats->materials
				&& RNG::percent(factor)
				&& RNG::percent(s.second))
			{
				s.first->getResearchExperience()->materials++;
			}
			statsN++;
		}
		if (projStats.designing > 0)
		{
			statEffort = stats->designing;
			soldierEffort += (statEffort / projStats.designing);
			if (stats->designing < caps.designing
				&& RNG::generate(0, caps.designing) > stats->designing
				&& RNG::percent(factor)
				&& RNG::percent(s.second))
			{
				s.first->getResearchExperience()->designing++;
			}
			statsN++;
		}
		if (projStats.alienTech > 0)
		{
			statEffort = stats->alienTech;
			soldierEffort += (statEffort / projStats.alienTech);
			if (stats->alienTech < caps.alienTech
				&& RNG::generate(0, caps.alienTech) > stats->alienTech
				&& RNG::percent(factor)
				&& RNG::percent(s.second))
			{
				s.first->getResearchExperience()->alienTech++;
			}
			statsN++;
		}
		if (projStats.psionics > 0)
		{
			statEffort = stats->psionics;
			soldierEffort += (statEffort / projStats.psionics);
			if (stats->psionics < caps.psionics
				&& RNG::generate(0, caps.psionics) > stats->psionics
				&& RNG::percent(factor)
				&& RNG::percent(s.second))
			{
				s.first->getResearchExperience()->psionics++;
			}
			statsN++;
		}
		if (projStats.xenolinguistics > 0)
		{
			statEffort = stats->xenolinguistics;
			soldierEffort += (statEffort / projStats.xenolinguistics);
			if (stats->psionics < caps.xenolinguistics
				&& RNG::generate(0, caps.xenolinguistics) > stats->xenolinguistics
				&& RNG::percent(factor)
				&& RNG::percent(s.second))
			{
				s.first->getResearchExperience()->xenolinguistics++;
			}
			statsN++;
		}

		if (statsN > 0)
		{
			soldierEffort /= statsN;
		}
		Log(LOG_INFO) << "Adjusted effort value: " << effort;

		double insightBonus = RNG::generate(0, stats->insight);
		soldierEffort += insightBonus / 10;

		effort += soldierEffort;
		soldierEffort = 0;
	}
	// If one woman can carry a baby in nine months, nine women can't do it in a month...
	if (assignedScientists.size() > 1)
	{
		effort *= (100 - (19 * log(assignedScientists.size()))) / 100;
	}
	
	effort *= (double)rating / 100;
	progress = static_cast<int>(ceil(effort / 24));
	Log(LOG_INFO) << " >>> Total hourly progress for project " << _project->getName() << ": " << progress;

	return progress;
}

/**
 * gets state of project.
 */
bool ResearchProject::isFinished()
{
	return _spent >= getCost();
}

/**
 * Changes the number of scientist to the ResearchProject
 * @param nb number of scientist assigned to this ResearchProject
 */
void ResearchProject::setAssigned (int nb)
{
	_assigned = nb;
}

const RuleResearch * ResearchProject::getRules() const
{
	return _project;
}

/**
 * Returns the number of scientist assigned to this project
 * @return Number of assigned scientist.
 */
int ResearchProject::getAssigned() const
{
	return _assigned;
}

/**
 * Returns the time already spent on this project
 * @return the time already spent on this ResearchProject(in man/day)
 */
int ResearchProject::getSpent() const
{
	return _spent;
}

/**
 * Changes the cost of the ResearchProject
 * @param spent new project cost(in man/day)
 */
void ResearchProject::setSpent (int spent)
{
	_spent = spent;
}

/**
 * Returns the cost of the ResearchProject
 * @return the cost of the ResearchProject(in man/day)
 */
int ResearchProject::getCost() const
{
	return _cost;
}

/**
 * Changes the cost of the ResearchProject
 * @param f new project cost(in man/day)
 */
void ResearchProject::setCost(int f)
{
	_cost = f;
}

/**
 * Loads the research project from a YAML file.
 * @param node YAML node.
 */
void ResearchProject::load(const YAML::Node& node)
{
	setAssigned(node["assigned"].as<int>(getAssigned()));
	setSpent(node["spent"].as<int>(getSpent()));
	setCost(node["cost"].as<int>(getCost()));
}

/**
 * Saves the research project to a YAML file.
 * @return YAML node.
 */
YAML::Node ResearchProject::save() const
{
	YAML::Node node;
	node["project"] = getRules()->getName();
	node["assigned"] = getAssigned();
	node["spent"] = getSpent();
	node["cost"] = getCost();
	return node;
}

/**
 * Return a string describing Research progress.
 * @return a string describing Research progress.
 */
std::string ResearchProject::getResearchProgress() const
{
	float progress = (float)getSpent() / getRules()->getCost();
	if (getAssigned() == 0)
	{
		return "STR_NONE";
	}
	else if (progress <= PROGRESS_LIMIT_UNKNOWN)
	{
		return "STR_UNKNOWN";
	}
	else
	{
		float rating = (float)getAssigned();
		rating /= getRules()->getCost();
		if (rating <= PROGRESS_LIMIT_POOR)
		{
			return "STR_POOR";
		}
		else if (rating <= PROGRESS_LIMIT_AVERAGE)
		{
			return "STR_AVERAGE";
		}
		else if (rating <= PROGRESS_LIMIT_GOOD)
		{
			return "STR_GOOD";
		}
		return "STR_EXCELLENT";
	}
}

}

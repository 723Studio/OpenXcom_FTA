#pragma once
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
#include <yaml-cpp/yaml.h>

namespace OpenXcom
{

class RuleIntelProject;
class RuleIntelStage;
class Soldier;
class Game;
class SavedGame;
class Mod;
class Base;

/**
 Represent a IntelProject
 Contain information about assigned scientist, time already spent and cost of the project.
 */
class IntelProject
{
	const RuleIntelProject * _rules;
	std::map<std::string, int> _stageRolls;
	bool _active;
	int _spent;
	int _rolls;
	int _cost;
public:
	IntelProject(const RuleIntelProject *rule, int cost);
	/// Game logic. Called every new day (every hour for FtA) to compute time spent.
	int getStepProgress(std::map<Soldier*, int> &assignedAgents, Mod *mod, int rating);
	bool roll(Game *game, Base *base, int progress);
	const std::vector<const RuleIntelStage*> getAvailableStages(SavedGame* save, Base *base);
	/// get the IntelProject Mod
	const RuleIntelProject* getRules() const { return _rules; };
	/// load the IntelProject from YAML
	void load(const YAML::Node& node);
	/// save the IntelProject to YAML
	YAML::Node save() const;
	/// Get a string describing current progress.
	std::string getState() const;
};

}

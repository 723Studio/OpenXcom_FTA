#pragma once
/*
 * Copyright 2010-2021 OpenXcom Developers.
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
#include <yaml-cpp/yaml.h>

namespace OpenXcom
{

class DiplomacyFaction;
class RuleResearch;

/**
* Represents ongoing factional research.
* Handles data and methods required to process research.
*/
class FactionalResearch
{
private:
	const RuleResearch* _rule;
	DiplomacyFaction* _faction;
	int _priority;
	int _timeLeft;
	int _scientists;
public:
	/// Creates a blank Factional Research.
	FactionalResearch(const RuleResearch* rule, DiplomacyFaction* faction);
	/// Cleans up the Factional Research info.
	~FactionalResearch();
	/// Loads the FactionalResearch from YAML.
	void load(const YAML::Node& node);
	/// Saves the FactionalResearch to YAML.
	YAML::Node save() const;
	/// Process ongoing research project, decreasing timer and returns true if it is over.
	bool step();
	int getScientists() { return _scientists; };
	void setScientists(int scientists) { _scientists = scientists; };
	int getTimeLeft() { return _timeLeft; };
	void setTimeLeft(int timeLeft) { _timeLeft = timeLeft; }
	int getPriority() { return _priority; };
	void setPriority(int priority) { _priority = priority; }
	const std::string& getName();
	const RuleResearch* getRules() const { return _rule; };
};
}

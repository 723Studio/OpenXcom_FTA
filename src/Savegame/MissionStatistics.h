#pragma once
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
#include <yaml-cpp/yaml.h>
#include <map>
#include <string>
#include <sstream>
#include "GameTime.h"
#include "../Engine/Language.h"
#include "../Mod/Mod.h"

namespace OpenXcom
{

/**
 * Container for mission statistics.
 */
struct MissionStatistics
{
	// Variables
	int id;
	std::string markerName;
	int markerId;
	GameTime time;
	std::string region, country, type, ufo, craft;
	bool success;
	bool aborted;
	std::string rating;
	int score;
	std::string alienRace;
	int daylight;
	std::map<int, int> injuryList;
	bool valiantCrux;
	int lootValue;

	/// Load
	void load(const YAML::Node &node)
	{
		id = node["id"].as<int>(id);
		markerName = node["markerName"].as<std::string>(markerName);
		markerId = node["markerId"].as<int>(markerId);
		time.load(node["time"]);
		region = node["region"].as<std::string>(region);
		country = node["country"].as<std::string>(country);
		type = node["type"].as<std::string>(type);
		ufo = node["ufo"].as<std::string>(ufo);
		craft = node["craft"].as<std::string>(craft);
		success = node["success"].as<bool>(success);
		aborted = node["aborted"].as<bool>(aborted);
		score = node["score"].as<int>(score);
		rating = node["rating"].as<std::string>(rating);
		alienRace = node["alienRace"].as<std::string>(alienRace);
		daylight = node["daylight"].as<int>(daylight);
		injuryList = node["injuryList"].as< std::map<int, int> >(injuryList);
		valiantCrux = node["valiantCrux"].as<bool>(valiantCrux);
		lootValue = node["lootValue"].as<int>(lootValue);
	}

	/// Save
	YAML::Node save() const
	{
		YAML::Node node;
		node["id"] = id;
		if (!markerName.empty())
		{
			node["markerName"] = markerName;
			node["markerId"] = markerId;
		}
		node["time"] = time.save();
		node["region"] = region;
		node["country"] = country;
		node["type"] = type;
		node["ufo"] = ufo;
		node["craft"] = craft;
		node["success"] = success;
		node["aborted"] = aborted;
		node["score"] = score;
		node["rating"] = rating;
		node["alienRace"] = alienRace;
		node["daylight"] = daylight;
		if (!injuryList.empty())
			node["injuryList"] = injuryList;
		if (valiantCrux) node["valiantCrux"] = valiantCrux;
		if (lootValue) node["lootValue"] = lootValue;
		return node;
	}

	std::string getMissionName(Language *lang) const
	{
		if (!markerName.empty())
		{
			return lang->getString(markerName).arg(markerId);
		}
		else
		{
			return lang->getString(type);
		}
	}

	std::string getRatingString(Language *lang) const
	{
		std::ostringstream ss;
		if (success)
		{
			ss << lang->getString("STR_VICTORY");
		}
		else
		{
			ss << lang->getString("STR_DEFEAT");
		}
		ss << " - " << lang->getString(rating);
		return ss.str();
	}

	std::string getLocationString() const
	{
		if (country == "STR_UNKNOWN")
		{
			return region;
		}
		else
		{
			return country;
		}
	}

	bool isDarkness(const Mod* mod) const
	{
		return daylight > mod->getMaxDarknessToSeeUnits();
	}

	std::string getDaylightString(const Mod* mod) const
	{
		if (isDarkness(mod))
		{
			return "STR_NIGHT";
		}
		else
		{
			return "STR_DAY";
		}
	}

	bool isAlienBase() const
	{
		if (type.find("STR_ALIEN_BASE") != std::string::npos || type.find("STR_ALIEN_COLONY") != std::string::npos)
		{
			return true;
		}
		return false;
	}

	bool isBaseDefense() const
	{
		if (type == "STR_BASE_DEFENSE")
		{
			return true;
		}
		return false;
	}

	bool isUfoMission() const
	{
		if(ufo != "NO_UFO")
		{
			return true;
		}
		return false;
	}

	MissionStatistics(const YAML::Node& node) : time(0, 0, 0, 0, 0, 0, 0) { load(node); }
	MissionStatistics() : id(0), markerId(0), time(0, 0, 0, 0, 0, 0, 0), region("STR_REGION_UNKNOWN"), country("STR_UNKNOWN"), ufo("NO_UFO"), craft("NO_CRAFT"), success(false), aborted(false), score(0), alienRace("STR_UNKNOWN"), daylight(0), valiantCrux(false), lootValue(0) { }
	~MissionStatistics() { }
};

}

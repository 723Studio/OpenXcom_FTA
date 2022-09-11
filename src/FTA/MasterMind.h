#pragma once
/*
 * Copyright 2010-2020 OpenXcom Developers.
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

#include <vector>
#include <string>
#include <set>

namespace OpenXcom
{

enum LoyaltySource {XCOM_BATTLESCAPE, XCOM_DOGFIGHT, XCOM_GEOSCAPE, XCOM_RESEARCH, ALIEN_MISSION_DESPAWN, ALIEN_UFO_ACTIVITY, ALIEN_BASE, ABSOLUTE_COEF};
enum ProcessorSource {SCRIPT_MONTHLY, SCRIPT_FACTIONAL, SCRIPT_XCOM, OTHER_SCRIPT};

class Game;
class GeoscapeState;
class Globe;
class Base;
class DiplomacyFaction;

class MasterMind
{
private:
	Game* _game;

public:
	MasterMind(Game * engine);
	~MasterMind();

	/// Method that help create the new game in FtA way
	void newGameHelper(int diff, GeoscapeState* gs);

	/// Process event script from different sources
	void eventScriptProcessor(std::vector<std::string> scripts, ProcessorSource source);
	/// Spawn the alien mission with given parameters.
	bool spawnAlienMission(const std::string& missionName, const Globe& globe, Base* base = nullptr);
	/// Loyalty update handler
	int updateLoyalty(int score, LoyaltySource source = XCOM_GEOSCAPE);
	/// Return bonus that would be applied to base service performance because of loyalty level.
	int getLoyaltyPerformanceBonus();
	/// Update reputations level based on its current reputation score.
	bool updateReputationLvl(DiplomacyFaction* faction, bool initial = false);

};

}

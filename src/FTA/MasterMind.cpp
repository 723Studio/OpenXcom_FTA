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

#include "MasterMind.h"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <climits>
#include <functional>
#include "../Engine/RNG.h"
#include "../Engine/Game.h"
#include "../Engine/Logger.h"
#include "../Mod/Mod.h"
#include "../Geoscape/Globe.h"
#include "../Savegame/GameTime.h"
#include "../Savegame/SavedGame.h"
#include "../Savegame/Base.h"
#include "../Savegame/BaseFacility.h"
#include "../Mod/RuleBaseFacility.h"
#include "../Mod/RuleArcScript.h"
#include "../Mod/RuleEventScript.h"
#include "../Mod/RuleEvent.h"
#include "../Mod/RuleMissionScript.h"
#include "../Savegame/DiplomacyFaction.h"
#include "../Savegame/CovertOperation.h"
#include "../Savegame/Soldier.h"
#include "../Savegame/SoldierDiary.h"
#include "../FTA/DiplomacyStartState.h"
#include "../Savegame/ResearchProject.h"
#include "../Mod/RuleResearch.h"
#include "../Savegame/Production.h"
#include "../Mod/RuleManufacture.h"
#include "../Savegame/ItemContainer.h"
#include "../Savegame/MissionSite.h"
#include "../Savegame/AlienBase.h"
#include "../Mod/RuleRegion.h"
#include "../Savegame/Region.h"
#include "../Savegame/Country.h"
#include "../Mod/RuleCountry.h"
#include "../Mod/RuleAlienMission.h"
#include "../Savegame/AlienStrategy.h"
#include "../Savegame/AlienMission.h"
#include "../Savegame/GeoscapeEvent.h"
#include "../Geoscape/GeoscapeEventState.h"
#include "../Savegame/SavedBattleGame.h"
#include "../Battlescape/BattlescapeGenerator.h"
#include "../Battlescape/BriefingState.h"
#include "../Mod/RuleGlobe.h"
#include "../Engine/Exception.h"
#include "../Mod/AlienDeployment.h"
#include "../fmath.h"
#include "../fallthrough.h"

namespace OpenXcom
{

MasterMind::MasterMind(Game* engine): _game(engine)
{
	
}
MasterMind::~MasterMind()
{
}

/**
* Handle request for generation of Geoscape Event.
* @param eventName - string with rules name of the event.
* @return true is event was generater successfully.
*/
bool MasterMind::spawnEvent(std::string name)
{
	RuleEvent* eventRules = _game->getMod()->getEvent(name);
	if (eventRules == 0)
	{
		throw Exception("Error processing spawning of event: " + name + ", no such rules defined!");
		return false;
	}
	GeoscapeEvent* newEvent = new GeoscapeEvent(*eventRules);
	int minutes = (eventRules->getTimer() + (RNG::generate(0, eventRules->getTimerRandom()))) / 30 * 30;
	if (minutes < 30) minutes = 30; // just in case
	newEvent->setSpawnCountdown(minutes);
	_game->getSavedGame()->getGeoscapeEvents().push_back(newEvent);
	// remember that it has been generated
	_game->getSavedGame()->addGeneratedEvent(eventRules);

	return true;
}

void MasterMind::updateLoyalty(int score, LoyaltySource source)
{
	int coef = 100;
	switch (source)
	{
	case OpenXcom::XCOM_BATTLESCAPE:
		coef = _game->getMod()->getCoefBattlescape();
		break;
	case OpenXcom::XCOM_DOGFIGHT:
		coef = _game->getMod()->getCoefDogfight();
		break;
	case OpenXcom::XCOM_GEOSCAPE:
		coef = _game->getMod()->getCoefGeoscape();
		break;
	case OpenXcom::XCOM_RESEARCH:
		coef = _game->getMod()->getCoefResearch();
		break;
	case OpenXcom::ALIEN_MISSION_DESPAWN:
		coef = _game->getMod()->getCoefAlienMission();
		break;
	case OpenXcom::ALIEN_UFO_ACTIVITY:
		coef = _game->getMod()->getCoefUfo();
		break;
	case OpenXcom::ALIEN_BASE:
		coef = _game->getMod()->getCoefAlienBase();
		break;
	default:
		break;
	}
	int loyalty = _game->getSavedGame()->getLoyalty();
	loyalty += std::round((score * coef) / 100);
	Log(LOG_DEBUG) << "Loyalty updating to:  " << loyalty << " from coef: " << coef << " and scope value: " << score; //FtATODO remove for next release
	_game->getSavedGame()->setLoyalty(loyalty);
}



}

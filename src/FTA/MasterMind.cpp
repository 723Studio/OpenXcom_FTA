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
#include "../Engine/Exception.h"
#include "../Engine/State.h"
#include "../Engine/LocalizedText.h"
#include "../Engine/Language.h"
#include "../Mod/Mod.h"
#include "../Mod/RuleBaseFacility.h"
#include "../Mod/RuleArcScript.h"
#include "../Mod/RuleEventScript.h"
#include "../Mod/RuleEvent.h"
#include "../Mod/RuleMissionScript.h"
#include "../Mod/RuleResearch.h"
#include "../Mod/RuleRegion.h"
#include "../Mod/RuleCountry.h"
#include "../Mod/RuleAlienMission.h"
#include "../Mod/RuleGlobe.h"
#include "../Mod/AlienDeployment.h"
#include "../Mod/RuleManufacture.h"
#include "../Mod/RuleDiplomacyFaction.h"
#include "../Savegame/GameTime.h"
#include "../Savegame/SavedGame.h"
#include "../Savegame/Base.h"
#include "../Savegame/BaseFacility.h"
#include "../Savegame/DiplomacyFaction.h"
#include "../Savegame/CovertOperation.h"
#include "../Savegame/Soldier.h"
#include "../Savegame/SoldierDiary.h"
#include "../Savegame/ResearchProject.h"
#include "../Savegame/Production.h"
#include "../Savegame/MissionSite.h"
#include "../Savegame/AlienBase.h"
#include "../Savegame/Region.h"
#include "../Savegame/Country.h"
#include "../Savegame/AlienStrategy.h"
#include "../Savegame/AlienMission.h"
#include "../Savegame/SavedBattleGame.h"
#include "../Savegame/GeoscapeEvent.h"
#include "../Geoscape/GeoscapeState.h"
#include "../Geoscape/Globe.h"
#include "../Battlescape/BattlescapeGenerator.h"
#include "../Battlescape/BriefingState.h"
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
* Handle additional operations and functions .
* @param eventName - string with rules name of the event.
* @return true is event was generater successfully.
*/
void MasterMind::newGameHelper(int diff, GeoscapeState* gs)
{
	SavedGame* save = _game->getSavedGame();
	Mod* mod = _game->getMod();
	Base* base = save->getBases()->at(0);
	double lon, lat;
	lon = RNG::generate(0.20, 0.22); //TODO random array here
	lat = RNG::generate(-0.832, -0.87) ; //TODO random array here
	base->setLongitude(lon);
	base->setLatitude(lat);
	std::string baseName = _game->getLanguage()->getString("STR_LAST_STAND"); //TODO random array here
	base->setName(baseName);
	gs->getGlobe()->center(lon, lat);

	for (std::vector<Craft*>::iterator i = base->getCrafts()->begin(); i != base->getCrafts()->end(); ++i)
	{
		(*i)->setLongitude(lon);
		(*i)->setLatitude(lat);
	}
	//spawn regional ADVENT center
	AlienDeployment* aBaseDeployment = mod->getDeployment("STR_INITIAL_REGIONAL_HQ");
	AlienBase* aBase = new AlienBase(aBaseDeployment, 0);
	aBase->setId(save->getId(aBaseDeployment->getMarkerName()));
	aBase->setAlienRace(aBaseDeployment->getRace());
	aBase->setLongitude(lon + RNG::generate(0.20, 0.26)); //TODO random array here
	aBase->setLatitude(lat - RNG::generate(0.04, 0.06)); //TODO random array here
	aBase->setDiscovered(false);
	save->getAlienBases()->push_back(aBase);

	//init the Game
	gs->init();

	//init Factions
	for (std::vector<std::string>::const_iterator i = mod->getDiplomacyFactionList()->begin(); i != mod->getDiplomacyFactionList()->end(); ++i)
	{
		RuleDiplomacyFaction* factionRules = mod->getDiplomacyFaction(*i);
		DiplomacyFaction* faction = new DiplomacyFaction(*factionRules);
		if (factionRules->getDiscoverResearch().empty() || save->isResearched(mod->getResearch(factionRules->getDiscoverResearch())))
		{
			faction->setDiscovered(true);
		}
		faction->setReputationScore(factionRules->getStartingReputation());
		updateReputationLvl(faction);
		save->getDiplomacyFactions().push_back(faction);
	}
	//adjust funding
	int funds = mod->getInitialFunding();
	funds = funds * 1000 + static_cast<int>(RNG::generate(-1258, 6365)); 
	save->setFunds(funds);

	//start base defense mission
	SavedBattleGame* bgame = new SavedBattleGame(mod, _game->getLanguage());
	_game->getSavedGame()->setBattleGame(bgame);
	bgame->setMissionType("STR_BASE_DEFENSE");
	BattlescapeGenerator bgen = BattlescapeGenerator(_game);
	bgen.setBase(base);
	bgen.setAlienCustomDeploy(mod->getDeployment("STR_INITIAL_BASE_DEFENSE"));
	bgen.setWorldShade(0);
	bgen.run();
	_game->pushState(new BriefingState(0, base));
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

/**
* Handle updating of faction reputation level based on current score, rules and other conditions.
* @param faction - DiplomacyFaction we are updating.
* @paran initial - to define if we are performing updating of reputation due to new game initialisation (default false).
* @return true if reputation level was updated.
*/
bool MasterMind::updateReputationLvl(DiplomacyFaction* faction, bool initial)
{
	int repScore = faction->getReputationScore();
	int curLvl = faction->getReputationLevel();
	bool changed = false;
	std::string repName = "STR_NEUTRAL";
	int newLvl = 3; //STR_NEUTRAL is default resolve

	const std::map<int, std::string>* repLevels = _game->getMod()->getReputationLevels();
	if (repLevels)
	{
		int temp = INT_MIN;
		for (auto& i : *repLevels)
		{
			if (i.first > temp && i.first <= repScore)
			{
				temp = i.first;
				repName = i.second;
			}
		}

		if (repName == "STR_ALLY") newLvl = 6;
		else if (repName == "STR_HONORED") newLvl = 5;
		else if (repName == "STR_FRIENDLY") newLvl = 4;
		else if (repName == "STR_UNFRIENDLY") newLvl = 2;
		else if (repName == "STR_HOSTILE") newLvl = 1;
		else if (repName == "STR_HATED") newLvl = 0;

		if (curLvl != newLvl)
		{
			if (!faction->isThisMonthDiscovered() || initial)
			{
				changed = true;
				faction->setReputationLevel(newLvl);
				faction->setReputationName(repName);
				//FINNIKTODO add cross-factional relations
			}
		}
	}

	return changed;
}

}

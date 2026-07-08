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
#include "NewGameState.h"
#include "../Engine/Game.h"
#include "../Mod/Mod.h"
#include "../Mod/AlienDeployment.h"
#include "../Mod/RuleDiplomacyFaction.h"
#include "../Interface/TextButton.h"
#include "../Interface/ToggleTextButton.h"
#include "../Interface/Window.h"
#include "../Interface/Text.h"
#include "../Geoscape/GeoscapeState.h"
#include "../Geoscape/Globe.h"
#include "../Engine/Options.h"
#include "../Engine/RNG.h"
#include "../Engine/Exception.h"
#include "../Savegame/SavedGame.h"
#include "../Savegame/Base.h"
#include "../Savegame/AlienBase.h"
#include "../Savegame/DiplomacyFaction.h"
#include "../Savegame/ItemContainer.h"
#include "../Savegame/SavedBattleGame.h"
#include "../Savegame/SoldierPool.h"
#include "../Battlescape/BattlescapeGenerator.h"
#include "../Battlescape/BriefingState.h"

#include <vector>

namespace OpenXcom
{

/**
 * Initializes all the elements in the Difficulty window.
 * @param game Pointer to the core game.
 */
NewGameState::NewGameState()
{
	// Create objects
	_window = new Window(this, 192, 180, 64, 10, POPUP_VERTICAL);
	_btnBeginner = new TextButton(160, 18, 80, 32);
	_btnExperienced = new TextButton(160, 18, 80, 52);
	_btnVeteran = new TextButton(160, 18, 80, 72);
	_btnGenius = new TextButton(160, 18, 80, 92);
	_btnSuperhuman = new TextButton(160, 18, 80, 112);
	_btnIronman = new ToggleTextButton(78, 18, 80, 138);
	_btnOk = new TextButton(78, 16, 80, 164);
	_btnCancel = new TextButton(78, 16, 162, 164);
	_txtTitle = new Text(192, 9, 64, 20);
	_txtIronman = new Text(90, 24, 162, 135);

	switch (_game->getMod()->getStartingDifficulty())
	{
	case 0:
		_difficulty = _btnBeginner;
		break;
	case 1:
		_difficulty = _btnExperienced;
		break;
	case 2:
		_difficulty = _btnVeteran;
		break;
	case 3:
		_difficulty = _btnGenius;
		break;
	case 4:
		_difficulty = _btnSuperhuman;
		break;
	default:
		_difficulty = _btnBeginner;
		break;
	}

	// Set palette
	setInterface("newGameMenu");

	add(_window, "window", "newGameMenu");
	add(_btnBeginner, "button", "newGameMenu");
	add(_btnExperienced, "button", "newGameMenu");
	add(_btnVeteran, "button", "newGameMenu");
	add(_btnGenius, "button", "newGameMenu");
	add(_btnSuperhuman, "button", "newGameMenu");
	add(_btnIronman, "ironman", "newGameMenu");
	add(_btnOk, "button", "newGameMenu");
	add(_btnCancel, "button", "newGameMenu");
	add(_txtTitle, "text", "newGameMenu");
	add(_txtIronman, "ironman", "newGameMenu");

	centerAllSurfaces();

	// Set up objects
	setWindowBackground(_window, "newGameMenu");

	_btnBeginner->setText(tr("STR_1_BEGINNER"));
	_btnBeginner->setGroup(&_difficulty);

	_btnExperienced->setText(tr("STR_2_EXPERIENCED"));
	_btnExperienced->setGroup(&_difficulty);

	_btnVeteran->setText(tr("STR_3_VETERAN"));
	_btnVeteran->setGroup(&_difficulty);

	_btnGenius->setText(tr("STR_4_GENIUS"));
	_btnGenius->setGroup(&_difficulty);

	_btnSuperhuman->setText(tr("STR_5_SUPERHUMAN"));
	_btnSuperhuman->setGroup(&_difficulty);

	_btnIronman->setText(tr("STR_IRONMAN"));

	_btnOk->setText(tr("STR_OK"));
	_btnOk->onMouseClick((ActionHandler)&NewGameState::btnOkClick);
	_btnOk->onKeyboardPress((ActionHandler)&NewGameState::btnOkClick, Options::keyOk);

	_btnCancel->setText(tr("STR_CANCEL"));
	_btnCancel->onMouseClick((ActionHandler)&NewGameState::btnCancelClick);
	_btnCancel->onKeyboardPress((ActionHandler)&NewGameState::btnCancelClick, Options::keyCancel);

	_txtTitle->setAlign(ALIGN_CENTER);
	_txtTitle->setText(tr("STR_SELECT_DIFFICULTY_LEVEL"));

	_txtIronman->setWordWrap(true);
	_txtIronman->setVerticalAlign(ALIGN_MIDDLE);
	_txtIronman->setText(tr("STR_IRONMAN_DESC"));

	_btnGenius->setVisible(false);
	_btnSuperhuman->setY(_btnGenius->getY());
	if (!_game->getMod()->getIsIronManEnabled())
	{
		_btnIronman->setVisible(false);
		_txtIronman->setText(tr("STR_IRONMAN_ALPHA_DESC"));
	}
	else
	{
		_btnSuperhuman->setVisible(false);
	}
}

/**
 *
 */
NewGameState::~NewGameState()
{

}

/**
 * Returns to the previous screen.
 * @param action Pointer to an action.
 */
void NewGameState::btnOkClick(Action *)
{
	GameDifficulty diff = DIFF_BEGINNER;
	if (_difficulty == _btnBeginner)
	{
		diff = DIFF_BEGINNER;
	}
	else if (_difficulty == _btnExperienced)
	{
		diff = DIFF_EXPERIENCED;
	}
	else if (_difficulty == _btnVeteran)
	{
		diff = DIFF_VETERAN;
	}
	else if (_difficulty == _btnGenius)
	{
		diff = DIFF_GENIUS;
	}
	else if (_difficulty == _btnSuperhuman)
	{
		diff = DIFF_SUPERHUMAN;
	}

	// Reset touch flags
	_game->resetTouchButtonFlags();

	SavedGame *save = _game->getMod()->newSave(diff);
	save->setDifficulty(diff);
	save->setIronman(_btnIronman->getPressed());
	_game->setSavedGame(save);
	save->setGamePtr(_game);

	GeoscapeState* gs = new GeoscapeState;
	_game->setState(gs);

	initFtaNewGame(gs);
}

/**
 * Initializes the FTA campaign start after the save and geoscape state exist.
 * @param gs Pointer to the geoscape state.
 */
void NewGameState::initFtaNewGame(GeoscapeState *gs)
{
	SavedGame* save = _game->getSavedGame();
	Mod* mod = _game->getMod();
	Base* base = save->getBases()->at(0);
	double lon, lat;
	lon = RNG::generate(0.20, 0.22); //#FINNIKTODO random array here
	lat = RNG::generate(-0.832, -0.87) ; //#FINNIKTODO random array here
	base->setLongitude(lon);
	base->setLatitude(lat);
	std::string baseName = tr("STR_LAST_STAND"); //#FINNIKTODO random array here
	base->setName(baseName);
	base->calculateServices(save);
	gs->getGlobe()->center(lon, lat);

	for (auto& c : *base->getCrafts())
	{
		c->setLongitude(lon);
		c->setLatitude(lat);
	}

	//spawn regional MIB HQ
	AlienDeployment* aBaseDeployment = mod->getDeployment("STR_INITIAL_REGIONAL_HQ");
	AlienBase* aBase = new AlienBase(aBaseDeployment, 0);
	aBase->setId(save->getId(aBaseDeployment->getMarkerName()));
	aBase->setAlienRace(aBaseDeployment->getRace());
	aBase->setLongitude(lon + RNG::generate(0.20, 0.26)); //#FINNIKTODO random array here
	aBase->setLatitude(lat - RNG::generate(0.04, 0.06)); //#FINNIKTODO random array here
	aBase->setDiscovered(false);
	save->getAlienBases()->push_back(aBase);

	//init the Game
	gs->init();

	//init Factions
	for (std::vector<std::string>::const_iterator i = mod->getDiplomacyFactionList()->begin(); i != mod->getDiplomacyFactionList()->end(); ++i)
	{
		RuleDiplomacyFaction* factionRules = mod->getDiplomacyFaction(*i);
		DiplomacyFaction* faction = new DiplomacyFaction(mod, factionRules->getName());

		if (factionRules->getDiscoverResearch().empty() || save->isResearched(mod->getResearch(factionRules->getDiscoverResearch())))
		{
			faction->setDiscovered(true);
		}

		// set up starting values
		faction->setReputationScore(factionRules->getStartingReputation());
		faction->updateReputationLvl(*mod, true);
		faction->setFunds(factionRules->getStartingFunds());
		faction->setPower(factionRules->getStartingPower()); //we always start with 0 vigilance
		for (auto research : factionRules->getStartingResearches())
		{
			faction->unlockResearch(research);
		}

		// populate faction item stores and staff
		auto items = faction->getItems();
		for (auto &item : factionRules->getStartingItems())
		{
			if (RuleItem* itemRule = mod->getItem(item.first))
			{
				items->addItem(itemRule, item.second);
			}
			else
			{
				throw Exception("Error in FTA game initialization process: fails to add item " + item.first + " for faction " + factionRules->getName() +
					" ; no item ruleset defined!");
			}
		}

		std::vector<std::string> soldierTypes;
		for (auto &s : factionRules->getStartingStaff())
		{
			for (int i = 0; i < s.second; ++i)
			{
				soldierTypes.push_back(s.first);
			}
		}

		// Generate soldiers
		for (auto& soldierType : soldierTypes)
		{
			const RuleSoldier* ruleSoldier = mod->getSoldier(soldierType, true);
			faction->getStaffPool()->createSoldier(ruleSoldier, mod, save);
		}

		// finish faction initialization process
		save->getDiplomacyFactions().push_back(faction);
	}

	//adjust funding
	int funds = mod->getInitialFunding();
	funds = funds * 1000 + RNG::generate(-1258, 6365);
	save->setFunds(funds);

	//start base defense mission
	SavedBattleGame* bgame = new SavedBattleGame(mod, _game->getLanguage());
	save->setBattleGame(bgame);
	bgame->setMissionType("STR_BASE_DEFENSE");
	BattlescapeGenerator bgen = BattlescapeGenerator(_game);
	bgen.setBase(base);
	bgen.setAlienCustomDeploy(mod->getDeployment("STR_INITIAL_BASE_DEFENSE"));
	bgen.setWorldShade(0);
	bgen.run();
	_game->pushState(new BriefingState(0, base));
}

/**
 * Returns to the previous screen.
 * @param action Pointer to an action.
 */
void NewGameState::btnCancelClick(Action *)
{
	_game->setSavedGame(0);
	_game->popState();
}

}

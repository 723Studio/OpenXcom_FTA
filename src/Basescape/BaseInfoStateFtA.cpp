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
#include "BaseInfoStateFtA.h"
#include <sstream>
#include <cmath>
#include "../Engine/Game.h"
#include "../Engine/Action.h"
#include "../Mod/Mod.h"
#include "../Engine/Options.h"
#include "../Interface/Bar.h"
#include "../Interface/TextButton.h"
#include "../Interface/Text.h"
#include "../Interface/TextEdit.h"
#include "../Engine/Surface.h"
#include "MiniBaseView.h"
#include "../Savegame/SavedGame.h"
#include "../Savegame/Base.h"
#include "../Savegame/Soldier.h"
#include "MonthlyCostsState.h"
#include "TransfersState.h"
#include "StoresState.h"
#include "DisposeState.h"
#include "BasescapeState.h"

namespace OpenXcom
{

/**
 * Initializes all the elements in the Base Info screen for FtA game.
 * @param game Pointer to the core game.
 * @param base Pointer to the base to get info from.
 * @param state Pointer to the Basescape state.
 */
BaseInfoStateFtA::BaseInfoStateFtA(Base *base, BasescapeState *state) : _base(base), _state(state)
{

	// Create objects
	_bg = new Surface(320, 200, 0, 0);
	_mini = new MiniBaseView(128, 16, 182, 7);
	_btnOk = new TextButton(21, 14, 8, 180);
	_btnTransfers = new TextButton(63, 14, 31, 180);
	_btnStores = new TextButton(50, 14, 96, 180);
	_btnDispose = new TextButton(85, 14, 148, 180);
	_btnMonthlyCosts = new TextButton(77, 14, 235, 180);
	_edtBase = new TextEdit(this, 167, 16, 8, 7);
	int y = 28;
	int dY = 10;
	_txtPersonnel = new Text(300, 9, 8, y);
	y += dY;
	_txtSoldiers = new Text(112, 9, 8, y);
	_numSoldiers = new Text(37, 9, 126, y);
	_barSoldiers = new Bar(150, 5, 166, y + 2); //bar is always 2 pixel lower
	y += dY;
	_txtQuarters = new Text(112, 9, 8, y);
	_numQuarters = new Text(112, 9, 126, y);
	_barQuarters = new Bar(112, 5, 166, y + 2);
	y += dY;
	_txtStores = new Text(112, 9, 8, y);
	_numStores = new Text(37, 9, 126, y);
	_barStores = new Bar(150, 5, 166, y + 2);
	y += dY;
	_txtLaboratories = new Text(112, 9, 8, y);
	_numLaboratories = new Text(112, 9, 126, y);
	_barLaboratories = new Bar(112, 5, 166, y + 2);
	y += dY;
	_txtWorkshops = new Text(112, 9, 8, y);
	_numWorkshops = new Text(37, 9, 126, y);
	_barWorkshops = new Bar(150, 5, 166, y + 2);
	y += dY;
	_txtGym = new Text(112, 9, 8, y);
	_numGym = new Text(112, 9, 126, y);
	_barGym = new Bar(112, 5, 166, y + 2);
	y += dY;
	_txtInterrogation = new Text(112, 9, 8, y);
	_numInterrogation = new Text(37, 9, 126, y);
	_barInterrogation = new Bar(150, 5, 166, y + 2);
	y += dY;
	_txtPrison = new Text(112, 9, 8, y);
	_numPrison = new Text(112, 9, 126, y);
	_barPrison = new Bar(112, 5, 166, y + 2);
	y += dY;
	_txtMonsters = new Text(112, 9, 8, y);
	_numMonsters = new Text(37, 9, 126, y);
	_barMonsters = new Bar(150, 5, 166, y + 2);
	y += dY;
	_txtContainment = new Text(112, 9, 8, y);
	_numContainment = new Text(112, 9, 126, y);
	_barContainment = new Bar(112, 5, 166, y + 2);
	y += dY;
	_txtPower = new Text(112, 9, 8, y);
	_numPower = new Text(37, 9, 126, y);
	_barPower = new Bar(150, 5, 166, y + 2);
	y += dY;
	_txtDefense = new Text(112, 9, 8, y);
	_numDefense = new Text(112, 9, 126, y);
	_barDefense = new Bar(112, 5, 166, y + 2);
	y += dY;

	_txtRadar = new Text(112, 9, 8, y);
	_numRadar = new Text(37, 9, 126, y);
	_barRadar = new Bar(150, 5, 166, y + 2);
	y += dY;
	_txtGlobalDetection = new Text(112, 9, 8, y);
	_numGlobalDetection = new Text(112, 9, 126, y);
	_barGlobalDetection = new Bar(112, 5, 166, y + 2);
	y += dY;

	// Set palette
	setStandardPalette("PAL_BASE_INFO");
	setInterface("baseInfoFta");

	add(_bg);
	add(_mini, "miniBase", "baseInfoFta");
	add(_btnOk, "button", "baseInfoFta");
	add(_btnTransfers, "button", "baseInfoFta");
	add(_btnStores, "button", "baseInfoFta");
	add(_btnDispose, "button", "baseInfoFta");
	add(_btnMonthlyCosts, "button", "baseInfoFta");
	add(_edtBase, "text1", "baseInfoFta");

	add(_txtPersonnel, "text1", "baseInfoFta");
	add(_txtSoldiers, "text2", "baseInfoFta");
	add(_numSoldiers, "numbers", "baseInfoFta");
	add(_barSoldiers, "personnelBar", "baseInfoFta");

	add(_txtQuarters, "text2", "baseInfoFta");
	add(_numQuarters, "numbers", "baseInfoFta");
	add(_barQuarters, "facilityBars", "baseInfoFta");

	add(_txtStores, "text2", "baseInfoFta");
	add(_numStores, "numbers", "baseInfoFta");
	add(_barStores, "facilityBars", "baseInfoFta");

	add(_txtLaboratories, "text2", "baseInfoFta");
	add(_numLaboratories, "numbers", "baseInfoFta");
	add(_barLaboratories, "facilityBars", "baseInfoFta");

	add(_txtWorkshops, "text2", "baseInfoFta");
	add(_numWorkshops, "numbers", "baseInfoFta");
	add(_barWorkshops, "facilityBars", "baseInfoFta");

	add(_txtGym, "text2", "baseInfoFta");
	add(_numGym, "numbers", "baseInfoFta");
	add(_barGym, "facilityBars", "baseInfoFta");

	add(_txtInterrogation, "text2", "baseInfoFta");
	add(_numInterrogation, "numbers", "baseInfoFta");
	add(_barInterrogation, "facilityBars", "baseInfoFta");

	add(_txtPrison, "text2", "baseInfoFta");
	add(_numPrison, "numbers", "baseInfoFta");
	add(_barPrison, "prisonBar", "baseInfoFta");

	add(_txtMonsters, "text2", "baseInfoFta");
	add(_numMonsters, "numbers", "baseInfoFta");
	add(_barMonsters, "monstersBar", "baseInfoFta");

	add(_txtContainment, "text2", "baseInfoFta");
	add(_numContainment, "numbers", "baseInfoFta");
	add(_barContainment, "containmentBar", "baseInfoFta");

	add(_txtPower, "text2", "baseInfoFta");
	add(_numPower, "numbers", "baseInfoFta");
	add(_barPower, "powerBar", "baseInfoFta");

	add(_txtDefense, "text2", "baseInfoFta");
	add(_numDefense, "numbers", "baseInfoFta");
	add(_barDefense, "defenceBar", "baseInfoFta");

	add(_txtRadar, "text2", "baseInfoFta");
	add(_numRadar, "numbers", "baseInfoFta");
	add(_barRadar, "radarBar", "baseInfoFta");

	add(_txtGlobalDetection, "text2", "baseInfoFta");
	add(_numGlobalDetection, "numbers", "baseInfoFta");
	add(_barGlobalDetection, "detectionBar", "baseInfoFta");

	centerAllSurfaces();

	// Set up objects
	_game->getMod()->getSurface("BaseInfoScreen")->blitNShade(_bg, 0, 0);

	_mini->setTexture(_game->getMod()->getSurfaceSet("BaseInfoMinibaseTile"));
	_mini->setBases(_game->getSavedGame()->getBases());
	for (size_t i = 0; i < _game->getSavedGame()->getBases()->size(); ++i)
	{
		if (_game->getSavedGame()->getBases()->at(i) == _base)
		{
			_mini->setSelectedBase(i);
			break;
		}
	}
	_mini->onMouseClick((ActionHandler)&BaseInfoStateFtA::miniClick);
	_mini->onKeyboardPress((ActionHandler)&BaseInfoStateFtA::handleKeyPress);

	_btnOk->setText(tr("STR_OK"));
	_btnOk->onMouseClick((ActionHandler)&BaseInfoStateFtA::btnOkClick);
	_btnOk->onKeyboardPress((ActionHandler)&BaseInfoStateFtA::btnOkClick, Options::keyOk);
	_btnOk->onKeyboardPress((ActionHandler)&BaseInfoStateFtA::btnOkClick, Options::keyCancel);

	_btnTransfers->setText(tr("STR_TRANSFERS_LC"));
	_btnTransfers->onMouseClick((ActionHandler)&BaseInfoStateFtA::btnTransfersClick);

	_btnStores->setText(tr("STR_STORES_LC"));
	_btnStores->onMouseClick((ActionHandler)&BaseInfoStateFtA::btnStoresClick);

	_btnDispose->setText(tr("STR_DISPOSE_DISMISS_LC"));
	_btnDispose->onMouseClick((ActionHandler)&BaseInfoStateFtA::btnDisposeClick);

	_btnMonthlyCosts->setText(tr("STR_MONTHLY_COSTS"));
	_btnMonthlyCosts->onMouseClick((ActionHandler)&BaseInfoStateFtA::btnMonthlyCostsClick);

	_edtBase->setBig();
	_edtBase->onChange((ActionHandler)&BaseInfoStateFtA::edtBaseChange);

	_txtPersonnel->setText(tr("STR_BASE_INFO_HEADER"));

	_txtSoldiers->setText(tr("STR_PERSONNEL"));
	_barSoldiers->setScale(1.0);

	_txtQuarters->setText(tr("STR_LIVING_QUARTERS_PLURAL"));
	_barQuarters->setScale(1.0);

	_txtStores->setText(tr("STR_STORES"));
	_barStores->setScale(0.2);

	_txtLaboratories->setText(tr("STR_LABORATORIES"));
	_barLaboratories->setScale(1);

	_txtWorkshops->setText(tr("STR_WORK_SHOPS"));
	_barWorkshops->setScale(1);

	_txtGym->setText(tr("STR_GYM"));
	_barGym->setScale(2.5);

	_txtInterrogation->setText(tr("STR_INTERROGATION_SPACE"));
	_barInterrogation->setScale(2.5);

	_txtPrison->setText(tr("STR_PRISON_CELLS"));
	_barPrison->setScale(2.5);

	_txtMonsters->setText(tr("STR_CREATURES_CONTAINMENT"));
	_barMonsters->setScale(2.5);

	_txtContainment->setText(tr("STR_ALIEN_CONTAINMENT"));
	_barContainment->setScale(2.5);

	_txtPower->setText(tr("STR_POWER_GRID"));
	_barPower->setScale(1);

	_txtDefense->setText(tr("STR_DEFENSE_STRENGTH"));
	_barDefense->setScale(0.025);

	_txtRadar->setText(tr("STR_RADAR_STREINGH"));
	_barRadar->setScale(0.125);

	_txtGlobalDetection->setText(tr("STR_GLOBAL_DETECTION"));
	_barGlobalDetection->setScale(18.0);

	if (!_game->getSavedGame()->isResearched(_game->getMod()->getResearch("STR_CREATURES_CONTAINMENT")))
	{
		_txtMonsters->setVisible(false);
		_numMonsters->setVisible(false);
		_barMonsters->setVisible(false);
	}
	if (!_game->getSavedGame()->isResearched(_game->getMod()->getResearch("STR_ALIEN_CONTAINMENT")))
	{
		_txtContainment->setVisible(false);
		_numContainment->setVisible(false);
		_barContainment->setVisible(false);
	}
}

BaseInfoStateFtA::~BaseInfoStateFtA()
{
}

/**
 * The player can change the selected base.
 */
void BaseInfoStateFtA::init()
{
	State::init();
	_edtBase->setText(_base->getName());

	int freeSoldiers = 0;
	int size = _base->getSoldiers()->size();
	auto recovery = _base->getSumRecoveryPerDay();
	bool isBusy = false, isFree = false;
	for (std::vector<Soldier*>::iterator s = _base->getSoldiers()->begin(); s != _base->getSoldiers()->end(); ++s)
	{
		(*s)->getCurrentDuty(_game->getLanguage(), recovery, isBusy, isFree);
		if (isFree)
		{
			freeSoldiers++;
		}
	}
	std::ostringstream ss;
	ss << size - freeSoldiers << ":" << size;
	_numSoldiers->setText(ss.str());
	_barSoldiers->setMax(size);
	_barSoldiers->setValue(size - freeSoldiers);

	std::ostringstream ss2;
	ss2 << _base->getUsedQuarters() << ":" << _base->getAvailableQuarters();
	_numQuarters->setText(ss2.str());
	_barQuarters->setMax(_base->getAvailableQuarters());
	_barQuarters->setValue(_base->getUsedQuarters());

	std::ostringstream ss3;
	ss3 << (int)floor(_base->getUsedStores() + 0.05) << ":" << _base->getAvailableStores();
	_numStores->setText(ss3.str());
	_barStores->setMax(_base->getAvailableStores());
	_barStores->setValue((int)floor(_base->getUsedStores() + 0.05));

	std::ostringstream ss4;
	ss4 << _base->getUsedLaboratories(true) << ":" << _base->getAvailableLaboratories();
	_numLaboratories->setText(ss4.str());
	_barLaboratories->setMax(_base->getAvailableLaboratories());
	_barLaboratories->setValue(_base->getUsedLaboratories());

	std::ostringstream ss5;
	ss5 << _base->getUsedWorkshops(true) << ":" << _base->getAvailableWorkshops();
	_numWorkshops->setText(ss5.str());
	_barWorkshops->setMax(_base->getAvailableWorkshops());
	_barWorkshops->setValue(_base->getUsedWorkshops());

	std::ostringstream ss6;
	ss6 << _base->getUsedTraining() << ":" << _base->getAvailableTraining();
	_numGym->setText(ss6.str());
	_barGym->setMax(_base->getAvailableTraining());
	_barGym->setValue(_base->getUsedTraining());

	std::ostringstream ss7;
	ss7 << _base->getUsedInterrogationSpace() << ":" << _base->getAvailableInterrogationSpace();
	_numInterrogation->setText(ss7.str());
	_barInterrogation->setMax(_base->getAvailableInterrogationSpace());
	_barInterrogation->setValue(_base->getUsedInterrogationSpace());

	std::ostringstream ss8;
	ss8 << _base->getUsedPrisonSpace() << ":" << _base->getAvailablePrisonSpace();
	_numPrison->setText(ss8.str());
	_barPrison->setMax(_base->getAvailablePrisonSpace());
	_barPrison->setValue(_base->getUsedPrisonSpace());

	std::ostringstream ss9; //monsters use containment type 1
	ss9 << _base->getUsedContainment(1) << ":" << _base->getAvailableContainment(1);
	_numMonsters->setText(ss9.str());
	_barMonsters->setMax(_base->getAvailableContainment(1));
	_barMonsters->setValue(_base->getUsedContainment(1));

	std::ostringstream ss10; //aliens use containment type 0
	ss10 << _base->getUsedContainment(0) << ":" << _base->getAvailableContainment(0);
	_numContainment->setText(ss10.str());
	_barContainment->setMax(_base->getAvailableContainment(0));
	_barContainment->setValue(_base->getUsedContainment(0));

	//#FINNIKTODO power generation here https://github.com/723Studio/OpenXcom_FTA/issues/177
	//std::ostringstream ss11;
	//_numPower->setText(ss11.str());
	//_barPower->setMax(value);
	//_barPower->setValue(value2);
	_txtPower->setVisible(false);
	_numPower->setVisible(false);
	_barPower->setVisible(false);

	std::ostringstream ss12;
	ss12 << _base->getDefenseValue();
	_numDefense->setText(ss12.str());
	_barDefense->setMax(_base->getDefenseValue());
	_barDefense->setValue(_base->getDefenseValue());

	std::ostringstream ss13;
	int i = _base->getRadarStrength();
	ss13 << i;
	_numRadar->setText(ss13.str());
	_barRadar->setMax(i);
	_barRadar->setValue(i);

	std::ostringstream ss14;
	int j = _base->getGlobalRadarStrength();
	ss14 << j;
	_numGlobalDetection->setText(ss14.str());
	_barGlobalDetection->setMax(j);
	_barGlobalDetection->setValue(j);
}

/**
 * Changes the base name.
 * @param action Pointer to an action.
 */
void BaseInfoStateFtA::edtBaseChange(Action *)
{
	_base->setName(_edtBase->getText());
}

/**
 * Selects a new base to display.
 * @param action Pointer to an action.
 */
void BaseInfoStateFtA::miniClick(Action *)
{
	size_t base = _mini->getHoveredBase();
	if (base < _game->getSavedGame()->getBases()->size())
	{
		_mini->setSelectedBase(base);
		_base = _game->getSavedGame()->getBases()->at(base);
		_state->setBase(_base);
		init();
	}
}

/**
 * Selects a new base to display.
 * @param action Pointer to an action.
 */
void BaseInfoStateFtA::handleKeyPress(Action *action)
{
	if (action->getDetails()->type == SDL_KEYDOWN)
	{
		SDLKey baseKeys[] = {
			Options::keyBaseSelect1,
			Options::keyBaseSelect2,
			Options::keyBaseSelect3,
			Options::keyBaseSelect4,
			Options::keyBaseSelect5,
			Options::keyBaseSelect6,
			Options::keyBaseSelect7,
			Options::keyBaseSelect8
		};
		int key = action->getDetails()->key.keysym.sym;
		for (size_t i = 0; i < _game->getSavedGame()->getBases()->size(); ++i)
		{
			if (key == baseKeys[i])
			{
				_mini->setSelectedBase(i);
				_base = _game->getSavedGame()->getBases()->at(i);
				_state->setBase(_base);
				init();
				break;
			}
		}
	}
}

/**
 * Returns to the previous screen.
 * @param action Pointer to an action.
 */
void BaseInfoStateFtA::btnOkClick(Action *)
{
	_game->popState();
}

/**
 * Goes to the Transfers window.
 * @param action Pointer to an action.
 */
void BaseInfoStateFtA::btnTransfersClick(Action *)
{
	_game->pushState(new TransfersState(_base));
}

/**
 * Goes to the Stores screen.
 * @param action Pointer to an action.
 */
void BaseInfoStateFtA::btnStoresClick(Action *)
{
	_game->pushState(new StoresState(_base));
}

/**
 * Goes to the Dispose/Dismiss screen.
 * @param action Pointer to an action.
 */
void BaseInfoStateFtA::btnDisposeClick(Action* action)
{
	_game->pushState(new DisposeState(_base, 0));
}

/**
 * Goes to the Monthly Costs screen.
 * @param action Pointer to an action.
 */
void BaseInfoStateFtA::btnMonthlyCostsClick(Action *)
{
	_game->pushState(new MonthlyCostsState(_base));
}

}

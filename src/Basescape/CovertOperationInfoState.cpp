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
#include "CovertOperationInfoState.h"
#include "../Engine/Action.h"
#include "../Engine/Game.h"
#include "../Mod/Mod.h"
#include "../Mod/RuleCovertOperation.h"
#include "../Engine/LocalizedText.h"
#include "../Engine/Options.h"
#include "../Interface/Window.h"
#include "../Interface/Text.h"
#include "../Interface/TextButton.h"
#include "../Interface/TextList.h"
#include "../Savegame/SavedGame.h"
#include "../Savegame/CovertOperation.h"
#include "../Savegame/Base.h"
#include <algorithm>
#include <unordered_set>

namespace OpenXcom
{
	/**
	 * Initializes all the elements on the UI.
	 */
	CovertOperationInfoState::CovertOperationInfoState(CovertOperation * operation) : _operation(operation)
	{

		// Create objects
		_window = new Window(this, 320, 200, 0, 0);
		_txtTitle = new Text(304, 17, 8, 7);
		_txtScientists = new Text(150, 9, 8, 20);
		_txtEngineers = new Text(150, 9, 8, 30);
		_txtDungeonLevel = new Text(150, 9, 164, 20);
		_txtProgress = new Text(150, 9, 164, 30);
		_txtSoldiers = new Text(150, 9, 8, 40);
		_txtAditionalInfo = new Text(150, 9, 164, 40);
		_lstSoldiers = new TextList(148, 112, 8, 50);
		_lstAditionalInfo = new TextList(148, 112, 164, 50);
		_btnTerminate = new TextButton(148, 16, 8, 176);
		_btnOk = new TextButton(148, 16, 164, 176);

		// Set palette
		setInterface("covertOperationInfoState");

		add(_window, "window", "covertOperationInfoState");
		add(_txtTitle, "text", "covertOperationInfoState");
		add(_btnOk, "button", "covertOperationInfoState");
		add(_btnTerminate, "button", "covertOperationInfoState");
		add(_txtScientists, "text", "covertOperationInfoState");
		add(_txtEngineers, "text", "covertOperationInfoState");
		add(_txtDungeonLevel, "text", "covertOperationInfoState");
		add(_txtProgress, "text", "covertOperationInfoState");
		add(_txtSoldiers, "text", "covertOperationInfoState");
		add(_txtAditionalInfo, "text", "covertOperationInfoState");
		add(_lstSoldiers, "list", "covertOperationInfoState");
		add(_lstAditionalInfo, "list", "covertOperationInfoState");

		centerAllSurfaces();

		// Set up objects
		setWindowBackground(_window, "covertOperationInfoState");

		_txtTitle->setBig();
		_txtTitle->setAlign(ALIGN_CENTER);
		std::string operationName = tr(_operation->getOperationName());
		_txtTitle->setText(operationName);

		_txtScientists->setText(tr("STR_SCIENTISTS").arg(_operation->getAssignedScientists()));
		_txtEngineers->setText(tr("STR_ENGINEERS").arg(_operation->getAssignedEngineers()));
		_txtDungeonLevel->setText(tr("STR_DANGER_LEVEL").arg(_operation->getSuccessChance()));
		_txtProgress->setText(tr("STR_OPERATION_PROGRESS").arg(_operation->getSpent()));

		_txtSoldiers->setText(tr("STR_SOLDIERS"));
		_lstSoldiers->setColumns(1, 148);
		_lstSoldiers->setSelectable(true);
		_lstSoldiers->setBackground(_window);
		_lstSoldiers->setMargin(2);
		_lstSoldiers->setWordWrap(true);
		fillSoldiersList();

		_txtAditionalInfo->setText(tr("STR_ADITIONALINFO"));
		_lstAditionalInfo->setColumns(1, 148);
		_lstAditionalInfo->setSelectable(true);
		_lstAditionalInfo->setBackground(_window);
		_lstAditionalInfo->setMargin(2);
		_lstAditionalInfo->setWordWrap(true);
		fillAditionalInfoList();

		_btnTerminate->setText(tr("STR_TERMINATE_OPERATION"));
		_btnTerminate->onMouseClick((ActionHandler)&CovertOperationInfoState::btnTerminateClick);

		_btnOk->setText(tr("STR_OK"));
		_btnOk->onMouseClick((ActionHandler)&CovertOperationInfoState::btnOkClick);
		_btnOk->onKeyboardPress((ActionHandler)&CovertOperationInfoState::btnOkClick, Options::keyCancel);
		
	}

	void CovertOperationInfoState::init()
	{
		State::init();
	}

	/**
	 *
	 */
	CovertOperationInfoState::~CovertOperationInfoState()
	{
	}

	/**
	* Returns to the previous screen.
	* @param action Pointer to an action.
	*/
	void CovertOperationInfoState::btnOkClick(Action*)
	{
		_game->popState();
	}

	/**
	* Opens the Select Topic screen.
	* @param action Pointer to an action.
	*/
	void CovertOperationInfoState::btnTerminateClick(Action*)
	{
		_game->pushState(new CovertOperationConfirmTerminateState(_operation));
	}

	void CovertOperationInfoState::fillSoldiersList()
	{
		std::vector<Soldier*> soldiers = _operation->getSoldiers();
		for (std::vector<Soldier*>::iterator i = soldiers.begin(); i < soldiers.end(); i++)
		{
			_lstSoldiers->addRow(1, (*i)->getName().c_str());
		}
	}

	void CovertOperationInfoState::fillAditionalInfoList()
	{
		if (_operation->getIsPsi())
		{
			_lstAditionalInfo->addRow(1, tr("STR_TEAM_HAS_PSIONIC_POWER").c_str());
		}
	}


	/**
	 * Initializes all the elements on the UI.
	 */
	CovertOperationConfirmTerminateState::CovertOperationConfirmTerminateState(CovertOperation* operation) : _operation(operation)
	{

		// Create objects
		_window = new Window(this, 320, 200, 0, 0);
		_txtText = new Text(304, 17, 8, 7);
		_btnOk = new TextButton(148, 16, 8, 176);
		_btnCancel = new TextButton(148, 16, 164, 176);

		// Set palette
		setInterface("covertOperationConfirmTerminateState");

		add(_window, "window", "covertOperationConfirmTerminateState");
		add(_txtText, "text", "covertOperationConfirmTerminateState");
		add(_btnOk, "button", "covertOperationConfirmTerminateState");

		centerAllSurfaces();

		// Set up objects
		setWindowBackground(_window, "covertOperationConfirmTerminateState");

		_txtText->setBig();
		_txtText->setAlign(ALIGN_CENTER);
		_txtText->setVerticalAlign(ALIGN_MIDDLE);
		_txtText->setText(tr("STR_TERMINATE_OPERATION_TEXT"));

		_btnOk->setText(tr("STR_OK"));
		_btnOk->onMouseClick((ActionHandler)&CovertOperationConfirmTerminateState::btnOkClick);

		_btnCancel->setText(tr("STR_CANCEL_UC"));
		_btnCancel->onMouseClick((ActionHandler)&CovertOperationConfirmTerminateState::btnCancelClick);
		_btnCancel->onKeyboardPress((ActionHandler)&CovertOperationConfirmTerminateState::btnCancelClick, Options::keyCancel);

	}

	CovertOperationConfirmTerminateState::~CovertOperationConfirmTerminateState()
	{
	}

	/**
	* Returns to the previous screen.
	* @param action Pointer to an action.
	*/
	void CovertOperationConfirmTerminateState::btnOkClick(Action*)
	{
		return; //TODO add operation termination here pls
	}

	/**
	* Opens the Select Topic screen.
	* @param action Pointer to an action.
	*/
	void CovertOperationConfirmTerminateState::btnCancelClick(Action*)
	{
		_game->popState();
	}

}

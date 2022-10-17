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
#include "CovertOperationState.h"
#include <sstream>
#include "../Engine/Action.h"
#include "../Engine/Game.h"
#include "../Mod/Mod.h"
#include "../Engine/LocalizedText.h"
#include "../Engine/Options.h"
#include "../Interface/TextButton.h"
#include "../Interface/Window.h"
#include "../Interface/Text.h"
#include "../Interface/TextList.h"
#include "../Savegame/Base.h"
#include "../Savegame/CovertOperation.h"
#include "../Mod/RuleCovertOperation.h"
#include "../Basescape/CovertOperationStartState.h"
#include "../Basescape/CovertOperationInfoState.h"
#include "../Basescape/CovertOperationsListState.h"
#include <algorithm>
#include "../Engine/Logger.h"


namespace OpenXcom
{

/**
* Initializes all the elements in the CovertOperationState screen.
* @param game Pointer to the core game.
* @param base Pointer to the base to get info from.
*/
CovertOperationState::CovertOperationState(Base* base) : _base(base)
{
	// Create objects
	_window = new Window(this, 320, 200, 0, 0);
	_btnNew = new TextButton(148, 16, 8, 176);
	_btnOk = new TextButton(148, 16, 164, 176);
	_txtTitle = new Text(310, 17, 5, 8);
	_txtSoldiersAvailable = new Text(150, 9, 10, 24);
	_txtScientistsAvailable = new Text(150, 9, 160, 24);
	_txtEngineersAvailable = new Text(150, 9, 160, 34);
	_txtOperation = new Text(150, 17, 10, 44);
	_txtChances = new Text(52, 17, 160, 44);
	_txtProgress = new Text(84, 17, 212, 44);
	_lstOperations = new TextList(288, 112, 8, 62);

	// Set palette
	setInterface("covertOperationsMenu");

	add(_window, "window", "covertOperationsMenu");
	add(_btnNew, "button", "covertOperationsMenu");
	add(_btnOk, "button", "covertOperationsMenu");
	add(_txtTitle, "text", "covertOperationsMenu");
	add(_txtSoldiersAvailable, "text", "covertOperationsMenu");
	add(_txtScientistsAvailable, "text", "covertOperationsMenu");
	add(_txtEngineersAvailable, "text", "covertOperationsMenu");
	add(_txtOperation, "text", "covertOperationsMenu");
	add(_txtChances, "text", "covertOperationsMenu");
	add(_txtProgress, "text", "covertOperationsMenu");
	add(_lstOperations, "list", "covertOperationsMenu");

	centerAllSurfaces();

	// Set up objects
	setWindowBackground(_window, "covertOperationsMenu");

	_btnNew->setText(tr("STR_NEW_OPERATION"));
	_btnNew->onMouseClick((ActionHandler)&CovertOperationState::btnNewClick);
	_btnNew->onKeyboardPress((ActionHandler)&CovertOperationState::btnNewClick, Options::keyToggleQuickSearch);
	_btnNew->onKeyboardPress((ActionHandler)&CovertOperationState::onCurrentGlobalResearchClick, Options::keyGeoGlobalResearch);

	_btnOk->setText(tr("STR_OK"));
	_btnOk->onMouseClick((ActionHandler)&CovertOperationState::btnOkClick);
	_btnOk->onKeyboardPress((ActionHandler)&CovertOperationState::btnOkClick, Options::keyCancel);

	_txtTitle->setBig();
	_txtTitle->setAlign(ALIGN_CENTER);
	_txtTitle->setText(tr("STR_COVERT_OPERATIONS"));

	_txtOperation->setWordWrap(true);
	_txtOperation->setText(tr("STR_OPERATION_NAME"));

	_txtChances->setWordWrap(true);
	_txtChances->setText(tr("STR_OPERATION_CHANCES"));

	_txtProgress->setWordWrap(true);
	_txtProgress->setText(tr("STR_APPROX_TIME"));

	_lstOperations->setColumns(3, 150, 52, 84);
	_lstOperations->setSelectable(true);
	_lstOperations->setBackground(_window);
	_lstOperations->setMargin(2);
	_lstOperations->setWordWrap(true);
	_lstOperations->onMouseClick((ActionHandler)&CovertOperationState::onSelectOperation, SDL_BUTTON_LEFT);
}

/**
*
*/
CovertOperationState::~CovertOperationState()
{
}

/**
* Returns to the previous screen.
* @param action Pointer to an action.
*/
void CovertOperationState::btnOkClick(Action*)
{
	_game->popState();
}

/**
* Returns to the previous screen.
* @param action Pointer to an action.
*/
void CovertOperationState::btnNewClick(Action*)
{
	_game->pushState(new CovertOperationsListState(_base));
}

/**
* Displays the selected running covert operation.
* @param action Pointer to an action.
*/
void CovertOperationState::onSelectOperation(Action*)
{
	auto baseProjects(_base->getCovertOperations());
	CovertOperation* selectedTopic = baseProjects[_lstOperations->getSelectedRow()];
	_game->pushState(new CovertOperationInfoState(selectedTopic));
}

/**
* Opens the Current Global Research UI.
* @param action Pointer to an action.
*/
void CovertOperationState::onCurrentGlobalResearchClick(Action*)
{
	return; //#FINNIKTODO
	//_game->pushState(new GlobalResearchState(true));
}
/**
* Updates the research list
* after going to other screens.
*/
void CovertOperationState::init()
{
	State::init();
	fillProjectList(0);
}

/**
* Fills the list with Base CovertOperation. Also updates count of available lab space and available personell.
*/
void CovertOperationState::fillProjectList(size_t scrl)
{
	const std::vector<CovertOperation*>& baseOperations(_base->getCovertOperations());
	_lstOperations->clearList();
	for (std::vector<CovertOperation*>::const_iterator iter = baseOperations.begin(); iter != baseOperations.end(); ++iter)
	{
		/*std::ostringstream sstr;
		sstr << (*iter)->getOddsName();*/

		std::string wstr = tr((*iter)->getOperationName());
		auto test = (*iter)->getOddsName();
		_lstOperations->addRow(3, wstr.c_str(), tr((*iter)->getOddsName()).c_str(), tr((*iter)->getTimeLeftName()).c_str());
	}
	_txtSoldiersAvailable->setText(tr("STR_SOLDIERS_AVAILABLE").arg(_base->getAvailableSoldiers()));
	_txtScientistsAvailable->setText(tr("STR_SCIENTISTS_AVAILABLE").arg(_base->getAvailableScientists()));
	_txtEngineersAvailable->setText(tr("STR_ENGINEERS_AVAILABLE").arg(_base->getAvailableEngineers()));

	if (scrl)
		_lstOperations->scrollTo(scrl);
}

}

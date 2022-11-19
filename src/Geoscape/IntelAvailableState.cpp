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
#include <assert.h>
#include "IntelAvailableState.h"
#include "../Engine/Game.h"
#include "../Mod/Mod.h"
#include "../Engine/LocalizedText.h"
#include "../Interface/TextButton.h"
#include "../Interface/Window.h"
#include "../Interface/Text.h"
#include "../Interface/TextList.h"
#include "GeoscapeState.h"
#include "../Engine/Options.h"
#include "../Basescape/BasescapeState.h"
#include "../Savegame/Base.h"
#include "../Savegame/IntelProject.h"
#include "../Savegame/SavedGame.h"

namespace OpenXcom
{

/**
 * Initializes all the elements in a Production Complete window.
 * @param game Pointer to the core game.
 * @param base Pointer to base the production belongs to.
 * @param item Item that finished producing.
 * @param state Pointer to the Geoscape state.
 * @param endType What ended the production.
 * @param production Pointer to the production details.
 */
IntelAvailableState::IntelAvailableState(IntelProject *project, Base *base, GeoscapeState* state) : _project(project), _base(base), _state(state)
{
	_screen = false;

	_window = new Window(this, 230, 140, 45, 30, POPUP_BOTH);
	_btnOk = new TextButton(80, 16, 64, 146);
	_btnGotoBase = new TextButton(80, 16, 176, 146);
	_txtTitle = new Text(230, 17, 45, 70);
	_txtProject = new Text(230, 32, 45, 96);

	// Set palette
	setInterface("geoIntelAvailable");

	add(_window, "window", "geoIntelAvailable");
	add(_btnOk, "button", "geoIntelAvailable");
	add(_btnGotoBase, "button", "geoIntelAvailable");
	add(_txtTitle, "text1", "geoIntelAvailable");
	add(_txtProject, "text2", "geoIntelAvailable");

	centerAllSurfaces();

	// Set up objects
	setWindowBackground(_window, "geoIntelAvailable");

	_btnOk->setText(tr("STR_OK"));
	_btnOk->onMouseClick((ActionHandler)&IntelAvailableState::btnOkClick);
	_btnOk->onKeyboardPress((ActionHandler)&IntelAvailableState::btnOkClick, Options::keyCancel);

	_btnGotoBase->setText(tr("STR_ALLOCATE"));
	_btnGotoBase->onMouseClick((ActionHandler)&IntelAvailableState::btnGotoBaseClick);
	_btnGotoBase->onKeyboardPress((ActionHandler)&IntelAvailableState::btnGotoBaseClick, Options::keyOk);

	_txtTitle->setBig();
	_txtTitle->setAlign(ALIGN_CENTER);
	_txtTitle->setText(tr("STR_INTEL_PROJECT_AVAILABLE"));

	_txtProject->setAlign(ALIGN_CENTER);
	_txtProject->setBig();
	_txtProject->setWordWrap(true);
	if (project)
	{
		_txtProject->setText(tr(project->getName()));
	}
}

IntelAvailableState::~IntelAvailableState()
{
}

/**
 * Closes the window.
 * @param action Pointer to an action.
 */
void IntelAvailableState::btnOkClick(Action *)
{
	_game->popState();
}

/**
 * Goes to the base for the respective production.
 * @param action Pointer to an action.
 */
void IntelAvailableState::btnGotoBaseClick(Action *)
{
	_state->timerReset();
	_game->popState();
	_game->pushState(new BasescapeState(_base, _state->getGlobe()));
}

}

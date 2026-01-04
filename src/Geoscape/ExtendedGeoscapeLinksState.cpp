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

#include "ExtendedGeoscapeLinksState.h"
#include "FundingState.h"
#include "GeoscapeState.h"
#include "../Engine/Game.h"
#include "../Engine/Action.h"
#include "../Engine/Options.h"
#include "../Engine/LocalizedText.h"
#include "../Engine/Unicode.h"
#include "../Interface/Window.h"
#include "../Interface/Text.h"
#include "../Interface/TextButton.h"
#include "../Menu/NotesState.h"
#include "../Menu/TestState.h"
#include "../Mod/Mod.h"
#include "../Savegame/SavedGame.h"

namespace OpenXcom
{

/**
 * Initializes all the elements in the ExtendedGeoscapeLinksState screen.
 */
ExtendedGeoscapeLinksState::ExtendedGeoscapeLinksState(GeoscapeState* parent) : _parent(parent)
{
	_screen = false;
	const int dY = 13 * 4;

	// Create objects
	_window = new Window(this, 256, 180, 32, 10, POPUP_BOTH);
	_txtTitle = new Text(220, 17, 50, 33);
	_btnOk = new TextButton(220, 12, 50, 167);

	_btnUfoTracker = new TextButton(116, 25, 44, 50);
	_btnSoldierExp = new TextButton(116, 25, 161, 50);
	_btnNotes = new TextButton(116, 25, 44, 76);
	_btnMusic = new TextButton(116, 25, 161, 76);
	//more buttons can be added here later
	_btnTest = new TextButton(116, 25, 44, 154);
	_btnOk = new TextButton(116, 25, 161, 154);

	// Set palette
	setInterface("oxceLinks");

	add(_window, "window", "oxceLinks");
	add(_txtTitle, "text", "oxceLinks");
	add(_btnOk, "button", "oxceLinks");

	add(_btnUfoTracker, "button", "oxceLinks");
	add(_btnSoldierExp, "button", "oxceLinks");
	add(_btnNotes, "button", "oxceLinks");
	add(_btnMusic, "button", "oxceLinks");
	add(_btnTest, "button", "oxceLinks");

	centerAllSurfaces();

	// Set up objects
	setWindowBackground(_window, "oxceLinks");

	_txtTitle->setBig();
	_txtTitle->setAlign(ALIGN_CENTER);
	_txtTitle->setText(tr("STR_EXTENDED_LINKS"));

	_btnOk->setText(tr("STR_OK"));
	_btnOk->onMouseClick((ActionHandler)&ExtendedGeoscapeLinksState::btnOkClick);
	_btnOk->onKeyboardPress((ActionHandler)&ExtendedGeoscapeLinksState::btnOkClick, Options::keyCancel);
	std::string tmp;

	tmp = tr("STR_UFO_TRACKER");
	Unicode::upperCase(tmp);
	_btnUfoTracker->setText(tmp);
	_btnUfoTracker->onMouseClick((ActionHandler)&ExtendedGeoscapeLinksState::btnUfoTrackerClick);

	tmp = tr("STR_DAILY_PILOT_EXPERIENCE");
	Unicode::upperCase(tmp);
	_btnSoldierExp->setText(tmp);
	_btnSoldierExp->onMouseClick((ActionHandler)&ExtendedGeoscapeLinksState::btnSoldierExpClick);

	tmp = tr("STR_NOTES");
	Unicode::upperCase(tmp);
	_btnNotes->setText(tmp);
	_btnNotes->onMouseClick((ActionHandler)&ExtendedGeoscapeLinksState::btnNotesClick);

	tmp = tr("STR_SELECT_MUSIC_TRACK");
	Unicode::upperCase(tmp);
	_btnMusic->setText(tmp);
	_btnMusic->onMouseClick((ActionHandler)&ExtendedGeoscapeLinksState::btnMusicClick);

	if (Options::debug)
	{
		_btnTest->setText(tr("STR_TOGGLE_DEBUG_MODE"));
	}
	else
	{
		_btnTest->setText(tr("STR_TEST_SCREEN"));
	}
	_btnTest->onMouseClick((ActionHandler)&ExtendedGeoscapeLinksState::btnTestClick);
}

void ExtendedGeoscapeLinksState::btnUfoTrackerClick(Action *)
{
	_game->popState();
	_parent->btnUfoTrackerClick(nullptr);
}

void ExtendedGeoscapeLinksState::btnSoldierExpClick(Action *)
{
	_game->popState();
	_parent->btnSoldierExperienceClick(nullptr);
}

void ExtendedGeoscapeLinksState::btnNotesClick(Action *)
{
	_game->popState();
	_game->pushState(new NotesState(OPT_GEOSCAPE));
}

void ExtendedGeoscapeLinksState::btnMusicClick(Action *)
{
	_game->popState();
	_parent->btnSelectMusicTrackClick(nullptr);
}

void ExtendedGeoscapeLinksState::btnTestClick(Action *)
{
	_game->popState();
	if (Options::debug)
	{
		_parent->btnDebugClick(nullptr);
	}
	else
	{
		_game->pushState(new TestState);
	}
}

/**
 * Returns to the previous screen.
 * @param action Pointer to an action.
 */
void ExtendedGeoscapeLinksState::btnOkClick(Action *)
{
	_game->popState();
}

}

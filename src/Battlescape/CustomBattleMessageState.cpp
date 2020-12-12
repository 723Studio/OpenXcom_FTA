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
#include "CustomBattleMessageState.h"
#include "../Engine/Game.h"
#include "../Engine/LocalizedText.h"
#include "../Mod/Mod.h"
#include "../Mod/BattleScript.h"
#include "../Interface/TextButton.h"
#include "../Interface/Window.h"
#include "../Interface/Text.h"
#include "../Interface/Cursor.h"
#include "../Engine/Options.h"
#include "../Savegame/SavedGame.h"
#include "../Savegame/SavedBattleGame.h"

#include "BattlescapeGame.h"
#include "BattlescapeState.h"

namespace OpenXcom
{

/**
* Initializes all the elements.
* @param game Pointer to the core game.
* @param msg Message string.
*/
CustomBattleMessageState::CustomBattleMessageState(BattleMessage msg)
{
	_battleGame = _game->getSavedGame()->getSavedBattle();
	_screen = false;

	// Create objects
	_window = new Window(this, 320, 144, 0, 0);
	_txtTitle = new Text(304, 16, 8, 8);
	_txtContent = new Text(304, 89, 8, 26);
	_btnOk = new TextButton(304, 18, 8, 118);

	// Set palette
	_game->getSavedGame()->getSavedBattle()->setPaletteByDepth(this);

	add(_window, "messageWindowBorder", "battlescape");
	add(_txtTitle, "messageWindows", "battlescape");
	add(_txtContent, "messageWindows", "battlescape");
	add(_btnOk, "messageWindowButtons", "battlescape");

	centerAllSurfaces();

	// Set up objects
	_window->setHighContrast(true);
	std::string bkgr = msg.background;
	if (!bkgr.empty())
	{
		_window->setBackground(_game->getMod()->getSurface(bkgr));
	}
	else
	{
		_window->setBackground(_game->getMod()->getSurface("TAC00.SCR"));
	}

	_txtTitle->setBig();
	_txtTitle->setAlign(ALIGN_CENTER);
	_txtTitle->setVerticalAlign(ALIGN_MIDDLE);
	_txtTitle->setHighContrast(true);
	_txtTitle->setWordWrap(true);
	_txtTitle->setText(tr(msg.title));

	_txtContent->setVerticalAlign(ALIGN_MIDDLE);
	_txtContent->setHighContrast(true);
	_txtContent->setWordWrap(true);
	_txtContent->setText(tr(msg.content));

	if (msg.title.empty())
	{
		_txtTitle->setVisible(false);
		_txtContent->setY(8);
		_txtContent->setHeight(107);
	}

	std::string btnLable = msg.answer;
	if (!btnLable.empty())
	{
		_btnOk->setText(tr(btnLable));
	}
	else
	{
		_btnOk->setText(tr("STR_OK"));
	}
	_btnOk->onMouseClick((ActionHandler)&CustomBattleMessageState::btnOkClick);
	_btnOk->onKeyboardPress((ActionHandler)&CustomBattleMessageState::btnOkClick, Options::keyOk);
	_btnOk->onKeyboardPress((ActionHandler)&CustomBattleMessageState::btnOkClick, Options::keyCancel);
	_btnOk->setHighContrast(true);

	_game->getCursor()->setVisible(true);

}

/**
*
*/
CustomBattleMessageState::~CustomBattleMessageState()
{

}

/**
* Returns to the previous screen.
* @param action Pointer to an action.
*/
void CustomBattleMessageState::btnOkClick(Action*)
{
	_game->popState();
}

}

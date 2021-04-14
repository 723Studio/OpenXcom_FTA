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

#include "HackingState.h"
#include "HackingView.h"
//#include "BattlescapeGame.h"
#include "../Engine/InteractiveSurface.h"
#include "../Engine/Game.h"
#include "../Engine/Action.h"
#include "../Engine/Timer.h"
#include "../Engine/Screen.h"
#include "../Engine/Options.h"
#include "../Interface/Text.h"
//#include "../Savegame/BattleUnit.h"
#include "../Mod/Mod.h"
#include "../Savegame/SavedGame.h"
#include "../Savegame/SavedBattleGame.h"

namespace OpenXcom
{

/**
 * Initializes the Hacking State.
 * @param game Pointer to the core game.
 * @param action Pointer to an action.
 */
HackingState::HackingState(BattleAction* action) : _action(action)
{
	if (Options::maximizeInfoScreens)
	{
		Options::baseXResolution = Screen::ORIGINAL_WIDTH;
		Options::baseYResolution = Screen::ORIGINAL_HEIGHT;
		_game->getScreen()->resetDisplay(false);
	}
	_bg = new Surface(320, 200);
	//_hackingView = new HackingView(152, 152, 56, 24, _game, _action->actor);
	_exitButton = new InteractiveSurface(20, 20, 257, 150);
	//_someTxt = new Text(20, 20, 257, 150);

	if (_game->getScreen()->getDY() > 50)
	{
		_screen = false;
	}

	// Set palette
	_game->getSavedGame()->getSavedBattle()->setPaletteByDepth(this);

//	add(_hackingView);
	add(_bg);
	//add(_exitButton, "buttonExit", "hackingTool", _bg); // TODO: add new interface to rulesets
	add(_exitButton);


	centerAllSurfaces();

	Surface* backgroundSprite = 0;

	// TODO: see if we need to use something like this:
	//if (!_item->getRules()->getMediKitCustomBackground().empty())
	//{
	//	backgroundSprite = _game->getMod()->getSurface(_item->getRules()->getMediKitCustomBackground(), false);
	//}
	if (!backgroundSprite)
	{
		backgroundSprite = _game->getMod()->getSurface("HackingUI");
	}

	backgroundSprite->blitNShade(_bg, 0, 0);

	_exitButton->onMouseClick((ActionHandler)&HackingState::onExitClick);
	_exitButton->onKeyboardPress((ActionHandler)&HackingState::onExitClick, Options::keyCancel);

	// TODO: enable animation handling when it's ready
	//_timerAnimate = new Timer(125);
	//_timerAnimate->onTimer((StateHandler)&HackingState::animate);
	//_timerAnimate->start();

	update();
}

HackingState::~HackingState()
{
//	delete _timerAnimate;
}

/**
 * Closes the window on right-click.
 * @param action Pointer to an action.
 */
void HackingState::handle(Action* action)
{
	State::handle(action);
	if (action->getDetails()->type == SDL_MOUSEBUTTONDOWN && _game->isRightClick(action))
	{
		onExitClick(action);
	}
}

/**
 * Updates hacking state.
 */
void HackingState::update()
{
	// TODO: update hacking interface
}

/**
 * Animation handler. Updates the minigame view animation.
 */
void HackingState::animate()
{
//	_hackingView->animate(); // TODO: uncomment when ready
}

/**
 * Handles timers.
 */
void HackingState::think()
{
	State::think();
//	_timerAnimate->think(this, 0); // TODO: uncomment when ready
}

/**
 * Exits the screen.
 * @param action Pointer to an action.
 */
void HackingState::onExitClick(Action*)
{
	if (Options::maximizeInfoScreens)
	{
		Screen::updateScale(Options::battlescapeScale, Options::baseXBattlescape, Options::baseYBattlescape, true);
		_game->getScreen()->resetDisplay(false);
	}
	_game->popState();
}

} // namespace

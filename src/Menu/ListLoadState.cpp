/*
 * Copyright 2010-2026 OpenXcom Developers.
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
#include <algorithm>
#include "ListLoadState.h"
#include "../Engine/Game.h"
#include "../Engine/Action.h"
#include "../Engine/Options.h"
#include "../Interface/Text.h"
#include "../Interface/TextList.h"
#include "../Interface/ToggleTextButton.h"
#include "ConfirmLoadState.h"
#include "LoadGameState.h"

namespace OpenXcom
{

/**
 * Initializes all the elements in the Load Game screen.
 * @param game Pointer to the core game.
 * @param origin Game section that originated this state.
 */
ListLoadState::ListLoadState(OptionsOrigin origin) : ListGamesState(origin, 0, true)
{
	_txtTitle->setText(tr("STR_SELECT_GAME_TO_LOAD"));

	centerAllSurfaces();
}

/**
 *
 */
ListLoadState::~ListLoadState()
{

}

/**
 * Loads the selected save.
 * @param action Pointer to an action.
 */
void ListLoadState::lstSavesPress(Action *action)
{
	ListGamesState::lstSavesPress(action);
	if (action->getDetails()->button.button == SDL_BUTTON_LEFT && !_btnDelete->getPressed())
	{
		loadSave(_lstSaves->getSelectedRow());
	}
}
void ListLoadState::loadSave(size_t list_idx)
{
	bool confirm = false;
	const SaveInfo &saveInfo(_saves[list_idx]);
	for (const auto& modName : saveInfo.mods)
	{
		std::string name = SavedGame::sanitizeModName(modName);
		if (std::find(Options::mods.begin(), Options::mods.end(), std::make_pair(name, true)) == Options::mods.end())
		{
			confirm = true;
			break;
		}
	}
	if (confirm)
	{
		_game->pushState(new ConfirmLoadState(_origin, saveInfo.fileName));
	}
	else
	{
		_game->pushState(new LoadGameState(_origin, saveInfo.fileName, _palette));
	}
}
void ListLoadState::init()
{
	ListGamesState::init();
	if (_origin == OPT_MENU && Options::getLoadLastSave())
	{
		// make it so that this fires only once
		Options::expendLoadLastSave();

		// if the save was explicitly specified, load that one and skip all checks and confirmations
		if (!Options::getLoadThisSave().empty())
		{
			// hide the ui
			toggleScreen();
			hideAll();

			_game->pushState(new LoadGameState(_origin, Options::getLoadThisSave(), _palette));
			return;
		}

		// find the absolutely latest save game including quick and autos
		time_t timestamp = 0;
		int idx = -1, i = 0;
		for (auto it = _saves.begin(); it !=_saves.end(); ++it, ++i)
		{
			if ((*it).timestamp > timestamp)
			{
				idx = i;
				timestamp = (*it).timestamp;
			}
		}
		if (idx != -1)
		{
			// hide the ui
			toggleScreen();
			hideAll();
			loadSave(idx);
		}
	}
}

}

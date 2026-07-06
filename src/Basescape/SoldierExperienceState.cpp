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
#include "SoldierExperienceState.h"
#include <algorithm>
#include "../Engine/Action.h"
#include "../Engine/Game.h"
#include "../Mod/Mod.h"
#include "../Engine/LocalizedText.h"
#include "../Engine/Options.h"
#include "../Interface/TextButton.h"
#include "../Interface/Window.h"
#include "../Interface/Text.h"
#include "../Interface/TextList.h"
#include "../Savegame/Soldier.h"

namespace OpenXcom
{
/**
 * Initializes all the elements in the Soldier Armor window.
 * @param soldier Pointer to the soldier.
 */
SoldierExperienceState::SoldierExperienceState(Soldier* soldier) :_soldier(soldier)
{
	_screen = false;

	// Create objects
	_window = new Window(this, 192, 130, 64, 20, POPUP_BOTH);
	_btnOk = new TextButton(140, 16, 90, 123);
	_txtTitle = new Text(108, 8, 106, 28);
	_txtRole = new Text(35, 9, 80, 41);
	_txtRank = new Text(35, 9, 132, 41);
	_txtExperience = new Text(61, 9, 172, 41);
	_lstExperience = new TextList(170, 66, 74, 53);

	setInterface("soldierExperience");
	add(_window, "window", "soldierExperience");
	add(_btnOk, "button", "soldierExperience");
	add(_txtTitle, "text", "soldierExperience");
	add(_txtRole, "text", "soldierExperience");
	add(_txtRank, "text", "soldierExperience");
	add(_txtExperience, "text", "soldierExperience");
	add(_lstExperience, "list", "soldierExperience");

	centerAllSurfaces();

	// Set up objects
	setWindowBackground(_window, "soldierExperience");

	_btnOk->setText(tr("STR_OK"));
	_btnOk->onMouseClick((ActionHandler)&SoldierExperienceState::btnOKClick);
	_btnOk->onKeyboardPress((ActionHandler)&SoldierExperienceState::btnOKClick, Options::keyCancel);

	
	_txtTitle->setAlign(ALIGN_CENTER);
	_txtTitle->setText(tr("STR_SOLDIER_EXPERIENCE_UC"));

	_txtRole->setText(tr("STR_ROLE_UC"));

	_txtRank->setText(tr("STR_RANK_UC"));

	_txtExperience->setText(tr("STR_EXPERIENCE_UC"));

	_lstExperience->setColumns(3, 58, 35, 77);
	_lstExperience->setSelectable(false);
	_lstExperience->setBackground(_window);
	_lstExperience->setMargin(8);

	for (SoldierRoleRanks* rr : _soldier->getRoles())
	{
		std::string roleName;
		switch (rr->role)
		{
		case ROLE_NONE:
			break;
		case ROLE_SOLDIER:
			roleName = "STR_SOLDIER";
			break;
		case ROLE_ROBOT:
			break;
		case ROLE_PILOT:
			roleName = "STR_PILOT";
			break;
		case ROLE_AGENT:
			roleName = "STR_AGENT";
			break;
		case ROLE_SCIENTIST:
			roleName = "STR_SCIENTIST";
			break;
		case ROLE_ENGINEER:
			roleName = "STR_ENGINEER";
			break;
		default:
			break;;
		}

		if (rr->experience > 0 && !roleName.empty())
		{
			std::ostringstream ssr, sse;
			ssr << rr->rank;
			sse << rr->experience;
			sse << " / ";
			sse << _soldier->getRules()->getRequiredExperience(rr->role, rr->rank);

			_lstExperience->addRow(3, tr(roleName).c_str(), ssr.str().c_str(), sse.str().c_str());
		}
	}
}

/**
 *
 */
SoldierExperienceState::~SoldierExperienceState()
{}


/**
 * Returns to the previous screen.
 * @param action Pointer to an action.
 */
void SoldierExperienceState::btnOKClick(Action *)
{
	_game->popState();
}
}

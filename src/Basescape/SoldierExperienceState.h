#pragma once
/*
 * Copyright 2010-2025 OpenXcom Developers.
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
#include "../Engine/State.h"

namespace OpenXcom
{
class TextButton;
class Window;
class Text;
class TextEdit;
class TextList;
class Soldier;

/**
 * Select Armor window that allows changing
 * of the armor equipped on a soldier.
 */
class SoldierExperienceState : public State
{
private:
	Soldier *_soldier;
	Window *_window;
	Text *_txtTitle, *_txtRole, *_txtRank, *_txtExperience;
	TextButton* _btnOk;
	TextList *_lstExperience;
public:
	/// Creates the Soldier Experience state.
	SoldierExperienceState(Soldier* soldier);
	/// Cleans up the Soldier Experience state.
	~SoldierExperienceState();
	/// Handler for clicking the OK button.
	void btnOKClick(Action *action);

};

}

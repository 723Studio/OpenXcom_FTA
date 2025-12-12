#pragma once
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
#include "../Engine/State.h"
#include <vector>

namespace OpenXcom
{

class TextButton;
class Window;
class Text;
class TextList;
class ComboBox;
class Base;
class Soldier;

/**
 * Soldiers screen that lets the player
 * manage all the soldiers in a base.
 */
class MontlySoldierExperienceState : public State
{
private:
	TextButton *_btnOk;
	Window *_window;
	Text *_txtTitle, *_txtName, *_txtRank, *_txtCraft;
	TextList *_lstSoldiers;
	Base *_base;
	std::vector<Base*> _bases;
	///initializes the display list based on the craft soldier's list and the position to display
	void initList(size_t scrl);
  public:
	/// Creates the Soldiers state.
	MontlySoldierExperienceState(Base *base);
	/// Cleans up the Soldiers state.
	~MontlySoldierExperienceState();
	/// Updates the soldier names.
	void init() override;
	/// Handler for clicking the OK button.
	void btnOkClick(Action *action);
};

}

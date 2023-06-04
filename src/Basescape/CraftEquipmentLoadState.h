#pragma once
/*
* Copyright 2010-2015 OpenXcom Developers.
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
class ToggleTextButton;
class Window;
class Text;
class TextList;
class CraftEquipmentState;

/**
* Craft Loadout Load window that allows changing of the equipment onboard.
*/
class CraftEquipmentLoadState : public State
{
private:
	CraftEquipmentState *_parent;
	Window *_window;
	Text *_txtTitle;
	TextList *_lstLoadout;
	ToggleTextButton *_btnOnlyAdd;
	TextButton *_btnCancel;
public:
	/// Creates the Load Craft Loadout state.
	CraftEquipmentLoadState(CraftEquipmentState *parent);
	/// Cleans up the Load Craft Loadout state.
	~CraftEquipmentLoadState();
	/// Handler for clicking the Cancel button.
	void btnCancelClick(Action *action);
	/// Handler for clicking the Loadout list.
	void lstLoadoutClick(Action *action);
};

}

#pragma once
/*
 * Copyright 2010-2023 OpenXcom Developers.
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
class Window;
class TextButton;
class Text;
class TextList;
class DebriefingState;

/**
 * Screen which displays detailed info about manufacturing project.
 */
class DebriefingExtraStatsState : public State
{
private:
	Window *_window;
	TextButton *_btnOk;
	Text* _txtTitle, * _txtName; // , * _txtIncrease;
	TextList* _lstStats;
	DebriefingState* _debriefing;
	void generateStatsList();
public:
	/// Creates the State.
	DebriefingExtraStatsState(DebriefingState* debriefing);
	/// Handler for the OK button.
	void btnOKClick(Action *action);
};

}

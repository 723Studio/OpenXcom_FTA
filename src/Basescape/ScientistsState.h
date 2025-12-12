#pragma once
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
#include "../Engine/State.h"
#include <vector>

namespace OpenXcom
{

class TextButton;
class Window;
class Text;
class TextList;
class Base;
class Soldier;
class RuleSoldier;
class ResearchProject;

/**
 * Soldiers screen that lets the player
 * manage all the soldiers in a base.
 */
class ScientistsState : public State
{
private:
	TextButton *_btnOk;
	Window *_window;
	Text *_txtTitle, *_txtName, *_txtProject;
	TextList *_lstScientist;
	Base *_base;
	Uint8 _otherDutyColor;
	//std::vector<Soldier *> _origPilotOrder, _filteredListOfPilots;
	//std::vector<SortFunctor *> _sortFunctors;
	std::vector<int> _soldierNumbers;
	//getStatFn_t _dynGetter;
	/// initializes the display list based on the craft soldier's list and the position to display
	void initList(size_t scrl);

public:
	/// Creates the Soldiers state.
	ScientistsState(Base *base);
	/// Cleans up the Soldiers state.
	~ScientistsState();
	/// Updates the soldier names.
	void init() override;
	/// Handler for clicking the OK button.
	void btnOkClick(Action *action);
	/// Handler for clicking the Scientists list.
	void lstScientistsClick(Action *action);
};

}

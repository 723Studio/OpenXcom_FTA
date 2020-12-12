#pragma once
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
#include "../Engine/State.h"
#include <vector>
#include "SoldierSortUtil.h"

namespace OpenXcom
{

class TextButton;
class ToggleTextButton;
class Window;
class Text;
class TextEdit;
class TextList;
class ComboBox;
class Base;
class RuleSoldierTransformation;

/**
 * Transformations Overview screen that lets the player
 * manage all the soldier transformations in a base.
 */
class SoldierTransformationListState : public State
{
private:
	Base* _base;
	ComboBox* _screenActions;
	TextButton* _btnOK;
	ToggleTextButton* _btnOnlyEligible;
	Window* _window;
	Text* _txtTitle, * _txtProject, * _txtNumber, * _txtSoldierNumber;
	ComboBox* _cbxSoldierType, * _cbxSoldierStatus;
	TextEdit* _btnQuickSearch;
	TextList* _lstTransformations;
	std::vector<int> _transformationIndices;
	std::vector<RuleSoldierTransformation*> _availableTransformations;
public:
	/// Creates the Transformations Overview state.
	SoldierTransformationListState(Base* base, ComboBox* screenActions);
	/// Cleans up the Transformations Overview state.
	~SoldierTransformationListState();
	/// Refreshes the list of transformations.
	void initList();
	/// Toggles the quick search.
	void btnQuickSearchToggle(Action* action);
	/// Applies the quick search.
	void btnQuickSearchApply(Action* action);
	/// Handler for changing the soldier type combo box.
	void cbxSoldierTypeChange(Action* action);
	/// Handler for changing the soldier status combo box.
	void cbxSoldierStatusChange(Action* action);
	/// Handler for toggling showing only transformations with eligible soldiers.
	void btnOnlyEligibleClick(Action* action);
	/// Closes the transformations overview without selecting one.
	void btnOkClick(Action* action);
	/// Handler for clicking the Transformations list.
	void lstTransformationsClick(Action* action);
};

}

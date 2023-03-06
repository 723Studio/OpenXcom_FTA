#pragma once
/*
 * Copyright 2010-2018 OpenXcom Developers.
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
class TextEdit;
class TextList;
class Base;
class Soldier;
class RuleSoldierTransformation;

/**
 * Screen that allocates a soldier to a transformation project
 */
class SoldierTransformationState : public State
{
private:
	RuleSoldierTransformation *_transformationRule;
	Base *_base;
	Soldier *_sourceSoldier;
	std::vector<Soldier *> *_filteredListOfSoldiers;
	Window *_window;
	TextEdit *_edtSoldier;
	Text *_txtDescription, *_txtCost, *_txtTransferTime, *_txtRecoveryTime, *_txtRequiredItems, *_txtItemNameColumn, *_txtUnitRequiredColumn, *_txtUnitAvailableColumn;
	TextList *_lstRequiredItems, *_lstStatChanges;
	TextButton *_btnCancel, *_btnLeftArrow, *_btnRightArrow, *_btnStart;

	/// Creates a string for the soldier stats table
	std::string formatStat(int stat, bool plus, bool hide);

public:
	/// Creates the soldier transformation state
	SoldierTransformationState(RuleSoldierTransformation *transformationRule, Base *base, Soldier *selectedSoldier, std::vector<Soldier *> *filteredListOfSoldiers);
	/// Cleans up the soldier transformation state
	~SoldierTransformationState();
	/// Initialize all the values in the data fields
	void initTransformationData();
	/// Handler for pressing the Cancel button
	void btnCancelClick(Action *action);
	/// Handler for pressing the Start button
	void btnStartClick(Action *action);
	void performTransformation();
	void retire();
	/// Handler for pressing the Left arrow button
	void btnLeftArrowClick(Action *action);
	/// Handler for pressing the Right arrow button
	void btnRightArrowClick(Action *action);

};

}

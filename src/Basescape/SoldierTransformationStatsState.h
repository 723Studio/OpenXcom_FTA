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
struct UnitStats;
template<typename T, typename I> class ScriptValues;

/**
 * Screen that allocates a soldier to a transformation project
 */
class SoldierTransformationStatsState : public State
{
private:
	RuleSoldierTransformation *_transformationRule;
	Soldier *_sourceSoldier;
	Window *_window;
	Text *_txtStateHeader, *_txtStatChangesHeader, *_txtCurrent, *_txtMin, *_txtMax, *_txtBonus;
	TextList *_lstStats;
	TextButton *_btnOK;

	/// Creates a string for the soldier stats table
	std::string formatStat(int stat, bool plus, bool hide);

public:
	/// Creates the soldier transformation stats state
	SoldierTransformationStatsState(RuleSoldierTransformation *transformationRule, Soldier * sourceSoldier);
	/// Cleans up the soldier transformation stats state
	~SoldierTransformationStatsState();
	/// Handler for pressing the OK button
	void btnOKClick(Action *action);

};

}

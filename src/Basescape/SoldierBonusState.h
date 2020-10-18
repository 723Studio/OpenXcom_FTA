#pragma once
/*
 * Copyright 2010-2019 OpenXcom Developers.
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
#include <vector>
#include <string>
#include "../Engine/State.h"

namespace OpenXcom
{

class Base;
class TextButton;
class ToggleTextButton;
class Window;
class Text;
class TextList;

/**
 * SoldierBonus window displays all soldier bonuses.
 */
class SoldierBonusState : public State
{
private:
	Base *_base;
	size_t _soldier;
	std::vector<std::string> _bonuses;

	TextButton *_btnCancel;
	ToggleTextButton *_btnSummary;
	Window *_window;
	Text *_txtTitle, *_txtType;
	TextList *_lstBonuses;
	TextList *_lstSummary;
public:
	/// Creates the SoldierBonus state.
	SoldierBonusState(Base *base, size_t soldier);
	/// Cleans up the SoldierBonus state.
	~SoldierBonusState();
	/// Handler for clicking the Summary button.
	void btnSummaryClick(Action *action);
	/// Handler for clicking the Cancel button.
	void btnCancelClick(Action *action);
	/// Handler for clicking the Bonuses list.
	void lstBonusesClick(Action *action);
};

}

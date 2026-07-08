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
#include <vector>
#include "../Basescape/SoldierSortUtil.h"

namespace OpenXcom
{

class TextButton;
class Window;
class Text;
class TextList;
class ComboBox;
class Base;
class Soldier;
class DiplomacyFaction;
struct SortFunctor;

/**
 * Select FtA's Hire Personnel State that lets the player
 * pick the soldiers for hire.
 */
class DiplomacyHirePersonnelState : public State
{
private:
	TextButton *_btnOk;
	TextButton *_btnCancel;
	Window *_window;
	Text *_txtTitle, *_txtName, *_txtRank, *_txtCost, *_txtAvailable, *_txtUsed, *_txtTotal;
	ComboBox *_cbxSortBy;
	TextList *_lstSoldiers;

	Base *_base;
	DiplomacyFaction* _faction;
	//Uint8 _otherCraftColor;
	std::vector<Soldier *> _filteredListOfSoldiers, _selectedSoldiers;
	std::vector<SortFunctor *> _sortFunctors;
	std::vector<int> _soldierNumbers;
	int64_t _total = 0;
	
	getStatFn_t _dynGetter;
	/// initializes the display list based on the craft soldier's list and the position to display
	void initList(size_t scrl);
	void updateState();
public:
	/// Creates the Craft Soldiers state.
	DiplomacyHirePersonnelState(Base *base, DiplomacyFaction* faction);
	/// Cleans up the Craft Soldiers state.
	~DiplomacyHirePersonnelState();
	/// Handler for changing the sort by combobox.
	void cbxSortByChange(Action *action);
	/// Handler for clicking the OK button.
	void btnOkClick(Action *action);
	/// Handler for clicking the Preview button.
	void btnCancelClick(Action *action);
	/// Updates the soldiers list.
	void init() override;
	void lstSoldiersClick(Action *action);
};

}

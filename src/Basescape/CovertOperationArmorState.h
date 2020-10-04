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
#include "SoldierArmorState.h"

namespace OpenXcom
{

class TextButton;
class Window;
class Text;
class TextList;
class ComboBox;
class Base;
class Soldier;
class RuleCovertOperation;
class CovertOperationStartState;

class Armor;
class ArrowButton;

struct SortFunctor;



/**
* Select Armor screen that lets the player
* pick armor for the soldiers for planned operation.
*/
class CovertOperationArmorState : public State
{
private:
	TextButton* _btnOk;
	Window* _window;
	Text* _txtTitle, * _txtName, * _txtCraft, * _txtArmor, * _txtChances;
	ComboBox* _cbxSortBy;
	TextList* _lstSoldiers;

	Base* _base;
	size_t _savedScrollPosition;
	CovertOperationStartState* _operation;
	std::vector<Soldier*> _origSoldierOrder;
	std::vector<SortFunctor*> _sortFunctors;
	getStatFn_t _dynGetter;
	///initializes the display list based on the craft soldier's list and the position to display
	void initList(size_t scrl);
public:
	/// Creates the Craft Armor state.
	CovertOperationArmorState(Base* base, CovertOperationStartState* operation);
	/// Cleans up the Craft Armor state.
	~CovertOperationArmorState();
	/// Handler for changing the sort by combobox.
	void cbxSortByChange(Action* action);
	/// Updates the soldier armors.
	void init() override;
	/// Handler for clicking the Soldiers reordering button.
	void lstItemsLeftArrowClick(Action* action);
	/// Moves a soldier up.
	void moveSoldierUp(Action* action, unsigned int row, bool max = false);
	/// Handler for clicking the Soldiers reordering button.
	void lstItemsRightArrowClick(Action* action);
	/// Moves a soldier down.
	void moveSoldierDown(Action* action, unsigned int row, bool max = false);
	/// Handler for clicking the OK button.
	void btnOkClick(Action* action);
	/// Handler for clicking the Soldiers list.
	void lstSoldiersClick(Action* action);
	/// Handler for pressing-down a mouse-button in the list.
	void lstSoldiersMousePress(Action* action);
	/// Handler for clicking the De-equip All Armor button.
	void btnDeequipAllArmorClick(Action* action);
	void btnDeequipCraftArmorClick(Action* action);
};


/**
 * Select Armor window that allows changing
 * of the armor equipped on a soldier.
 */
class CovertOperationSoldierArmorState : public State
{
private:
	Base* _base;
	size_t _soldier;

	SoldierArmorOrigin _origin;
	TextButton* _btnCancel;
	Window* _window;
	Text* _txtTitle, * _txtType, * _txtQuantity;
	TextList* _lstArmor;
	ArrowButton* _sortName;
	std::vector<std::string> _allowedArmor;
	std::vector<ArmorItem> _armors;
	ArmorSort _armorOrder;
	void updateArrows();
public:
	/// Creates the Soldier Armor state.
	CovertOperationSoldierArmorState(Base* base, size_t soldier, SoldierArmorOrigin origin, std::vector<std::string> allowedArmor);
	/// Cleans up the Soldier Armor state.
	~CovertOperationSoldierArmorState();
	/// Sorts the armor list.
	void sortList();
	/// Updates the armor list.
	void updateList();
	/// Handler for clicking the Cancel button.
	void btnCancelClick(Action* action);
	/// Handler for clicking the Weapons list.
	void lstArmorClick(Action* action);
	/// Handler for clicking the Weapons list.
	void lstArmorClickMiddle(Action* action);
	/// Handler for clicking the Name arrow.
	void sortNameClick(Action* action);
};

}

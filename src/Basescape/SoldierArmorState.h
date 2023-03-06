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
#include <vector>
#include "../Engine/State.h"

namespace OpenXcom
{

enum SoldierArmorOrigin
{
	SA_GEOSCAPE,
	SA_BATTLESCAPE
};

class Base;
class TextButton;
class Window;
class Text;
class TextEdit;
class TextList;
class Armor;
class ArrowButton;

/// Armor sorting modes.
enum ArmorSort
{
	ARMOR_SORT_NONE,
	ARMOR_SORT_NAME_ASC,
	ARMOR_SORT_NAME_DESC,
};

struct ArmorItem
{
	ArmorItem(const std::string &_type, const std::string &_name, const std::string &_quantity) : type(_type), name(_name), quantity(_quantity)
	{
	}
	std::string type;
	std::string name, quantity;
};

/**
 * Select Armor window that allows changing
 * of the armor equipped on a soldier.
 */
class SoldierArmorState : public State
{
private:
	Base *_base;
	size_t _soldier;

	SoldierArmorOrigin _origin;
	TextButton *_btnCancel;
	TextEdit *_btnQuickSearch;
	Window *_window;
	Text *_txtTitle, *_txtType, *_txtQuantity;
	TextList *_lstArmor;
	ArrowButton *_sortName;
	std::vector<ArmorItem> _armors;
	std::vector<int> _indices;
	ArmorSort _armorOrder;
	void updateArrows();
public:
	/// Creates the Soldier Armor state.
	SoldierArmorState(Base *base, size_t soldier, SoldierArmorOrigin origin);
	/// Cleans up the Soldier Armor state.
	~SoldierArmorState();
	/// Sorts the armor list.
	void sortList();
	/// Updates the armor list.
	void updateList();
	/// Handler for clicking the Cancel button.
	void btnCancelClick(Action *action);
	/// Handlers for Quick Search.
	void btnQuickSearchToggle(Action* action);
	void btnQuickSearchApply(Action* action);
	/// Handler for clicking the Weapons list.
	void lstArmorClick(Action *action);
	/// Handler for clicking the Weapons list.
	void lstArmorClickMiddle(Action *action);
	/// Handler for clicking the Name arrow.
	void sortNameClick(Action *action);
};

}

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
#include "../Engine/State.h"
#include "DebriefingState.h"
#include <vector>

namespace OpenXcom
{

class TextButton;
class Window;
class Text;
class TextList;
class Base;

/**
 * Screen shown when there's not enough equipment
 * to re-equip a craft after a mission.
 */
class CannotReequipState : public State
{
private:
	std::vector<ReequipStat> _missingItems;
	Base *_base;

	TextButton *_btnOk, *_btnManufacture, *_btnPurchase;
	Window *_window;
	Text *_txtTitle, *_txtItem, *_txtQuantity, *_txtCraft;
	TextList *_lstItems;
public:
	/// Creates the Cannot Reequip state.
	CannotReequipState(std::vector<ReequipStat> &missingItems, Base *base);
	/// Cleans up the Cannot Reequip state.
	~CannotReequipState();
	/// Resets state.
	void init() override;
	/// Handler for clicking the OK button.
	void btnOkClick(Action *action);
	/// Handler for clicking the Manufacture button.
	void btnManufactureClick(Action *action);
	/// Handler for clicking the Purchase/Hire button.
	void btnPurchaseClick(Action *action);
	/// Gets the list of missing items.
	const std::vector<ReequipStat>& getMissingItems() const;
	// Decreases the number of missing items by the bought amount.
	void decreaseMissingItemCount(const RuleItem* rule, int amount);
};

}

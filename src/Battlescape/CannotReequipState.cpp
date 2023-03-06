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
#include "CannotReequipState.h"
#include <sstream>
#include "../Engine/Game.h"
#include "../Mod/Mod.h"
#include "../Engine/LocalizedText.h"
#include "../Interface/TextButton.h"
#include "../Interface/Window.h"
#include "../Interface/Text.h"
#include "../Interface/TextList.h"
#include "../Engine/Options.h"
#include "../Savegame/Base.h"
#include "../FTA/DiplomacyStartState.h"
#include "../Basescape/ManufactureState.h"
#include "../Basescape/PurchaseState.h"

namespace OpenXcom
{

/**
 * Initializes all the elements in the Cannot Reequip screen.
 * @param missingItems List of items still needed for reequip.
 * @param base Relevant xcom base.
 */
CannotReequipState::CannotReequipState(std::vector<ReequipStat> &missingItems, Base *base) : _missingItems(missingItems), _base(base)
{
	// Create objects
	_window = new Window(this, 320, 200, 0, 0);
	_btnManufacture = new TextButton(128, 14, 10, 178);
	_btnPurchase = new TextButton(128, 14, 144, 178);
	_btnOk = new TextButton(34, 14, 278, 178);
	_txtTitle = new Text(220, 32, 50, 8);
	_txtItem = new Text(142, 9, 10, 50);
	_txtQuantity = new Text(88, 9, 152, 50);
	_txtCraft = new Text(74, 9, 218, 50);
	_lstItems = new TextList(288, 112, 8, 58);

	// Set palette
	setInterface("cannotReequip");

	add(_window, "window", "cannotReequip");
	add(_btnManufacture, "button", "cannotReequip");
	add(_btnPurchase, "button", "cannotReequip");
	add(_btnOk, "button", "cannotReequip");
	add(_txtTitle, "heading", "cannotReequip");
	add(_txtItem, "text", "cannotReequip");
	add(_txtQuantity, "text", "cannotReequip");
	add(_txtCraft, "text", "cannotReequip");
	add(_lstItems, "list", "cannotReequip");

	centerAllSurfaces();

	// Set up objects
	setWindowBackground(_window, "cannotReequip");

	_btnManufacture->setText(tr("STR_MANUFACTURE"));
	_btnManufacture->onMouseClick((ActionHandler)&CannotReequipState::btnManufactureClick);

	if (_game->getMod()->isFTAGame())
	{
		_btnPurchase->setText(tr("STR_DIPLOMACY_UC"));
	}
	else
	{
		_btnPurchase->setText(tr("STR_PURCHASE_RECRUIT"));
	}
	_btnPurchase->onMouseClick((ActionHandler)&CannotReequipState::btnPurchaseClick);

	_btnOk->setText(tr("STR_OK"));
	_btnOk->onMouseClick((ActionHandler)&CannotReequipState::btnOkClick);
	_btnOk->onKeyboardPress((ActionHandler)&CannotReequipState::btnOkClick, Options::keyOk);
	_btnOk->onKeyboardPress((ActionHandler)&CannotReequipState::btnOkClick, Options::keyCancel);

	_txtTitle->setText(tr("STR_NOT_ENOUGH_EQUIPMENT_TO_FULLY_RE_EQUIP_SQUAD"));
	_txtTitle->setAlign(ALIGN_CENTER);
	_txtTitle->setBig();
	_txtTitle->setWordWrap(true);

	_txtItem->setText(tr("STR_ITEM"));

	_txtQuantity->setText(tr("STR_QUANTITY_UC"));

	_txtCraft->setText(tr("STR_CRAFT"));

	_lstItems->setColumns(3, 162, 46, 80);
	_lstItems->setSelectable(true);
	_lstItems->setBackground(_window);
	_lstItems->setMargin(2);
}

/**
 *
 */
CannotReequipState::~CannotReequipState()
{
}

/**
 * Resets stuff when coming back from other screens.
 */
void CannotReequipState::init()
{
	State::init();

	_lstItems->clearList();

	for (std::vector<ReequipStat>::iterator i = _missingItems.begin(); i != _missingItems.end(); ++i)
	{
		if (i->qty > 0)
		{
			std::ostringstream ss;
			ss << i->qty;
			_lstItems->addRow(3, tr(i->item).c_str(), ss.str().c_str(), i->craft.c_str());
		}
	}
}

/**
 * Returns to the previous screen.
 * @param action Pointer to an action.
 */
void CannotReequipState::btnOkClick(Action *)
{
	_game->popState();
}

/**
 * Goes to the Manufacture screen.
 * @param action Pointer to an action.
 */
void CannotReequipState::btnManufactureClick(Action *)
{
	_game->pushState(new ManufactureState(_base));
}

/**
 * Goes to the Purchase screen.
 * @param action Pointer to an action.
 */
void CannotReequipState::btnPurchaseClick(Action *)
{
	if (_game->getMod()->isFTAGame())
	{
		_game->pushState(new DiplomacyStartState(_base, false));
	}
	else
	{
		_game->pushState(new PurchaseState(_base, this));
	}
}

/**
 * Gets the list of missing items.
 */
const std::vector<ReequipStat>& CannotReequipState::getMissingItems() const
{
	return _missingItems;
}

/**
 * Decreases the number of missing items by the bought amount.
 * @param rule Type of item.
 * @param amount Number of items bought.
 */
void CannotReequipState::decreaseMissingItemCount(const RuleItem* rule, int amount)
{
	for (auto& i : _missingItems)
	{
		if (i.item == rule->getType())
		{
			i.qty = std::max(0, i.qty - amount);
			break;
		}
	}
}

}

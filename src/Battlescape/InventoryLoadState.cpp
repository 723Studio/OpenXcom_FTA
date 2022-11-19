/*
* Copyright 2010-2015 OpenXcom Developers.
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
#include "InventoryLoadState.h"
#include "InventoryState.h"
#include "../Engine/Game.h"
#include "../Mod/Mod.h"
#include "../Engine/LocalizedText.h"
#include "../Engine/Options.h"
#include "../Interface/TextButton.h"
#include "../Interface/Window.h"
#include "../Interface/Text.h"
#include "../Interface/TextList.h"
#include "../Savegame/SavedGame.h"

namespace OpenXcom
{

/**
* Initializes all the elements in the Load Inventory window.
*/
InventoryLoadState::InventoryLoadState(InventoryState *parent) : _parent(parent)
{
	_screen = false;

	// Create objects
	_window = new Window(this, 240, 136, 40, 36+1, POPUP_BOTH);
	_txtTitle = new Text(230, 16, 45, 44+3);
	_lstLayout = new TextList(208, 80, 48, 60);
	_btnCancel = new TextButton(120, 16, 90, 148);

	setStandardPalette("PAL_BATTLESCAPE");

	add(_window, "messageWindowBorder", "battlescape");
	add(_txtTitle, "messageWindows", "battlescape");
	add(_lstLayout, "optionLists", "battlescape");
	add(_btnCancel, "messageWindowButtons", "battlescape");

	centerAllSurfaces();

	// Set up objects
	_window->setHighContrast(true);
	_window->setBackground(_game->getMod()->getSurface("TAC00.SCR"));

	_txtTitle->setHighContrast(true);
	_txtTitle->setAlign(ALIGN_CENTER);
	_txtTitle->setText(tr("STR_LOAD_EQUIPMENT_TEMPLATE"));

	_lstLayout->setHighContrast(true);
	_lstLayout->setColumns(1, 192);
	_lstLayout->setSelectable(true);
	_lstLayout->setBackground(_window);
	_lstLayout->setMargin(8);
	_lstLayout->onMouseClick((ActionHandler)&InventoryLoadState::lstLayoutClick);

	_btnCancel->setHighContrast(true);
	_btnCancel->setText(tr("STR_CANCEL_UC"));
	_btnCancel->onMouseClick((ActionHandler)&InventoryLoadState::btnCancelClick);
	_btnCancel->onKeyboardPress((ActionHandler)&InventoryLoadState::btnCancelClick, Options::keyCancel);

	for (int i = 0; i < Options::oxceMaxEquipmentLayoutTemplates; ++i)
	{
		std::vector<EquipmentLayoutItem*> *item = _game->getSavedGame()->getGlobalEquipmentLayout(i);
		std::ostringstream ss;
		const std::string& armorName = _game->getSavedGame()->getGlobalEquipmentLayoutArmor(i);
		if (!armorName.empty())
		{
			ss << "[" << tr(armorName) << "] ";
		}
		if (item->empty())
		{
			ss << tr("STR_EMPTY_SLOT_N").arg(i + 1);
		}
		else
		{
			const std::string &itemName = _game->getSavedGame()->getGlobalEquipmentLayoutName(i);
			if (itemName.empty())
				ss << tr("STR_UNNAMED_SLOT_N").arg(i + 1);
			else
				ss << itemName;
		}
		_lstLayout->addRow(1, ss.str().c_str());
	}
}

/**
*
*/
InventoryLoadState::~InventoryLoadState()
{

}

/**
* Returns to the previous screen.
* @param action Pointer to an action.
*/
void InventoryLoadState::btnCancelClick(Action *)
{
	_game->popState();
}

/**
* Loads the template and returns to the previous screen.
* @param action Pointer to an action.
*/
void InventoryLoadState::lstLayoutClick(Action *)
{
	auto index = _lstLayout->getSelectedRow();
	bool armorChanged = _parent->loadGlobalLayoutArmor(index);
	_parent->setGlobalLayoutIndex(index, armorChanged);
	_game->popState();
}

}

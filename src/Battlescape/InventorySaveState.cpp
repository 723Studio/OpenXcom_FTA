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
#include "InventorySaveState.h"
#include "InventoryState.h"
#include "../Engine/Game.h"
#include "../Engine/Action.h"
#include "../Engine/Language.h"
#include "../Mod/Mod.h"
#include "../Engine/LocalizedText.h"
#include "../Engine/Options.h"
#include "../Interface/TextButton.h"
#include "../Interface/Window.h"
#include "../Interface/Text.h"
#include "../Interface/TextEdit.h"
#include "../Interface/TextList.h"
#include "../Savegame/SavedGame.h"

namespace OpenXcom
{

/**
* Initializes all the elements in the Save Inventory window.
*/
InventorySaveState::InventorySaveState(InventoryState *parent) : _parent(parent), _previousSelectedRow(-1), _selectedRow(-1)
{
	_screen = false;

	// Create objects
	_window = new Window(this, 240, 136, 40, 36 + 1, POPUP_BOTH);
	_txtTitle = new Text(230, 16, 45, 44 + 3);
	_lstLayout = new TextList(208, 80, 48, 60);
	_btnCancel = new TextButton(70, 16, 201, 148);
	_btnSave = new TextButton(70, 16, 50, 148);
	_btnSaveWithArmor = new TextButton(70, 16, 126, 148);
	_edtSave = new TextEdit(this, 188, 9, 0, 0);

	setStandardPalette("PAL_BATTLESCAPE");

	add(_window, "messageWindowBorder", "battlescape");
	add(_txtTitle, "messageWindows", "battlescape");
	add(_lstLayout, "optionLists", "battlescape");
	add(_btnCancel, "messageWindowButtons", "battlescape");
	add(_btnSave, "messageWindowButtons", "battlescape");
	add(_btnSaveWithArmor, "messageWindowButtons", "battlescape");
	add(_edtSave);

	centerAllSurfaces();

	// Set up objects
	_window->setHighContrast(true);
	_window->setBackground(_game->getMod()->getSurface("TAC00.SCR"));

	_txtTitle->setHighContrast(true);
	_txtTitle->setAlign(ALIGN_CENTER);
	_txtTitle->setText(tr("STR_SAVE_EQUIPMENT_TEMPLATE"));

	_lstLayout->setHighContrast(true);
	_lstLayout->setColumns(1, 192);
	_lstLayout->setSelectable(true);
	_lstLayout->setBackground(_window);
	_lstLayout->setMargin(8);
	_lstLayout->onMousePress((ActionHandler)&InventorySaveState::lstLayoutPress);

	_btnCancel->setHighContrast(true);
	_btnCancel->setText(tr("STR_CANCEL_UC"));
	_btnCancel->onMouseClick((ActionHandler)&InventorySaveState::btnCancelClick);
	_btnCancel->onKeyboardPress((ActionHandler)&InventorySaveState::btnCancelClick, Options::keyCancel);

	_btnSave->setHighContrast(true);
	_btnSave->setText(tr("STR_SAVE_UC"));
	_btnSave->onMouseClick((ActionHandler)&InventorySaveState::btnSaveClick);

	_btnSaveWithArmor->setHighContrast(true);
	std::string savePlus = tr("STR_SAVE_UC");
	savePlus += "+";
	_btnSaveWithArmor->setText(savePlus);
	_btnSaveWithArmor->onMouseClick((ActionHandler)&InventorySaveState::btnSaveWithArmorClick);

	_edtSave->setHighContrast(true);
	_edtSave->setColor(_lstLayout->getSecondaryColor());
	_edtSave->setVisible(false);
	_edtSave->onKeyboardPress((ActionHandler)&InventorySaveState::edtSaveKeyPress);

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
InventorySaveState::~InventorySaveState()
{

}

/**
* Returns to the previous screen.
* @param action Pointer to an action.
*/
void InventorySaveState::btnCancelClick(Action *)
{
	_game->popState();
}

/**
* Saves the selected template.
* @param action Pointer to an action.
*/
void InventorySaveState::btnSaveClick(Action *)
{
	if (_selectedRow != -1)
	{
		saveTemplate(false);
	}
}

/**
 * Saves the selected template (including the armor).
 * @param action Pointer to an action.
 */
void InventorySaveState::btnSaveWithArmorClick(Action*)
{
	if (_selectedRow != -1)
	{
		saveTemplate(true);
	}
}

/**
* Names the selected template.
* @param action Pointer to an action.
*/
void InventorySaveState::lstLayoutPress(Action *action)
{
	_previousSelectedRow = _selectedRow;
	_selectedRow = _lstLayout->getSelectedRow();
	if (_previousSelectedRow > -1)
	{
		_lstLayout->setCellText(_previousSelectedRow, 0, _selected);
	}
	_selected = _lstLayout->getCellText(_selectedRow, 0);

	if (_game->isRightClick(action) && _edtSave->isFocused())
	{
		_previousSelectedRow = -1;
		_selectedRow = -1;

		_edtSave->setText("");
		_edtSave->setVisible(false);
		_edtSave->setFocus(false, false);
		_lstLayout->setScrolling(true);
	}
	if (_game->isLeftClick(action))
	{
		_lstLayout->setCellText(_selectedRow, 0, "");

		std::size_t pos = _selected.find("] ");
		if (pos != std::string::npos)
		{
			// cut off the armor info
			_edtSave->setText(_selected.substr(pos + 2));
		}
		else
		{
			_edtSave->setText(_selected);
		}
		_edtSave->setX(_lstLayout->getColumnX(0));
		_edtSave->setY(_lstLayout->getRowY(_selectedRow));
		_edtSave->setVisible(true);
		_edtSave->setFocus(true, false);
		_lstLayout->setScrolling(false);
	}
}


/**
* Saves the selected template.
* @param action Pointer to an action.
*/
void InventorySaveState::edtSaveKeyPress(Action *action)
{
	if (action->getDetails()->key.keysym.sym == SDLK_RETURN ||
		action->getDetails()->key.keysym.sym == SDLK_KP_ENTER)
	{
		saveTemplate(false);
	}
}

/**
* Saves the selected template.
*/
void InventorySaveState::saveTemplate(bool includingArmor)
{
	if (_selectedRow >= 0 && _selectedRow < Options::oxceMaxEquipmentLayoutTemplates)
	{
		_game->getSavedGame()->setGlobalEquipmentLayoutName(_selectedRow, _edtSave->getText());
		_parent->saveGlobalLayout(_selectedRow, includingArmor);

		_game->popState();
	}
}

}

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
#include "DebriefingExtraStatsState.h"
#include "DebriefingState.h"
#include "../Interface/Window.h"
#include "../Interface/TextButton.h"
#include "../Interface/Text.h"
#include "../Interface/TextList.h"
#include "../Engine/Game.h"
#include "../Engine/LocalizedText.h"
#include "../Engine/Options.h"
#include "../Mod/Mod.h"
#include "../Savegame/SavedGame.h"
#include "../Savegame/Soldier.h"

namespace OpenXcom
{

/**
 * Initializes all the elements in the productions start screen.
 * @param game Pointer to the core game.
 * @param base Pointer to the base to get info from.
 * @param item The RuleManufacture to produce.
 */
DebriefingExtraStatsState::DebriefingExtraStatsState(DebriefingState* debriefing) :  _debriefing(debriefing)
{
	_screen = false;

	_window = new Window(this, 156, 200, 164, 0);
	_btnOk = new TextButton(124, 12, 180, 180);
	_txtTitle = new Text(124, 13, 180, 8);
	_txtName = new Text(62, 8, 180, 24);
	//_txtIncrease = new Text(59, 8, 245, 24);
	_lstStats = new TextList(124, 142, 180, 32);

	// Set palette
	setInterface("debriefing");

	add(_window, "window", "debriefing");
	add(_txtTitle, "text", "debriefing");
	add(_txtName, "text", "debriefing");
	//add(_txtIncrease, "text", "debriefing");
	add(_lstStats, "list", "debriefing");
	add(_btnOk, "button", "debriefing");

	centerAllSurfaces();

	setWindowBackground(_window, "debriefing");

	_txtTitle->setText(tr("STR_NON_COMBAT_STATS"));
	_txtTitle->setBig();
	_txtTitle->setAlign(ALIGN_LEFT);

	_txtTitle->setText(tr("STR_NON_COMBAT_STATS"));
	_txtTitle->setBig();
	_txtTitle->setAlign(ALIGN_LEFT);

	_txtName->setText(tr("STR_NAME_UC"));

	_lstStats->setColumns(2, 96, 28);
	_lstStats->setDot(true);

	_btnOk->setText(tr("STR_OK"));
	_btnOk->onMouseClick((ActionHandler)&DebriefingExtraStatsState::btnOKClick);
	_btnOk->onKeyboardPress((ActionHandler)&DebriefingExtraStatsState::btnOKClick, Options::keyOk);
	_btnOk->onKeyboardPress((ActionHandler)&DebriefingExtraStatsState::btnOKClick, Options::keyCancel);
}

/**
 * Returns to previous screen.
 * @param action A pointer to an Action.
 */
void DebriefingExtraStatsState::btnOKClick(Action *)
{
	_game->popState();
}


void DebriefingExtraStatsState::generateStatsList()
{
	auto soldierMap = _debriefing->getNonCombatStatIncreaseList();
	auto it = soldierMap.cbegin();
	int row = 0;

	while (it != soldierMap.cend())
	{
		auto soldier = it->first;
		auto stats = it->second;
		_lstStats->addRow(1, soldier->getName().c_str());
		row++;
		if (stats.biology > 0)
		{
			_lstStats->addRow(2, tr(UnitStats::getStatString(&UnitStats::designing, UnitStats::STATSTR_LC)).c_str(), stats.biology);
			_lstStats->setRowColor(row, _lstStats->getSecondaryColor());
			row++;
		}
		if (stats.hacking > 0)
		{
			_lstStats->addRow(2, tr(UnitStats::getStatString(&UnitStats::designing, UnitStats::STATSTR_LC)).c_str(), stats.hacking);
			_lstStats->setRowColor(row, _lstStats->getSecondaryColor());
			row++;
		}
	}
}
}

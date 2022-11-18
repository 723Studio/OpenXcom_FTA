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
#include "../Engine/Action.h"
#include "../Engine/Game.h"
#include "../Engine/LocalizedText.h"
#include "../Engine/Options.h"
#include "../Engine/Screen.h"
#include "../Interface/Text.h"
#include "../Interface/TextButton.h"
#include "../Interface/TextList.h"
#include "../Interface/Window.h"
#include "../Mod/Mod.h"
#include "../Mod/RuleSoldier.h"
#include "../Mod/RuleResearch.h"
#include "../Mod/RuleInterface.h"
#include "../Savegame/Base.h"
#include "../Savegame/ResearchProject.h"
#include "../Savegame/Soldier.h"
#include "ScientistsState.h"
#include "SoldierInfoState.h"
#include <climits>

namespace OpenXcom
{

/**
 * Initializes all the elements in the Soldiers screen.
 * @param game Pointer to the core game.
 * @param base Pointer to the base to get info from.
 */
ScientistsState::ScientistsState(Base *base) : _base(base)
{
	// Create objects
	_window = new Window(this, 320, 200, 0, 0);
	_btnOk = new TextButton(288, 16, 16, 176);
	_txtTitle = new Text(168, 17, 16, 8);
	_txtName = new Text(114, 9, 16, 32);
	_txtProject = new Text(102, 9, 122, 32);
	_lstScientist = new TextList(288, 128, 8, 40);

	// Set palette
	setInterface("scientistsList");

	add(_window, "window", "scientistsList");
	add(_btnOk, "button", "scientistsList");
	add(_txtTitle, "text", "scientistsList");
	add(_txtName, "text", "scientistsList");
	add(_txtProject, "text", "scientistsList");
	add(_lstScientist, "list", "scientistsList");

	_otherDutyColor = _game->getMod()->getInterface("scientistsList")->getElement("otherDuty")->color;

	centerAllSurfaces();

	// Set up objects
	setWindowBackground(_window, "scientistsList");

	_btnOk->setText(tr("STR_OK"));
	_btnOk->onMouseClick((ActionHandler)&ScientistsState::btnOkClick);
	_btnOk->onKeyboardPress((ActionHandler)&ScientistsState::btnOkClick, Options::keyCancel);

	_txtTitle->setBig();
	_txtTitle->setAlign(ALIGN_LEFT);
	_txtTitle->setText(tr("STR_SCIENTISTS_LIST"));

	_txtName->setText(tr("STR_NAME_UC"));

	_txtProject->setText(tr("STR_ASSIGNMENT"));

	_lstScientist->setColumns(2, 106, 194); // 174?
	_lstScientist->setAlign(ALIGN_RIGHT, 2);
	_lstScientist->setSelectable(true);
	_lstScientist->setBackground(_window);
	_lstScientist->setMargin(8);
	_lstScientist->onMouseClick((ActionHandler)&ScientistsState::lstScientistsClick);
}

/**
 * cleans up dynamic state
 */
ScientistsState::~ScientistsState()
{
}

/**
 * Updates the soldiers list
 * after going to other screens.
 */
void ScientistsState::init()
{
	State::init();
	_base->prepareSoldierStatsWithBonuses(); // refresh stats for sorting
	initList(0);

	
}

void ScientistsState::initList(size_t scrl)
{
	_lstScientist->clearList();
	_soldierNumbers.clear();

	auto recovery = _base->getSumRecoveryPerDay();
	bool isBusy = false, isFree = false;
	unsigned int row = 0;
	unsigned int it = 0;
	for (std::vector<Soldier *>::iterator i = _base->getSoldiers()->begin(); i != _base->getSoldiers()->end(); ++i)
	{
		if ((*i)->getRoleRank(ROLE_SCIENTIST) > 0)
		{
			_soldierNumbers.push_back(it); // don't forget soldier's number on the base!
			std::string duty = (*i)->getCurrentDuty(_game->getLanguage(), recovery, isBusy, isFree);
			Uint8 color;
			if ((*i)->getResearchProject() == 0)
			{
				_lstScientist->addRow(2, (*i)->getName(true).c_str(), duty.c_str());
				if (isBusy || !isFree)
				{
					color = _otherDutyColor;
				}
				else
				{
					color = _lstScientist->getSecondaryColor();
				}
			}
			else
			{
				_lstScientist->addRow(2, (*i)->getName(true).c_str(), tr((*i)->getResearchProject()->getRules()->getName()).c_str());
				color = _lstScientist->getColor();
			}

			_lstScientist->setRowColor(row, color);
			row++;
		}
		it++;
	}
}

/**
 * Returns to the previous screen.
 * @param action Pointer to an action.
 */
void ScientistsState::btnOkClick(Action *)
{
	_game->popState();
}

/**
 * Shows the selected soldier's info.
 * @param action Pointer to an action.
 */
void ScientistsState::lstScientistsClick(Action *action)
{
	_game->pushState(new SoldierInfoState(_base, _soldierNumbers.at(_lstScientist->getSelectedRow())));
}

}

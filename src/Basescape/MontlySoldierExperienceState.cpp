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
#include "MontlySoldierExperienceState.h"
#include "../Engine/Screen.h"
#include "../Engine/Game.h"
#include "../Mod/Mod.h"
#include "../Mod/RuleSoldierTransformation.h"
#include "../Mod/RuleSoldier.h"
#include "../Engine/LocalizedText.h"
#include "../Engine/Options.h"
#include "../Engine/Action.h"
#include "../Geoscape/AllocatePsiTrainingState.h"
#include "../Interface/TextButton.h"
#include "../Interface/Window.h"
#include "../Interface/Text.h"
#include "../Interface/TextList.h"
#include "../Savegame/Base.h"
#include "../Savegame/Soldier.h"
#include "../Savegame/SavedGame.h"

namespace OpenXcom
{

/**
 * Initializes all the elements in the Soldiers screen.
 * @param base Pointer to the base to get info from (optional, all bases are used if it 0).
 */
MontlySoldierExperienceState::MontlySoldierExperienceState(Base *base): _base(base)
{
	if (!_base)
	{
		for (auto b : *_game->getSavedGame()->getBases())
		{
			_bases.push_back(b);
		}
	}
	else
	{
		_bases.push_back(_base);
	}
	// Create objects
	_window = new Window(this, 320, 200, 0, 0);
	_btnOk = new TextButton(288, 16, 16, 176);
	_txtTitle = new Text(168, 17, 16, 8);
	_txtName = new Text(114, 9, 16, 32);
	_txtRank = new Text(102, 9, 122, 32);
	_txtCraft = new Text(82, 9, 220, 32);
	_lstSoldiers = new TextList(288, 128, 8, 40);

	// Set palette
	setInterface("soldierExpList");

	add(_window, "window", "soldierExpList");
	add(_btnOk, "button", "soldierExpList");
	add(_txtTitle, "text1", "soldierExpList");
	add(_txtName, "text2", "soldierExpList");
	add(_txtRank, "text2", "soldierExpList");
	add(_txtCraft, "text2", "soldierExpList");
	add(_lstSoldiers, "list", "soldierExpList");

	centerAllSurfaces();

	// Set up objects
	setWindowBackground(_window, "soldierExpList");

	_btnOk->setText(tr("STR_OK"));
	_btnOk->onMouseClick((ActionHandler)&MontlySoldierExperienceState::btnOkClick);
	_btnOk->onKeyboardPress((ActionHandler)&MontlySoldierExperienceState::btnOkClick, Options::keyCancel);

	_txtTitle->setBig();
	_txtTitle->setAlign(ALIGN_LEFT);
	_txtTitle->setText(tr("STR_SOLDIER_LIST"));

	_txtName->setText(tr("STR_NAME_UC"));

	_txtRank->setText(tr("STR_RANK"));

	_txtCraft->setText(tr("STR_ASSIGNMENT"));
	if (_bases.size() > 1)
	{
		_txtCraft->setText(tr("STR_BASE_UC"));
	}

	_lstSoldiers->setColumns(3, 106, 98, 76);
	_lstSoldiers->setAlign(ALIGN_RIGHT, 3);
	//_lstSoldiers->setSelectable(true);
	_lstSoldiers->setBackground(_window);
	_lstSoldiers->setMargin(8);
}

/**
 * cleans up dynamic state
 */
MontlySoldierExperienceState::~MontlySoldierExperienceState()
{
}

/**
 * Updates the soldiers list
 * after going to other screens.
 */
void MontlySoldierExperienceState::init()
{
	State::init();
	initList(0);
}

/**
 * Shows the soldiers in a list at specified offset/scroll.
 */
void MontlySoldierExperienceState::initList(size_t scrl)
{
	_lstSoldiers->clearList();

	for (auto b : _bases)
	{
		unsigned int soldierRow = 0;
		unsigned int statRow = 0;
		auto recovery = b->getSumRecoveryPerDay();
		bool isBusy = false, isFree = false;
		for (std::vector<Soldier*>::iterator s = b->getSoldiers()->begin(); s != b->getSoldiers()->end(); ++s)
		{
			if (_bases.size() == 1)
			{
				std::string duty = (*s)->getCurrentDuty(_game->getLanguage(), recovery, isBusy, isFree);
				_lstSoldiers->addRow(3, (*s)->getName(true).c_str(), tr((*s)->getRankString(true)).c_str(), duty.c_str());
			}
			else
			{
				_lstSoldiers->addRow(3, (*s)->getName(true).c_str(), tr((*s)->getRankString(true)).c_str(), b->getName().c_str());
			}

			UnitStats *exp = (*s)->getMonthlyExperienceCache();
			exp->fieldLoop(
				[&](UnitStats::Ptr p)
				{
					if (exp->*p > 0)
					{
						std::ostringstream ss;
						ss << "  " << tr(UnitStats::getStatString(p, UnitStats::STATSTR_LC));
						std::ostringstream stat;
						stat << exp->*p;
						_lstSoldiers->addRow(2, ss.str().c_str(), stat.str().c_str());
						statRow++;
						_lstSoldiers->setRowColor(statRow, _lstSoldiers->getSecondaryColor());
					}
					
				}
			);

			if ((*s)->getDeath())
			{
				_lstSoldiers->setRowColor(soldierRow, _txtCraft->getColor());
			}

			statRow++;
			soldierRow++;
		}
	}
}

/**
 * Returns to the previous screen.
 * @param action Pointer to an action.
 */
void MontlySoldierExperienceState::btnOkClick(Action*)
{
	_game->popState();
}

}

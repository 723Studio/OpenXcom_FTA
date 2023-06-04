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
#include "CommendationState.h"
#include <sstream>
#include "../Engine/Game.h"
#include "../Mod/Mod.h"
#include "../Engine/LocalizedText.h"
#include "../Interface/TextButton.h"
#include "../Interface/Window.h"
#include "../Interface/Text.h"
#include "../Interface/TextList.h"
#include "../Savegame/Soldier.h"
#include "../Savegame/SoldierDiary.h"
#include "../Engine/Options.h"
#include "../Mod/RuleCommendations.h"
#include "../Ufopaedia/Ufopaedia.h"

namespace OpenXcom
{

/**
 * Initializes all the elements in the Medals screen.
 * @param soldiersMedalled List of soldiers with medals.
 */
CommendationState::CommendationState(std::vector<Soldier*> soldiersMedalled)
{
	// Create object
	_window = new Window(this, 320, 200, 0, 0);
	_btnOk = new TextButton(288, 16, 16, 176);
	_txtTitle = new Text(300, 16, 10, 8);
	_lstSoldiers = new TextList(288, 128, 8, 32);

	// Set palette
	setInterface("commendations");

	add(_window, "window", "commendations");
	add(_btnOk, "button", "commendations");
	add(_txtTitle, "heading", "commendations");
	add(_lstSoldiers, "list", "commendations");

	centerAllSurfaces();

	// Set up object
	setWindowBackground(_window, "commendations");

	_btnOk->setText(tr("STR_OK"));
	_btnOk->onMouseClick((ActionHandler)&CommendationState::btnOkClick);
	_btnOk->onKeyboardPress((ActionHandler)&CommendationState::btnOkClick, Options::keyOk);
	_btnOk->onKeyboardPress((ActionHandler)&CommendationState::btnOkClick, Options::keyCancel);

	_txtTitle->setText(tr("STR_MEDALS"));
	_txtTitle->setAlign(ALIGN_CENTER);
	_txtTitle->setBig();

	_lstSoldiers->setColumns(2, 204, 84);
	_lstSoldiers->setSelectable(true);
	_lstSoldiers->setBackground(_window);
	_lstSoldiers->setMargin(8);
	_lstSoldiers->onMouseClick((ActionHandler)&CommendationState::lstSoldiersMouseClick);

	int row = 0;
	int titleRow = 0;
	const auto& commendationsList = _game->getMod()->getCommendationsList();
	bool modularCommendation;
	std::string noun;
	bool titleChosen = true;

	for (auto commIter = commendationsList.begin(); commIter != commendationsList.end();)
	{
		const auto& commType = (*commIter).first;
		const auto* commRule = (*commIter).second;

		modularCommendation = false;
		noun = "noNoun";
		if (titleChosen)
		{
			_lstSoldiers->addRow(2, "", ""); // Blank row, will be filled in later
			_commendationsNames.push_back("");
			row++;
		}
		titleChosen = false;
		titleRow = row - 1;

		for (auto* soldier : soldiersMedalled)
		{
			for (auto* soldierComm : *soldier->getDiary()->getSoldierCommendations())
			{
				if (soldierComm->getType() == commType && soldierComm->isNew() && noun == "noNoun")
				{
					soldierComm->makeOld();
					row++;

					if (soldierComm->getNoun() != "noNoun")
					{
						noun = soldierComm->getNoun();
						modularCommendation = true;
					}

					// Soldier name
					std::ostringstream wssName;
					wssName << "   ";
					wssName << soldier->getName();
					// Decoration level name
					int skipCounter = 0;
					int lastInt = -2;
					int thisInt = -1;
					int vectorIterator = 0;
					for (auto wtf = commRule->getCriteria()->begin()->second.begin(); wtf != commRule->getCriteria()->begin()->second.end(); ++wtf)
					{
						if (vectorIterator == soldierComm->getDecorationLevelInt() + 1)
						{
							break;
						}
						thisInt = *wtf;
						if (wtf != commRule->getCriteria()->begin()->second.begin())
						{
							--wtf;
							lastInt = *wtf;
							++wtf;
						}
						if (thisInt == lastInt)
						{
							skipCounter++;
						}
						vectorIterator++;
					}
					_lstSoldiers->addRow(2, wssName.str().c_str(), tr(soldierComm->getDecorationLevelName(skipCounter)).c_str());
					_commendationsNames.push_back("");
					break;
				}
			}
		}
		if (titleRow != row - 1)
		{
			// Medal name
			if (modularCommendation)
			{
				_lstSoldiers->setCellText(titleRow, 0, tr(commType).arg(tr(noun)));
			}
			else
			{
				_lstSoldiers->setCellText(titleRow, 0, tr(commType));
			}
			_lstSoldiers->setRowColor(titleRow, _lstSoldiers->getSecondaryColor());
			_commendationsNames[titleRow] = commType;
			titleChosen = true;
		}
		if (noun == "noNoun")
		{
			++commIter;
		}
	}
}

/**
 *
 */
CommendationState::~CommendationState()
{
}

/*
*
*/
void CommendationState::lstSoldiersMouseClick(Action *)
{
	Ufopaedia::openArticle(_game, _commendationsNames[_lstSoldiers->getSelectedRow()]);
}

/**
 * Returns to the previous screen.
 * @param action Pointer to an action.
 */
void CommendationState::btnOkClick(Action *)
{
	_game->popState();
}

}

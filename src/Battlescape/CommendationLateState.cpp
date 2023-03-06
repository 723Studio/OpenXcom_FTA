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
#include "CommendationLateState.h"
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
CommendationLateState::CommendationLateState(std::vector<Soldier*> soldiersMedalled)
{
	// Create object
	_window = new Window(this, 320, 200, 0, 0);
	_btnOk = new TextButton(288, 16, 16, 176);
	_txtTitle = new Text(300, 16, 10, 8);
	_lstSoldiers = new TextList(288, 128, 8, 32);

	// Set palette
	setInterface("commendationsLate");

	add(_window, "window", "commendationsLate");
	add(_btnOk, "button", "commendationsLate");
	add(_txtTitle, "text", "commendationsLate");
	add(_lstSoldiers, "list", "commendationsLate");

	centerAllSurfaces();

	// Set up object
	setWindowBackground(_window, "commendationsLate");

	_btnOk->setText(tr("STR_OK"));
	_btnOk->onMouseClick((ActionHandler)&CommendationLateState::btnOkClick);
	_btnOk->onKeyboardPress((ActionHandler)&CommendationLateState::btnOkClick, Options::keyOk);
	_btnOk->onKeyboardPress((ActionHandler)&CommendationLateState::btnOkClick, Options::keyCancel);

	_txtTitle->setBig();
	_txtTitle->setAlign(ALIGN_CENTER);
	_txtTitle->setText(tr("STR_LOST_IN_SERVICE"));

	_lstSoldiers->setColumns(3, 114, 90, 84);
	_lstSoldiers->setSelectable(true);
	_lstSoldiers->setBackground(_window);
	_lstSoldiers->setMargin(8);
	_lstSoldiers->setFlooding(true);
	_lstSoldiers->onMouseClick((ActionHandler)&CommendationLateState::lstSoldiersMouseClick);

    /***

                                            LOST IN SERVICE

    SOLDIER NAME, RANK: ___, SCORE: ___, KILLS: ___, NUMBER OF MISSIONS: ___, DAYS WOUNDED: ___, TIMES HIT: ___
      COMMENDATION
      COMMENDATION
      COMMENDATION
      CAUSE OF DEATH: KILLED BY ALIEN_RACE ALIEN_RANK, USING WEAPON


    ***/

	const std::map<std::string, RuleCommendations *> commendationsList = _game->getMod()->getCommendationsList();
	bool modularCommendation;
	std::string noun;


	int row = 0;
	// Loop over dead soldiers
	for (std::vector<Soldier*>::iterator s = soldiersMedalled.begin() ; s != soldiersMedalled.end(); ++s)
	{
		// Establish some base information
		bool fta = _game->getMod()->isFTAGame();
		_lstSoldiers->addRow(
			3,
			(*s)->getName().c_str(),
			tr((*s)->getRankString(fta)).c_str(),
			tr("STR_KILLS").arg((*s)->getDiary()->getKillTotal()).c_str());
		_lstSoldiers->setRowColor(row, _lstSoldiers->getSecondaryColor());
		_commendationsNames.push_back("");
		row++;

		// Loop over all commendation
		for (std::map<std::string, RuleCommendations *>::const_iterator commList = commendationsList.begin(); commList != commendationsList.end();)
		{
			std::ostringstream wssCommendation;
			modularCommendation = false;
			noun = "noNoun";

			// Loop over soldier's commendation
			for (std::vector<SoldierCommendations*>::const_iterator soldierComm = (*s)->getDiary()->getSoldierCommendations()->begin(); soldierComm != (*s)->getDiary()->getSoldierCommendations()->end(); ++soldierComm)
			{
				if ((*soldierComm)->getType() == (*commList).first && (*soldierComm)->isNew() && noun == "noNoun")
				{
					(*soldierComm)->makeOld();

					if ((*soldierComm)->getNoun() != "noNoun")
					{
						noun = (*soldierComm)->getNoun();
						modularCommendation = true;
					}
					// Decoration level name
					int skipCounter = 0;
					int lastInt = -2;
					int thisInt = -1;
					int vectorIterator = 0;
					for (std::vector<int>::const_iterator k = (*commList).second->getCriteria()->begin()->second.begin(); k != (*commList).second->getCriteria()->begin()->second.end(); ++k)
					{
						if (vectorIterator == (*soldierComm)->getDecorationLevelInt() + 1)
						{
							break;
						}
						thisInt = *k;
						if (k != (*commList).second->getCriteria()->begin()->second.begin())
						{
							--k;
							lastInt = *k;
							++k;
						}
						if (thisInt == lastInt)
						{
							skipCounter++;
						}
						vectorIterator++;
					}
					// Establish commendation's name
					// Medal name
					wssCommendation << "   ";
					if (modularCommendation)
					{
						wssCommendation << tr((*commList).first).arg(tr(noun));
					}
					else
					{
						wssCommendation << tr((*commList).first);
					}
					_lstSoldiers->addRow(3, wssCommendation.str().c_str(), "", tr((*soldierComm)->getDecorationLevelName(skipCounter)).c_str());
					_commendationsNames.push_back((*commList).first);
					row++;
					break;
				}
			} // END SOLDIER COMMS LOOP

			if (noun == "noNoun")
			{
				++commList;
			}
		} // END COMMS LOOPS
		_lstSoldiers->addRow(3, "", "", ""); // Separator
		_commendationsNames.push_back("");
		row++;
	} // END SOLDIER LOOP    
}

/**
 *
 */
CommendationLateState::~CommendationLateState()
{
}

/*
*
*/
void CommendationLateState::lstSoldiersMouseClick(Action *)
{
	Ufopaedia::openArticle(_game, _commendationsNames[_lstSoldiers->getSelectedRow()]);
}

/**
 * Returns to the previous screen.
 * @param action Pointer to an action.
 */
void CommendationLateState::btnOkClick(Action *)
{
	_game->popState();
}

}

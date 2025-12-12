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
#include "AgentsState.h"
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
#include "../Mod/RuleInterface.h"
#include "../Savegame/Base.h"
#include "../Savegame/Soldier.h"
#include "SoldierInfoStateFtA.h"
#include "../Mod/RuleIntelProject.h"
#include "../Mod/RuleManufacture.h"
#include "../Savegame/BasePrisoner.h"
#include "../Savegame/IntelProject.h"

namespace OpenXcom
{

/**
 * Initializes all the elements in the Soldiers screen.
 * @param game Pointer to the core game.
 * @param base Pointer to the base to get info from.
 */
AgentsState::AgentsState(Base *base) : _base(base)
{
	// Create objects
	_window = new Window(this, 320, 200, 0, 0);
	_btnOk = new TextButton(288, 16, 16, 176);
	_txtTitle = new Text(168, 17, 16, 8);
	_txtName = new Text(114, 9, 16, 32);
	_txtProject = new Text(102, 9, 122, 32);
	_lstAgents = new TextList(288, 128, 8, 40);

	// Set palette
	setInterface("agentsList");

	add(_window, "window", "agentsList");
	add(_btnOk, "button", "agentsList");
	add(_txtTitle, "text", "agentsList");
	add(_txtName, "text", "agentsList");
	add(_txtProject, "text", "agentsList");
	add(_lstAgents, "list", "agentsList");

	_otherDutyColor = _game->getMod()->getInterface("agentsList")->getElement("otherDuty")->color;

	centerAllSurfaces();

	// Set up objects
	setWindowBackground(_window, "agentsList");

	_btnOk->setText(tr("STR_OK"));
	_btnOk->onMouseClick((ActionHandler)&AgentsState::btnOkClick);
	_btnOk->onKeyboardPress((ActionHandler)&AgentsState::btnOkClick, Options::keyCancel);

	_txtTitle->setBig();
	_txtTitle->setAlign(ALIGN_LEFT);
	_txtTitle->setText(tr("STR_AGENTS_LIST"));

	_txtName->setText(tr("STR_NAME_UC"));

	_txtProject->setText(tr("STR_ASSIGNMENT"));

	_lstAgents->setColumns(2, 106, 194); // 174?
	_lstAgents->setAlign(ALIGN_RIGHT, 2);
	_lstAgents->setSelectable(true);
	_lstAgents->setBackground(_window);
	_lstAgents->setMargin(8);
	_lstAgents->onMouseClick((ActionHandler)&AgentsState::lstAgentsClick);
}

/**
 * cleans up dynamic state
 */
AgentsState::~AgentsState()
{
}

/**
 * Updates the soldiers list
 * after going to other screens.
 */
void AgentsState::init()
{
	State::init();
	_base->prepareSoldierStatsWithBonuses(); // refresh stats for sorting
	initList(0);

	
}

void AgentsState::initList(size_t scrl)
{
	_lstAgents->clearList();
	_soldierNumbers.clear();

	auto recovery = _base->getSumRecoveryPerDay();
	bool isBusy = false, isFree = false;
	unsigned int row = 0;
	unsigned int it = 0;
	for (std::vector<Soldier *>::iterator i = _base->getSoldiers()->begin(); i != _base->getSoldiers()->end(); ++i)
	{
		if ((*i)->getRoleRank(ROLE_AGENT) > 0)
		{
			_soldierNumbers.push_back(it); // don't forget soldier's number on the base!
			std::string duty = (*i)->getCurrentDuty(_game->getLanguage(), recovery, isBusy, isFree);
			Uint8 color;
			auto project = (*i)->getIntelProject();
			auto prisoner = (*i)->getActivePrisoner();
			if (project == 0 && prisoner == 0)
			{
				_lstAgents->addRow(2, (*i)->getName(true).c_str(), duty.c_str());
				if (isBusy || !isFree)
				{
					color = _otherDutyColor;
				}
				else
				{
					color = _lstAgents->getSecondaryColor();
				}
			}
			else
			{
				std::ostringstream ss;
				if (project)
				{
					ss << tr(project->getRules()->getName());
				}
				else if (prisoner)
				{
					auto pState = prisoner->getPrisonerState();
					if (pState == PRISONER_STATE_INTERROGATION)
					{
						ss << tr("STR_INTERROGATING") << ": ";
					}
					else if (pState == PRISONER_STATE_TORTURE)
					{
						ss << tr("STR_TORTURING") << ": ";
					}
					else if (pState == PRISONER_STATE_REQRUITING)
					{
						ss << tr("STR_RECRUITING") << ": ";
					}
					ss << prisoner->getNameAndId(_game->getLanguage());
				}
				_lstAgents->addRow(2, (*i)->getName(true).c_str(), ss.str().c_str());
				color = _lstAgents->getColor();
			}
			_lstAgents->setRowColor(row, color);
			row++;
		}
		it++;
	}
}

/**
 * Returns to the previous screen.
 * @param action Pointer to an action.
 */
void AgentsState::btnOkClick(Action *)
{
	_game->popState();
}

/**
 * Shows the selected soldier's info.
 * @param action Pointer to an action.
 */
void AgentsState::lstAgentsClick(Action *action)
{
	_game->pushState(new SoldierInfoStateFtA(_base, _soldierNumbers.at(_lstAgents->getSelectedRow())));
}

}

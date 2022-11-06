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
#include "PrisonManagementState.h"
#include "PrisonerInfoState.h"
#include <sstream>
#include "../Engine/Action.h"
#include "../Engine/Game.h"
#include "../Engine/LocalizedText.h"
#include "../Interface/TextButton.h"
#include "../Interface/Window.h"
#include "../Interface/Text.h"
#include "../Interface/TextList.h"
#include "../Savegame/Base.h"
#include "../Savegame/SavedGame.h"
#include "../Savegame/BasePrisoner.h"
#include "../Savegame/Soldier.h"
#include "../Mod/Mod.h"
#include "../Mod/RulePrisoner.h"

namespace OpenXcom
{

/**
 * Initializes all the elements in the PrisonManagementState.
 * @param base Pointer to the base to get info from.
 */
PrisonManagementState::PrisonManagementState(Base *base) : _base(base)
{
	// Create objects
	_window = new Window(this, 320, 200, 0, 0);
	_btnOk = new TextButton(304, 16, 8, 176);
	_txtTitle = new Text(300, 17, 10, 8);
	_txtAvailable = new Text(150, 9, 10, 24);
	_txtAllocated = new Text(150, 9, 160, 24);
	_txtSpace = new Text(300, 9, 10, 34);
	_txtPrisoner = new Text(110, 17, 10, 44);
	_txtAgents = new Text(106, 17, 120, 44);
	_txtState = new Text(84, 9, 226, 44);
	_lstPrisoners = new TextList(288, 112, 8, 62);

	// Set palette
	setInterface("prisonManagement");

	add(_window, "window", "prisonManagement");
	add(_btnOk, "button", "prisonManagement");
	add(_txtTitle, "text1", "prisonManagement");
	add(_txtAvailable, "text1", "prisonManagement");
	add(_txtAllocated, "text1", "prisonManagement");
	add(_txtSpace, "text1", "prisonManagement");
	add(_txtPrisoner, "text2", "prisonManagement");
	add(_txtAgents, "text2", "prisonManagement");
	add(_txtState, "text2", "prisonManagement");
	add(_lstPrisoners, "list", "prisonManagement");

	centerAllSurfaces();

	// Set up objects
	setWindowBackground(_window, "prisonManagement");

	_btnOk->setText(tr("STR_OK"));
	_btnOk->onMouseClick((ActionHandler)&PrisonManagementState::btnOkClick);
	_btnOk->onKeyboardPress((ActionHandler)&PrisonManagementState::btnOkClick, Options::keyCancel);

	_txtTitle->setBig();
	_txtTitle->setAlign(ALIGN_CENTER);
	_txtTitle->setText(tr("STR_PRISON_MANAGEMENT"));

	_txtPrisoner->setText(tr("STR_PRISONER_NAME"));

	_txtAgents->setAlign(ALIGN_CENTER);
	_txtAgents->setText(tr("STR_AGENTS_ALLOCATED_UC"));

	_txtState->setWordWrap(true);
	_txtState->setText(tr("STR_PRISONER_STATE"));

	_lstPrisoners->setColumns(3, 158, 58, 70);
	_lstPrisoners->setSelectable(true);
	_lstPrisoners->setBackground(_window);
	_lstPrisoners->setMargin(2);
	_lstPrisoners->setWordWrap(true);
	_lstPrisoners->onMouseClick((ActionHandler)&PrisonManagementState::onSelectPrisoner, SDL_BUTTON_LEFT);

	// as we don't change soldier duty here, we can set the string just once here
	bool isBusy = false, isFree = false;
	const auto recovery = _base->getSumRecoveryPerDay();
	size_t n = 0;
	for (auto s : *_base->getSoldiers())
	{
		std::string duty = s->getCurrentDuty(_game->getLanguage(), recovery, isBusy, isFree, INTEL);
		if (s->getRoleRank(ROLE_AGENT) > 0 )
		{
			n++;
		}
	}
	_txtAvailable->setText(tr("STR_AVAILABLE_AGENTS").arg(n));
}


/**
 * Returns to the previous screen.
 * @param action Pointer to an action.
 */
void PrisonManagementState::btnOkClick(Action *)
{
	_game->popState();
}

/**
 * Opens state with selected Research Project
 * @param action Pointer to an action.
 */
void PrisonManagementState::onSelectPrisoner(Action *)
{
	auto prisoner = _base->getPrisoners()[_lstPrisoners->getSelectedRow()];
	_game->pushState(new PrisonerInfoState(_base, prisoner, prisoner->getRules()));
}

/**
 * Updates the research list
 * after going to other screens.
 */
void PrisonManagementState::init()
{
	State::init();
	fillPrisonList(0);
}

/**
 * Fills the list with Base Prisoners.
 */
void PrisonManagementState::fillPrisonList(size_t scrl)
{
	_lstPrisoners->clearList();
	for (auto p : _base->getPrisoners())
	{
		std::ostringstream status, assigned;
		auto pState = p->getPrisonerState();
		bool dying = p->getRules()->getDamageOverTime() > 0 && pState != PRISONER_STATE_CONTAINING;
		bool hasActiveState = true;
		switch (pState)
		{
		case PRISONER_STATE_NONE:
			hasActiveState = false;
			break;
		case PRISONER_STATE_CONTAINING:
			status << tr("STR_PRISONER_STATE_CONTAINING");
			break;
		case PRISONER_STATE_INTERROGATION:
			status << tr("STR_PRISONER_STATE_INTERROGATION");
			break;
		case PRISONER_STATE_TORTURE:
			status << tr("STR_PRISONER_STATE_TORTURE");
			break;
		case PRISONER_STATE_REQRUITING:
			status << tr("STR_PRISONER_STATE_RECRUIT");
			break;
		}

		if (hasActiveState && dying)
		{
			status << ", ";
			status << tr("STR_DYING_LC");
		}
		else if (dying)
		{
			status << tr("STR_DYING");
		}

		size_t n = 0;
		for (auto s : *_base->getSoldiers())
		{
			if (s->getActivePrisoner() == p)
			{
				n++;
			}
		}
		assigned << n;

		_lstPrisoners->addRow(3, p->getNameAndId().c_str(), assigned.str().c_str() ,status.str().c_str());
	}

	auto recovery = _base->getSumRecoveryPerDay();
	size_t freeAgents = 0, busyAgents = 0;
	bool isBusy = false, isFree = false;
	for (auto a : _base->getPersonnel(ROLE_AGENT))
	{
		a->getCurrentDuty(_game->getLanguage(), recovery, isBusy, isFree, LAB);
		if (!isBusy && isFree)
		{
			freeAgents++;
		}
		if (a->getActivePrisoner())
		{
			busyAgents++;
		}
	}

	_txtAvailable->setText(tr("STR_AGENTS_AVAILABLE").arg(freeAgents));
	_txtAllocated->setText(tr("STR_AGENTS_ALLOCATED").arg(busyAgents));
	_txtSpace->setText(tr("STR_FREE_INTERROGATION_SPACE").arg(_base->getFreeInterrogationSpace()));

	if (scrl)
	{
		_lstPrisoners->scrollTo(scrl);
	}
}

}

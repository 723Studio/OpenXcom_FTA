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
#include "PrisonerInfoState.h"
#include "ResearchAllocateScientistsState.h"
#include "../Engine/Action.h"
#include "../Engine/Game.h"
#include "../Engine/LocalizedText.h"
#include "../Engine/Options.h"
#include "../Engine/RNG.h"
#include "../Interface/Text.h"
#include "../Interface/TextButton.h"
#include "../Interface/TextList.h"
#include "../Interface/Window.h"
#include "../Savegame/Base.h"
#include "../Savegame/BasePrisoner.h"
#include "../Savegame/ResearchProject.h"
#include "../Savegame/SavedGame.h"
#include "../Savegame/Soldier.h"

namespace OpenXcom
{

/**
 * Initializes all the elements in the PrisonerInfoState screen.
 * @param game Pointer to the core game.
 * @param base Pointer to the base to get info from.
 * @param prisoner
 * @param rule A RuleResearch which will be used to create a new ResearchProject
 */
PrisonerInfoState::PrisonerInfoState(Base* base, BasePrisoner* prisoner, const RulePrisoner* rule) : _base(base), _prisoner(prisoner), _rule(rule)
{
	_screen = false;
	_display = _prisoner->getPrisonerState();

	_window = new Window(this, 320, 156, 0, 22);
	_txtTitle = new Text(302, 17, 9, 31);
	_txtAvailableAgents = new Text(166, 9, 9, 52);
	_txtAvailableSpace = new Text(166, 9, 9, 62);

	_txtHealth = new Text(77, 9, 165, 77);
	_txtMorale = new Text(69, 9, 242, 77);
	_txtCooperation = new Text(146, 9, 165, 86);
	_txtAggressiveness = new Text(146, 9, 165, 95);

	_btnInterrogate = new TextButton(69, 16, 165, 107);
	_btnRecruit = new TextButton(69, 16, 242, 107);
	_btnTorture = new TextButton(69, 16, 165, 128);
	_btnContain = new TextButton(69, 16, 242, 128);
	_btnHolder = new TextButton(1, 1, 0, 0); // a hidden button required for null stance of the group (status NONE for prisoner)

	_btnOk = new TextButton(148, 16, 165, 153);
	_btnCancel = new TextButton(148, 16, 9, 153);
	_btnAllocate = new TextButton(110, 16, 9, 72);
	_btnTerminate = new TextButton(115, 16, 188, 56);
	_lstAgents = new TextList(301, 63, 9, 89);

	// Set palette
	setInterface("prisonerInfo");

	add(_window, "window", "prisonerInfo");
	add(_btnOk, "button1", "prisonerInfo");
	add(_btnCancel, "button1", "prisonerInfo");
	add(_btnAllocate, "button1", "prisonerInfo");
	add(_btnTerminate, "button2", "prisonerInfo");
	add(_btnInterrogate, "button3", "prisonerInfo");
	add(_btnRecruit, "button3", "prisonerInfo");
	add(_btnTorture, "button3", "prisonerInfo");
	add(_btnContain, "button3", "prisonerInfo");
	add(_btnHolder, "button3", "prisonerInfo");

	add(_txtTitle, "text", "prisonerInfo");
	add(_txtAvailableAgents, "text", "prisonerInfo");
	add(_txtAvailableSpace, "text", "prisonerInfo");
	add(_lstAgents, "list", "prisonerInfo");

	centerAllSurfaces();

	// Set up objects
	setWindowBackground(_window, "prisonerInfo");

	_txtTitle->setBig();
	_txtTitle->setAlign(ALIGN_CENTER);
	_txtTitle->setText(tr(_prisoner->getNameAndId()));

	_btnAllocate->setText(tr("STR_ALLOCATE_AGENTS"));
	_btnAllocate->onMouseClick((ActionHandler)&PrisonerInfoState::btnAllocateClick);

	_btnOk->onMouseClick((ActionHandler)&PrisonerInfoState::btnOkClick);
	_btnOk->onKeyboardPress((ActionHandler)&PrisonerInfoState::btnOkClick, Options::keyOk);

	_btnOk->setText(tr("STR_OK"));

	_btnCancel->setText(tr("STR_CANCEL_UC"));
	_btnCancel->onMouseClick((ActionHandler)&PrisonerInfoState::btnCancelClick);
	_btnCancel->onKeyboardPress((ActionHandler)&PrisonerInfoState::btnCancelClick, Options::keyCancel);

	_btnTerminate->setText(tr("STR_TERMINATE"));
	_btnTerminate->onMouseClick((ActionHandler)&PrisonerInfoState::btnTerminateClick);

	setAssignedAgents();

	_lstAgents->setColumns(1, 116);
	_lstAgents->setAlign(ALIGN_CENTER);
	_lstAgents->setAlign(ALIGN_LEFT, 0);
	_lstAgents->setBackground(_window);
	//_lstScientists->setMargin(2);
	_lstAgents->setWordWrap(true);

	_btnInterrogate->setText(tr("STR_INTERROGATE"));
	_btnInterrogate->onMouseClick((ActionHandler)&PrisonerInfoState::btnInterrogateToggle);
	_btnRecruit->setText(tr("STR_RECRUIT"));
	_btnRecruit->onMouseClick((ActionHandler)&PrisonerInfoState::btnRecruitToggle);
	_btnTorture->setText(tr("STR_TORTURE"));
	_btnTorture->onMouseClick((ActionHandler)&PrisonerInfoState::btnTortureToggle);
	_btnContain->setText(tr("STR_CONTAIN"));
	_btnContain->onMouseClick((ActionHandler)&PrisonerInfoState::btnContainToggle);
	_btnHolder->setVisible(false);

	if (_display == PRISONER_STATE_INTERROGATION)
	{
		_group = _btnInterrogate;
	}
	else if (_display == PRISONER_STATE_REQRUITING)
	{
		_group = _btnRecruit;
	}
	else if (_display == PRISONER_STATE_TORTURE)
	{
		_group = _btnTorture;
	}
	else if (_display == PRISONER_STATE_CONTAINING)
	{
		_group = _btnContain;
	}
	else
	{
		_group = _btnHolder;
	}

	_btnInterrogate->setGroup(&_group);
	_btnRecruit->setGroup(&_group);
	_btnTorture->setGroup(&_group);
	_btnContain->setGroup(&_group);

}

/**
 * Frees up memory that's not automatically cleaned on exit
 */
PrisonerInfoState::~PrisonerInfoState()
= default;

/**
 * Updates the research list
 * after going to other screens.
 */
void PrisonerInfoState::init()
{
	State::init();
	fillAgentsList(0);
}

void PrisonerInfoState::fillAgentsList(size_t scrl)
{
	_lstAgents->clearList();
	for (auto s : _agents)
	{
		_lstAgents->addRow(9, s->getName().c_str());
	}
}


/**
 * Returns to the previous screen.
 * @param action Pointer to an action.
 */
void PrisonerInfoState::btnOkClick(Action *)
{
	for (auto s : _agents)
	{
		s->clearBaseDuty();
		s->setActivePrisoner(_prisoner);
	}

	_prisoner->setPrisonerState(_display);

	_game->popState();
}

/**
 * Returns to the previous screen, removing the current project from the active
 * research list.
 * @param action Pointer to an action.
 */
void PrisonerInfoState::btnCancelClick(Action *)
{
	_game->popState();
}

/**
 * Returns to the previous screen, removing the current project from the active
 * research list.
 * @param action Pointer to an action.
 */
void PrisonerInfoState::btnTerminateClick(Action *)
{
	for (auto s : _agents)
	{
		s->setResearchProject(0);
	}
	_base->removePrisoner(_prisoner);
	_game->popState();
}

/**
 * Opens Allocate Scientists state.
 * @param action Pointer to an action.
 */
void PrisonerInfoState::btnAllocateClick(Action *action)
{
	//_game->pushState(new ResearchAllocateScientistsState(_base, this));
}

/**
 * Opens Allocate Scientists state.
 * @param action Pointer to an action.
 */
void PrisonerInfoState::btnInterrogateToggle(Action* action)
{
	_display = PRISONER_STATE_INTERROGATION;
}

/**
 * Opens Allocate Scientists state.
 * @param action Pointer to an action.
 */
void PrisonerInfoState::btnRecruitToggle(Action* action)
{
	_display = PRISONER_STATE_REQRUITING;
}

/**
 * Opens Allocate Scientists state.
 * @param action Pointer to an action.
 */
void PrisonerInfoState::btnTortureToggle(Action* action)
{
	_display = PRISONER_STATE_TORTURE;
}

/**
 * Opens Allocate Scientists state.
 * @param action Pointer to an action.
 */
void PrisonerInfoState::btnContainToggle(Action* action)
{
	_display = PRISONER_STATE_CONTAINING;
}

/**
 * Updates count of assigned/free scientists and available lab space.
 */
void PrisonerInfoState::setAssignedAgents()
{
	auto recovery = _base->getSumRecoveryPerDay();
	size_t freeAgents = 0;
	bool isBusy = false, isFree = false;
	for (auto a : _base->getPersonnel(ROLE_AGENT))
	{
		a->getCurrentDuty(_game->getLanguage(), recovery, isBusy, isFree);
		if (a->getActivePrisoner() && a->getActivePrisoner() != _prisoner)
		{
			_agents.insert(a);
		}
		else if (!isBusy && isFree)
		{
			if (_agents.find(a) != _agents.end())
			{
			}
			else
				freeAgents++;
		}
	}

	_txtAvailableAgents->setText(tr("STR_AGENTS_AVAILABLE").arg(freeAgents));

	int teamSize = (int)_agents.size();
	for (auto s : _agents)
	{
		if (s->getActivePrisoner() && s->getActivePrisoner() != _prisoner)
		{
			teamSize--;
		}
	}
	_workSpace = _base->getFreeInterrogationSpace() - teamSize;
	_txtAvailableSpace->setText(tr("STR_FREE_INTERROGATION_SPACE").arg(_workSpace));

}


}

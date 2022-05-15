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
#include "../Engine/Action.h"
#include "../Engine/Game.h"
#include "../Engine/LocalizedText.h"
#include "../Engine/Options.h"
#include "../Engine/RNG.h"
#include "../Interface/Text.h"
#include "../Interface/TextButton.h"
#include "../Interface/Window.h"
#include "../Mod/Mod.h"
#include "../Mod/RuleResearch.h"
#include "../Savegame/Base.h"
#include "../Savegame/ItemContainer.h"
#include "../Savegame/ResearchProject.h"
#include "../Savegame/SavedGame.h"
#include "ResearchInfoStateFtA.h"
#include <climits>

namespace OpenXcom
{

/**
 * Initializes all the elements in the ResearchProject screen.
 * @param game Pointer to the core game.
 * @param base Pointer to the base to get info from.
 * @param rule A RuleResearch which will be used to create a new ResearchProject
 */
ResearchInfoStateFtA::ResearchInfoStateFtA(Base *base, RuleResearch *rule) : _base(base), _project(nullptr), _rule(rule)
{
	buildUi();
}

/**
 * Initializes all the elements in the ResearchProject screen.
 * @param game Pointer to the core game.
 * @param base Pointer to the base to get info from.
 * @param project A ResearchProject to modify
 */
ResearchInfoStateFtA::ResearchInfoStateFtA(Base *base, ResearchProject *project) : _base(base), _project(project), _rule(0)
{
	getAssignedScientists();
	buildUi();
}



/**
 * Builds dialog.
 */
void ResearchInfoStateFtA::buildUi()
{
	_screen = false;

	_window = new Window(this, 320, 156, 0, 22);
	_txtTitle = new Text(302, 17, 9, 31);

	_txtAvailableScientist = new Text(166, 9, 9, 52);
	_txtAvailableSpace = new Text(166, 9, 9, 62);

	_btnOk = new TextButton(90, 16, 221, 153);
	_btnCancel = new TextButton(90, 16, 9, 153);
	_btnAllocate = new TextButton(110, 16, 9, 72);
	_btnAbandon = new TextButton(115, 16, 188, 56);



	// Set palette
	setInterface("allocateResearch");

	add(_window, "window", "allocateResearch");
	add(_btnOk, "button2", "allocateResearch");
	add(_btnCancel, "button2", "allocateResearch");
	add(_txtTitle, "text", "allocateResearch");
	add(_txtAvailableScientist, "text", "allocateResearch");
	add(_txtAvailableSpace, "text", "allocateResearch");

	centerAllSurfaces();

	// Set up objects
	setWindowBackground(_window, "allocateResearch");

	_txtTitle->setBig();

	_txtTitle->setText(_rule ? tr(_rule->getName()) : tr(_project->getRules()->getName()));

	if (_rule)
	{
		_base->addResearch(_project);
		if (_rule->needItem() && _rule->destroyItem())
		{
			_base->getStorageItems()->removeItem(_rule->getName(), 1);
		}
	}
	setAssignedScientist();

	_btnOk->onMouseClick((ActionHandler)&ResearchInfoStateFtA::btnOkClick);
	_btnOk->onKeyboardPress((ActionHandler)&ResearchInfoStateFtA::btnOkClick, Options::keyOk);
	if (_rule)
	{
		_btnOk->setText(tr("STR_START_PROJECT"));
		_btnCancel->setText(tr("STR_CANCEL_UC"));
		_btnCancel->onKeyboardPress((ActionHandler)&ResearchInfoStateFtA::btnCancelClick, Options::keyCancel);
	}
	else
	{
		_btnOk->setText(tr("STR_OK"));
		_btnCancel->setText(tr("STR_CANCEL_PROJECT"));
		_btnOk->onKeyboardPress((ActionHandler)&ResearchInfoStateFtA::btnOkClick, Options::keyCancel);
	}
	_btnCancel->onMouseClick((ActionHandler)&ResearchInfoStateFtA::btnCancelClick);

	if (_rule)
	{
		// mark new as normal
		if (_game->getSavedGame()->isResearchRuleStatusNew(_rule->getName()))
		{
			_game->getSavedGame()->setResearchRuleStatus(_rule->getName(), RuleResearch::RESEARCH_STATUS_NORMAL);
		}
	}
}

/**
 * Frees up memory that's not automatically cleaned on exit
 */
ResearchInfoStateFtA::~ResearchInfoStateFtA()
{
}

/**
 * Populate list of soldiers, assigned to project
 */
void ResearchInfoStateFtA::getAssignedScientists()
{
	for (auto s : *_base->getSoldiers())
	{
		if (s->getResearchProject() == _project )
		{
			_scientists.push_back(s);
		}
	}
}

/**
 * Returns to the previous screen.
 * @param action Pointer to an action.
 */
void ResearchInfoStateFtA::btnOkClick(Action *)
{
	int rng = RNG::generate(50, 150);
	int randomizedCost = _rule->getCost() * rng / 100;
	if (_rule->getCost() > 0)
	{
		randomizedCost = std::max(1, randomizedCost);
	}
	_project = new ResearchProject(_rule, randomizedCost);

	_game->popState();
}

/**
 * Returns to the previous screen, removing the current project from the active
 * research list.
 * @param action Pointer to an action.
 */
void ResearchInfoStateFtA::btnCancelClick(Action *)
{
	_base->removeResearch(_project);
	_game->popState();
}

/**
 * Returns to the previous screen, removing the current project from the active
 * research list.
 * @param action Pointer to an action.
 */
void ResearchInfoStateFtA::btnAbandonClick(Action *)
{
	_base->removeResearch(_project);
	_game->popState();
}

/**
 * Updates count of assigned/free scientists and available lab space.
 */
void ResearchInfoStateFtA::setAssignedScientist()
{
	unsigned int freeScientists = 0;
	auto recovery = _base->getSumRecoveryPerDay();
	bool isBusy = false, isFree = false;
	for (auto s : _base->getPersonnel(ROLE_SCIENTIST))
	{
		s->getCurrentDuty(_game->getLanguage(), recovery, isBusy, isFree, LAB);
		if (!isBusy || isFree)
		{
			freeScientists++;
		}
	}
	_txtAvailableScientist->setText(tr("STR_SCIENTISTS_AVAILABLE_UC").arg(freeScientists)); //ПЕРЕДЕЛАТЬ! Нужно обновлять в соответствии с планируемым проектом!
	_txtAvailableSpace->setText(tr("STR_LABORATORY_SPACE_AVAILABLE_UC").arg(_base->getFreeLaboratories()));
}

}

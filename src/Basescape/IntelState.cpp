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
#include "IntelState.h"
#include <sstream>
#include "../Engine/Action.h"
#include "../Engine/Game.h"
#include "../FTA/MasterMind.h"
#include "../Mod/Mod.h"
#include "../Engine/LocalizedText.h"
#include "../Engine/Options.h"
#include "../Interface/TextButton.h"
#include "../Interface/Window.h"
#include "../Interface/Text.h"
#include "../Interface/TextList.h"
#include "../Savegame/Base.h"
#include "../Savegame/SavedGame.h"
#include "../Savegame/Soldier.h"
//#include "../Basescape/ScientistsState.h"
//#include "NewResearchListState.h"
//#include "GlobalResearchState.h"
#include "../Savegame/IntelProject.h"
//#include "../Mod/RuleIntelProject.h"
//#include "ResearchInfoState.h"
//#include "ResearchInfoStateFtA.h"
//#include "TechTreeViewerState.h"
#include <algorithm>

namespace OpenXcom
{

/**
 * Initializes all the elements in the Research screen.
 * @param game Pointer to the core game.
 * @param base Pointer to the base to get info from.
 */
IntelState::IntelState(Base *base) : _base(base)
{
	// Create objects
	_window = new Window(this, 320, 200, 0, 0);
	_btnOk = new TextButton(96, 16, 216, 176);
	_btnPrisonCells = new TextButton(96, 16, 8, 176);
	_btnAgents = new TextButton(96, 16, 112, 176);
	_txtTitle = new Text(310, 17, 5, 8);
	_txtAgentsAvailable = new Text(150, 9, 10, 24);
	_txtAgentsAllocated = new Text(150, 9, 160, 24);
	_txtIntelSpace = new Text(150, 9, 10, 34);
	_txtProject = new Text(110, 17, 10, 44);
	_txtAgents = new Text(106, 17, 120, 44);
	_txtProgress = new Text(84, 9, 226, 44);
	_lstProjects = new TextList(288, 112, 8, 62);

	// Set palette
	setInterface("intelMenu");

	add(_window, "window", "intelMenu");
	add(_btnPrisonCells, "button", "intelMenu");
	add(_btnOk, "button", "intelMenu");
	add(_btnAgents, "button", "intelMenu");
	add(_txtTitle, "text", "intelMenu");
	add(_txtAgentsAvailable, "text", "intelMenu");
	add(_txtAgentsAllocated, "text", "intelMenu");
	add(_txtIntelSpace, "text", "intelMenu");
	add(_txtProject, "text", "intelMenu");
	add(_txtAgents, "text", "intelMenu");
	add(_txtProgress, "text", "intelMenu");
	add(_lstProjects, "list", "intelMenu");

	centerAllSurfaces();

	// Set up objects
	setWindowBackground(_window, "researchMenu");

	_btnPrisonCells->setText(tr("STR_MANAGE_PRISON_CELLS"));
	_btnPrisonCells->onMouseClick((ActionHandler)&IntelState::btnPrisonClick);
	_btnPrisonCells->onKeyboardPress((ActionHandler)&IntelState::btnPrisonClick, Options::keyToggleQuickSearch);

	_btnOk->setText(tr("STR_OK"));
	_btnOk->onMouseClick((ActionHandler)&IntelState::btnOkClick);
	_btnOk->onKeyboardPress((ActionHandler)&IntelState::btnOkClick, Options::keyCancel);

	_btnAgents->setText(tr("STR_AGENTS_LC"));
	_btnAgents->onMouseClick((ActionHandler)&IntelState::btnAgentsClick);

	_txtTitle->setBig();
	_txtTitle->setAlign(ALIGN_CENTER);
	_txtTitle->setText(tr("STR_INTELLIGENCE_UC"));

	_txtProject->setWordWrap(true);
	_txtProject->setText(tr("STR_INTELLIGENCE_PROJECT"));

	_txtAgents->setWordWrap(true);
	_txtAgents->setText(tr("STR_AGENTS_ALLOCATED_UC"));

	_txtProgress->setText(tr("STR_PROGRESS"));

	_lstProjects->setColumns(3, 158, 58, 70);
	_lstProjects->setSelectable(true);
	_lstProjects->setBackground(_window);
	_lstProjects->setMargin(2);
	_lstProjects->setWordWrap(true);
	_lstProjects->onMouseClick((ActionHandler)&IntelState::onSelectProject, SDL_BUTTON_LEFT);
	_lstProjects->onMouseClick((ActionHandler)&IntelState::onProjectDetails, SDL_BUTTON_MIDDLE);
	_lstProjects->onMouseClick((ActionHandler)&IntelState::onProjectDetails, SDL_BUTTON_RIGHT);
}

/**
 *
 */
IntelState::~IntelState()
{
}

/**
 * Returns to the previous screen.
 * @param action Pointer to an action.
 */
void IntelState::btnOkClick(Action *)
{
	_game->popState();
}

/**
 * Displays the list of possible ResearchProjects.
 * @param action Pointer to an action.
 */
void IntelState::btnPrisonClick(Action *)
{
	//_game->pushState(new NewResearchListState(_base, sortByCost));
}

/**
 * Returns to the previous screen.
 * @param action Pointer to an action.
 */
void IntelState::btnAgentsClick(Action *action)
{
	//_game->pushState(new ScientistsState(_base));
}

/**
 * Opens state with selected Research Project
 * @param action Pointer to an action.
 */
void IntelState::onSelectProject(Action *)
{
	const std::vector<IntelProject*> & baseProjects(_base->getIntelProjects());
	auto project = baseProjects[_lstProjects->getSelectedRow()];

	//_game->pushState(new ResearchInfoState(_base, project));
}

/**
* Opens the IntelProjectDetails for the corresponding project.
* @param action Pointer to an action.
*/
void IntelState::onProjectDetails(Action* action)
{
	const std::vector<IntelProject*>& baseProjects(_base->getIntelProjects());
	auto selectedTopic = baseProjects[_lstProjects->getSelectedRow()]->getRules();
	//_game->pushState(new TechTreeViewerState(selectedTopic, 0));
}

/**
 * Updates the research list
 * after going to other screens.
 */
void IntelState::init()
{
	State::init();
	fillProjectList(0);

	if (Options::oxceResearchScrollSpeed > 0 || Options::oxceResearchScrollSpeedWithCtrl > 0)
	{
		// 175 +/- 20
		_lstProjects->setNoScrollArea(_txtAgentsAllocated->getX() - 5, _txtAgentsAllocated->getX() + 35);
	}
	else
	{
		_lstProjects->setNoScrollArea(0, 0);
	}
}

/**
 * Fills the list with Base ResearchProjects. Also updates count of available lab space and available/allocated scientists.
 */
void IntelState::fillProjectList(size_t scrl)
{
	_lstProjects->clearList();
	for (auto p : _base->getIntelProjects())
	{
		std::ostringstream sstr, sspr;
		std::map<Soldier*, int> soldiers;
		size_t n = 0;
		for (auto s : *_base->getSoldiers())
		{
			if (s->getIntelProject() == p)
			{
				n++;
				int coef = 100;
				if (s->getBestRole() != ROLE_AGENT)
				{
					coef = 50;
				}
				soldiers.emplace(std::make_pair(s, coef));

			}
		}
		sstr << n;
		std::string des = "";
		p->getStepProgress(soldiers, _game->getMod(), _game->getMasterMind()->getLoyaltyPerformanceBonus(), des, true);
		sspr << tr(des);
		_lstProjects->addRow(3, tr(p->getName()).c_str(), sstr.str().c_str(), sspr.str().c_str());
	}

	auto recovery = _base->getSumRecoveryPerDay();
	size_t freeAgents = 0, busyAgents = 0;
	bool isBusy = false, isFree = false;
	for (auto s : _base->getPersonnel(ROLE_AGENT))
	{
		s->getCurrentDuty(_game->getLanguage(), recovery, isBusy, isFree, INTEL);
		if (!isBusy && isFree)
		{
			freeAgents++;
		}
		if (s->getIntelProject())
		{
			busyAgents++;
		}
	}
	
	_txtAgentsAvailable->setText(tr("STR_AGENTS_AVAILABLE").arg(freeAgents));
	_txtAgentsAllocated->setText(tr("STR_AGENTS_ALLOCATED").arg(busyAgents));
	_txtIntelSpace->setText(tr("STR_INTEL_SPACE_AVAILABLE").arg(_base->getFreeLaboratories(true)));

	if (scrl)
		_lstProjects->scrollTo(scrl);
}

}

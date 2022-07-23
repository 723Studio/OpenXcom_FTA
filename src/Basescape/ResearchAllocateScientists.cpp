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
#include "../Engine/Unicode.h"
#include "../Interface/ComboBox.h"
#include "../Interface/Text.h"
#include "../Interface/TextButton.h"
#include "../Interface/TextList.h"
#include "../Interface/Window.h"
#include "../Menu/ErrorMessageState.h"
#include "../Mod/Armor.h"
#include "../Mod/Mod.h"
#include "../Mod/RuleInterface.h"
#include "../Mod/RuleSoldier.h"
#include "../Savegame/Base.h"
#include "../Savegame/SavedGame.h"
#include "../Savegame/Soldier.h"
#include "../Savegame/ResearchProject.h"
#include "../Basescape/ResearchInfoStateFtA.h"
#include "ResearchAllocateScientists.h"
#include "ResearchProjectDetailsState.h"
#include "SoldierInfoState.h"
#include <algorithm>
#include <climits>

namespace OpenXcom
{

/**
 * Initializes all the elements in the Soldiers screen.
 * @param base Pointer to the base to get info from.
 * @param operation Pointer to starting (not committed) covert operation.
 */
ResearchAllocateScientists::ResearchAllocateScientists(Base *base, ResearchInfoStateFtA *planningProject)
	: _base(base), _planningProject(planningProject), _otherCraftColor(0), _origScientistOrder(*_base->getSoldiers()), _dynGetter(NULL)
{
	_freeSpace = planningProject->getWorkspace();

	// Create objects
	_window = new Window(this, 320, 200, 0, 0);
	_btnOk = new TextButton(148, 16, 164, 176);
	_btnInfo = new TextButton(42, 16, 270, 8);
	_txtTitle = new Text(300, 17, 16, 7);
	_txtName = new Text(114, 9, 16, 32);
	_txtAssignment = new Text(84, 9, 122, 32);
	_txtFreeSpace = new Text(150, 9, 16, 24);
	_cbxSortBy = new ComboBox(this, 148, 16, 8, 176, true);
	_lstScientists = new TextList(288, 128, 8, 40);

	// Set palette
	setInterface("researchAllocateScientists");

	add(_window, "window", "researchAllocateScientists");
	add(_btnOk, "button", "researchAllocateScientists");
	add(_btnInfo, "button", "researchAllocateScientists");
	add(_txtTitle, "text", "researchAllocateScientists");
	add(_txtName, "text", "researchAllocateScientists");
	add(_txtAssignment, "text", "researchAllocateScientists");
	add(_txtFreeSpace, "text", "researchAllocateScientists");
	add(_lstScientists, "list", "researchAllocateScientists");
	add(_cbxSortBy, "button", "researchAllocateScientists");

	_otherCraftColor = _game->getMod()->getInterface("researchAllocateScientists")->getElement("otherCraft")->color;

	centerAllSurfaces();

	// Set up objects
	setWindowBackground(_window, "researchAllocateScientists");

	_btnOk->setText(tr("STR_OK"));
	_btnOk->onMouseClick((ActionHandler)&ResearchAllocateScientists::btnOkClick);
	_btnOk->onKeyboardPress((ActionHandler)&ResearchAllocateScientists::btnOkClick, Options::keyCancel);

	_btnInfo->setText(tr("STR_INFO"));
	_btnInfo->onMouseClick((ActionHandler)&ResearchAllocateScientists::btnInfoClick);

	_txtTitle->setBig();
	_txtTitle->setText(tr(planningProject->getResearchRules()->getName()));
	_txtTitle->setWordWrap(true);
	_txtTitle->setVerticalAlign(ALIGN_MIDDLE);

	_txtName->setText(tr("STR_NAME_UC"));

	_txtAssignment->setText(tr("STR_ASSIGNMENT"));

	// populate sort options
	std::vector<std::string> sortOptions;
	sortOptions.push_back(tr("STR_ORIGINAL_ORDER"));
	_sortFunctors.push_back(NULL);

#define PUSH_IN(strId, functor)       \
	sortOptions.push_back(tr(strId)); \
	_sortFunctors.push_back(new SortFunctor(_game, functor));

	PUSH_IN("STR_ID", idStat);
	PUSH_IN("STR_NAME_UC", nameStat);
	
	// scientist section
	PUSH_IN("STR_PHYSICS_UC", physicsStat);
	PUSH_IN("STR_CHEMISTRY_UC", chemistryStat);
	PUSH_IN("STR_BIOLOGY_UC", biologyStat);
	PUSH_IN("STR_INSIGHT_UC", insightStat);
	PUSH_IN("STR_DATA_ANALISIS_UC", dataStat);
	PUSH_IN("STR_COMPUTER_SCIENCE_UC", computersStat);
	PUSH_IN("STR_TACTICS_UC", tacticsStat);
	PUSH_IN("STR_MATERIAL_SCIENCE_UC", materialsStat);
	PUSH_IN("STR_DESIGNING_UC", designingStat);
	if (_game->getSavedGame()->isResearched(_game->getMod()->getAlienTechUnlockResearch()))
	{
		PUSH_IN("STR_ALIEN_TECH_UC", alienTechStat);
	}
	if (_game->getSavedGame()->isResearched(_game->getMod()->getPsiRequirements()))
	{
		PUSH_IN("STR_PSIONICS_UC", psionicsStat);
	}
	if (_game->getSavedGame()->isResearched(_game->getMod()->getXenologyUnlockResearch()))
	{
		PUSH_IN("STR_XENOLINGUISTICS_UC", xenolinguisticsStat);
	}

#undef PUSH_IN

	_cbxSortBy->setOptions(sortOptions);
	_cbxSortBy->setSelected(0);
	_cbxSortBy->onChange((ActionHandler)&ResearchAllocateScientists::cbxSortByChange);
	_cbxSortBy->setText(tr("STR_SORT_BY"));

	_lstScientists->setColumns(2, 106, 174);
	_lstScientists->setAlign(ALIGN_RIGHT, 3);
	_lstScientists->setSelectable(true);
	_lstScientists->setBackground(_window);
	_lstScientists->setMargin(8);
	_lstScientists->onMouseClick((ActionHandler)&ResearchAllocateScientists::lstScientistsClick, 0);
}

/**
 * cleans up dynamic state
 */
ResearchAllocateScientists::~ResearchAllocateScientists()
{
	for (std::vector<SortFunctor *>::iterator it = _sortFunctors.begin();
		 it != _sortFunctors.end(); ++it)
	{
		delete (*it);
	}
}

/**
 * Sorts the soldiers list by the selected criterion
 * @param action Pointer to an action.
 */
void ResearchAllocateScientists::cbxSortByChange(Action *)
{
	bool ctrlPressed = _game->isCtrlPressed();
	size_t selIdx = _cbxSortBy->getSelected();
	if (selIdx == (size_t)-1)
	{
		return;
	}

	SortFunctor *compFunc = _sortFunctors[selIdx];
	_dynGetter = NULL;
	if (compFunc)
	{
		if (selIdx != 2)
		{
			_dynGetter = compFunc->getGetter();
		}

		// if CTRL is pressed, we only want to show the dynamic column, without actual sorting
		if (!ctrlPressed)
		{
			if (selIdx == 2)
			{
				std::stable_sort(_base->getSoldiers()->begin(), _base->getSoldiers()->end(),
								 [](const Soldier *a, const Soldier *b)
								 {
									 return Unicode::naturalCompare(a->getName(), b->getName());
								 });
			}
			else
			{
				std::stable_sort(_base->getSoldiers()->begin(), _base->getSoldiers()->end(), *compFunc);
			}
			if (_game->isShiftPressed())
			{
				std::reverse(_base->getSoldiers()->begin(), _base->getSoldiers()->end());
			}
		}
	}
	else
	{
		// restore original ordering, ignoring (of course) those
		// soldiers that have been sacked since this state started
		for (std::vector<Soldier *>::const_iterator it = _origScientistOrder.begin();
			 it != _origScientistOrder.end(); ++it)
		{
			std::vector<Soldier *>::iterator soldierIt =
				std::find(_base->getSoldiers()->begin(), _base->getSoldiers()->end(), *it);
			if (soldierIt != _base->getSoldiers()->end())
			{
				Soldier *s = *soldierIt;
				_base->getSoldiers()->erase(soldierIt);
				_base->getSoldiers()->insert(_base->getSoldiers()->end(), s);
			}
		}
	}

	size_t originalScrollPos = _lstScientists->getScroll();
	initList(originalScrollPos);
}

/**
 * Returns to the previous screen.
 * @param action Pointer to an action.
 */
void ResearchAllocateScientists::btnOkClick(Action *)
{
	_planningProject->setAssignedScientists();
	_game->popState();
}

void ResearchAllocateScientists::btnInfoClick(Action* action)
{
	_game->pushState(new ResearchProjectDetailsState(_base, _planningProject->getResearchRules()));
}

/**
 * Shows the soldiers in a list at specified offset/scroll.
 */
void ResearchAllocateScientists::initList(size_t scrl)
{
	int row = 0;
	_lstScientists->clearList();

	if (_dynGetter != NULL)
	{
		_lstScientists->setColumns(3, 106, 158, 16);
	}
	else
	{
		_lstScientists->setColumns(2, 106, 174);
	}

	auto recovery = _base->getSumRecoveryPerDay();
	bool isBusy = false, isFree = false;
	unsigned int it = 0;
	for (std::vector<Soldier *>::iterator i = _base->getSoldiers()->begin(); i != _base->getSoldiers()->end(); ++i)
	{
		if ((*i)->getRoleRank(ROLE_SCIENTIST) > 0)
		{
			_scientistsNumbers.push_back(it); // don't forget soldier's number on the base!
			std::string duty = (*i)->getCurrentDuty(_game->getLanguage(), recovery, isBusy, isFree, LAB);
			if (_dynGetter != NULL)
			{
				// call corresponding getter
				int dynStat = (*_dynGetter)(_game, *i);
				std::ostringstream ss;
				ss << dynStat;
				_lstScientists->addRow(3, (*i)->getName(true, 19).c_str(), duty.c_str(), ss.str().c_str());
			}
			else
			{
				_lstScientists->addRow(2, (*i)->getName(true, 19).c_str(), duty.c_str());
			}

			Uint8 color = _lstScientists->getColor();
			bool matched = false;
			auto scientists = _planningProject->getScientists();
			auto iter = std::find(std::begin(scientists), std::end(scientists), (*i));
			if (iter != std::end(scientists))
			{
				matched = true;
			}

			if (matched)
			{
				color = _lstScientists->getSecondaryColor();
				_lstScientists->setCellText(row, 1, tr("STR_ASSIGNED_UC"));
			}
			else if (isBusy || !isFree)
			{
				color = _otherCraftColor;
			}

			_lstScientists->setRowColor(row, color);
			row++;
		}
		it++;
	}
	if (scrl)
		_lstScientists->scrollTo(scrl);
	_lstScientists->draw();

	_txtFreeSpace->setText(tr("STR_LABORATORY_SPACE_AVAILABLE").arg(_freeSpace));
}

/**
 * Shows the soldiers in a list.
 */
void ResearchAllocateScientists::init()
{
	State::init();
	_base->prepareSoldierStatsWithBonuses(); // refresh stats for sorting
	initList(0);
}

/**
 * Shows the selected soldier's info.
 * @param action Pointer to an action.
 */
void ResearchAllocateScientists::lstScientistsClick(Action *action)
{
	double mx = action->getAbsoluteXMouse();
	if (mx >= _lstScientists->getArrowsLeftEdge() && mx < _lstScientists->getArrowsRightEdge())
	{
		return;
	}
	int row = _lstScientists->getSelectedRow();
	if (action->getDetails()->button.button == SDL_BUTTON_LEFT)
	{
		Soldier *s = _base->getSoldiers()->at(_scientistsNumbers.at(row));
		Uint8 color = _lstScientists->getColor();
		bool isBusy = false, isFree = false, matched = false;
		std::string duty = s->getCurrentDuty(_game->getLanguage(), _base->getSumRecoveryPerDay(), isBusy, isFree, LAB);
		auto scientists = _planningProject->getScientists();
		auto iter = std::find(std::begin(scientists), std::end(scientists), s);
		if (iter != std::end(scientists))
		{
			matched = true;
		}
		if (matched)
		{
			_planningProject->removeScientist(s);
			if (s->getResearchProject())
			{
				if (s->getResearchProject()->getRules() == _planningProject->getResearchRules())
				{
					s->setResearchProject(0);
					color = _lstScientists->getColor();
					_lstScientists->setCellText(row, 1, tr("STR_NONE_UC"));
					_freeSpace++;
				}
				else
				{
					color = _otherCraftColor;
					_lstScientists->setCellText(row, 1, duty);
				}
			}
			else
			{
				_lstScientists->setCellText(row, 1, duty);
				_freeSpace++;
				if (isBusy || !isFree || s->getCraft())
				{
					color = _otherCraftColor;
				}
			}
		}
		else if (s->hasFullHealth() && !isBusy)
		{
			bool noProject = s->getResearchProject() == 0;
			if (noProject && _freeSpace <= 0)
			{
				_game->pushState(new ErrorMessageState(tr("STR_NOT_ENOUGH_LABSPACE"),
					_palette,
					_game->getMod()->getInterface("soldierInfo")->getElement("errorMessage")->color,
					"BACK01.SCR",
					_game->getMod()->getInterface("soldierInfo")->getElement("errorPalette")->color));
			}
			else
			{
				_lstScientists->setCellText(row, 1, tr("STR_ASSIGNED_UC"));
				color = _lstScientists->getSecondaryColor();
				_planningProject->addScientist(s);
				if (noProject)
				{
					_freeSpace--;
				}
			}
		}

		_lstScientists->setRowColor(row, color);
		_txtFreeSpace->setText(tr("STR_LABORATORY_SPACE_AVAILABLE").arg(_freeSpace));
	}
	else if (action->getDetails()->button.button == SDL_BUTTON_RIGHT)
	{
		_game->pushState(new SoldierInfoState(_base, _scientistsNumbers.at(row)));
	}
}

}

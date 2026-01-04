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
#include "ManufactureState.h"
#include <sstream>
#include "../Engine/Action.h"
#include "../Engine/Game.h"
#include "../FTA/MasterMind.h"
#include "../Mod/Mod.h"
#include "../Engine/LocalizedText.h"
#include "../Engine/Unicode.h"
#include "../Interface/TextButton.h"
#include "../Interface/Window.h"
#include "../Interface/Text.h"
#include "../Interface/TextList.h"
#include "../Savegame/Base.h"
#include "../Savegame/SavedGame.h"
#include "../Mod/RuleManufacture.h"
#include "../Savegame/Production.h"
#include "NewManufactureListState.h"
#include "ManufactureInfoStateFtA.h"
#include "TechTreeViewerState.h"
#include "EngineersState.h"
#include "../Ufopaedia/Ufopaedia.h"

namespace OpenXcom
{

/**
 * Initializes all the elements in the Manufacture screen.
 * @param game Pointer to the core game.
 * @param base Pointer to the base to get info from.
 */
ManufactureState::ManufactureState(Base *base) : _base(base)
{
	// Create objects
	_window = new Window(this, 320, 200, 0, 0);
	_btnOk = new TextButton(96, 16, 216, 176);
	_btnNew = new TextButton(96, 16, 8, 176);
	_btnEngineers = new TextButton(96, 16, 112, 176);
	_txtTitle = new Text(310, 17, 5, 8);
	_txtAvailable = new Text(150, 9, 8, 24);
	_txtAllocated = new Text(150, 9, 160, 24);
	_txtSpace = new Text(150, 9, 8, 34);
	_txtFunds = new Text(150, 9, 160, 34);
	_txtItem = new Text(80, 9, 10, 52);
	_txtEngineers = new Text(56, 18, 112, 44);
	_txtProduced = new Text(56, 18, 168, 44);
	_txtCost = new Text(44, 27, 222, 44);
	_txtTimeLeft = new Text(60, 27, 260, 44);
	_lstManufacture = new TextList(288, 88, 8, 80);

	// Set palette
	setInterface("manufactureMenu");

	add(_window, "window", "manufactureMenu");
	add(_btnNew, "button", "manufactureMenu");
	add(_btnOk, "button", "manufactureMenu");
	add(_btnEngineers, "button", "manufactureMenu");
	add(_txtTitle, "text1", "manufactureMenu");
	add(_txtAvailable, "text1", "manufactureMenu");
	add(_txtAllocated, "text1", "manufactureMenu");
	add(_txtSpace, "text1", "manufactureMenu");
	add(_txtFunds, "text1", "manufactureMenu");
	add(_txtItem, "text2", "manufactureMenu");
	add(_txtEngineers, "text2", "manufactureMenu");
	add(_txtProduced, "text2", "manufactureMenu");
	add(_txtCost, "text2", "manufactureMenu");
	add(_txtTimeLeft, "text2", "manufactureMenu");
	add(_lstManufacture, "list", "manufactureMenu");

	centerAllSurfaces();

	// Set up objects
	setWindowBackground(_window, "manufactureMenu");

	_btnNew->setText(tr("STR_NEW_PRODUCTION"));
	_btnNew->onMouseClick((ActionHandler)&ManufactureState::btnNewProductionClick);
	_btnNew->onKeyboardPress((ActionHandler)&ManufactureState::btnNewProductionClick, Options::keyToggleQuickSearch);

	_btnOk->setText(tr("STR_OK"));
	_btnOk->onMouseClick((ActionHandler)&ManufactureState::btnOkClick);
	_btnOk->onKeyboardPress((ActionHandler)&ManufactureState::btnOkClick, Options::keyCancel);

	_btnEngineers->setText(tr("STR_ENGINEERS_LC"));
	_btnEngineers->onMouseClick((ActionHandler)&ManufactureState::btnEngineersClick);
	_btnEngineers->setVisible(true);

	_txtTitle->setBig();
	_txtTitle->setAlign(ALIGN_CENTER);
	_txtTitle->setText(tr("STR_CURRENT_PRODUCTION"));

	_txtItem->setText(tr("STR_ITEM"));

	_txtEngineers->setText(tr("STR_ENGINEERS__ALLOCATED"));
	_txtEngineers->setWordWrap(true);

	_txtProduced->setText(tr("STR_UNITS_PRODUCED"));
	_txtProduced->setWordWrap(true);

	_txtCost->setText(tr("STR_COST__PER__UNIT"));
	_txtCost->setWordWrap(true);

	_txtTimeLeft->setText(tr("STR_DAYS_HOURS_LEFT"));
	_txtTimeLeft->setWordWrap(true);

	_lstManufacture->setColumns(5, 114, 16, 52, 56, 48);
	_lstManufacture->setAlign(ALIGN_RIGHT);
	_lstManufacture->setAlign(ALIGN_LEFT, 0);
	_lstManufacture->setSelectable(true);
	_lstManufacture->setBackground(_window);
	_lstManufacture->setMargin(2);
	_lstManufacture->setWordWrap(true);
	_lstManufacture->onMouseClick((ActionHandler)&ManufactureState::lstManufactureClickLeft, SDL_BUTTON_LEFT);
	_lstManufacture->onMouseClick((ActionHandler)&ManufactureState::lstManufactureClickMiddle, SDL_BUTTON_MIDDLE);
}

/**
 *
 */
ManufactureState::~ManufactureState()
{

}

/**
 * Updates the production list
 * after going to other screens.
 */
void ManufactureState::init()
{
	State::init();
	fillProductionList(0);
	_lstManufacture->setNoScrollArea(0, 0);
}

/**
 * Returns to the previous screen.
 * @param action Pointer to an action.
 */
void ManufactureState::btnOkClick(Action *)
{
	_game->popState();
}
/**
* Opens Scientists list screen.
* @param action Pointer to an action.
*/
void ManufactureState::btnEngineersClick(Action* action)
{
	_game->pushState(new EngineersState(_base));
}

/**
 * Opens the screen with the list of possible productions.
 * @param action Pointer to an action.
 */
void ManufactureState::btnNewProductionClick(Action *)
{
	_game->pushState(new NewManufactureListState(_base));
}

/**
 * Fills the list of base productions.
 */
void ManufactureState::fillProductionList(size_t scrl)
{
	_lstManufacture->clearList();
	for (auto* prod : _base->getProductions())
	{
		std::ostringstream s1;
		size_t engineers = prod->getAssignedSoldiers(_base).size();
		s1 << engineers;
		std::ostringstream s2;
		s2 << prod->getAmountProduced() << "/";
		if (prod->getInfiniteAmount())
		{
			s2 << "∞";
		}
		else
		{
			s2 << prod->getAmountTotal();
		}

		if (prod->getSellItems())
		{
			s2 << " $";
		}

		std::ostringstream s3;
		s3 << Unicode::formatFunding(prod->getRules()->getManufactureCost());
		std::ostringstream s4;
		if (prod->getInfiniteAmount())
		{
			s4 << "∞";
		}
		else if (engineers > 0)
		{
			int timeLeft = prod->getAmountTotal() * prod->getRules()->getManufactureTime() - prod->getTimeSpent();
			int numEffectiveEngineers = prod->getProgress(_base, _game->getSavedGame(), _game->getMod(), _game->getMasterMind()->getLoyaltyPerformanceBonus(), true);
			// ensure we round up since it takes an entire hour to manufacture any part of that hour's capacity
			if (numEffectiveEngineers > 0)
			{
				int hoursLeft = (timeLeft + numEffectiveEngineers - 1) / numEffectiveEngineers;
				if (hoursLeft < 1)
				{
					s4 << tr("STR_FINISHING"); //for corner cases
				}
				else
				{
					int daysLeft = hoursLeft / 24;
					int hours = hoursLeft % 24;
					s4 << daysLeft << "/" << hours;
				}
			}
			else //case FTA engineer is really ineffective
			{
				s4 << "-";
			}
		}
		else
		{
			s4 << "-";
		}
		_lstManufacture->addRow(5, tr(prod->getRules()->getName()).c_str(), s1.str().c_str(), s2.str().c_str(), s3.str().c_str(), s4.str().c_str());
	}

	auto recovery = _base->getSumRecoveryPerDay();
	size_t freeEngineers = 0, busyEngineers = 0;
	bool isBusy = false, isFree = false;
	for (auto s : _base->getPersonnel(ROLE_ENGINEER))
	{
		s->getCurrentDuty(_game->getLanguage(), recovery, isBusy, isFree, WORK);
		if (!isBusy && isFree)
		{
			freeEngineers++;
		}
		if (s->getProductionProject())
		{
			busyEngineers++;
		}
	}
	_txtAvailable->setText(tr("STR_ENGINEERS_AVAILABLE").arg(freeEngineers));
	_txtAllocated->setText(tr("STR_ENGINEERS_ALLOCATED").arg(busyEngineers));

	_txtSpace->setText(tr("STR_WORKSHOP_SPACE_AVAILABLE").arg(_base->getFreeWorkshops()));
	_txtFunds->setText(tr("STR_CURRENT_FUNDS").arg(Unicode::formatFunding(_game->getSavedGame()->getFunds())));

	if (scrl)
		_lstManufacture->scrollTo(scrl);
}

/**
 * Opens the screen displaying production settings.
 * @param action Pointer to an action.
 */
void ManufactureState::lstManufactureClickLeft(Action *)
{
	const std::vector<Production*> productions(_base->getProductions());
	_game->pushState(new ManufactureInfoStateFtA(_base, productions[_lstManufacture->getSelectedRow()]));
}

/**
* Opens the TechTreeViewer for the corresponding topic.
* @param action Pointer to an action.
*/
void ManufactureState::lstManufactureClickMiddle(Action *)
{
	const std::vector<Production*> productions(_base->getProductions());
	const RuleManufacture *selectedTopic = productions[_lstManufacture->getSelectedRow()]->getRules();
	if (_game->getMod()->getIsResearchTreeDisabled() || !_game->getSavedGame()->getDebugMode())
	{
		return;
	}

	if (_game->isCtrlPressed())
	{
		std::string articleId = selectedTopic->getName();
		Ufopaedia::openArticle(_game, articleId);
	}
	else
	{
		_game->pushState(new TechTreeViewerState(0, selectedTopic));
	}
}

}

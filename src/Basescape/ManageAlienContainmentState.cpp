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
#include "ManageAlienContainmentState.h"
#include <climits>
#include <sstream>
#include <algorithm>
#include "../Engine/Action.h"
#include "../Engine/Game.h"
#include "../Mod/Mod.h"
#include "../Engine/LocalizedText.h"
#include "../Interface/TextButton.h"
#include "../Interface/Window.h"
#include "../Interface/Text.h"
#include "../Interface/TextList.h"
#include "../Savegame/ResearchProject.h"
#include "../Savegame/SavedGame.h"
#include "../Savegame/Base.h"
#include "../Savegame/ItemContainer.h"
#include "../Mod/RuleItem.h"
#include "../Mod/RuleResearch.h"
#include "../Mod/Armor.h"
#include "../Engine/Timer.h"
#include "../Engine/Options.h"
#include "../Menu/ErrorMessageState.h"
#include "SellState.h"
#include "../Mod/RuleInterface.h"
#include "TechTreeViewerState.h"
#include "TransferBaseState.h"

namespace OpenXcom
{

/**
 * Initializes all the elements in the Manage Alien Containment screen.
 * @param game Pointer to the core game.
 * @param base Pointer to the base to get info from.
 * @param origin Game section that originated this state.
 */
ManageAlienContainmentState::ManageAlienContainmentState(Base *base, int prisonType, OptionsOrigin origin) :
	_base(base), _prisonType(prisonType), _origin(origin), _sel(0), _aliensSold(0), _total(0), _doNotReset(false), _threeButtons(false)
{
	_threeButtons = Options::canSellLiveAliens && Options::retainCorpses;

	// Create objects
	_window = new Window(this, 320, 200, 0, 0);
	if (_threeButtons)
	{
		// 3 buttons
		_btnOk = new TextButton(96, 16, 8, 176);
		_btnSell = new TextButton(96, 16, 112, 176);
		_btnCancel = new TextButton(96, 16, 216, 176);
		_btnTransfer = new TextButton(96, 16, 216, 176);
	}
	else
	{
		// 2 buttons
		_btnOk = new TextButton(148, 16, 8, 176);
		_btnSell = new TextButton(148, 16, 8, 176);
		_btnCancel = new TextButton(148, 16, 164, 176);
		_btnTransfer = new TextButton(148, 16, 164, 176);
	}
	_txtTitle = new Text(310, 17, 5, 8);
	_txtAvailable =  new Text(190, 9, 10, 24);
	_txtValueOfSales =  new Text(190, 9, 10, 32);
	_txtUsed = new Text(110, 9, 136, 24);
	_txtItem = new Text(120, 9, 10, 41);
	_txtLiveAliens = new Text(54, 18, 153, 32);
	_txtDeadAliens = new Text(54, 18, 207, 32);
	_txtInterrogatedAliens = new Text(54, 18, 261, 32);
	_lstAliens = new TextList(286, 112, 8, 53);

	// Set palette
	setInterface("manageContainment");

	add(_window, "window", "manageContainment");
	add(_btnSell, "button", "manageContainment");
	add(_btnOk, "button", "manageContainment");
	add(_btnCancel, "button", "manageContainment");
	add(_btnTransfer, "button", "manageContainment");
	add(_txtTitle, "text", "manageContainment");
	add(_txtAvailable, "text", "manageContainment");
	add(_txtValueOfSales, "text", "manageContainment");
	add(_txtUsed, "text", "manageContainment");
	add(_txtItem, "text", "manageContainment");
	add(_txtLiveAliens, "text", "manageContainment");
	add(_txtDeadAliens, "text", "manageContainment");
	add(_txtInterrogatedAliens, "text", "manageContainment");
	add(_lstAliens, "list", "manageContainment");

	centerAllSurfaces();

	// Set up objects
	setWindowBackground(_window, "manageContainment");

	_btnOk->setText(trAlt(_threeButtons ? "STR_KILL_SELECTED" : "STR_REMOVE_SELECTED", _prisonType));
	_btnOk->onMouseClick((ActionHandler)&ManageAlienContainmentState::btnOkClick);
	_btnOk->onKeyboardPress((ActionHandler)&ManageAlienContainmentState::btnOkClick, Options::keyOk);

	_btnSell->setText(trAlt("STR_SELL_SELECTED", _prisonType));
	_btnSell->onMouseClick((ActionHandler)&ManageAlienContainmentState::btnSellClick);

	_btnCancel->setText(tr("STR_CANCEL"));
	_btnCancel->onMouseClick((ActionHandler)&ManageAlienContainmentState::btnCancelClick);
	_btnCancel->onKeyboardPress((ActionHandler)&ManageAlienContainmentState::btnCancelClick, Options::keyCancel);

	_btnTransfer->setText(tr("STR_GO_TO_TRANSFERS"));
	_btnTransfer->onMouseClick((ActionHandler)&ManageAlienContainmentState::btnTransferClick);

	_txtTitle->setBig();
	_txtTitle->setAlign(ALIGN_CENTER);
	_txtTitle->setText(trAlt("STR_MANAGE_CONTAINMENT", _prisonType));

	_txtItem->setText(trAlt("STR_ALIEN", _prisonType));

	_txtLiveAliens->setText(trAlt("STR_LIVE_ALIENS", _prisonType));
	_txtLiveAliens->setWordWrap(true);
	_txtLiveAliens->setVerticalAlign(ALIGN_BOTTOM);

	_txtDeadAliens->setText(trAlt("STR_DEAD_ALIENS", _prisonType));
	_txtDeadAliens->setWordWrap(true);
	_txtDeadAliens->setVerticalAlign(ALIGN_BOTTOM);

	_txtInterrogatedAliens->setText(trAlt("STR_UNDER_INTERROGATION", _prisonType));
	_txtInterrogatedAliens->setWordWrap(true);
	_txtInterrogatedAliens->setVerticalAlign(ALIGN_BOTTOM);

	_lstAliens->setArrowColumn(184, ARROW_HORIZONTAL);
	if (Options::canSellLiveAliens) {
		_lstAliens->setColumns(5, 120, 40, 64, 46, 46);
	} else {
		_lstAliens->setColumns(5, 150, 10, 64, 46, 46);
	}
	_lstAliens->setSelectable(true);
	_lstAliens->setBackground(_window);
	_lstAliens->setMargin(2);
	_lstAliens->onLeftArrowPress((ActionHandler)&ManageAlienContainmentState::lstItemsLeftArrowPress);
	_lstAliens->onLeftArrowRelease((ActionHandler)&ManageAlienContainmentState::lstItemsLeftArrowRelease);
	_lstAliens->onLeftArrowClick((ActionHandler)&ManageAlienContainmentState::lstItemsLeftArrowClick);
	_lstAliens->onRightArrowPress((ActionHandler)&ManageAlienContainmentState::lstItemsRightArrowPress);
	_lstAliens->onRightArrowRelease((ActionHandler)&ManageAlienContainmentState::lstItemsRightArrowRelease);
	_lstAliens->onRightArrowClick((ActionHandler)&ManageAlienContainmentState::lstItemsRightArrowClick);
	_lstAliens->onMousePress((ActionHandler)&ManageAlienContainmentState::lstItemsMousePress);

	_timerInc = new Timer(250);
	_timerInc->onTimer((StateHandler)&ManageAlienContainmentState::increase);
	_timerDec = new Timer(250);
	_timerDec->onTimer((StateHandler)&ManageAlienContainmentState::decrease);
}

/**
 *
 */
ManageAlienContainmentState::~ManageAlienContainmentState()
{
	delete _timerInc;
	delete _timerDec;
}

/**
* Resets stuff when coming back from other screens.
*/
void ManageAlienContainmentState::init()
{
	State::init();

	// coming back from TechTreeViewer
	if (_doNotReset)
	{
		_doNotReset = false;
		return;
	}

	resetListAndTotals();
}

/**
 * Resets the list and the totals, updates button visibility.
 */
void ManageAlienContainmentState::resetListAndTotals()
{
	_qtys.clear();
	_aliens.clear();
	_sel = 0;
	_aliensSold = 0;
	_total = 0;

	_lstAliens->clearList();

	std::vector<std::string> researchList;
	for (std::vector<ResearchProject*>::const_iterator iter = _base->getResearch().begin(); iter != _base->getResearch().end(); ++iter)
	{
		const RuleResearch *research = (*iter)->getRules();
		RuleItem *item = _game->getMod()->getItem(research->getName());
		if (item && item->isAlien() && item->getPrisonType() == _prisonType)
		{
			researchList.push_back(research->getName());
		}
	}

	const std::vector<std::string> &items = _game->getMod()->getItemsList();
	for (std::vector<std::string>::const_iterator i = items.begin(); i != items.end(); ++i)
	{
		int qty = _base->getStorageItems()->getItem(*i);
		RuleItem *rule = _game->getMod()->getItem(*i, true);
		if (qty > 0 && rule->isAlien() && rule->getPrisonType() == _prisonType)
		{
			_qtys.push_back(0);
			_aliens.push_back(*i);
			std::ostringstream ss;
			ss << qty;
			std::string rqty;
			std::vector<std::string>::iterator research = std::find(researchList.begin(), researchList.end(), *i);
			if (research != researchList.end())
			{
				rqty = "1";
				researchList.erase(research);
			}
			else
			{
				rqty = "0";
			}

			std::string formattedCost = "";
			if (Options::canSellLiveAliens)
			{
				int sellCost = rule->getSellCost();
				formattedCost = Unicode::formatFunding(sellCost / 1000).append("K");
			}

			_lstAliens->addRow(5, tr(*i).c_str(), formattedCost.c_str(), ss.str().c_str(), "0", rqty.c_str());
		}
	}

	for (std::vector<std::string>::const_iterator i = researchList.begin(); i != researchList.end(); ++i)
	{
		_aliens.push_back(*i);
		_qtys.push_back(0);
		_lstAliens->addRow(5, tr(*i).c_str(), Options::canSellLiveAliens ? "-" : "", "0", "0", "1");
		_lstAliens->setRowColor(_qtys.size() -1, _lstAliens->getSecondaryColor());
	}

	// update totals
	int availableContainment = _base->getAvailableContainment(_prisonType);
	int usedContainment = _base->getUsedContainment(_prisonType);
	int freeContainment = availableContainment - usedContainment;
	{
		_txtAvailable->setText(tr("STR_SPACE_AVAILABLE").arg(freeContainment));

		_txtUsed->setText(tr("STR_SPACE_USED").arg(usedContainment));

		if (Options::canSellLiveAliens)
		{
			_txtValueOfSales->setText(tr("STR_VALUE_OF_SALES").arg(Unicode::formatFunding(_total)));
		}
	}

	// update buttons
	{
		bool overCrowded = false;
		if (availableContainment == 0 || Options::containmentLimitsEnforced)
		{
			overCrowded = (freeContainment < 0);
		}

		_btnCancel->setVisible(!overCrowded);
		_btnOk->setVisible(!overCrowded);
		_btnSell->setVisible(!overCrowded && _threeButtons);
		_btnTransfer->setVisible(overCrowded);
	}
}

/**
 * Runs the arrow timers.
 */
void ManageAlienContainmentState::think()
{
	State::think();

	_timerInc->think(this, 0);
	_timerDec->think(this, 0);
}

/**
 * Deals with the selected aliens.
 * @param action Pointer to an action.
 */
void ManageAlienContainmentState::btnOkClick(Action *)
{
	bool sell = Options::canSellLiveAliens && !Options::retainCorpses; // in all other cases, it's kill, not sell
	dealWithSelectedAliens(sell);
}

/**
 * Deals with the selected aliens.
 * @param action Pointer to an action.
 */
void ManageAlienContainmentState::btnSellClick(Action *)
{
	dealWithSelectedAliens(true); // this one is always sell
}

/**
 * Deals with the selected aliens.
 */
void ManageAlienContainmentState::dealWithSelectedAliens(bool sell)
{
	for (size_t i = 0; i < _qtys.size(); ++i)
	{
		if (_qtys[i] > 0)
		{
			// remove the aliens
			_base->getStorageItems()->removeItem(_aliens[i], _qtys[i]);

			if (sell)
			{
				_game->getSavedGame()->setFunds(_game->getSavedGame()->getFunds() + _game->getMod()->getItem(_aliens[i], true)->getSellCost() * _qtys[i]);
			}
			else
			{
				// add the corpses
				auto ruleUnit = _game->getMod()->getUnit(_aliens[i], false);
				if (ruleUnit)
				{
					auto ruleCorpse = ruleUnit->getArmor()->getCorpseGeoscape();
					if (ruleCorpse && ruleCorpse->isRecoverable() && ruleCorpse->isCorpseRecoverable())
					{
						_base->getStorageItems()->addItem(ruleCorpse->getType(), _qtys[i]);
					}
				}
			}
		}
	}
	_game->popState();

	if (Options::storageLimitsEnforced && _base->storesOverfull())
	{
		if (_origin == OPT_BATTLESCAPE)
		{
			// not used anymore, because multiple prison types could spam this screen; it will pop up in Geoscape anyway
		}
		else
		{
			_game->pushState(new SellState(_base, 0, _origin));
			_game->pushState(new ErrorMessageState(tr("STR_STORAGE_EXCEEDED").arg(_base->getName()), _palette, _game->getMod()->getInterface("manageContainment")->getElement("errorMessage")->color, "BACK13.SCR", _game->getMod()->getInterface("manageContainment")->getElement("errorPalette")->color));
		}
 	}
}

/**
 * Returns to the previous screen.
 * @param action Pointer to an action.
 */
void ManageAlienContainmentState::btnCancelClick(Action *)
{
	_game->popState();
}

/**
* Opens the Transfer UI and gives the player an option to transfer stuff instead of selling it.
* Returns back to this screen when finished.
* @param action Pointer to an action.
*/
void ManageAlienContainmentState::btnTransferClick(Action *)
{
	_game->pushState(new TransferBaseState(_base, nullptr));
}

/**
 * Starts increasing the alien count.
 * @param action Pointer to an action.
 */
void ManageAlienContainmentState::lstItemsRightArrowPress(Action *action)
{
	_sel = _lstAliens->getSelectedRow();
	if (action->getDetails()->button.button == SDL_BUTTON_LEFT && !_timerInc->isRunning()) _timerInc->start();
}

/**
 * Stops increasing the alien count.
 * @param action Pointer to an action.
 */
void ManageAlienContainmentState::lstItemsRightArrowRelease(Action *action)
{
	if (action->getDetails()->button.button == SDL_BUTTON_LEFT)
	{
		_timerInc->stop();
	}
}

/**
 * Increases the selected alien count;
 * by one on left-click, to max on right-click.
 * @param action Pointer to an action.
 */
void ManageAlienContainmentState::lstItemsRightArrowClick(Action *action)
{
	if (action->getDetails()->button.button == SDL_BUTTON_RIGHT) increaseByValue(INT_MAX);
	if (action->getDetails()->button.button == SDL_BUTTON_LEFT)
	{
		increaseByValue(1);
		_timerInc->setInterval(250);
		_timerDec->setInterval(250);
	}
}

/**
 * Starts decreasing the alien count.
 * @param action Pointer to an action.
 */
void ManageAlienContainmentState::lstItemsLeftArrowPress(Action *action)
{
	_sel = _lstAliens->getSelectedRow();
	if (action->getDetails()->button.button == SDL_BUTTON_LEFT && !_timerDec->isRunning()) _timerDec->start();
}

/**
 * Stops decreasing the alien count.
 * @param action Pointer to an action.
 */
void ManageAlienContainmentState::lstItemsLeftArrowRelease(Action *action)
{
	if (action->getDetails()->button.button == SDL_BUTTON_LEFT)
	{
		_timerDec->stop();
	}
}

/**
 * Decreases the selected alien count;
 * by one on left-click, to 0 on right-click.
 * @param action Pointer to an action.
 */
void ManageAlienContainmentState::lstItemsLeftArrowClick(Action *action)
{
	if (action->getDetails()->button.button == SDL_BUTTON_RIGHT) decreaseByValue(INT_MAX);
	if (action->getDetails()->button.button == SDL_BUTTON_LEFT)
	{
		decreaseByValue(1);
		_timerInc->setInterval(250);
		_timerDec->setInterval(250);
	}
}

/**
 * Handles the mouse-wheels on the arrow-buttons.
 * @param action Pointer to an action.
 */
void ManageAlienContainmentState::lstItemsMousePress(Action *action)
{
	_sel = _lstAliens->getSelectedRow();
	if (action->getDetails()->button.button == SDL_BUTTON_WHEELUP)
	{
		_timerInc->stop();
		_timerDec->stop();
		if (action->getAbsoluteXMouse() >= _lstAliens->getArrowsLeftEdge() &&
			action->getAbsoluteXMouse() <= _lstAliens->getArrowsRightEdge())
		{
			increaseByValue(Options::changeValueByMouseWheel);
		}
	}
	else if (action->getDetails()->button.button == SDL_BUTTON_WHEELDOWN)
	{
		_timerInc->stop();
		_timerDec->stop();
		if (action->getAbsoluteXMouse() >= _lstAliens->getArrowsLeftEdge() &&
			action->getAbsoluteXMouse() <= _lstAliens->getArrowsRightEdge())
		{
			decreaseByValue(Options::changeValueByMouseWheel);
		}
	}
	else if (action->getDetails()->button.button == SDL_BUTTON_MIDDLE)
	{
		RuleResearch *selectedTopic = _game->getMod()->getResearch(_aliens[_sel]);
		if (selectedTopic != 0)
		{
			_doNotReset = true;
			if (_game->getMod()->getIsResearchTreeDisabled() && !_game->getSavedGame()->getDebugMode())
			{
				return;
			}
			else
			{
				_game->pushState(new TechTreeViewerState(selectedTopic, 0));
			}
		}
	}
}

/**
 * Gets the quantity of the currently selected alien on the base.
 * @return Quantity of selected alien on the base.
 */
int ManageAlienContainmentState::getQuantity()
{
	return _base->getStorageItems()->getItem(_aliens[_sel]);
}

/**
 * Increases the quantity of the selected alien to exterminate by one.
 */
void ManageAlienContainmentState::increase()
{
	_timerDec->setInterval(50);
	_timerInc->setInterval(50);
	increaseByValue(1);
}

/**
 * Increases the quantity of the selected alien to exterminate by "change".
 * @param change How much we want to add.
 */
void ManageAlienContainmentState::increaseByValue(int change)
{
	int qty = getQuantity() - _qtys[_sel];
	if (change <= 0 || qty <= 0) return;

	change = std::min(qty, change);
	_qtys[_sel] += change;
	_aliensSold += change;
	updateStrings();
}

/**
 * Decreases the quantity of the selected alien to exterminate by one.
 */
void ManageAlienContainmentState::decrease()
{
	_timerInc->setInterval(50);
	_timerDec->setInterval(50);
	decreaseByValue(1);
}

/**
 * Decreases the quantity of the selected alien to exterminate by "change".
 * @param change How much we want to remove.
 */
void ManageAlienContainmentState::decreaseByValue(int change)
{
	if (change <= 0 || _qtys[_sel] <= 0) return;
	change = std::min(_qtys[_sel], change);
	_qtys[_sel] -= change;
	_aliensSold -= change;
	updateStrings();
}

/**
 * Updates the quantity-strings of the selected alien.
 */
void ManageAlienContainmentState::updateStrings()
{
	std::ostringstream ss, ss2;
	int qty = getQuantity() - _qtys[_sel];
	ss << qty;
	ss2 << _qtys[_sel];

	_lstAliens->setRowColor(_sel, (qty == 0)? _lstAliens->getSecondaryColor() : _lstAliens->getColor());
	_lstAliens->setCellText(_sel, 2, ss.str());
	_lstAliens->setCellText(_sel, 3, ss2.str());

	int aliens = _base->getUsedContainment(_prisonType) - _aliensSold;
	int availableContainment = _base->getAvailableContainment(_prisonType);
	int spaces = availableContainment - aliens;
	if (availableContainment == 0 || Options::containmentLimitsEnforced)
	{
		_btnOk->setVisible(spaces >= 0);
		_btnSell->setVisible(spaces >= 0 && _threeButtons);
	}
	_txtAvailable->setText(tr("STR_SPACE_AVAILABLE").arg(spaces));
	_txtUsed->setText(tr("STR_SPACE_USED").arg(aliens));

	if (Options::canSellLiveAliens)
	{
		// we could probably keep track of _total with each change (only adding deltas), but I am lazy today
		_total = 0;
		for (size_t i = 0; i < _qtys.size(); ++i)
		{
			if (_qtys[i] > 0)
			{
				_total += _game->getMod()->getItem(_aliens[i])->getSellCost() * _qtys[i];
			}
		}
		_txtValueOfSales->setText(tr("STR_VALUE_OF_SALES").arg(Unicode::formatFunding(_total)));
	}
}

}

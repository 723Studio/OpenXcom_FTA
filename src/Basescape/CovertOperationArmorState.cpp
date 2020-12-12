// CovertOperationArmorState

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
#include "CovertOperationArmorState.h"
#include "CovertOperationStartState.h"
#include <climits>
#include "../Engine/Game.h"
#include "../Mod/Mod.h"
#include "../Engine/LocalizedText.h"
#include "../Engine/Options.h"
#include "../Interface/ComboBox.h"
#include "../Engine/Action.h"
#include "../Interface/TextButton.h"
#include "../Interface/Window.h"
#include "../Interface/Text.h"
#include "../Interface/TextList.h"
#include "../Interface/ArrowButton.h"
#include "../Savegame/Base.h"
#include "../Savegame/Soldier.h"
#include "../Savegame/SavedGame.h"
#include "../Savegame/Craft.h"
#include "../Mod/Armor.h"
#include "SoldierArmorState.h"
#include "../Savegame/SavedGame.h"
#include "../Savegame/ItemContainer.h"
#include "../Mod/RuleInterface.h"
#include "../Mod/RuleSoldier.h"
#include "../Mod/RuleCovertOperation.h"
#include "../Ufopaedia/Ufopaedia.h"
#include <algorithm>
#include "../Engine/Unicode.h"



#include "../Engine/Action.h"
#include "../Engine/Game.h"
#include "../Mod/Mod.h"
#include "../Engine/LocalizedText.h"
#include "../Engine/Options.h"
#include "../Interface/ArrowButton.h"
#include "../Interface/TextButton.h"
#include "../Interface/Window.h"
#include "../Interface/Text.h"
#include "../Interface/TextList.h"
#include "../Menu/ErrorMessageState.h"
#include "../Mod/Armor.h"
#include "../Mod/RuleInterface.h"
#include "../Savegame/SavedGame.h"
#include "../Savegame/Craft.h"
#include "../Savegame/Soldier.h"
#include "../Savegame/Base.h"
#include "../Savegame/ItemContainer.h"
#include "../Mod/RuleSoldier.h"
#include "../Ufopaedia/Ufopaedia.h"

namespace OpenXcom
{

/**
* Initializes all the elements in the Craft Armor screen.
* @param game Pointer to the core game.
* @param base Pointer to the base to get info from.
* @param craft ID of the selected craft.
*/
CovertOperationArmorState::CovertOperationArmorState(Base* base, CovertOperationStartState* operation) : _base(base), _operation(operation), _savedScrollPosition(0), _origSoldierOrder(*_base->getSoldiers()), _dynGetter(NULL)
{
	// Create objects
	_window = new Window(this, 320, 200, 0, 0);
	_btnOk = new TextButton(148, 16, 164, 176);
	_txtTitle = new Text(300, 17, 16, 7);
	_txtName = new Text(114, 9, 16, 32);
	_txtCraft = new Text(76, 9, 122, 32);
	_txtArmor = new Text(100, 9, 192, 32);
	_txtChances = new Text(115, 9, 192, 24);
	_lstSoldiers = new TextList(288, 128, 8, 40);
	_cbxSortBy = new ComboBox(this, 148, 16, 8, 176, true);

	// Set palette
	setInterface("operationArmor");

	add(_window, "window", "operationArmor");
	add(_btnOk, "button", "operationArmor");
	add(_txtTitle, "text", "operationArmor");
	add(_txtName, "text", "operationArmor");
	add(_txtCraft, "text", "operationArmor");
	add(_txtArmor, "text", "operationArmor");
	add(_txtChances, "text", "operationArmor");
	add(_lstSoldiers, "list", "operationArmor");
	add(_cbxSortBy, "button", "operationArmor");

	centerAllSurfaces();

	// Set up objects
	setWindowBackground(_window, "operationArmor");

	_btnOk->setText(tr("STR_OK"));
	_btnOk->onMouseClick((ActionHandler)&CovertOperationArmorState::btnOkClick);
	_btnOk->onKeyboardPress((ActionHandler)&CovertOperationArmorState::btnOkClick, Options::keyCancel);
	_btnOk->onKeyboardPress((ActionHandler)&CovertOperationArmorState::btnDeequipAllArmorClick, Options::keyRemoveArmorFromAllCrafts);
	_btnOk->onKeyboardPress((ActionHandler)&CovertOperationArmorState::btnDeequipCraftArmorClick, Options::keyRemoveArmorFromCraft);

	_txtTitle->setBig();
	_txtTitle->setText(tr("STR_SELECT_ARMOR"));

	_txtName->setText(tr("STR_NAME_UC"));

	_txtCraft->setText(tr("STR_CRAFT"));

	_txtArmor->setText(tr("STR_ARMOR"));

	bool mod = _game->getSavedGame()->getDebugMode();
	_txtChances->setText(tr("STR_OPERATION_CHANCES_US").arg(tr(_operation->getOperationOddsString(mod))));

	// populate sort options
	std::vector<std::string> sortOptions;
	sortOptions.push_back(tr("STR_ORIGINAL_ORDER"));
	_sortFunctors.push_back(NULL);

	#define PUSH_IN(strId, functor) \
	sortOptions.push_back(tr(strId)); \
	_sortFunctors.push_back(new SortFunctor(_game, functor));

	PUSH_IN("STR_ID", idStat);
	PUSH_IN("STR_NAME_UC", nameStat);
	PUSH_IN("STR_SOLDIER_TYPE", typeStat);
	PUSH_IN("STR_RANK", rankStat);
	PUSH_IN("STR_IDLE_DAYS", idleDaysStat);
	PUSH_IN("STR_MISSIONS2", missionsStat);
	PUSH_IN("STR_KILLS2", killsStat);
	PUSH_IN("STR_WOUND_RECOVERY2", woundRecoveryStat);
	if (_game->getMod()->isManaFeatureEnabled() && !_game->getMod()->getReplenishManaAfterMission())
	{
		PUSH_IN("STR_MANA_MISSING", manaMissingStat);
	}
	PUSH_IN("STR_TIME_UNITS", tuStat);
	PUSH_IN("STR_STAMINA", staminaStat);
	PUSH_IN("STR_HEALTH", healthStat);
	PUSH_IN("STR_BRAVERY", braveryStat);
	PUSH_IN("STR_REACTIONS", reactionsStat);
	PUSH_IN("STR_FIRING_ACCURACY", firingStat);
	PUSH_IN("STR_THROWING_ACCURACY", throwingStat);
	PUSH_IN("STR_MELEE_ACCURACY", meleeStat);
	PUSH_IN("STR_STRENGTH", strengthStat);
	if (_game->getMod()->isManaFeatureEnabled())
	{
		// "unlock" is checked later
		PUSH_IN("STR_MANA_POOL", manaStat);
	}
	PUSH_IN("STR_PSIONIC_STRENGTH", psiStrengthStat);
	PUSH_IN("STR_PSIONIC_SKILL", psiSkillStat);

	#undef PUSH_IN

	_cbxSortBy->setOptions(sortOptions);
	_cbxSortBy->setSelected(0);
	_cbxSortBy->onChange((ActionHandler)&CovertOperationArmorState::cbxSortByChange);
	_cbxSortBy->setText(tr("STR_SORT_BY"));

	//_lstSoldiers->setArrowColumn(-1, ARROW_VERTICAL);
	_lstSoldiers->setColumns(3, 106, 70, 104);
	_lstSoldiers->setAlign(ALIGN_RIGHT, 3);
	_lstSoldiers->setSelectable(true);
	_lstSoldiers->setBackground(_window);
	_lstSoldiers->setMargin(8);
	_lstSoldiers->onLeftArrowClick((ActionHandler)&CovertOperationArmorState::lstItemsLeftArrowClick);
	_lstSoldiers->onRightArrowClick((ActionHandler)&CovertOperationArmorState::lstItemsRightArrowClick);
	_lstSoldiers->onMouseClick((ActionHandler)&CovertOperationArmorState::lstSoldiersClick, 0);
	_lstSoldiers->onMousePress((ActionHandler)&CovertOperationArmorState::lstSoldiersMousePress);
}

/**
* cleans up dynamic state
*/
CovertOperationArmorState::~CovertOperationArmorState()
{
	for (std::vector<SortFunctor*>::iterator it = _sortFunctors.begin(); it != _sortFunctors.end(); ++it)
	{
		delete(*it);
	}
}

/**
* Sorts the soldiers list by the selected criterion
* @param action Pointer to an action.
*/
void CovertOperationArmorState::cbxSortByChange(Action* action)
{
	bool ctrlPressed = SDL_GetModState() & KMOD_CTRL;
	size_t selIdx = _cbxSortBy->getSelected();
	if (selIdx == (size_t)-1)
	{
		return;
	}

	SortFunctor* compFunc = _sortFunctors[selIdx];
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
					[](const Soldier* a, const Soldier* b)
					{
						return Unicode::naturalCompare(a->getName(), b->getName());
					}
				);
			}
			else
			{
				std::stable_sort(_base->getSoldiers()->begin(), _base->getSoldiers()->end(), *compFunc);
			}
			bool shiftPressed = SDL_GetModState() & KMOD_SHIFT;
			if (shiftPressed)
			{
				std::reverse(_base->getSoldiers()->begin(), _base->getSoldiers()->end());
			}
		}
	}
	else
	{
		// restore original ordering, ignoring (of course) those
		// soldiers that have been sacked since this state started
		for (std::vector<Soldier*>::const_iterator it = _origSoldierOrder.begin();
			it != _origSoldierOrder.end(); ++it)
		{
			std::vector<Soldier*>::iterator soldierIt =
				std::find(_base->getSoldiers()->begin(), _base->getSoldiers()->end(), *it);
			if (soldierIt != _base->getSoldiers()->end())
			{
				Soldier* s = *soldierIt;
				_base->getSoldiers()->erase(soldierIt);
				_base->getSoldiers()->insert(_base->getSoldiers()->end(), s);
			}
		}
	}

	initList(_lstSoldiers->getScroll());
}

/**
* The soldier armors can change
* after going into other screens.
*/
void CovertOperationArmorState::init()
{
	State::init();
	_base->prepareSoldierStatsWithBonuses(); // refresh stats for sorting
	initList(_savedScrollPosition);

	int row = 0;
	for (std::vector<Soldier*>::iterator i = _base->getSoldiers()->begin(); i != _base->getSoldiers()->end(); ++i)
	{
		_lstSoldiers->setCellText(row, 2, tr((*i)->getArmor()->getType()));
		row++;
	}
	bool mod = _game->getSavedGame()->getDebugMode();
	_txtChances->setText(tr("STR_OPERATION_CHANCES_US").arg(tr(_operation->getOperationOddsString(mod))));
}

/**
* Shows the soldiers in a list at specified offset/scroll.
*/
void CovertOperationArmorState::initList(size_t scrl)
{
	Uint8 otherCraftColor = _game->getMod()->getInterface("operationArmor")->getElement("otherCraft")->color;
	int row = 0;
	_lstSoldiers->clearList();

	if (_dynGetter != NULL)
	{
		_lstSoldiers->setArrowColumn(160, ARROW_VERTICAL);
		_lstSoldiers->setColumns(4, 106, 70, 88, 16);
	}
	else
	{
		_lstSoldiers->setArrowColumn(-1, ARROW_VERTICAL);
		_lstSoldiers->setColumns(3, 106, 70, 104);
	}

	auto recovery = _base->getSumRecoveryPerDay();
	for (std::vector<Soldier*>::iterator i = _base->getSoldiers()->begin(); i != _base->getSoldiers()->end(); ++i)
	{
		if (_dynGetter != NULL)
		{
			// call corresponding getter
			int dynStat = (*_dynGetter)(_game, *i);
			std::ostringstream ss;
			ss << dynStat;
			_lstSoldiers->addRow(4, (*i)->getName(true).c_str(), (*i)->getCraftString(_game->getLanguage(), recovery).c_str(), tr((*i)->getArmor()->getType()).c_str(), ss.str().c_str());
		}
		else
		{
			_lstSoldiers->addRow(3, (*i)->getName(true).c_str(), (*i)->getCraftString(_game->getLanguage(), recovery).c_str(), tr((*i)->getArmor()->getType()).c_str());
		}

		Uint8 color;

		auto opSoldiers = _operation->getSoldiers();
		bool matched = false;

		auto iter = std::find(std::begin(opSoldiers), std::end(opSoldiers), (*i));
		if (iter != std::end(opSoldiers)) {
			matched = true;
		}

		if (matched)
		{
			color = _lstSoldiers->getSecondaryColor();
			_lstSoldiers->setCellText(row, 2, tr("STR_ASSIGNED_UC"));
		}
		else if ((*i)->getCraft() != 0 || (*i)->getCovertOperation() != 0)
		{
			color = otherCraftColor;
		}
		else
		{
			color = _lstSoldiers->getColor();
		}
		_lstSoldiers->setRowColor(row, color);
		row++;
	}
	if (scrl)
		_lstSoldiers->scrollTo(scrl);
	_lstSoldiers->draw();
}

/**
* Reorders a soldier up.
* @param action Pointer to an action.
*/
void CovertOperationArmorState::lstItemsLeftArrowClick(Action* action)
{
	unsigned int row = _lstSoldiers->getSelectedRow();
	if (row > 0)
	{
		if (action->getDetails()->button.button == SDL_BUTTON_LEFT)
		{
			moveSoldierUp(action, row);
		}
		else if (action->getDetails()->button.button == SDL_BUTTON_RIGHT)
		{
			moveSoldierUp(action, row, true);
		}
	}
	_cbxSortBy->setText(tr("STR_SORT_BY"));
	_cbxSortBy->setSelected(-1);
}

/**
* Moves a soldier up on the list.
* @param action Pointer to an action.
* @param row Selected soldier row.
* @param max Move the soldier to the top?
*/
void CovertOperationArmorState::moveSoldierUp(Action* action, unsigned int row, bool max)
{
	Soldier* s = _base->getSoldiers()->at(row);
	if (max)
	{
		_base->getSoldiers()->erase(_base->getSoldiers()->begin() + row);
		_base->getSoldiers()->insert(_base->getSoldiers()->begin(), s);
	}
	else
	{
		_base->getSoldiers()->at(row) = _base->getSoldiers()->at(row - 1);
		_base->getSoldiers()->at(row - 1) = s;
		if (row != _lstSoldiers->getScroll())
		{
			SDL_WarpMouse(action->getLeftBlackBand() + action->getXMouse(), action->getTopBlackBand() + action->getYMouse() - static_cast<Uint16>(8 * action->getYScale()));
		}
		else
		{
			_lstSoldiers->scrollUp(false);
		}
	}
	initList(_lstSoldiers->getScroll());
}

/**
* Reorders a soldier down.
* @param action Pointer to an action.
*/
void CovertOperationArmorState::lstItemsRightArrowClick(Action* action)
{
	unsigned int row = _lstSoldiers->getSelectedRow();
	size_t numSoldiers = _base->getSoldiers()->size();
	if (0 < numSoldiers && INT_MAX >= numSoldiers && row < numSoldiers - 1)
	{
		if (action->getDetails()->button.button == SDL_BUTTON_LEFT)
		{
			moveSoldierDown(action, row);
		}
		else if (action->getDetails()->button.button == SDL_BUTTON_RIGHT)
		{
			moveSoldierDown(action, row, true);
		}
	}
	_cbxSortBy->setText(tr("STR_SORT_BY"));
	_cbxSortBy->setSelected(-1);
}

/**
* Moves a soldier down on the list.
* @param action Pointer to an action.
* @param row Selected soldier row.
* @param max Move the soldier to the bottom?
*/
void CovertOperationArmorState::moveSoldierDown(Action* action, unsigned int row, bool max)
{
	Soldier* s = _base->getSoldiers()->at(row);
	if (max)
	{
		_base->getSoldiers()->erase(_base->getSoldiers()->begin() + row);
		_base->getSoldiers()->insert(_base->getSoldiers()->end(), s);
	}
	else
	{
		_base->getSoldiers()->at(row) = _base->getSoldiers()->at(row + 1);
		_base->getSoldiers()->at(row + 1) = s;
		if (row != _lstSoldiers->getVisibleRows() - 1 + _lstSoldiers->getScroll())
		{
			SDL_WarpMouse(action->getLeftBlackBand() + action->getXMouse(), action->getTopBlackBand() + action->getYMouse() + static_cast<Uint16>(8 * action->getYScale()));
		}
		else
		{
			_lstSoldiers->scrollDown(false);
		}
	}
	initList(_lstSoldiers->getScroll());
}

/**
* Returns to the previous screen.
* @param action Pointer to an action.
*/
void CovertOperationArmorState::btnOkClick(Action*)
{
	_game->popState();
}

/**
* Shows the Select Armor window.
* @param action Pointer to an action.
*/
void CovertOperationArmorState::lstSoldiersClick(Action* action)
{
	double mx = action->getAbsoluteXMouse();
	if (mx >= _lstSoldiers->getArrowsLeftEdge() && mx < _lstSoldiers->getArrowsRightEdge())
	{
		return;
	}

	Soldier* s = _base->getSoldiers()->at(_lstSoldiers->getSelectedRow());
	if (!(s->getCraft() && s->getCraft()->getStatus() == "STR_OUT") && !(s->getCovertOperation() != 0))
	{
		std::vector<std::string> allowedArmor = _operation->getRule()->getAllowedArmor();
		if (action->getDetails()->button.button == SDL_BUTTON_LEFT)
		{
			_savedScrollPosition = _lstSoldiers->getScroll();
			if (!allowedArmor.empty()) _game->pushState(new SoldierArmorState(_base, _lstSoldiers->getSelectedRow(), SA_GEOSCAPE));// #FINNIKTODO _game->pushState(new CovertOperationSoldierArmorState(_base, _lstSoldiers->getSelectedRow(), SA_GEOSCAPE, allowedArmor));
			else _game->pushState(new SoldierArmorState(_base, _lstSoldiers->getSelectedRow(), SA_GEOSCAPE));
		}
		else if (action->getDetails()->button.button == SDL_BUTTON_RIGHT)
		{
			SavedGame* save;
			save = _game->getSavedGame();
			Armor* a = _game->getMod()->getArmor(save->getLastSelectedArmor());
			if (a && a->getCanBeUsedBy(s->getRules()))
			{
				if (save->getMonthsPassed() != -1)
				{
					if (a->getStoreItem() == nullptr || _base->getStorageItems()->getItem(a->getStoreItem()) > 0)
					{
						if (s->getArmor()->getStoreItem())
						{
							_base->getStorageItems()->addItem(s->getArmor()->getStoreItem());
						}
						if (a->getStoreItem())
						{
							_base->getStorageItems()->removeItem(a->getStoreItem());
						}

						s->setArmor(a);
						s->prepareStatsWithBonuses(_game->getMod()); // refresh stats for sorting
						_lstSoldiers->setCellText(_lstSoldiers->getSelectedRow(), 2, tr(a->getType()));
					}
				}
				else
				{
					s->setArmor(a);
					s->prepareStatsWithBonuses(_game->getMod()); // refresh stats for sorting
					_lstSoldiers->setCellText(_lstSoldiers->getSelectedRow(), 2, tr(a->getType()));
				}
			}
		}
		else if (action->getDetails()->button.button == SDL_BUTTON_MIDDLE)
		{
			std::string articleId = s->getArmor()->getType();
			Ufopaedia::openArticle(_game, articleId);
		}
	}
}

/**
* Handles the mouse-wheels on the arrow-buttons.
* @param action Pointer to an action.
*/
void CovertOperationArmorState::lstSoldiersMousePress(Action* action)
{
	if (Options::changeValueByMouseWheel == 0)
		return;
	unsigned int row = _lstSoldiers->getSelectedRow();
	size_t numSoldiers = _base->getSoldiers()->size();
	if (action->getDetails()->button.button == SDL_BUTTON_WHEELUP &&
		row > 0)
	{
		if (action->getAbsoluteXMouse() >= _lstSoldiers->getArrowsLeftEdge() &&
			action->getAbsoluteXMouse() <= _lstSoldiers->getArrowsRightEdge())
		{
			moveSoldierUp(action, row);
		}
	}
	else if (action->getDetails()->button.button == SDL_BUTTON_WHEELDOWN &&
		0 < numSoldiers && INT_MAX >= numSoldiers && row < numSoldiers - 1)
	{
		if (action->getAbsoluteXMouse() >= _lstSoldiers->getArrowsLeftEdge() &&
			action->getAbsoluteXMouse() <= _lstSoldiers->getArrowsRightEdge())
		{
			moveSoldierDown(action, row);
		}
	}
}

/**
* De-equip armor from all soldiers located in the base (i.e. not out on a mission).
* @param action Pointer to an action.
*/
void CovertOperationArmorState::btnDeequipAllArmorClick(Action* action)
{
	int row = 0;
	for (std::vector<Soldier*>::iterator i = _base->getSoldiers()->begin(); i != _base->getSoldiers()->end(); ++i)
	{
		if (!((*i)->getCraft() && (*i)->getCraft()->getStatus() == "STR_OUT") && (*i)->getCovertOperation() != 0)
		{
			Armor* a = _game->getMod()->getArmor((*i)->getRules()->getArmor());

			if (a->getStoreItem() == nullptr || _base->getStorageItems()->getItem(a->getStoreItem()) > 0)
			{
				if ((*i)->getArmor()->getStoreItem())
				{
					_base->getStorageItems()->addItem((*i)->getArmor()->getStoreItem());
				}
				if (a->getStoreItem())
				{
					_base->getStorageItems()->removeItem(a->getStoreItem());
				}

				(*i)->setArmor(a);
				(*i)->prepareStatsWithBonuses(_game->getMod()); // refresh stats for sorting
				_lstSoldiers->setCellText(row, 2, tr(a->getType()));
			}
		}
		row++;
	}
}

/**
* De-equip armor of all soldiers on the current craft, and also all soldiers not assigned to any craft.
* @param action Pointer to an action.
*/
void CovertOperationArmorState::btnDeequipCraftArmorClick(Action* action)
{
	int row = 0;
	for (auto s : *_base->getSoldiers())
	{
		auto opSoldiers = _operation->getSoldiers();
		bool matched = false;

		auto iter = std::find(std::begin(opSoldiers), std::end(opSoldiers), (s));
		if (iter != std::end(opSoldiers)) {
			matched = true;
		}
		if (matched || s->getCraft() == 0)
		{
			Armor* a = _game->getMod()->getArmor(s->getRules()->getArmor());

			if (a->getStoreItem() == nullptr || _base->getStorageItems()->getItem(a->getStoreItem()) > 0)
			{
				if (s->getArmor()->getStoreItem())
				{
					_base->getStorageItems()->addItem(s->getArmor()->getStoreItem());
				}
				if (a->getStoreItem())
				{
					_base->getStorageItems()->removeItem(a->getStoreItem());
				}

				s->setArmor(a);
				s->prepareStatsWithBonuses(_game->getMod()); // refresh stats for sorting
				_lstSoldiers->setCellText(row, 2, tr(a->getType()));
			}
		}
		row++;
	}
}

struct compareArmorName
{
	typedef ArmorItem& first_argument_type;
	typedef ArmorItem& second_argument_type;
	typedef bool result_type;

	bool _reverse;

	compareArmorName(bool reverse) : _reverse(reverse) {}

	bool operator()(const ArmorItem& a, const ArmorItem& b) const
	{
		return Unicode::naturalCompare(a.name, b.name);
	}
};


/**
 * Initializes all the elements in the Soldier Armor window.
 * @param game Pointer to the core game.
 * @param base Pointer to the base to get info from.
 * @param soldier ID of the selected soldier.
 */
CovertOperationSoldierArmorState::CovertOperationSoldierArmorState(Base* base, size_t soldier, SoldierArmorOrigin origin, std::vector<std::string> allowedArmor) :
	_base(base), _soldier(soldier), _origin(origin), _allowedArmor(allowedArmor)
{
	_screen = false;

	// Create objects
	_window = new Window(this, 192, 160, 64, 20, POPUP_BOTH);
	_btnCancel = new TextButton(140, 16, 90, 156);
	_txtTitle = new Text(182, 16, 69, 28);
	_txtType = new Text(90, 9, 80, 52);
	_txtQuantity = new Text(70, 9, 190, 52);
	_lstArmor = new TextList(160, 80, 73, 68);
	_sortName = new ArrowButton(ARROW_NONE, 11, 8, 80, 52);

	// Set palette
	if (_origin == SA_BATTLESCAPE)
	{
		setStandardPalette("PAL_BATTLESCAPE");
	}
	else
	{
		setInterface("covertOperationSoldierArmor");
	}

	add(_window, "window", "covertOperationSoldierArmor");
	add(_btnCancel, "button", "covertOperationSoldierArmor");
	add(_txtTitle, "text", "covertOperationSoldierArmor");
	add(_txtType, "text", "covertOperationSoldierArmor");
	add(_txtQuantity, "text", "covertOperationSoldierArmor");
	add(_lstArmor, "list", "covertOperationSoldierArmor");
	add(_sortName, "text", "covertOperationSoldierArmor");

	centerAllSurfaces();

	// Set up objects
	setWindowBackground(_window, "covertOperationSoldierArmor");


	_btnCancel->setText(tr("STR_CANCEL_UC"));
	_btnCancel->onMouseClick((ActionHandler)&CovertOperationSoldierArmorState::btnCancelClick);
	_btnCancel->onKeyboardPress((ActionHandler)&CovertOperationSoldierArmorState::btnCancelClick, Options::keyCancel);

	Soldier* s = _base->getSoldiers()->at(_soldier);
	_txtTitle->setAlign(ALIGN_CENTER);
	_txtTitle->setText(tr("STR_SELECT_ARMOR_FOR_SOLDIER").arg(s->getName()));

	_txtType->setText(tr("STR_TYPE"));

	_txtQuantity->setText(tr("STR_QUANTITY_UC"));

	_lstArmor->setColumns(2, 132, 21);
	_lstArmor->setSelectable(true);
	_lstArmor->setBackground(_window);
	_lstArmor->setMargin(8);

	_sortName->setX(_sortName->getX() + _txtType->getTextWidth() + 4);
	_sortName->onMouseClick((ActionHandler)&SoldierArmorState::sortNameClick);

	const auto& armors = _game->getMod()->getArmorsForSoldiers();
	for (auto* a : armors)
	{
		if (a->getRequiredResearch() && !_game->getSavedGame()->isResearched(a->getRequiredResearch()))
			continue;
		if (!a->getCanBeUsedBy(s->getRules()))
			continue;
		if (a->hasInfiniteSupply())
		{
			_armors.push_back(ArmorItem(a->getType(), tr(a->getType()), ""));
		}
		else if (_base->getStorageItems()->getItem(a->getStoreItem()) > 0)
		{
			std::ostringstream ss;
			if (_game->getSavedGame()->getMonthsPassed() > -1)
			{
				ss << _base->getStorageItems()->getItem(a->getStoreItem());
			}
			else
			{
				ss << "-";
			}
			_armors.push_back(ArmorItem(a->getType(), tr(a->getType()), ss.str()));
		}
	}

	_armorOrder = ARMOR_SORT_NONE;
	updateArrows();
	updateList();

	_lstArmor->onMouseClick((ActionHandler)&SoldierArmorState::lstArmorClick);
	_lstArmor->onMouseClick((ActionHandler)&SoldierArmorState::lstArmorClickMiddle, SDL_BUTTON_MIDDLE);

	// switch to battlescape theme if called from inventory
	if (_origin == SA_BATTLESCAPE)
	{
		applyBattlescapeTheme();
	}
}

/**
 *
 */
CovertOperationSoldierArmorState::~CovertOperationSoldierArmorState()
{

}

/**
* Updates the sorting arrows based
* on the current setting.
*/
void CovertOperationSoldierArmorState::updateArrows()
{
	_sortName->setShape(ARROW_NONE);
	switch (_armorOrder)
	{
	case ARMOR_SORT_NAME_ASC:
		_sortName->setShape(ARROW_SMALL_UP);
		break;
	case ARMOR_SORT_NAME_DESC:
		_sortName->setShape(ARROW_SMALL_DOWN);
		break;
	default:
		break;
	}
}

/**
* Sorts the armor list.
* @param sort Order to sort the armors in.
*/
void CovertOperationSoldierArmorState::sortList()
{
	switch (_armorOrder)
	{
	case ARMOR_SORT_NAME_ASC:
		std::sort(_armors.begin(), _armors.end(), compareArmorName(false));
		break;
	case ARMOR_SORT_NAME_DESC:
		std::sort(_armors.rbegin(), _armors.rend(), compareArmorName(true));
		break;
	default:
		break;
	}
	updateList();
}

/**
* Updates the armor list with the current list
* of available armors.
*/
void CovertOperationSoldierArmorState::updateList()
{
	_lstArmor->clearList();
	int row = 0;
	for (std::vector<ArmorItem>::const_iterator j = _armors.begin(); j != _armors.end(); ++j)
	{
		_lstArmor->addRow(2, (*j).name.c_str(), (*j).quantity.c_str());
		bool allowed = false;
		auto iter = std::find(std::begin(_allowedArmor), std::end(_allowedArmor), (*j).type);
		if (iter != std::end(_allowedArmor)) {
			allowed = true;
		}
		Uint8 color;
		if (!allowed)
		{
			color = _lstArmor->getSecondaryColor();
		}
		else
		{
			color = _lstArmor->getColor();
		}
		_lstArmor->setRowColor(row, color);
		row++;
	}
}

/**
 * Returns to the previous screen.
 * @param action Pointer to an action.
 */
void CovertOperationSoldierArmorState::btnCancelClick(Action*)
{
	_game->popState();
}

/**
 * Equips the armor on the soldier and returns to the previous screen.
 * @param action Pointer to an action.
 */
void CovertOperationSoldierArmorState::lstArmorClick(Action*)
{
	Soldier* soldier = _base->getSoldiers()->at(_soldier);
	Armor* prev = soldier->getArmor();
	Armor* next = _game->getMod()->getArmor(_armors[_lstArmor->getSelectedRow()].type);
	Craft* craft = soldier->getCraft();
	if (craft != 0 && next->getSize() > prev->getSize())
	{
		if (craft->getNumVehicles() >= craft->getRules()->getVehicles() || craft->getSpaceAvailable() < 3)
		{
			_game->pushState(new ErrorMessageState(tr("STR_NOT_ENOUGH_CRAFT_SPACE"), _palette, _game->getMod()->getInterface("soldierInfo")->getElement("errorMessage")->color, "BACK01.SCR", _game->getMod()->getInterface("soldierInfo")->getElement("errorPalette")->color));
			return;
		}
	}
	if (_game->getSavedGame()->getMonthsPassed() != -1)
	{
		if (prev->getStoreItem())
		{
			_base->getStorageItems()->addItem(prev->getStoreItem());
		}
		if (next->getStoreItem())
		{
			_base->getStorageItems()->removeItem(next->getStoreItem());
		}
	}
	soldier->setArmor(next);
	_game->getSavedGame()->setLastSelectedArmor(next->getType());

	_game->popState();
}

/**
* Shows corresponding Ufopaedia article.
* @param action Pointer to an action.
*/
void CovertOperationSoldierArmorState::lstArmorClickMiddle(Action* action)
{
	std::string articleId = _armors[_lstArmor->getSelectedRow()].type;
	Ufopaedia::openArticle(_game, articleId);
}

/**
* Sorts the armors by name.
* @param action Pointer to an action.
*/
void CovertOperationSoldierArmorState::sortNameClick(Action*)
{
	if (_armorOrder == ARMOR_SORT_NAME_ASC)
	{
		_armorOrder = ARMOR_SORT_NAME_DESC;
	}
	else
	{
		_armorOrder = ARMOR_SORT_NAME_ASC;
	}
	updateArrows();
	sortList();
}

}

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
#include "FacilityAllocateEngineersState.h"
#include "../Engine/Action.h"
#include "../Engine/Game.h"
#include "../Engine/LocalizedText.h"
#include "../Engine/Options.h"
#include "../Engine/Unicode.h"
#include "../Interface/Text.h"
#include "../Interface/TextButton.h"
#include "../Interface/TextList.h"
#include "../Interface/Window.h"
#include "../Menu/ErrorMessageState.h"
#include "../Mod/Mod.h"
#include "../Mod/RuleInterface.h"
#include "../Mod/RuleSoldier.h"
#include "../Savegame/Base.h"
#include "../Savegame/SavedGame.h"
#include "../Savegame/Soldier.h"
#include "../Savegame/Production.h"
#include "../Savegame/BaseFacility.h"
#include "../Mod/RuleManufacture.h"
#include "../Ufopaedia/Ufopaedia.h"
#include "SoldierInfoState.h"
#include <algorithm>
#include <climits>

namespace OpenXcom
{

/**
* Initializes all the elements in the CovertOperation Soldiers screen.
* @param base Pointer to the base to get info from.
* @param operation Pointer to starting (not committed) covert operation.
*/
FacilityAllocateEngineersState::FacilityAllocateEngineersState(Base* base, Production* production)
	: _base(base), _production(production), _otherCraftColor(0)
{

	// Create objects
	_window = new Window(this, 320, 200, 0, 0);
	_btnOk = new TextButton(148, 16, 164, 176);
	_btnInfo = new TextButton(42, 16, 270, 8);
	_txtTitle = new Text(300, 17, 16, 7);
	_txtName = new Text(114, 9, 16, 32);
	_txtAssignment = new Text(84, 9, 122, 32);
	_txtStat = new Text(82, 9, 214, 32);
	_lstEngineers = new TextList(288, 128, 8, 40);

	// Set palette
	setInterface("manufactureAllocateEngineers");

	add(_window, "window", "manufactureAllocateEngineers");
	add(_btnOk, "button", "manufactureAllocateEngineers");
	add(_btnInfo, "button2", "manufactureAllocateEngineers");
	add(_txtTitle, "text", "manufactureAllocateEngineers");
	add(_txtName, "text", "manufactureAllocateEngineers");
	add(_txtAssignment, "text", "manufactureAllocateEngineers");
	add(_txtStat, "text", "manufactureAllocateEngineers");
	add(_lstEngineers, "list", "manufactureAllocateEngineers");

	_otherCraftColor = _game->getMod()->getInterface("manufactureAllocateEngineers")->getElement("otherCraft")->color;

	centerAllSurfaces();

	// Set up objects
	setWindowBackground(_window, "manufactureAllocateEngineers");

	_btnOk->setText(tr("STR_OK"));
	_btnOk->onMouseClick((ActionHandler)&FacilityAllocateEngineersState::btnOkClick);
	_btnOk->onKeyboardPress((ActionHandler)&FacilityAllocateEngineersState::btnOkClick, Options::keyCancel);

	_btnInfo->setText(tr("STR_INFO"));
	_btnInfo->onMouseClick((ActionHandler)&FacilityAllocateEngineersState::btnInfoClick);

	_txtTitle->setBig();
	_txtTitle->setText(tr(production->getFacility()->getRules()->getType()));
	_txtTitle->setWordWrap(true);
	_txtTitle->setVerticalAlign(ALIGN_MIDDLE);

	_txtName->setText(tr("STR_NAME_UC"));

	_txtAssignment->setText(tr("STR_ASSIGNMENT"));

	_txtStat->setText(tr("STR_CONSTRUCTION_UC"));
	_txtStat->setAlign(ALIGN_RIGHT);

	//_lstEngineers->setColumns(2, 106, 174);
	_lstEngineers->setColumns(3, 106, 158, 16);
	_lstEngineers->setAlign(ALIGN_RIGHT, 3);
	_lstEngineers->setSelectable(true);
	_lstEngineers->setBackground(_window);
	_lstEngineers->setMargin(8);
	_lstEngineers->onMouseClick((ActionHandler)&FacilityAllocateEngineersState::lstEngineersClick, 0);

}

/**
* 
*/
FacilityAllocateEngineersState::~FacilityAllocateEngineersState()
{
}

/**
* Returns to the previous screen.
* @param action Pointer to an action.
*/
void FacilityAllocateEngineersState::btnOkClick(Action*)
{

	for (auto s : _engineers)
	{
		s->setProductionProject(_production);
	}

	_production->setEfficiency(100);
	_production->setSellItems(false);
	_production->setAmountTotal(1);
	_production->setInfiniteAmount(false);
	_production->setAssignedEngineers(0);

	_game->popState();
	// extra popState to close PlaceFacilityState
	_game->popState();
}

void FacilityAllocateEngineersState::btnInfoClick(Action* action)
{
	Ufopaedia::openArticle(_game, _production->getFacility()->getRules()->getType());
}

/**
* Shows the soldiers in a list at specified offset/scroll.
*/
void FacilityAllocateEngineersState::initList(size_t scrl)
{
	int row = 0;
	_lstEngineers->clearList();
	_lstEngineers->setColumns(3, 106, 158, 16);

	auto recovery = _base->getSumRecoveryPerDay();
	bool isBusy = false, isFree = false;
	unsigned int it = 0;
	for (std::vector<Soldier*>::iterator i = _base->getSoldiers()->begin(); i != _base->getSoldiers()->end(); ++i)
	{
		if ((*i)->getRoleRank(ROLE_ENGINEER) > 0)
		{
			_engineerNumbers.push_back(it); // don't forget soldier's number on the base!
			std::string duty = (*i)->getCurrentDuty(_game->getLanguage(), recovery, isBusy, isFree, WORK);
			std::ostringstream ss;
			ss << (*i)->getStatsWithAllBonuses()->construction;
			_lstEngineers->addRow(3, (*i)->getName(true, 19).c_str(), duty.c_str(), ss.str().c_str());

			Uint8 color = _lstEngineers->getColor();
			bool matched = false;
			auto iter = std::find(std::begin(_engineers), std::end(_engineers), (*i));
			if (iter != std::end(_engineers))
			{
				matched = true;
			}

			if (matched)
			{
				color = _lstEngineers->getSecondaryColor();
				_lstEngineers->setCellText(row, 1, tr("STR_ASSIGNED_UC"));
			}
			else if (isBusy || !isFree)
			{
				color = _otherCraftColor;
			}

			_lstEngineers->setRowColor(row, color);
			row++;
		}
		it++;
	}
	if (scrl)
		_lstEngineers->scrollTo(scrl);
	_lstEngineers->draw();

}

/**
* Shows the soldiers in a list.
*/
void FacilityAllocateEngineersState::init()
{
	State::init();
	_base->prepareSoldierStatsWithBonuses(); // refresh stats for sorting
	initList(0);
}

/**
* Shows the selected soldier's info.
* @param action Pointer to an action.
*/
void FacilityAllocateEngineersState::lstEngineersClick(Action* action)
{
	double mx = action->getAbsoluteXMouse();
	if (mx >= _lstEngineers->getArrowsLeftEdge() && mx < _lstEngineers->getArrowsRightEdge())
	{
		return;
	}
	int row = _lstEngineers->getSelectedRow();
	if (action->getDetails()->button.button == SDL_BUTTON_LEFT)
	{
		Soldier* s = _base->getSoldiers()->at(_engineerNumbers.at(row));
		Uint8 color = _lstEngineers->getColor();
		bool isBusy = false, isFree = false, matched = false;
		std::string duty = s->getCurrentDuty(_game->getLanguage(), _base->getSumRecoveryPerDay(), isBusy, isFree, WORK);
		auto iter = std::find(std::begin(_engineers), std::end(_engineers), s);
		if (iter != std::end(_engineers))
		{
			matched = true;
		}
		if (matched)
		{
			_engineers.erase(s);
			if (s->getProductionProject())
			{
				if (s->getProductionProject() == _production)
				{
					s->setProductionProject(0);
					color = _lstEngineers->getColor();
					_lstEngineers->setCellText(row, 1, tr("STR_NONE_UC"));
				}
				else
				{
					color = _otherCraftColor;
					_lstEngineers->setCellText(row, 1, duty);
				}
			}
			else
			{
				_lstEngineers->setCellText(row, 1, duty);
				if (isBusy || !isFree || s->getCraft())
				{
					color = _otherCraftColor;
				}
			}
		}
		else if (s->hasFullHealth() && !isBusy)
		{
			_lstEngineers->setCellText(row, 1, tr("STR_ASSIGNED_UC"));
			color = _lstEngineers->getSecondaryColor();
			_engineers.insert(s);

		}

		_lstEngineers->setRowColor(row, color);
	}
	else if (action->getDetails()->button.button == SDL_BUTTON_RIGHT)
	{
		_game->pushState(new SoldierInfoState(_base, _engineerNumbers.at(row)));
	}
}

}

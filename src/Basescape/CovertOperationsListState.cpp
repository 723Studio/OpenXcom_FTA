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
#include "CovertOperationsListState.h"
#include <sstream>
#include "../Engine/Action.h"
#include "../Engine/Game.h"
#include "../Mod/Mod.h"
#include "../Engine/LocalizedText.h"
#include "../Engine/Options.h"
#include "../Interface/TextButton.h"
#include "../Interface/Window.h"
#include "../Interface/Text.h"
#include "../Interface/TextList.h"
#include "../Savegame/Base.h"
#include "../Savegame/CovertOperation.h"
#include "../Mod/RuleCovertOperation.h"
#include "../Basescape/CovertOperationStartState.h"
#include "../Basescape/CovertOperationInfoState.h"
#include <algorithm>
#include "../Engine/Logger.h"

#include "../Interface/TextEdit.h"
#include "../Engine/Game.h"
#include "../Savegame/SavedGame.h"
#include "../Savegame/DiplomacyFaction.h"
#include "../Mod/RuleDiplomacyFaction.h"


namespace OpenXcom
{


/**
 * Initializes all the elements in the CovertOperations list screen.
 * @param game Pointer to the core game.
 * @param base Pointer to the base to get info from.
 */
CovertOperationsListState::CovertOperationsListState(Base* base) : _base(base), _lstScroll(0)
{
	_screen = false;

	_window = new Window(this, 230, 140, 45, 30, POPUP_BOTH);
	_btnOK = new TextButton(214, 16, 53, 146);
	_txtTitle = new Text(214, 16, 53, 38);
	_lstOperations = new TextList(198, 88, 53, 54);

	// Set palette
	setInterface("covertOperationsMenu");

	add(_window, "window", "covertOperationsMenu");
	add(_btnOK, "button", "covertOperationsMenu");
	add(_txtTitle, "text", "covertOperationsMenu");
	add(_lstOperations, "list", "covertOperationsMenu");

	centerAllSurfaces();

	// Set up objects
	setWindowBackground(_window, "covertOperationsMenu");

	_btnOK->setText(tr("STR_OK"));
	_btnOK->onMouseClick((ActionHandler)&CovertOperationsListState::btnOKClick);
	_btnOK->onKeyboardPress((ActionHandler)&CovertOperationsListState::btnOKClick, Options::keyCancel);

	_txtTitle->setAlign(ALIGN_CENTER);
	_txtTitle->setText(tr("STR_NEW_COVERT_OPERATIONS"));

	_lstOperations->setColumns(1, 190);
	_lstOperations->setSelectable(true);
	_lstOperations->setBackground(_window);
	_lstOperations->setMargin(8);
	_lstOperations->setAlign(ALIGN_CENTER);
	_lstOperations->onMouseClick((ActionHandler)&CovertOperationsListState::onSelectOperation, SDL_BUTTON_LEFT);
}

/**
 * Initializes the screen (fills the list).
 */
void CovertOperationsListState::init()
{
	State::init();
	fillOperationList();
}

/**
 * Selects the CovertOperations to work on.
 * @param action Pointer to an action.
 */
void CovertOperationsListState::onSelectOperation(Action*)
{
	_game->pushState(new CovertOperationStartState(_base, _operationRules[_lstOperations->getSelectedRow()]));
}

/**
 * Returns to the previous screen.
 * @param action Pointer to an action.
 */
void CovertOperationsListState::btnOKClick(Action*)
{
	_game->popState();
}

/**
 * Fills the list with possible CovertOperations.
 */
void CovertOperationsListState::fillOperationList()
{
	_operationRules.clear();
	_lstOperations->clearList();
	const Mod *mod = _game->getMod();
	SavedGame *save = _game->getSavedGame();
	for (std::vector<std::string>::const_iterator i = mod->getCovertOperationList()->begin(); i != mod->getCovertOperationList()->end(); ++i)
	{
		RuleCovertOperation* rule = mod->getCovertOperation(*i);
		bool happy = true;
		// we dont want to do same operations twice
		const std::vector<std::string>& performedOperations = save->getPerformedCovertOperations();
		for (std::vector<std::string>::const_iterator i = performedOperations.begin(); i != performedOperations.end(); ++i)
		{
			if (rule->getName() == (*i)) happy = false;
		}
		//do we have required research...
		if (happy && !rule->getRequirements().empty())
		{
			if (!save->isResearched(mod->getResearch(rule->getRequirements())))	happy = false;
		}
		//or one that can close opportunity for this operation?
		if (happy && !rule->getCanceledBy().empty())
		{
			if (save->isResearched(mod->getResearch(rule->getCanceledBy()))) happy = false;
		}
		//lets see if our base fits requirements
		if (happy)
		{
			auto providedBaseFunc = _base->getProvidedBaseFunc({});
			auto ruleReqBaseFunc = rule->getRequiresBaseFunc();
			if (!((~providedBaseFunc & ruleReqBaseFunc).none())) happy = false;
		}
		//finally, reputation requirements
		if (happy && !rule->getRequiredReputationLvlList().empty())
		{
			for (std::map<std::string, int>::const_iterator i = rule->getRequiredReputationLvlList().begin(); i != rule->getRequiredReputationLvlList().end(); ++i)
			{
				for (std::vector<DiplomacyFaction*>::iterator j = save->getDiplomacyFactions().begin(); j != save->getDiplomacyFactions().end(); ++j)
				{
					if ((*j)->getRules()->getName() == (*i).first)
					{
						if ((*j)->getReputationLevel() < (*i).second || !(*j)->isDiscovered())
						{
							happy = false;
						}
					}
				}
			}
		}
		//all checks passed, we can show now operation to the player!
		if (happy)
		{
			_operationRules.push_back(rule);
			_lstOperations->addRow(1, tr((*i)).c_str());
		}
	}
	if (_lstScroll > 0)
	{
		_lstOperations->scrollTo(_lstScroll);
		_lstScroll = 0;
	}
}

}

/*
 * Copyright 2010-2015 OpenXcom Developers.
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
#include <sstream>
#include "TrainingState.h"
#include "AllocateTrainingState.h"
#include "../Engine/Game.h"
#include "../Engine/Language.h"
#include "../Mod/RuleInterface.h"
#include "../Interface/TextButton.h"
#include "../Interface/ToggleTextButton.h"
#include "../Interface/Window.h"
#include "../Interface/Text.h"
#include "../Savegame/SavedGame.h"
#include "../Savegame/Base.h"
#include "../Interface/TextList.h"
#include "../Savegame/Soldier.h"
#include "../Engine/Action.h"
#include "../Engine/Options.h"
#include "../Interface/ComboBox.h"
#include "../Mod/Mod.h"
#include "../Basescape/SoldierInfoState.h"
#include "../Basescape/SoldierSortUtil.h"
#include "../Basescape/SoldierInfoStateFtA.h"
#include <algorithm>
#include "../Engine/Unicode.h"

namespace OpenXcom
{

/**
 * Initializes all the elements in the Psi Training screen.
 * @param game Pointer to the core game.
 * @param base Pointer to the base to handle.
 */
AllocateTrainingState::AllocateTrainingState(Base *base) : _sel(0), _base(base), _origSoldierOrder(*_base->getSoldiers()), _doNotReset(false)
{

	_ftaUI = _game->getMod()->isFTAGame();

	// Create objects
	_window = new Window(this, 320, 200, 0, 0);
	_txtTitle = new Text(_ftaUI ? 168 : 300, 17, 16, 7);
	_txtRemaining = new Text(300, 10, 10, 24);
	_txtName = new Text(64, 10, 10, 40);
	_txtTraining = new Text(48, 20, 270, 32);
	_btnOk = new TextButton(148, 16, 164, 176);
	_lstSoldiers = new TextList(290, 112, 8, 52);
	_txtTu = new Text(18, 10, 120, 40);
	_txtStamina = new Text(18, 10, 138, 40);
	_txtHealth = new Text(18, 10, 156, 40);
	_txtReaction = new Text(18, 10, 174, 40);
	_txtFiring = new Text(18, 10, 192, 40);
	_txtThrowing = new Text(18, 10, 210, 40);
	_txtMelee = new Text(18, 10, 228, 40);
	_txtStrength = new Text(18, 10, 246, 40);
	_btnPlus = new ToggleTextButton(18, 16, 294, 8);
	if (_ftaUI)
	{
		_cbxSortBy = new ComboBox(this, 120, 16, 192, 8, false);
		_cbxScreenActions = new ComboBox(this, 148, 16, 8, 176, true);
	}
	else
	{
		_cbxSortBy = new ComboBox(this, 148, 16, 8, 176, true);
		_cbxScreenActions = new ComboBox(this, 17, 16, -16, -16, true); //would be hidden anyway
	}

	// Set palette
	setInterface("allocateMartial");

	add(_window, "window", "allocateMartial");
	add(_btnOk, "button", "allocateMartial");
	add(_txtName, "text", "allocateMartial");
	add(_txtTitle, "text", "allocateMartial");
	add(_txtRemaining, "text", "allocateMartial");
	add(_txtTraining, "text", "allocateMartial");
	add(_lstSoldiers, "list", "allocateMartial");
	add(_txtTu, "text", "allocateMartial");
	add(_txtStamina, "text", "allocateMartial");
	add(_txtHealth, "text", "allocateMartial");
	add(_txtFiring, "text", "allocateMartial");
	add(_txtThrowing, "text", "allocateMartial");
	add(_txtMelee, "text", "allocateMartial");
	add(_txtStrength, "text", "allocateMartial");
	add(_txtReaction, "text", "allocateMartial");
	add(_cbxSortBy, "button", "allocateMartial");
	add(_cbxScreenActions, "button", "allocateMartial");
	add(_btnPlus, "button", "allocateMartial");

	centerAllSurfaces();

	// Set up objects
	setWindowBackground(_window, "allocateMartial");

	_btnOk->setText(tr("STR_OK"));
	_btnOk->onMouseClick((ActionHandler)&AllocateTrainingState::btnOkClick);
	_btnOk->onKeyboardPress((ActionHandler)&AllocateTrainingState::btnOkClick, Options::keyCancel);
	_btnOk->onKeyboardPress((ActionHandler)& AllocateTrainingState::btnDeassignAllSoldiersClick, Options::keyRemoveSoldiersFromTraining);
	_btnOk->onKeyboardPress((ActionHandler)&AllocateTrainingState::btnAssignAllSoldiersClick, Options::keyAddSoldiersToTraining);

	_btnPlus->setText("+");
	_btnPlus->setPressed(false);
	if (_game->getMod()->getSoldierBonusList().empty() || _ftaUI)
	{
		_btnPlus->setVisible(false);
	}
	else
	{
		_btnPlus->onMouseClick((ActionHandler)&AllocateTrainingState::btnPlusClick, 0);
	}

	_txtTitle->setBig();
	_txtTitle->setAlign(_ftaUI ? ALIGN_LEFT : ALIGN_CENTER);
	_txtTitle->setText(tr("STR_PHYSICAL_TRAINING"));

	_space = base->getAvailableTraining() - base->getUsedTraining();
	_txtRemaining->setText(tr("STR_REMAINING_TRAINING_FACILITY_CAPACITY").arg(_space));

	_txtName->setText(tr("STR_NAME")); 
	_txtTu->setText(tr(OpenXcom::UnitStats::getStatString(&UnitStats::tu, UnitStats::STATSTR_ABBREV)));
	_txtStamina->setText(tr(OpenXcom::UnitStats::getStatString(&UnitStats::stamina, UnitStats::STATSTR_ABBREV)));
	_txtHealth->setText(tr(OpenXcom::UnitStats::getStatString(&UnitStats::health, UnitStats::STATSTR_ABBREV)));
	_txtReaction->setText(tr(OpenXcom::UnitStats::getStatString(&UnitStats::reactions, UnitStats::STATSTR_ABBREV)));
	_txtFiring->setText(tr(OpenXcom::UnitStats::getStatString(&UnitStats::firing, UnitStats::STATSTR_ABBREV)));
	_txtThrowing->setText(tr(OpenXcom::UnitStats::getStatString(&UnitStats::throwing, UnitStats::STATSTR_ABBREV)));
	_txtMelee->setText(tr(OpenXcom::UnitStats::getStatString(&UnitStats::melee, UnitStats::STATSTR_ABBREV)));
	_txtStrength->setText(tr(OpenXcom::UnitStats::getStatString(&UnitStats::strength, UnitStats::STATSTR_ABBREV)));
	_txtTraining->setText(tr("STR_IN_TRAINING"));

	// populate sort options
	std::vector<std::string> sortOptions;
	sortOptions.push_back(tr("STR_ORIGINAL_ORDER"));
	_sortFunctors.push_back(NULL);
	_sortFunctorsPlus.push_back(NULL);
	bool showPsiStats = _game->getSavedGame()->isResearched(_game->getMod()->getPsiRequirements());
	bool showMana = _game->getMod()->isManaFeatureEnabled() && _game->getSavedGame()->isManaUnlocked(_game->getMod());

#define PUSH_IN(strId, functor) \
	sortOptions.push_back(tr(strId)); \
	_sortFunctors.push_back(new SortFunctor(_game, functor)); \
	_sortFunctorsPlus.push_back(new SortFunctor(_game, functor));

	PUSH_IN("STR_ID", idStat);
	PUSH_IN("STR_NAME_UC", nameStat);
	PUSH_IN("STR_SOLDIER_TYPE", typeStat);
	PUSH_IN("STR_RANK", rankStat);
	PUSH_IN("STR_IDLE_DAYS", idleDaysStat);
	PUSH_IN("STR_MISSIONS2", missionsStat);
	PUSH_IN("STR_KILLS2", killsStat);
	PUSH_IN("STR_WOUND_RECOVERY2", woundRecoveryStat);
	if (showMana && !_game->getMod()->getReplenishManaAfterMission())
	{
		PUSH_IN("STR_MANA_MISSING", manaMissingStat);
	}

#undef PUSH_IN

#define PUSH_IN(strId, functor, functorPlus) \
	sortOptions.push_back(tr(strId)); \
	_sortFunctors.push_back(new SortFunctor(_game, functor)); \
	_sortFunctorsPlus.push_back(new SortFunctor(_game, functorPlus));

	PUSH_IN(OpenXcom::UnitStats::getStatString(&UnitStats::tu), tuStatBase, tuStatPlus);
	PUSH_IN(OpenXcom::UnitStats::getStatString(&UnitStats::stamina), staminaStatBase, staminaStatPlus);
	PUSH_IN(OpenXcom::UnitStats::getStatString(&UnitStats::health), healthStatBase, healthStatPlus);
	PUSH_IN(OpenXcom::UnitStats::getStatString(&UnitStats::bravery), braveryStatBase, braveryStatPlus);
	PUSH_IN(OpenXcom::UnitStats::getStatString(&UnitStats::reactions), reactionsStatBase, reactionsStatPlus);
	PUSH_IN(OpenXcom::UnitStats::getStatString(&UnitStats::firing), firingStatBase, firingStatPlus);
	PUSH_IN(OpenXcom::UnitStats::getStatString(&UnitStats::throwing), throwingStatBase, throwingStatPlus);
	PUSH_IN(OpenXcom::UnitStats::getStatString(&UnitStats::melee), meleeStatBase, meleeStatPlus);
	PUSH_IN(OpenXcom::UnitStats::getStatString(&UnitStats::strength), strengthStatBase, strengthStatPlus);
	if (showMana)
	{
		PUSH_IN(OpenXcom::UnitStats::getStatString(&UnitStats::mana), manaStatBase, manaStatPlus);
	}
	if (showPsiStats)
	{
		PUSH_IN(OpenXcom::UnitStats::getStatString(&UnitStats::psiStrength), psiStrengthStatBase, psiStrengthStatPlus);
		PUSH_IN(OpenXcom::UnitStats::getStatString(&UnitStats::strength), psiSkillStatBase, psiSkillStatPlus);
	}

#undef PUSH_IN

	_cbxSortBy->setOptions(sortOptions);
	_cbxSortBy->setSelected(0);
	_cbxSortBy->onChange((ActionHandler)&AllocateTrainingState::cbxSortByChange);
	_cbxSortBy->setText(tr("STR_SORT_BY"));

	if (_ftaUI)
	{
		_availableOptions.push_back("STR_ALL_ROLES");
		_availableOptions.push_back("STR_RECOMMENDED_ROLES");
	}
	else
	{
		_cbxScreenActions->setVisible(false);
	}

	_cbxScreenActions->setOptions(_availableOptions, true);
	_cbxScreenActions->setSelected(1);
	_cbxScreenActions->onChange((ActionHandler)&AllocateTrainingState::cbxScreenActionsChange);

	//_lstSoldiers->setArrowColumn(238, ARROW_VERTICAL);
	_lstSoldiers->setColumns(10, 110, 18, 18, 18, 18, 18, 18, 18, 24, 40);
	_lstSoldiers->setSelectable(true);
	_lstSoldiers->setBackground(_window);
	_lstSoldiers->setMargin(2);
	_lstSoldiers->onMouseClick((ActionHandler)&AllocateTrainingState::lstSoldiersClick);
	_lstSoldiers->onMouseClick((ActionHandler)&AllocateTrainingState::lstSoldiersClick, SDL_BUTTON_RIGHT);
}

/**
 * cleans up dynamic state
 */
AllocateTrainingState::~AllocateTrainingState()
{
	for (auto* sortFunctor : _sortFunctors)
	{
		delete sortFunctor;
	}
}

/**
 * Sorts the soldiers list by the selected criterion
 * @param action Pointer to an action.
 */
void AllocateTrainingState::cbxSortByChange(Action *action)
{
	size_t selIdx = _cbxSortBy->getSelected();
	if (selIdx == (size_t)-1)
	{
		return;
	}

	SortFunctor *compFunc = _btnPlus->getPressed() ? _sortFunctorsPlus[selIdx] : _sortFunctors[selIdx];
	if (compFunc)
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
		if (_game->isShiftPressed())
		{
			std::reverse(_base->getSoldiers()->begin(), _base->getSoldiers()->end());
		}
	}
	else
	{
		// restore original ordering, ignoring (of course) those
		// soldiers that have been sacked since this state started
		for (const auto* origSoldier : _origSoldierOrder)
		{
			auto soldierIt = std::find(_base->getSoldiers()->begin(), _base->getSoldiers()->end(), origSoldier);
			if (soldierIt != _base->getSoldiers()->end())
			{
				Soldier *s = *soldierIt;
				_base->getSoldiers()->erase(soldierIt);
				_base->getSoldiers()->insert(_base->getSoldiers()->end(), s);
			}
		}
	}

	size_t originalScrollPos = _lstSoldiers->getScroll();
	initList(originalScrollPos);
}

/**
 * Returns to the previous screen.
 * @param action Pointer to an action.
 */
void AllocateTrainingState::btnOkClick(Action *)
{
	// Note: statString updates not necessary
	_game->popState();
}

/**
 * Updates the soldier stats. Sorts the list if applicable.
 * @param action Pointer to an action.
 */
void AllocateTrainingState::btnPlusClick(Action *action)
{
	size_t selIdx = _cbxSortBy->getSelected();
	if (selIdx == (size_t)-1)
	{
		size_t originalScrollPos = _lstSoldiers->getScroll();
		initList(originalScrollPos);
	}
	else
	{
		cbxSortByChange(action);
	}
}

/**
 * The soldier info could maybe change (armor? something else?)
 * after going into other screens.
 */
void AllocateTrainingState::init()
{
	State::init();

	// coming back from SoldierInfoState
	if (_doNotReset)
	{
		_doNotReset = false;
		return;
	}

	_base->prepareSoldierStatsWithBonuses(); // refresh stats for sorting
	initList(0);
}

/**
 * Shows the soldiers in a list at specified offset/scroll.
 */
void AllocateTrainingState::initList(size_t scrl)
{
	int row = 0;
	_soldierNumbers.clear();
	_lstSoldiers->clearList();
	_filteredListOfSoldiers.clear();
	_soldierNumbers.clear();
	int i = 0;

	std::string selAction = "STR_RECOMMENDED_ROLES";
	if (!_availableOptions.empty())
	{
		selAction = _availableOptions.at(_cbxScreenActions->getSelected());
	}

	for (auto* soldier : *_base->getSoldiers())
	{
		if ((soldier->getRoleRank(ROLE_SOLDIER) > 0)
			|| (selAction == "STR_ALL_ROLES"
				&& !soldier->hasOnlyOneRole(ROLE_ROBOT)) //case we want to see everyone, except robots
			|| !_ftaUI)
		{
			_filteredListOfSoldiers.push_back(soldier);
			_soldierNumbers.push_back(i);
		}
		i++;
	}
	for (auto* soldier : _filteredListOfSoldiers)
	{
		std::ostringstream soldierName;
		if (soldier->getRoleRank(ROLE_SOLDIER) < 1)
		{
			soldierName << "*";
		}
		soldierName << soldier->getName(true);

		const UnitStats* stats = _btnPlus->getPressed() ? soldier->getStatsWithSoldierBonusesOnly() : soldier->getCurrentStats();

		std::ostringstream tu;
		tu << stats->tu;
		std::ostringstream stamina;
		stamina << stats->stamina;
		std::ostringstream health;
		health << stats->health;
		std::ostringstream reactions;
		reactions << stats->reactions;
		std::ostringstream firing;
		firing << stats->firing;
		std::ostringstream throwing;
		throwing << stats->throwing;
		std::ostringstream melee;
		melee << stats->melee;
		std::ostringstream strength;
		strength << stats->strength;

		bool isDone = soldier->isFullyTrained();
		bool isWounded = soldier->isWounded();
		bool isTraining = soldier->isInTraining();
		bool isOut = soldier->getCovertOperation() != 0;
		bool isBusy = soldier->getResearchProject() != 0 || soldier->getProductionProject() != 0 || soldier->getIntelProject() != 0 || soldier->getActivePrisoner() != 0;
		bool isTransforming = soldier->hasPendingTransformation();
		bool isQueued = !isTraining && soldier->getReturnToTrainingWhenHealed();

		std::string status;
		if (isDone)
			status = tr("STR_NO_DONE");
		else if (isQueued)
			status = tr("STR_NO_QUEUED");
		else if (isWounded)
			status = tr("STR_NO_WOUNDED");
		else if (isTransforming)
			status = tr("STR_NO_TRANSFORMATION");
		else if (isOut)
			status = tr("STR_NO_OUT");
		else if (isBusy)
			status = tr("STR_BUSY");
		else if (isTraining)
			status = tr("STR_YES");
		else
			status = tr("STR_NO");

		_lstSoldiers->addRow(10,
			soldierName.str().c_str(),
			tu.str().c_str(),
			stamina.str().c_str(),
			health.str().c_str(),
			reactions.str().c_str(),
			firing.str().c_str(),
			throwing.str().c_str(),
			melee.str().c_str(),
			strength.str().c_str(),
			status.c_str());
		_lstSoldiers->setRowColor(row, isTraining ? _lstSoldiers->getSecondaryColor() : _lstSoldiers->getColor());
		row++;
	}
	if (scrl)
		_lstSoldiers->scrollTo(scrl);
	_lstSoldiers->draw();
}


/**
 * Assigns / removes a soldier from Psi Training.
 * @param action Pointer to an action.
 */
void AllocateTrainingState::lstSoldiersClick(Action *action)
{
	double mx = action->getAbsoluteXMouse();
	if (mx >= _lstSoldiers->getArrowsLeftEdge() && mx < _lstSoldiers->getArrowsRightEdge())
	{
		return;
	}
	_sel = _lstSoldiers->getSelectedRow();
	Soldier* soldier = _filteredListOfSoldiers.at(_sel);

	if (action->getDetails()->button.button == SDL_BUTTON_LEFT)
	{
		// can't put fully trained soldiers back into training
		if (soldier->isFullyTrained()) return;

		// can't put soldier that is on covert operation
		if (soldier->getCovertOperation() != 0) return;

		// can't put soldier that has pending transformation
		if (soldier->hasPendingTransformation()) return;

		// wounded soldiers can be queued/dequeued
		if (soldier->isWounded())
		{
			if (soldier->getReturnToTrainingWhenHealed())
			{
				_lstSoldiers->setCellText(_sel, 9, tr("STR_NO_WOUNDED").c_str());
				soldier->setReturnToTrainingWhenHealed(false);
			}
			else
			{
				_lstSoldiers->setCellText(_sel, 9, tr("STR_NO_QUEUED").c_str());
				soldier->setReturnToTrainingWhenHealed(true);
			}
			return;
		}

		// healthy soldiers can be assigned/deassigned
		if (!soldier->isInTraining())
		{
			if (_base->getUsedTraining() < _base->getAvailableTraining())
			{
				_lstSoldiers->setCellText(_sel, 9, tr("STR_YES").c_str());
				_lstSoldiers->setRowColor(_sel, _lstSoldiers->getSecondaryColor());
				_space--;
				_txtRemaining->setText(tr("STR_REMAINING_TRAINING_FACILITY_CAPACITY").arg(_space));
				soldier->setTraining(true);
				soldier->setActivePrisoner(0);
				soldier->setProductionProject(0);
				soldier->setResearchProject(0);
				soldier->setReturnToTrainingWhenHealed(false);
			}
		}
		else
		{
			_lstSoldiers->setCellText(_sel, 9, tr("STR_NO").c_str());
			_lstSoldiers->setRowColor(_sel, _lstSoldiers->getColor());
			_space++;
			_txtRemaining->setText(tr("STR_REMAINING_TRAINING_FACILITY_CAPACITY").arg(_space));
			soldier->setTraining(false);
			soldier->setReturnToTrainingWhenHealed(false);
		}
	}
	else if (action->getDetails()->button.button == SDL_BUTTON_RIGHT)
	{
		_doNotReset = true;
		_game->pushState(new SoldierInfoStateFtA(_base, soldier));
	}
}

void AllocateTrainingState::cbxScreenActionsChange(Action* action)
{
	_cbxSortBy->setSelected(0);
	initList(0);
}

/**
 * Removes all soldiers from Martial Training.
 * @param action Pointer to an action.
 */
void AllocateTrainingState::btnDeassignAllSoldiersClick(Action* action)
{
	int row = 0;
	for (auto* soldier : *_base->getSoldiers())
	{
		if (soldier->isInTraining())
		{
			soldier->setTraining(false);
			soldier->setReturnToTrainingWhenHealed(false);

			std::string status;
			if (soldier->isFullyTrained())
				status = tr("STR_NO_DONE");
			else if (soldier->isWounded())
				status = tr("STR_NO_WOUNDED");
			else
				status = tr("STR_NO");

			_lstSoldiers->setCellText(row, 9, tr(status).c_str());
			_lstSoldiers->setRowColor(row, _lstSoldiers->getColor());
		}

		row++;
	}
	_space = _base->getAvailableTraining() - _base->getUsedTraining();
	_txtRemaining->setText(tr("STR_REMAINING_TRAINING_FACILITY_CAPACITY").arg(_space));
}

/**
 * Adds all soldiers to Martial Training.
 * @param action Pointer to an action.
 */
void AllocateTrainingState::btnAssignAllSoldiersClick(Action* action)
{
	int row = 0;
	for (auto* soldier : *_base->getSoldiers())
	{
		if (soldier->isFullyTrained() || soldier->getCovertOperation() != 0 || soldier->hasPendingTransformation())
		{
			// can't put fully trained soldiers back into training or busy
		}
		if (soldier->getResearchProject() != 0 || soldier->getProductionProject() != 0 || soldier->getIntelProject() != 0 || soldier->getActivePrisoner() != 0)
		{
			// don't assign busy soldier
		}
		else if (soldier->isWounded())
		{
			// wounded soldiers can be queued
			if (!soldier->getReturnToTrainingWhenHealed())
			{
				_lstSoldiers->setCellText(row, 9, tr("STR_NO_QUEUED").c_str());
				soldier->setReturnToTrainingWhenHealed(true);
			}
		}
		else if (_space > 0 && !soldier->isInTraining())
		{
			// healthy soldiers can be assigned
			_lstSoldiers->setCellText(row, 9, tr("STR_YES").c_str());
			_lstSoldiers->setRowColor(row, _lstSoldiers->getSecondaryColor());
			_space--;
			soldier->setTraining(true);
			soldier->setActivePrisoner(0);
			soldier->setProductionProject(0);
			soldier->setResearchProject(0);
			soldier->setReturnToTrainingWhenHealed(false);
		}
		row++;
	}
	_txtRemaining->setText(tr("STR_REMAINING_TRAINING_FACILITY_CAPACITY").arg(_space));
}

}

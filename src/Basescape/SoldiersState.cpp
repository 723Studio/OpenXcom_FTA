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
#include "SoldiersState.h"
#include "../Engine/Screen.h"
#include "../Engine/Game.h"
#include "../Mod/Mod.h"
#include "../Mod/RuleSoldierTransformation.h"
#include "../Mod/RuleSoldier.h"
#include "../Engine/LocalizedText.h"
#include "../Engine/Options.h"
#include "../Interface/ComboBox.h"
#include "../Engine/Action.h"
#include "../Geoscape/AllocatePsiTrainingState.h"
#include "../Geoscape/AllocateTrainingState.h"
#include "../Interface/TextButton.h"
#include "../Interface/Window.h"
#include "../Interface/Text.h"
#include "../Interface/TextList.h"
#include "../Savegame/Base.h"
#include "../Savegame/BattleUnit.h"
#include "../Savegame/Soldier.h"
#include "../Savegame/SavedGame.h"
#include "SoldierInfoStateFtA.h"
#include "SoldierMemorialState.h"
#include "SoldierTransformationState.h"
#include "SoldierTransformationListState.h"
#include "../Savegame/SavedBattleGame.h"
#include <algorithm>
#include "../Engine/Unicode.h"
#include "../Battlescape/BattlescapeGenerator.h"
#include "../Battlescape/InventoryState.h"

namespace OpenXcom
{

/**
 * Initializes all the elements in the Soldiers screen.
 * @param game Pointer to the core game.
 * @param base Pointer to the base to get info from.
 */
SoldiersState::SoldiersState(Base *base) : _base(base), _origSoldierOrder(*_base->getSoldiers()), _dynGetter(NULL), _mainOffset(0)
{
	bool isPsiBtnVisible = Options::anytimePsiTraining && _base->getAvailablePsiLabs() > 0;
	bool isTrnBtnVisible = _base->getAvailableTraining() > 0;
	std::vector<RuleSoldierTransformation* > availableTransformations;
	_game->getSavedGame()->getAvailableTransformations(availableTransformations, _game->getMod(), _base);
	bool isTransformationAvailable = availableTransformations.size() > 0;

	// Create objects
	_window = new Window(this, 320, 200, 0, 0);

	_btnOk = new TextButton(148, 16, 164, 176);
	_cbxScreenActions = new ComboBox(this, 148, 16, 8, 176, true);
	_txtTitle = new Text(168, 17, 16, 8);
	_cbxSortBy = new ComboBox(this, 120, 16, 192, 8, false);
	_txtName = new Text(114, 9, 16, 32);
	_txtRank = new Text(102, 9, 122, 32);
	_txtCraft = new Text(82, 9, 220, 32);
	_lstSoldiers = new TextList(288, 128, 8, 40);

	// Set palette
	setInterface("soldierList");

	add(_window, "window", "soldierList");
	add(_btnOk, "button", "soldierList");
	add(_txtTitle, "text1", "soldierList");
	add(_txtName, "text2", "soldierList");
	add(_txtRank, "text2", "soldierList");
	add(_txtCraft, "text2", "soldierList");
	add(_lstSoldiers, "list", "soldierList");
	add(_cbxSortBy, "button", "soldierList");
	add(_cbxScreenActions, "button", "soldierList");

	centerAllSurfaces();

	// Set up objects
	setWindowBackground(_window, "soldierList");

	_btnOk->setText(tr("STR_OK"));
	_btnOk->onMouseClick((ActionHandler)&SoldiersState::btnOkClick);
	_btnOk->onKeyboardPress((ActionHandler)&SoldiersState::btnOkClick, Options::keyCancel);
	_btnOk->onKeyboardPress((ActionHandler)&SoldiersState::btnInventoryClick, Options::keyBattleInventory);
	_btnOk->onKeyboardPress((ActionHandler)&SoldiersState::btnTransformationsOverviewClick, SDLK_t);

	_availableOptions.clear();

	_availableOptions.push_back("STR_PERSONNEL_INFO");
	_availableOptions.push_back("STR_SOLDIER_INFO");
	_availableOptions.push_back("STR_PILOT_INFO");
	_availableOptions.push_back("STR_AGENT_INFO");
	_availableOptions.push_back("STR_SCIENTIST_INFO");
	_availableOptions.push_back("STR_ENGINEER_INFO");
	_availableOptions.push_back("STR_ROBOT_INFO");

	_availableOptions.push_back("STR_MEMORIAL");
	_availableOptions.push_back("STR_INVENTORY");

	if (isPsiBtnVisible)
		_availableOptions.push_back("STR_PSI_TRAINING");

	if (isTrnBtnVisible)
		_availableOptions.push_back("STR_TRAINING");

	if (isTransformationAvailable)
	{
		_mainOffset = _availableOptions.size();
		_availableOptions.push_back("STR_TRANSFORMATIONS_OVERVIEW");
	}

	bool refreshDeadSoldierStats = false;
	for (const auto* transformationRule : availableTransformations)
	{
		_availableOptions.push_back(transformationRule->getName());
		if (transformationRule->isAllowingDeadSoldiers())
		{
			refreshDeadSoldierStats = true;
		}
	}
	if (refreshDeadSoldierStats)
	{
		for (auto* deadMan : *_game->getSavedGame()->getDeadSoldiers())
		{
			deadMan->prepareStatsWithBonuses(_game->getMod()); // refresh stats for sorting
		}
	}

	_cbxScreenActions->setOptions(_availableOptions, true);
	_cbxScreenActions->setSelected(0);
	_cbxScreenActions->onChange((ActionHandler)&SoldiersState::cbxScreenActionsChange);

	_txtTitle->setBig();
	_txtTitle->setAlign(ALIGN_LEFT);
	_txtTitle->setText(tr("STR_SOLDIER_LIST"));

	_txtName->setText(tr("STR_NAME_UC"));

	_txtRank->setText(tr("STR_RANK"));

	_txtCraft->setText(tr("STR_ASSIGNMENT"));

	// populate sort options
	std::vector<std::string> sortOptions;
	sortOptions.push_back(tr("STR_ORIGINAL_ORDER"));
	_sortFunctors.push_back(NULL);
	bool showPsiStats = _game->getSavedGame()->isResearched(_game->getMod()->getPsiRequirements());
	bool showMana = _game->getSavedGame()->isManaUnlocked(_game->getMod()) && _game->getMod()->isManaFeatureEnabled();

#define PUSH_IN(strId, functor) \
	sortOptions.push_back(tr(strId)); \
	_sortFunctors.push_back(new SortFunctor(_game, functor));

	PUSH_IN("STR_ID", idStat);
	PUSH_IN("STR_NAME_UC", nameStat);
	PUSH_IN("STR_CRAFT", craftIdStat);
	//PUSH_IN("STR_SOLDIER_TYPE", typeStat);
	PUSH_IN("STR_ROLE_UC", roleStat);
	PUSH_IN("STR_RANK", roleRankStat);
	// #FINNIKTODO add rank per roles
	PUSH_IN("STR_IDLE_DAYS", idleDaysStat);
	PUSH_IN("STR_MISSIONS2", missionsStat);
	PUSH_IN("STR_KILLS2", killsStat);
	PUSH_IN("STR_WOUND_RECOVERY2", woundRecoveryStat);
	if (showMana && !_game->getMod()->getReplenishManaAfterMission())
	{
		PUSH_IN("STR_MANA_MISSING", manaMissingStat);
	}
	PUSH_IN(OpenXcom::UnitStats::getStatString(&UnitStats::tu), tuStat);
	PUSH_IN(OpenXcom::UnitStats::getStatString(&UnitStats::stamina), staminaStat);
	PUSH_IN(OpenXcom::UnitStats::getStatString(&UnitStats::health), healthStat);
	PUSH_IN(OpenXcom::UnitStats::getStatString(&UnitStats::bravery), braveryStat);
	PUSH_IN(OpenXcom::UnitStats::getStatString(&UnitStats::reactions), reactionsStat);
	PUSH_IN(OpenXcom::UnitStats::getStatString(&UnitStats::firing), firingStat);
	PUSH_IN(OpenXcom::UnitStats::getStatString(&UnitStats::throwing), throwingStat);
	PUSH_IN(OpenXcom::UnitStats::getStatString(&UnitStats::melee), meleeStat);
	PUSH_IN(OpenXcom::UnitStats::getStatString(&UnitStats::strength), strengthStat);
	if (showMana)
	{
		PUSH_IN(OpenXcom::UnitStats::getStatString(&UnitStats::mana), manaStat);
	}
	if (showPsiStats)
	{
		PUSH_IN(OpenXcom::UnitStats::getStatString(&UnitStats::psiStrength), psiStrengthStat);
		PUSH_IN(OpenXcom::UnitStats::getStatString(&UnitStats::psiSkill), psiSkillStat);
	}
	//pilot section
	PUSH_IN(OpenXcom::UnitStats::getStatString(&UnitStats::maneuvering), maneuveringStat);
	PUSH_IN(OpenXcom::UnitStats::getStatString(&UnitStats::missiles), missilesStat);
	PUSH_IN(OpenXcom::UnitStats::getStatString(&UnitStats::dogfight), dogfightStat);
	PUSH_IN(OpenXcom::UnitStats::getStatString(&UnitStats::tracking), trackingStat);
	PUSH_IN(OpenXcom::UnitStats::getStatString(&UnitStats::cooperation), cooperationStat);
	if (_game->getSavedGame()->isResearched(_game->getMod()->getBeamOperationsUnlockResearch()))
	{
		PUSH_IN(OpenXcom::UnitStats::getStatString(&UnitStats::beams), beamsStat);
	}
	if (_game->getSavedGame()->isResearched(_game->getMod()->getCraftSynapseUnlockResearch()))
	{
		PUSH_IN(OpenXcom::UnitStats::getStatString(&UnitStats::synaptic), synapticStat);
	}
	if (_game->getSavedGame()->isResearched(_game->getMod()->getGravControlUnlockResearch()))
	{
		PUSH_IN(OpenXcom::UnitStats::getStatString(&UnitStats::gravity), gravityStat);
	}

	// scientist section
	PUSH_IN(OpenXcom::UnitStats::getStatString(&UnitStats::physics), physicsStat);
	PUSH_IN(OpenXcom::UnitStats::getStatString(&UnitStats::chemistry), chemistryStat);
	PUSH_IN(OpenXcom::UnitStats::getStatString(&UnitStats::biology), biologyStat);
	PUSH_IN(OpenXcom::UnitStats::getStatString(&UnitStats::insight), insightStat);
	PUSH_IN(OpenXcom::UnitStats::getStatString(&UnitStats::data), dataStat);
	PUSH_IN(OpenXcom::UnitStats::getStatString(&UnitStats::computers), computersStat);
	PUSH_IN(OpenXcom::UnitStats::getStatString(&UnitStats::tactics), tacticsStat);
	PUSH_IN(OpenXcom::UnitStats::getStatString(&UnitStats::materials), materialsStat);
	PUSH_IN(OpenXcom::UnitStats::getStatString(&UnitStats::designing), designingStat);
	if (_game->getSavedGame()->isResearched(_game->getMod()->getAlienTechUnlockResearch()))
	{
		PUSH_IN(OpenXcom::UnitStats::getStatString(&UnitStats::alienTech), alienTechStat);
	}
	if (showPsiStats)
	{
		PUSH_IN(OpenXcom::UnitStats::getStatString(&UnitStats::psionics), psionicsStat);
	}
	if (_game->getSavedGame()->isResearched(_game->getMod()->getXenolinguisticsUnlockResearch()))
	{
		PUSH_IN(OpenXcom::UnitStats::getStatString(&UnitStats::xenolinguistics), xenolinguisticsStat);
	}
	// engineer section
	PUSH_IN(OpenXcom::UnitStats::getStatString(&UnitStats::weaponry), weaponryStat);
	PUSH_IN(OpenXcom::UnitStats::getStatString(&UnitStats::explosives), explosivesStat);
	PUSH_IN(OpenXcom::UnitStats::getStatString(&UnitStats::microelectronics), microelectronicsStat);
	PUSH_IN(OpenXcom::UnitStats::getStatString(&UnitStats::metallurgy), metallurgyStat);
	PUSH_IN(OpenXcom::UnitStats::getStatString(&UnitStats::processing), processingStat);
	PUSH_IN(OpenXcom::UnitStats::getStatString(&UnitStats::efficiency), efficiencyStat);
	PUSH_IN(OpenXcom::UnitStats::getStatString(&UnitStats::diligence), diligenceStat);
	PUSH_IN(OpenXcom::UnitStats::getStatString(&UnitStats::hacking), hackingStat);
	PUSH_IN(OpenXcom::UnitStats::getStatString(&UnitStats::robotics), roboticsStat);
	if (_game->getSavedGame()->isResearched(_game->getMod()->getAlienTechUnlockResearch()))
	{
		PUSH_IN(OpenXcom::UnitStats::getStatString(&UnitStats::reverseEngineering), reverseEngineeringStat);
	}
	// agent section
	PUSH_IN(OpenXcom::UnitStats::getStatString(&UnitStats::stealth), stealthStat);
	PUSH_IN(OpenXcom::UnitStats::getStatString(&UnitStats::perception), perceptionStat);
	PUSH_IN(OpenXcom::UnitStats::getStatString(&UnitStats::charisma), charismaStat);
	PUSH_IN(OpenXcom::UnitStats::getStatString(&UnitStats::investigation), investigationStat);
	PUSH_IN(OpenXcom::UnitStats::getStatString(&UnitStats::deception), deceptionStat);
	PUSH_IN(OpenXcom::UnitStats::getStatString(&UnitStats::interrogation), interrogationStat);

#undef PUSH_IN

	_cbxSortBy->setOptions(sortOptions);
	_cbxSortBy->setSelected(0);
	_cbxSortBy->onChange((ActionHandler)&SoldiersState::cbxSortByChange);
	_cbxSortBy->setText(tr("STR_SORT_BY"));

	_lstSoldiers->setColumns(3, 106, 98, 76);
	_lstSoldiers->setAlign(ALIGN_RIGHT, 3);
	_lstSoldiers->setSelectable(true);
	_lstSoldiers->setBackground(_window);
	_lstSoldiers->setMargin(8);
	_lstSoldiers->onMouseClick((ActionHandler)&SoldiersState::lstSoldiersClick, SDL_BUTTON_LEFT);
	_lstSoldiers->onMouseClick((ActionHandler)&SoldiersState::lstSoldiersClick, SDL_BUTTON_RIGHT);
}

/**
 * cleans up dynamic state
 */
SoldiersState::~SoldiersState()
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
void SoldiersState::cbxSortByChange(Action *action)
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
		if (selIdx != 2 && selIdx != 3)
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
			else if (selIdx == 3)
			{
				std::stable_sort(_base->getSoldiers()->begin(), _base->getSoldiers()->end(),
					[](const Soldier* a, const Soldier* b)
					{
						if (a->getCraft())
						{
							if (b->getCraft())
							{
								if (a->getCraft()->getRules() == b->getCraft()->getRules())
								{
									return a->getCraft()->getId() < b->getCraft()->getId();
								}
								else
								{
									return a->getCraft()->getRules() < b->getCraft()->getRules();
								}
							}
							else
							{
								return true; // a < b
							}
						}
						return false; // b > a
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
 * Updates the soldiers list
 * after going to other screens.
 */
void SoldiersState::init()
{
	State::init();

	// resets the savegame when coming back from the inventory
	_game->getSavedGame()->setBattleGame(0);
	_base->setInBattlescape(false);

	_base->prepareSoldierStatsWithBonuses(); // refresh stats for sorting
	initList(_lstSoldiers->getScroll());
}

/**
 * Shows the soldiers in a list at specified offset/scroll.
 */
void SoldiersState::initList(size_t scrl)
{
	_lstSoldiers->clearList();
	_filteredListOfSoldiers.clear();
	_soldierNumbers.clear();
	int i = 0;

	std::string selAction = "STR_PERSONNEL_INFO";

	if (!_availableOptions.empty())
	{
		selAction = _availableOptions.at(_cbxScreenActions->getSelected());
	}

	int offset = 20;
	if (selAction == "STR_PERSONNEL_INFO" ||
		selAction == "STR_SOLDIER_INFO" ||
		selAction == "STR_PILOT_INFO" ||
		selAction == "STR_AGENT_INFO" ||
		selAction == "STR_SCIENTIST_INFO" ||
		selAction == "STR_ENGINEER_INFO" ||
		selAction == "STR_ROBOT_INFO")
	{
		for (auto& soldier : *_base->getSoldiers())
		{
			if (selAction == "STR_PERSONNEL_INFO" ||
				(soldier->getRoleRank(ROLE_SOLDIER) > 0 && selAction == "STR_SOLDIER_INFO") ||
				(soldier->getRoleRank(ROLE_PILOT) > 0 && selAction == "STR_PILOT_INFO") ||
				(soldier->getRoleRank(ROLE_AGENT) > 0 && selAction == "STR_AGENT_INFO") ||
				(soldier->getRoleRank(ROLE_SCIENTIST) > 0 && selAction == "STR_SCIENTIST_INFO") ||
				(soldier->getRoleRank(ROLE_ENGINEER) > 0 && selAction == "STR_ENGINEER_INFO") ||
				(soldier->getRoleRank(ROLE_ROBOT) > 0 && selAction == "STR_ROBOT_INFO"))
			{
				_filteredListOfSoldiers.push_back(soldier);
				_soldierNumbers.push_back(i); // don't forget soldier's number on the base!
			}
			i++;
		}
	}
	else
	{
		_lstSoldiers->setArrowColumn(-1, ARROW_VERTICAL);


		// filtered list of soldiers eligible for transformation
		RuleSoldierTransformation *transformationRule = _game->getMod()->getSoldierTransformation(selAction);
		if (transformationRule)
		{
			for (auto* soldier : *_base->getSoldiers())
			{
				if ((soldier->getCraft() && soldier->getCraft()->getStatus() == "STR_OUT") || soldier->getCovertOperation() != 0)
				{
					// soldiers outside of the base are not eligible
					continue;
				}
				if (soldier->isEligibleForTransformation(transformationRule))
				{
					_filteredListOfSoldiers.push_back(soldier);
					_soldierNumbers.push_back(i); // don't forget soldier's number on the base!
				}
				i++;
			}
			for (auto* deadMan : *_game->getSavedGame()->getDeadSoldiers())
			{
				if (deadMan->isEligibleForTransformation(transformationRule))
				{
					_filteredListOfSoldiers.push_back(deadMan);
				}
			}
		}
	}

	if (_dynGetter != NULL)
	{
		_lstSoldiers->setColumns(4, 106, 98 - offset, 60 + offset, 16);
	}
	else
	{
		_lstSoldiers->setColumns(3, 106, 98 - offset, 76 + offset);
	}
	_txtCraft->setX(_txtRank->getX() + 98 - offset);

	BaseSumDailyRecovery recovery = _base->getSumRecoveryPerDay();
	bool isBusy = false, isFree = false;
	unsigned int row = 0;
	for (const auto* soldier : _filteredListOfSoldiers)
	{
		std::string duty = soldier->getCurrentDuty(_game->getLanguage(), recovery, isBusy, isFree);
		if (_dynGetter != NULL)
		{
			// call corresponding getter
			int dynStat = (*_dynGetter)(_game, soldier);
			std::ostringstream ss;
			ss << dynStat;
			_lstSoldiers->addRow(4, soldier->getName(true).c_str(), tr(soldier->getRankString(true)).c_str(), duty.c_str(), ss.str().c_str());
		}
		else
		{
			_lstSoldiers->addRow(3, soldier->getName(true).c_str(), tr(soldier->getRankString(true)).c_str(), duty.c_str());
		}
		Uint8 color = _lstSoldiers->getColor();
		if (isBusy || !isFree || soldier->getCraft())
		{
			color = _lstSoldiers->getSecondaryColor();
		}

		_lstSoldiers->setRowColor(row, color);
		row++;
	}
	if (scrl)
		_lstSoldiers->scrollTo(scrl);
	_lstSoldiers->draw();
}

/**
 * Returns to the previous screen.
 * @param action Pointer to an action.
 */
void SoldiersState::btnOkClick(Action *)
{
	_game->popState();
}

/**
 * Opens the Psionic Training screen.
 * @param action Pointer to an action.
 */
void SoldiersState::btnPsiTrainingClick(Action *)
{
	_game->pushState(new AllocatePsiTrainingState(_base));
}

/**
 * Opens the Martial Training screen.
 * @param action Pointer to an action.
 */
void SoldiersState::btnTrainingClick(Action *)
{
	_game->pushState(new AllocateTrainingState(_base));
}

/**
 * Opens the Memorial screen.
 * @param action Pointer to an action.
 */
void SoldiersState::btnMemorialClick(Action *)
{
	_game->pushState(new SoldierMemorialState);
}

/**
 * Opens the Transformations Overview screen.
 * @param action Pointer to an action.
 */
void SoldiersState::btnTransformationsOverviewClick(Action *)
{
	if (_mainOffset > 0)
	{
		// needed in SoldierTransformationListState::lstTransformationsClick()
		_cbxScreenActions->setSelected(_mainOffset);

		_game->pushState(new SoldierTransformationListState(_base, _cbxScreenActions));
	}
}

/**
 * Opens the selected screen from the combo box
 * @param action Pointer to an action
 */
void SoldiersState::cbxScreenActionsChange(Action *action)
{
	const std::string selAction = _availableOptions.at(_cbxScreenActions->getSelected());

	if (selAction == "STR_MEMORIAL")
	{
		_cbxScreenActions->setSelected(0);
		_game->pushState(new SoldierMemorialState);
	}
	else if (selAction == "STR_INVENTORY")
	{
		_cbxScreenActions->setSelected(0);
		btnInventoryClick(nullptr); //#FINNIKCHECK
	}
	else if (selAction == "STR_PSI_TRAINING")
	{
		_cbxScreenActions->setSelected(0);
		_game->pushState(new AllocatePsiTrainingState(_base));
	}
	else if (selAction == "STR_TRAINING")
	{
		_cbxScreenActions->setSelected(0);
		_game->pushState(new AllocateTrainingState(_base));
	}
	else if (selAction == "STR_TRANSFORMATIONS_OVERVIEW")
	{
		_game->pushState(new SoldierTransformationListState(_base, _cbxScreenActions));
	}
	else
	{
		_cbxSortBy->setSelected(0);
		initList(0);
	}
}

/**
* Displays the inventory screen for the soldiers inside the base.
* @param action Pointer to an action.
*/
void SoldiersState::btnInventoryClick(Action *)
{
	if (_base->getAvailableSoldiers(true, true) > 0)
	{
		SavedBattleGame *bgame = new SavedBattleGame(_game->getMod(), _game->getLanguage());
		_game->getSavedGame()->setBattleGame(bgame);
		bgame->setMissionType("STR_BASE_DEFENSE");

		if (_game->isCtrlPressed() && _game->isAltPressed())
		{
			_game->getSavedGame()->setDisableSoldierEquipment(true);
		}
		BattlescapeGenerator bgen = BattlescapeGenerator(_game);
		bgen.setBase(_base);
		bgen.runInventory(0);

		// pre-select the soldier under the mouse cursor (if possible)
		if (_availableOptions.empty() || _cbxScreenActions->getSelected() == 0)
		{
			size_t idx = _lstSoldiers->getSelectedRow();
			if (idx < _base->getSoldiers()->size())
			{
				int soldierId = _base->getSoldiers()->at(idx)->getId();
				for (auto* unit : *bgame->getUnits())
				{
					if (unit->getId() == soldierId)
					{
						bgame->setSelectedUnit(unit);
						break;
					}
				}
			}
		}

		_game->getScreen()->clear();
		_game->pushState(new InventoryState(false, 0, _base, true));
	}
}

/**
 * Shows the selected soldier's info.
 * @param action Pointer to an action.
 */
void SoldiersState::lstSoldiersClick(Action *action)
{
	double mx = action->getAbsoluteXMouse();
	if (mx >= _lstSoldiers->getArrowsLeftEdge() && mx < _lstSoldiers->getArrowsRightEdge())
	{
		return;
	}

	std::string selAction = "STR_SOLDIER_INFO";
	if (!_availableOptions.empty())
	{
		selAction = _availableOptions.at(_cbxScreenActions->getSelected());
	}
	if ((selAction == "STR_PERSONNEL_INFO" ||
		selAction == "STR_SOLDIER_INFO" ||
		selAction == "STR_PILOT_INFO" ||
		selAction == "STR_AGENT_INFO" ||
		selAction == "STR_SCIENTIST_INFO" ||
		selAction == "STR_ENGINEER_INFO")
		|| action->getDetails()->button.button == SDL_BUTTON_RIGHT)
	{
		if (action->getDetails()->button.button == SDL_BUTTON_RIGHT)
		{
			btnInventoryClick(nullptr);
		}
		else
		{
			_game->pushState(new SoldierInfoStateFtA(_base, _soldierNumbers.at(_lstSoldiers->getSelectedRow())));
		}
	}
	else if (action->getDetails()->button.button == SDL_BUTTON_RIGHT)
	{
		size_t idx = _lstSoldiers->getSelectedRow();
		if (idx < _filteredListOfSoldiers.size())
		{
			_game->pushState(new SoldierInfoStateFtA(_base, _soldierNumbers.at(_lstSoldiers->getSelectedRow())));
		}
	}
	else
	{
		RuleSoldierTransformation *transformationRule = _game->getMod()->getSoldierTransformation(selAction);
		if (transformationRule)
		{
			_game->pushState(new SoldierTransformationState(
				transformationRule,
				_base,
				_filteredListOfSoldiers.at(_lstSoldiers->getSelectedRow()),
				&_filteredListOfSoldiers));
		}
	}
}

}

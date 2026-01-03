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
#include "CraftSoldiersState.h"
#include <algorithm>
#include <functional>
#include <climits>
#include "../Engine/Action.h"
#include "../Engine/Game.h"
#include "../Mod/Mod.h"
#include "../Engine/LocalizedText.h"
#include "../Engine/Options.h"
#include "../Interface/ComboBox.h"
#include "../Interface/TextButton.h"
#include "../Interface/Window.h"
#include "../Interface/Text.h"
#include "../Interface/TextList.h"
#include "../Menu/ErrorMessageState.h"
#include "../Savegame/Base.h"
#include "../Savegame/Soldier.h"
#include "../Savegame/Craft.h"
#include "../Savegame/SavedGame.h"
#include "SoldierInfoState.h"
#include "SoldierInfoStateFtA.h"
#include "../Mod/Armor.h"
#include "../Mod/RuleInterface.h"
#include "../Mod/RuleCraft.h"
#include "../Mod/RuleSoldier.h"
#include "../Engine/Unicode.h"
#include "../Battlescape/BattlescapeGenerator.h"
#include "../Battlescape/BriefingState.h"
#include "../Savegame/SavedBattleGame.h"

namespace OpenXcom
{

/**
 * Initializes all the elements in the Craft Soldiers screen.
 * @param game Pointer to the core game.
 * @param base Pointer to the base to get info from.
 * @param craft ID of the selected craft.
 */
CraftSoldiersState::CraftSoldiersState(Base *base, size_t craft)
		:  _base(base), _craft(craft), _otherCraftColor(0), _origSoldierOrder(*_base->getSoldiers()), _dynGetter(NULL)
{
	bool hidePreview = _game->getSavedGame()->getMonthsPassed() == -1;
	Craft *c = _base->getCrafts()->at(_craft);
	if (c && !c->getRules()->isForNewBattle())
	{
		// no battlescape map available
		hidePreview = true;
	}
	int pilots = c->getRules()->getPilots();
	_isInterceptor = pilots > 0 && !c->getRules()->getAllowLanding();
	_isMultipurpose = pilots > 0 && c->getRules()->getAllowLanding();
	_ftaUI = _game->getMod()->isFTAGame();

	// Create objects
	_window = new Window(this, 320, 200, 0, 0);
	_btnOk = new TextButton(hidePreview ? 148 : 38, 16, hidePreview ? 164 : 274, 176);
	_btnPreview = new TextButton(102, 16, 164, 176);
	_txtTitle = new Text(_ftaUI ? 168 : 300, 17, 16, 7);
	_txtName = new Text(114, 9, 16, 32);
	_txtRank = new Text(102, 9, 122, 32);
	_txtCraft = new Text(84, 9, 220, 32);
	_txtAvailable = new Text(110, 9, 16, 24);
	_txtUsed = new Text(110, 9, 122, 24);
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
	_lstSoldiers = new TextList(288, 128, 8, 40);

	touchComponentsCreate(_txtTitle, true);

	// Set palette
	setInterface("craftSoldiers");

	add(_window, "window", "craftSoldiers");
	add(_btnOk, "button", "craftSoldiers");
	add(_btnPreview, "button", "craftSoldiers");
	add(_txtTitle, "text", "craftSoldiers");
	add(_txtName, "text", "craftSoldiers");
	add(_txtRank, "text", "craftSoldiers");
	add(_txtCraft, "text", "craftSoldiers");
	add(_txtAvailable, "text", "craftSoldiers");
	add(_txtUsed, "text", "craftSoldiers");
	add(_lstSoldiers, "list", "craftSoldiers");
	add(_cbxSortBy, "button", "craftSoldiers");
	add(_cbxScreenActions, "button", "craftSoldiers");

	touchComponentsAdd("button2", "craftSoldiers", _window);

	_otherCraftColor = _game->getMod()->getInterface("craftSoldiers")->getElement("otherCraft")->color;

	centerAllSurfaces();

	// Set up objects
	setWindowBackground(_window, "craftSoldiers");

	touchComponentsConfigure();

	_btnOk->setText(tr("STR_OK"));
	_btnOk->onMouseClick((ActionHandler)&CraftSoldiersState::btnOkClick);
	_btnOk->onKeyboardPress((ActionHandler)&CraftSoldiersState::btnOkClick, Options::keyCancel);
	_btnOk->onKeyboardPress((ActionHandler)&CraftSoldiersState::btnDeassignAllSoldiersClick, Options::keyRemoveSoldiersFromAllCrafts);
	_btnOk->onKeyboardPress((ActionHandler)&CraftSoldiersState::btnDeassignCraftSoldiersClick, Options::keyRemoveSoldiersFromCraft);

	_btnPreview->setText(tr("STR_CRAFT_DEPLOYMENT_PREVIEW"));
	_btnPreview->setVisible(!hidePreview);
	_btnPreview->onMouseClick((ActionHandler)&CraftSoldiersState::btnPreviewClick);

	_txtTitle->setBig();
	if (Options::oxceBaseTouchButtons)
	{
		_txtTitle->setAlign(ALIGN_CENTER);
		_txtTitle->setText(c->getName(_game->getLanguage()));
	}
	else
	{
		_txtTitle->setAlign(ALIGN_LEFT);
		_txtTitle->setText(_ftaUI ? tr("STR_SELECT_SQUAD_UC") : tr("STR_SELECT_SQUAD_FOR_CRAFT").arg(c->getName(_game->getLanguage())));
	}

	_txtName->setText(tr("STR_NAME_UC"));

	_txtRank->setText(tr("STR_RANK"));

	if (_game->getMod()->isFTAGame())
	{
		_txtCraft->setText(tr("STR_ASSIGNMENT")); 
	}
	else
	{
		_txtCraft->setText(tr("STR_CRAFT"));
	}
	
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
	PUSH_IN("STR_SOLDIER_TYPE", typeStat);
	if (_ftaUI)
	{
		PUSH_IN("STR_ROLE_UC", roleStat);
		PUSH_IN("STR_RANK", roleRankStat);
	}
	else
	{
		PUSH_IN("STR_RANK", rankStat);
	}
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
	_cbxSortBy->onChange((ActionHandler)&CraftSoldiersState::cbxSortByChange);
	_cbxSortBy->setText(tr("STR_SORT_BY"));

	_availableOptions.clear();
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
	_cbxScreenActions->onChange((ActionHandler)&CraftSoldiersState::cbxScreenActionsChange);

	_lstSoldiers->setColumns(3, 106, 98, 76);
	_lstSoldiers->setAlign(ALIGN_RIGHT, 3);
	_lstSoldiers->setSelectable(true);
	_lstSoldiers->setBackground(_window);
	_lstSoldiers->setMargin(8);
	// Don't bind to button=0 (any click): mouse wheel would trigger this handler.
	_lstSoldiers->onMouseClick((ActionHandler)&CraftSoldiersState::lstSoldiersClick, SDL_BUTTON_LEFT);
	_lstSoldiers->onMouseClick((ActionHandler)&CraftSoldiersState::lstSoldiersClick, SDL_BUTTON_RIGHT);
}

/**
 * cleans up dynamic state
 */
CraftSoldiersState::~CraftSoldiersState()
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
void CraftSoldiersState::cbxSortByChange(Action *)
{
	bool ctrlPressed = _game->isCtrlPressed(true);
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
			if (_game->isShiftPressed(true))
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
 * Returns to the previous screen.
 * @param action Pointer to an action.
 */
void CraftSoldiersState::btnOkClick(Action *)
{
	_game->popState();
}

/**
 * Shows the battlescape preview.
 * @param action Pointer to an action.
 */
void CraftSoldiersState::btnPreviewClick(Action *)
{
	Craft* c = _base->getCrafts()->at(_craft);
	if (c->getSpaceUsed() <= 0)
	{
		// at least one unit must be onboard
		return;
	}

	SavedBattleGame* bgame = new SavedBattleGame(_game->getMod(), _game->getLanguage(), true);
	_game->getSavedGame()->setBattleGame(bgame);
	BattlescapeGenerator bgen = BattlescapeGenerator(_game);
	bgame->setMissionType(c->getRules()->getCustomPreviewType());
	bgame->setCraftForPreview(c);
	bgen.setCraft(c);
	bgen.run();

	// needed for preview of craft deployment tiles
	bgame->setCraftPos(bgen.getCraftPos());
	bgame->setCraftZ(bgen.getCraftZ());
	bgame->calculateCraftTiles();

	_game->pushState(new BriefingState(c));
}

/**
 * Shows the soldiers in a list at specified offset/scroll.
 */
void CraftSoldiersState::initList(size_t scrl)
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

	for (auto& soldier : *_base->getSoldiers())
	{
		if ((soldier->getRoleRank(ROLE_SOLDIER) > 0 && !_isInterceptor) //case for dropship
			|| (_isInterceptor && soldier->getRoleRank(ROLE_PILOT) > 0) //case for interceptor
			|| (_isMultipurpose && (soldier->getRoleRank(ROLE_PILOT) > 0 || soldier->getRoleRank(ROLE_SOLDIER) > 0)) //case for multipurpose craft
			|| selAction == "STR_ALL_ROLES" //case we want to see everyone
			|| !_ftaUI)
		{
			_filteredListOfSoldiers.push_back(soldier);
			_soldierNumbers.push_back(i);
		}
		i++;
	}

	if (_dynGetter != NULL)
	{
		_lstSoldiers->setColumns(4, 106, 98, 60, 16);
	}
	else
	{
		_lstSoldiers->setColumns(3, 106, 98, 76);
	}

	Craft *c = _base->getCrafts()->at(_craft);
	auto recovery = _base->getSumRecoveryPerDay();
	bool isBusy = false, isFree = false;
	for (auto* soldier : _filteredListOfSoldiers)
	{
		std::string duty = soldier->getCurrentDuty(_game->getLanguage(), recovery, isBusy, isFree);
		if (_dynGetter != NULL)
		{
			// call corresponding getter
			int dynStat = (*_dynGetter)(_game, soldier);
			std::ostringstream ss;
			ss << dynStat;
			_lstSoldiers->addRow(4, soldier->getName(true, 19).c_str(), tr(soldier->getRankString(_ftaUI)).c_str(), duty.c_str(), ss.str().c_str());
		}
		else
		{
			_lstSoldiers->addRow(3, soldier->getName(true, 19).c_str(), tr(soldier->getRankString(_ftaUI)).c_str(), duty.c_str());
		}

		Uint8 color;
		if (soldier->getCraft() == c)
		{
			color = _lstSoldiers->getSecondaryColor();
		}
		else if (isBusy || !isFree)
		{
			color = _otherCraftColor;
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

	_txtAvailable->setText(tr("STR_SPACE_AVAILABLE").arg(c->getSpaceAvailable()));
	_txtUsed->setText(tr("STR_SPACE_USED").arg(c->getSpaceUsed()));
}

/**
 * Shows the soldiers in a list.
 */
void CraftSoldiersState::init()
{
	State::init();
	_base->prepareSoldierStatsWithBonuses(); // refresh stats for sorting
	initList(_lstSoldiers->getScroll());

	// update the label to indicate presence of a saved craft deployment
	Craft* c = _base->getCrafts()->at(_craft);
	if (c->hasCustomDeployment())
		_btnPreview->setText(tr("STR_CRAFT_DEPLOYMENT_PREVIEW_SAVED"));
	else
		_btnPreview->setText(tr("STR_CRAFT_DEPLOYMENT_PREVIEW"));

	touchComponentsRefresh();
}

/**
 * Shows the selected soldier's info.
 * @param action Pointer to an action.
 */
void CraftSoldiersState::lstSoldiersClick(Action *action)
{
	double mx = action->getAbsoluteXMouse();
	if (mx >= _lstSoldiers->getArrowsLeftEdge() && mx < _lstSoldiers->getArrowsRightEdge())
	{
		return;
	}
	int row = _lstSoldiers->getSelectedRow();
	if (_game->isLeftClick(action, true))
	{
		Craft *c = _base->getCrafts()->at(_craft);

		Soldier* s = _filteredListOfSoldiers.at(row);

		bool isBusy = false, isFree = false;
		std::string duty = s->getCurrentDuty(_game->getLanguage(), _base->getSumRecoveryPerDay(), isBusy, isFree);

		if (s->getCraft() == c)
		{
			s->setCraftAndMoveEquipment(0, _base, _game->getSavedGame()->getMonthsPassed() == -1);
			_lstSoldiers->setCellText(row, 2, tr("STR_NONE_UC"));
			_lstSoldiers->setRowColor(row, _lstSoldiers->getColor());
		}
		else if ((s->getCraft() && s->getCraft()->getStatus() == "STR_OUT") || s->getCovertOperation() != 0 || s->hasPendingTransformation())
		{
			return;
		}
		else if (s->hasFullHealth())
		{
			if (_isInterceptor && _ftaUI && s->getRoleRank(ROLE_PILOT) < 1)
			{
				_game->pushState(new ErrorMessageState(tr("STR_IS_NOT_ALLOWED_PILOTING"),
					_palette,
					_game->getMod()->getInterface("soldierInfo")->getElement("errorMessage")->color,
					"BACK01.SCR",
					_game->getMod()->getInterface("soldierInfo")->getElement("errorPalette")->color));
				return;
			}

			int space = c->getSpaceAvailable();
			CraftPlacementErrors err = c->validateAddingSoldier(space, s);
			int relay = c->getCraftStats().relay;
			if (s->getRoleRank(ROLE_ROBOT) > 0) //#FINNIKTODO: refactor relay logic into "c->validateAddingSoldier(space, s)"
			{
				for (const auto* i : *_base->getSoldiers())
				{
					if (i->getCraft() == c)
					{
						if (s->hasOnlyOneRole(ROLE_ROBOT))// only robot role - not a sapient AI.
						{
							relay--;
						}
					}
				}
			}
			if (relay < 0)
			{
				_game->pushState(new ErrorMessageState(tr("STR_NOT_ENOUGH_RELAY_POWER"),
					_palette,
					_game->getMod()->getInterface("soldierInfo")->getElement("errorMessage")->color,
					"BACK01.SCR",
					_game->getMod()->getInterface("soldierInfo")->getElement("errorPalette")->color));
			}
			else if (err == CPE_None)
			{
				s->setCraftAndMoveEquipment(c, _base, _game->getSavedGame()->getMonthsPassed() == -1, true);
				_lstSoldiers->setCellText(row, 2, c->getName(_game->getLanguage()));
				_lstSoldiers->setRowColor(row, _lstSoldiers->getSecondaryColor());

				// update the label to indicate absence of a saved craft deployment
				_btnPreview->setText(tr("STR_CRAFT_DEPLOYMENT_PREVIEW"));
			}
			else if (err == CPE_SoldierGroupNotAllowed)
			{
				_game->pushState(new ErrorMessageState(tr("STR_SOLDIER_GROUP_NOT_ALLOWED"), _palette, _game->getMod()->getInterface("soldierInfo")->getElement("errorMessage")->color, "BACK01.SCR", _game->getMod()->getInterface("soldierInfo")->getElement("errorPalette")->color));
			}
			else if (err == CPE_SoldierGroupNotSame)
			{
				_game->pushState(new ErrorMessageState(tr("STR_SOLDIER_GROUP_NOT_SAME"), _palette, _game->getMod()->getInterface("soldierInfo")->getElement("errorMessage")->color, "BACK01.SCR", _game->getMod()->getInterface("soldierInfo")->getElement("errorPalette")->color));
			}
			else if (err == CPE_ArmorGroupNotAllowed)
			{
				_game->pushState(new ErrorMessageState(tr("STR_ARMOR_GROUP_NOT_ALLOWED"), _palette, _game->getMod()->getInterface("soldierInfo")->getElement("errorMessage")->color, "BACK01.SCR", _game->getMod()->getInterface("soldierInfo")->getElement("errorPalette")->color));
			}
			else if (err == CPE_SoldierPendingTransformation)
			{
				_game->pushState(new ErrorMessageState(tr("STR_SOLDIER_HAS_PENDING_TRANSFORMATION"), _palette, _game->getMod()->getInterface("soldierInfo")->getElement("errorMessage")->color, "BACK01.SCR", _game->getMod()->getInterface("soldierInfo")->getElement("errorPalette")->color));
			}
			else if (err == CPE_NotEnoughSpace)
			{
				_game->pushState(new ErrorMessageState(tr("STR_NOT_ENOUGH_CRAFT_SPACE"),
					_palette,
					_game->getMod()->getInterface("soldierInfo")->getElement("errorMessage")->color,
					"BACK01.SCR",
					_game->getMod()->getInterface("soldierInfo")->getElement("errorPalette")->color));
			}
		}

		_txtAvailable->setText(tr("STR_SPACE_AVAILABLE").arg(c->getSpaceAvailable()));
		_txtUsed->setText(tr("STR_SPACE_USED").arg(c->getSpaceUsed()));
	}
	else if (_game->isRightClick(action, true))
	{
		if (_ftaUI)
		{
			_game->pushState(new SoldierInfoStateFtA(_base, _soldierNumbers.at(row)));
		}
		else
		{
			_game->pushState(new SoldierInfoState(_base, _soldierNumbers.at(row), false));
		}
	}
}

void CraftSoldiersState::cbxScreenActionsChange(Action *action)
{
	_cbxSortBy->setSelected(0);
	initList(0);
}

/**
 * De-assign all soldiers from all craft located in the base (i.e. not out on a mission).
 * @param action Pointer to an action.
 */
void CraftSoldiersState::btnDeassignAllSoldiersClick(Action *action)
{
	Craft *c = _base->getCrafts()->at(_craft);
	int row = 0;
	for (auto soldier : _filteredListOfSoldiers)
	{
		if ((soldier->getCraft() && (soldier->getCraft()->getStatus() != "STR_OUT" && soldier->getCraft() == c)) 
		&& soldier->getCovertOperation() == 0) //just in case
		{
			soldier->setCraftAndMoveEquipment(0, _base, _game->getSavedGame()->getMonthsPassed() == -1);
			_lstSoldiers->setCellText(row, 2, tr("STR_NONE_UC"));
			_lstSoldiers->setRowColor(row, _lstSoldiers->getColor());
		}
		row++;
	}

	
	_txtAvailable->setText(tr("STR_SPACE_AVAILABLE").arg(c->getSpaceAvailable()));
	_txtUsed->setText(tr("STR_SPACE_USED").arg(c->getSpaceUsed()));
}

/**
 * De-assign all soldiers from the current craft.
 * @param action Pointer to an action.
 */
void CraftSoldiersState::btnDeassignCraftSoldiersClick(Action *action)
{
	Craft *c = _base->getCrafts()->at(_craft);
	int row = 0;
	for (auto* soldier : *_base->getSoldiers())
	{
		if (soldier->getCraft() == c)
		{
			soldier->setCraftAndMoveEquipment(0, _base, _game->getSavedGame()->getMonthsPassed() == -1);
			_lstSoldiers->setCellText(row, 2, tr("STR_NONE_UC"));
			_lstSoldiers->setRowColor(row, _lstSoldiers->getColor());
		}
		row++;
	}

	_txtAvailable->setText(tr("STR_SPACE_AVAILABLE").arg(c->getSpaceAvailable()));
	_txtUsed->setText(tr("STR_SPACE_USED").arg(c->getSpaceUsed()));
}

}

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
#include "DiplomacyHirePersonnelState.h"
#include <algorithm>
#include <algorithm>
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
#include "../Savegame/SavedGame.h"
#include "../Basescape/SoldierInfoStateFtA.h"
#include "../Mod/Armor.h"
#include "../Mod/RuleSoldier.h"
#include "../Engine/Unicode.h"
#include "../Mod/RuleInterface.h"
#include "../Savegame/DiplomacyFaction.h"
#include "../Savegame/SoldierPool.h"
#include "../Savegame/Transfer.h"

namespace OpenXcom
{

/**
 * Initializes all the elements in the Diplomacy Hire Personnel State.
 * @param game Pointer to the core game.
 * @param base Pointer to the base to get info from.
 * @param craft ID of the selected craft.
 */
DiplomacyHirePersonnelState::DiplomacyHirePersonnelState(Base *base, DiplomacyFaction* faction)
		:  _base(base), _faction(faction), _dynGetter(NULL)
{
	
	// Create objects
	_window = new Window(this, 320, 200, 0, 0);
	_btnOk = new TextButton(83, 16, 229, 176);
	_btnCancel = new TextButton(83, 16, 138, 176);
	_txtTitle = new Text(300, 17, 16, 7);
	_txtName = new Text(114, 9, 16, 32);
	_txtRank = new Text(102, 9, 122, 32);
	_txtCost = new Text(84, 9, 220, 32);
	_txtAvailable = new Text(110, 9, 16, 24);
	_txtUsed = new Text(110, 9, 122, 24);
	_txtTotal = new Text(93, 9, 220, 24);
	_cbxSortBy = new ComboBox(this, 120, 16, 8, 176, true);
	
	_lstSoldiers = new TextList(288, 128, 8, 40);

	// Set palette
	setInterface("diplomacyHirePersonnel");

	add(_window, "window", "diplomacyHirePersonnel");
	add(_btnOk, "button", "diplomacyHirePersonnel");
	add(_btnCancel, "button", "diplomacyHirePersonnel");
	add(_txtTitle, "text", "diplomacyHirePersonnel");
	add(_txtName, "text", "diplomacyHirePersonnel");
	add(_txtRank, "text", "diplomacyHirePersonnel");
	add(_txtCost, "text", "diplomacyHirePersonnel");
	add(_txtAvailable, "text", "diplomacyHirePersonnel");
	add(_txtUsed, "text", "diplomacyHirePersonnel");
	add(_txtTotal, "text", "diplomacyHirePersonnel");
	add(_lstSoldiers, "list", "diplomacyHirePersonnel");
	add(_cbxSortBy, "button", "diplomacyHirePersonnel");

	centerAllSurfaces();

	// Set up objects
	setWindowBackground(_window, "diplomacyHirePersonnel");

	_btnOk->setText(tr("STR_OK"));
	_btnOk->onMouseClick((ActionHandler)&DiplomacyHirePersonnelState::btnOkClick);
	_btnOk->onKeyboardPress((ActionHandler)&DiplomacyHirePersonnelState::btnOkClick, Options::keyOk);


	_btnCancel->setText(tr("STR_BACK"));
	_btnCancel->onMouseClick((ActionHandler)&DiplomacyHirePersonnelState::btnCancelClick);
	_btnCancel->onKeyboardPress((ActionHandler)&DiplomacyHirePersonnelState::btnCancelClick, Options::keyCancel);

	_txtTitle->setBig();
	_txtTitle->setText(tr("STR_HIRE_PERSONNEL"));

	_txtName->setText(tr("STR_NAME_UC"));

	_txtRank->setText(tr("STR_RANK"));

	_txtCost->setText(tr("STR_COST_TITLE"));
	
	// populate sort options
	std::vector<std::string> sortOptions;
	//sortOptions.push_back(tr("STR_ORIGINAL_ORDER"));
	_sortFunctors.push_back(NULL);
	bool showPsiStats = _game->getSavedGame()->isResearched(_game->getMod()->getPsiRequirements());
	bool showMana = _game->getMod()->isManaFeatureEnabled() && _game->getSavedGame()->isManaUnlocked(_game->getMod());

#define PUSH_IN(strId, functor) \
	sortOptions.push_back(tr(strId)); \
	_sortFunctors.push_back(new SortFunctor(_game, functor));

	//PUSH_IN("STR_ID", idStat);
	//PUSH_IN("STR_NAME_UC", nameStat);
	//PUSH_IN("STR_SOLDIER_TYPE", typeStat);
	PUSH_IN("STR_ROLE_UC", roleStat);
	PUSH_IN("STR_RANK", roleRankStat);
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
	_cbxSortBy->onChange((ActionHandler)&DiplomacyHirePersonnelState::cbxSortByChange);
	_cbxSortBy->setText(tr("STR_COMPARE_BY"));

	_lstSoldiers->setColumns(3, 106, 98, 76);
	_lstSoldiers->setAlign(ALIGN_RIGHT, 3);
	_lstSoldiers->setSelectable(true);
	_lstSoldiers->setBackground(_window);
	_lstSoldiers->setMargin(8);
	// Don't bind to button=0 (any click): mouse wheel would trigger this handler.
	_lstSoldiers->onMouseClick((ActionHandler)&DiplomacyHirePersonnelState::lstSoldiersClick, SDL_BUTTON_LEFT);
	_lstSoldiers->onMouseClick((ActionHandler)&DiplomacyHirePersonnelState::lstSoldiersClick, SDL_BUTTON_RIGHT);

	updateState();
}

/**
 * cleans up dynamic state
 */
DiplomacyHirePersonnelState::~DiplomacyHirePersonnelState()
{
	for (std::vector<SortFunctor *>::iterator it = _sortFunctors.begin();
		it != _sortFunctors.end(); ++it)
	{
		delete(*it);
	}
}

/**
 * Sorts the soldiers list by the selected criterion
 * @param action Pointer to an action.
 */
void DiplomacyHirePersonnelState::cbxSortByChange(Action *)
{
	size_t selIdx = _cbxSortBy->getSelected();
	if (selIdx == (size_t)-1)
	{
		return;
	}
	SortFunctor *compFunc = _sortFunctors[selIdx];
	_dynGetter = NULL;
	if (compFunc)
	{
		_dynGetter = compFunc->getGetter();
	}

	size_t originalScrollPos = _lstSoldiers->getScroll();
	initList(originalScrollPos);
}

/**
 * Returns to the previous screen.
 * @param action Pointer to an action.
 */
void DiplomacyHirePersonnelState::btnOkClick(Action *)
{
	for (auto s : _selectedSoldiers)
	{
		int time = s->getRules()->getTransferTime();
		if (time == 0)
			time = _game->getMod()->getPersonnelTime();
		Transfer* t = new Transfer(time);
		t->setSoldier(s);
		_base->getTransfers()->push_back(t);

		_faction->getStaffPool()->removeSoldier(s);
	}
	_game->popState();
}

/**
 * Shows the battlescape preview.
 * @param action Pointer to an action.
 */
void DiplomacyHirePersonnelState::btnCancelClick(Action *)
{
	_selectedSoldiers.clear();
	_game->popState();
}

/**
 * Shows the soldiers in a list at specified offset/scroll.
 */
void DiplomacyHirePersonnelState::initList(size_t scrl)
{
	int row = 0;
	_soldierNumbers.clear();
	_filteredListOfSoldiers.clear();
	_lstSoldiers->clearList();
	_soldierNumbers.clear();
	int i = 0;

	for (auto soldier : _faction->getStaffPool()->getSoldiers())
	{
		//if (some soldier filtering params...) {
		_filteredListOfSoldiers.push_back(soldier);
		_soldierNumbers.push_back(i);
		// } 
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

	for (auto s : _filteredListOfSoldiers)
	{
		std::ostringstream sh;
		sh << s->getHireValue();
		if (_dynGetter != NULL)
		{
			// call corresponding getter
			int dynStat = (*_dynGetter)(_game, s);
			std::ostringstream ss;
			ss << dynStat;
			
			_lstSoldiers->addRow(4, s->getName(true, 19).c_str(), tr(s->getRankString(true)).c_str(), sh.str().c_str(), ss.str().c_str());
		}
		else
		{
			_lstSoldiers->addRow(3, s->getName(true, 19).c_str(), tr(s->getRankString(true)).c_str(), sh.str().c_str());
		}

		Uint8 color;
		std::vector<Soldier*>::iterator soldierIt =
			std::find(_selectedSoldiers.begin(), _selectedSoldiers.end(), s);
		if (soldierIt != _selectedSoldiers.end())
		{
			color = _lstSoldiers->getSecondaryColor();
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
	updateState();
	
}

void DiplomacyHirePersonnelState::updateState()
{
	if (_selectedSoldiers.empty())
	{
		_btnOk->setVisible(false);
		_btnCancel->setWidth(174);
	}
	else
	{
		_btnOk->setVisible(true);
		_btnCancel->setWidth(83);
	}

	_txtAvailable->setText(tr("STR_SPACE_AVAILABLE").arg(_base->getAvailableQuarters() - _selectedSoldiers.size()));
	_txtUsed->setText(tr("STR_SPACE_USED").arg(_base->getUsedQuarters() + _selectedSoldiers.size()));
	_txtTotal->setText(tr("STR_TOTAL_UC_ARG").arg(_total));
}

/**
 * Shows the soldiers in a list.
 */
void DiplomacyHirePersonnelState::init()
{
	State::init();
	// refresh stats for sorting
	for (auto s : _faction->getStaffPool()->getSoldiers())
	{
		s->prepareStatsWithBonuses(_game->getMod());
	}
	initList(0);

}

/**
 * Shows the selected soldier's info.
 * @param action Pointer to an action.
 */
void DiplomacyHirePersonnelState::lstSoldiersClick(Action *action)
{
	Soldier* s = _faction->getStaffPool()->getSoldiers().at(_lstSoldiers->getSelectedRow());
	int row = _lstSoldiers->getSelectedRow();
	if (action->getDetails()->button.button == SDL_BUTTON_LEFT)
	{
		
		Uint8 color = _lstSoldiers->getColor();
		std::vector<Soldier*>::iterator soldierIt =
			std::find(_selectedSoldiers.begin(), _selectedSoldiers.end(), s);
		if (soldierIt != _selectedSoldiers.end())
		{
			//remove
			_selectedSoldiers.erase(soldierIt);
			_total -= static_cast<int64_t>(s->getHireValue());
		}
		else
		{
			// calculate soldier pool size
			int size = 0;
			for (auto i : _selectedSoldiers)
			{
				size += i->getRules()->getLivingSpace();
			}

			RuleInterface* menuInterface = _game->getMod()->getInterface("buyMenu");
			if (_total + static_cast<int64_t>(s->getHireValue()) > _game->getSavedGame()->getFunds())
			{
				_game->pushState(new ErrorMessageState("STR_NOT_ENOUGH_MONEY",
					_palette,
					menuInterface->getElement("errorMessage")->color,
					"BACK13.SCR",
					menuInterface->getElement("errorPalette")->color));
			}
			else if (_base->getUsedQuarters() + size >= _base->getAvailableQuarters())
			{
				_game->pushState(new ErrorMessageState("STR_NOT_ENOUGH_SPACE",
					_palette,
					menuInterface->getElement("errorMessage")->color,
					"BACK13.SCR",
					menuInterface->getElement("errorPalette")->color));
			}
			else
			{
				//add
				color = _lstSoldiers->getSecondaryColor();
				_selectedSoldiers.push_back(s);
				_total += static_cast<int64_t>(s->getHireValue());
			}
		}
		
		_lstSoldiers->setRowColor(row, color);

		updateState();
	}
	else if (action->getDetails()->button.button == SDL_BUTTON_RIGHT)
	{
		_game->pushState(new SoldierInfoStateFtA(_base, s, _faction));
	}
}

}

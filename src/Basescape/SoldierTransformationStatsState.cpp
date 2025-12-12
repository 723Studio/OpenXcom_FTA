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
#include "SoldierTransformationStatsState.h"
#include <sstream>
#include <algorithm>
#include "../Engine/Action.h"
#include "../Engine/Game.h"
#include "../Engine/LocalizedText.h"
#include "../Engine/Options.h"
#include "../Engine/Screen.h"
#include "../Interface/Text.h"
#include "../Interface/TextButton.h"
#include "../Interface/TextEdit.h"
#include "../Interface/TextList.h"
#include "../Interface/Window.h"
#include "../Mod/Mod.h"
#include "../Mod/RuleSoldier.h"
#include "../Mod/RuleSoldierBonus.h"
#include "../Mod/RuleSoldierTransformation.h"
#include "../Savegame/SavedGame.h"
#include "../Savegame/Soldier.h"
#include <complex.h>

namespace OpenXcom
{

/**
 * Initializes all the elements in the Soldier Transformation Stats State
 * @param transformationRule Pointer to the transformation ruleset
 * @param sourceSoldier Pointer to the selected soldier
 */
SoldierTransformationStatsState::SoldierTransformationStatsState(RuleSoldierTransformation *transformationRule, Soldier *sourceSoldier) :
			_transformationRule(transformationRule), _sourceSoldier(sourceSoldier)
{
	_screen = false;
	_window = new Window(this, 213, 200, 107, 0, POPUP_HORIZONTAL);
	_btnOK = new TextButton(193, 16, 117, 176);
	_txtStateHeader = new Text(187, 16, 120, 8);
	_txtStatChangesHeader = new Text(73, 9, 234, 27);
	_txtCurrent = new Text(117, 9, 117, 37);
	_txtMin = new Text(19, 9, 234, 37);
	_txtMax = new Text(19, 9, 253, 37);
	_txtBonus = new Text(35, 9, 272, 37);
	_lstStats = new TextList(181, 126, 117, 46);

	// Set palette
	setInterface("soldierTransformationStats");

	add(_window, "window", "soldierTransformationStats");
	add(_btnOK, "button", "soldierTransformationStats");
	
	add(_txtStateHeader, "text", "soldierTransformationStats");
	add(_txtStatChangesHeader, "text", "soldierTransformationStats");
	add(_txtCurrent, "text", "soldierTransformationStats");
	add(_txtMin, "text", "soldierTransformationStats");
	add(_txtMax, "text", "soldierTransformationStats");
	add(_txtBonus, "text", "soldierTransformationStats");

	add(_lstStats, "list", "soldierTransformationStats");

	centerAllSurfaces();

	// Set up objects
	setWindowBackground(_window, "soldierTransformation");

	_btnOK->setText(tr("STR_OK"));
	_btnOK->onMouseClick((ActionHandler)&SoldierTransformationStatsState::btnOKClick);
	_btnOK->onKeyboardPress((ActionHandler)&SoldierTransformationStatsState::btnOKClick, Options::keyCancel);
	_btnOK->onKeyboardPress((ActionHandler)&SoldierTransformationStatsState::btnOKClick, Options::keyOk);

	_txtStateHeader->setText(tr("STR_STATS_CHANGE"));
	_txtStateHeader->setBig();
	_txtStateHeader->setAlign(ALIGN_CENTER);

	_txtStatChangesHeader->setText(tr("STR_STAT_CHANGES"));
	_txtStatChangesHeader->setAlign(ALIGN_CENTER);

	_txtCurrent->setText(tr("STR_CURRENT_STATS"));
	_txtCurrent->setAlign(ALIGN_RIGHT);

	_txtMin->setText(tr("STR_MIN_STATS"));

	_txtMax->setText(tr("STR_MAX_STATS"));

	_txtBonus->setText(tr("STR_BONUS_STATS"));

	_lstStats->setColumns(5, 98, 19, 19, 19, 26);
	_lstStats->setWordWrap(true);
	_lstStats->setAlign(ALIGN_RIGHT);
	_lstStats->setAlign(ALIGN_LEFT, 0);

	UnitStats currentStats = *_sourceSoldier->getCurrentStats();
	UnitStats changedStatsMin = _sourceSoldier->calculateStatChanges(_game->getMod(), _transformationRule, _sourceSoldier, 1, _sourceSoldier->getRules());
	UnitStats changedStatsMax = _sourceSoldier->calculateStatChanges(_game->getMod(), _transformationRule, _sourceSoldier, 2, _sourceSoldier->getRules());
	UnitStats bonusStats;
	auto bonusRule = _game->getMod()->getSoldierBonus(_transformationRule->getSoldierBonusType(), false);
	if (bonusRule)
	{
		bonusStats += *bonusRule->getStats();
	}

	UnitStats rerollFlags = _transformationRule->getRerollStats();

	currentStats.fieldLoop(
		[&](UnitStats::Ptr p)
		{
			if (currentStats.*p > 0 || changedStatsMin.*p > 0 || changedStatsMax.*p > 0 || bonusStats.*p > 0)
			{
				if (!_game->getSavedGame()->isResearched(_game->getMod()->getPsiRequirements()) &&
					(UnitStats::getStatString(p) == "STR_PSIONIC_SKILL_UC"
						|| UnitStats::getStatString(p) == "STR_PSIONIC_STRENGTH_UC"
						|| UnitStats::getStatString(p) == "STR_MANA_POOL_UC"
						|| UnitStats::getStatString(p) == "STR_PSIONICS_UC"))
				{} //do nothing
				else if (!_game->getSavedGame()->isResearched(_game->getMod()->getAlienTechUnlockResearch())
					&& UnitStats::getStatString(p) == "STR_ALIEN_TECH_UC")
				{}
				else if (!_game->getSavedGame()->isResearched(_game->getMod()->getXenolinguisticsUnlockResearch())
					&& UnitStats::getStatString(p) == "STR_XENOLINGUISTICS_UC")
				{}
				else if (!_game->getSavedGame()->isResearched(_game->getMod()->getBeamOperationsUnlockResearch())
					&& UnitStats::getStatString(p) == "STR_BEAMS_OPERATION_UC")
				{}
				else if (!_game->getSavedGame()->isResearched(_game->getMod()->getCraftSynapseUnlockResearch())
					&& UnitStats::getStatString(p) == "STR_SYNAPTIC_CONNECTIVITY_UC")
				{}
				else if (!_game->getSavedGame()->isResearched(_game->getMod()->getGravControlUnlockResearch())
					&& UnitStats::getStatString(p) == "STR_GRAVITY_MANIPULATION_UC")
				{}
				else //finally
				{
					_lstStats->addRow(5,
						tr(UnitStats::getStatString(p, UnitStats::STATSTR_LC)).c_str(),
						formatStat(currentStats.*p, false, false).c_str(),
						formatStat(changedStatsMin.*p, true, rerollFlags.*p).c_str(),
						formatStat(changedStatsMax.*p, true, rerollFlags.*p).c_str(),
						formatStat(bonusStats.*p, true, false).c_str());
				}
			}
		}
	);
}

/**
 * cleans up dynamic state
 */
SoldierTransformationStatsState::~SoldierTransformationStatsState()
{

}

/**
 * Returns to previous screen.
 * @param action A pointer to an Action.
 */
void SoldierTransformationStatsState::btnOKClick(Action *action)
{
	_game->popState();
}

/**
 * Creates a string for the soldier stats table
 */
std::string SoldierTransformationStatsState::formatStat(int stat, bool plus, bool hide)
{
	if (hide) return "\x01?\x01"; //Unicode::TOK_COLOR_FLIP
	if (stat == 0) return "";

	std::ostringstream ss;
	ss << Unicode::TOK_COLOR_FLIP;
	if (plus && stat > 0) ss << '+';
	ss << stat << Unicode::TOK_COLOR_FLIP;
	return ss.str();
}

}

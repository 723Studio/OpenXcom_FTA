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
#include "BattlescapeGame.h"
#include "BattlescapeState.h"
#include "Camera.h"
#include "ExperienceOverviewState.h"
#include "Map.h"
#include "../Engine/Action.h"
#include "../Engine/Game.h"
#include "../Engine/Language.h"
#include "../Engine/Options.h"
#include "../Interface/TextButton.h"
#include "../Interface/Window.h"
#include "../Interface/Text.h"
#include "../Interface/TextList.h"
#include "../Mod/Mod.h"
#include "../Savegame/BattleUnit.h"
#include "../Savegame/SavedBattleGame.h"
#include "../Savegame/SavedGame.h"

namespace OpenXcom
{

/**
 * Initializes all the elements in the Experience Overview screen.
 */
ExperienceOverviewState::ExperienceOverviewState(BattlescapeState* parent) : _parent(parent)
{
	_screen = false;
	_showPsi = true; //_game->getSavedGame()->isResearched(_game->getMod()->getPsiRequirements());
	_showMana = true; //_game->getSavedGame()->isManaUnlocked(_game->getMod())

	// Create objects
	_window = new Window(this, 320, 200, 0, 0);
	_txtTitle = new Text(300, 17, 10, 13);
	_txtName = new Text(100, 10, 10, 40);
	_btnOk = new TextButton(160, 16, 80, 174);
	_lstSoldiers = new TextList(297, 112, 8, 52);
	_txtBravery = new Text(18, 10, 108, 40);
	_txtReactions = new Text(18, 10, 128, 40);
	_txtFiring = new Text(18, 10, 148, 40);
	_txtThrowing = new Text(18, 10, 168, 40);
	_txtMelee = new Text(18, 10, 188, 40);
	_txtHacking = new Text(18, 10, 208, 40);
	_txtBiology = new Text(18, 10, 228, 40);
	_txtPhysics = new Text(18, 10, 248, 40);
	_txtMana = new Text(18, 10, 268, 40);
	_txtPsiSkill = new Text(18, 10, 288, 40);

	// Set palette
	_game->getSavedGame()->getSavedBattle()->setPaletteByDepth(this);

	add(_window, "messageWindowBorder", "battlescape");
	add(_btnOk, "messageWindowButtons", "battlescape");
	add(_txtName, "messageWindows", "battlescape");
	add(_txtTitle, "messageWindows", "battlescape");
	add(_lstSoldiers, "optionLists", "battlescape");
	add(_txtBravery, "messageWindows", "battlescape");
	add(_txtReactions, "messageWindows", "battlescape");
	add(_txtFiring, "messageWindows", "battlescape");
	add(_txtThrowing, "messageWindows", "battlescape");
	add(_txtMelee, "messageWindows", "battlescape");
	add(_txtHacking, "messageWindows", "battlescape");
	add(_txtBiology, "messageWindows", "battlescape");
	add(_txtPhysics, "messageWindows", "battlescape");
	add(_txtMana, "messageWindows", "battlescape");
	add(_txtPsiSkill, "messageWindows", "battlescape");

	centerAllSurfaces();

	// Set up objects
	_window->setHighContrast(true);
	_window->setBackground(_game->getMod()->getSurface("TAC00.SCR"));

	_btnOk->setHighContrast(true);
	_btnOk->setText(tr("STR_OK"));
	_btnOk->onMouseClick((ActionHandler)&ExperienceOverviewState::btnOkClick);
	_btnOk->onKeyboardPress((ActionHandler)&ExperienceOverviewState::btnOkClick, Options::keyCancel);

	_txtTitle->setHighContrast(true);
	_txtTitle->setBig();
	_txtTitle->setAlign(ALIGN_CENTER);
	_txtTitle->setText(tr("STR_EXPERIENCE_OVERVIEW"));

	_txtName->setHighContrast(true);
	_txtBravery->setHighContrast(true);
	_txtReactions->setHighContrast(true);
	_txtFiring->setHighContrast(true);
	_txtThrowing->setHighContrast(true);
	_txtMelee->setHighContrast(true);
	_txtHacking->setHighContrast(true);
	_txtBiology->setHighContrast(true);
	_txtPhysics->setHighContrast(true);
	_txtMana->setHighContrast(true);
	_txtPsiSkill->setHighContrast(true);

	_txtName->setText(tr("STR_NAME"));
	_txtBravery->setText(tr(UnitStats::getStatString(&UnitStats::bravery, UnitStats::STATSTR_ABBREV)));
	_txtReactions->setText(tr(UnitStats::getStatString(&UnitStats::reactions, UnitStats::STATSTR_ABBREV)));
	_txtFiring->setText(tr(UnitStats::getStatString(&UnitStats::firing, UnitStats::STATSTR_ABBREV)));
	_txtThrowing->setText(tr(UnitStats::getStatString(&UnitStats::throwing, UnitStats::STATSTR_ABBREV)));
	_txtMelee->setText(tr(UnitStats::getStatString(&UnitStats::melee, UnitStats::STATSTR_ABBREV)));
	_txtHacking->setText(tr(UnitStats::getStatString(&UnitStats::hacking, UnitStats::STATSTR_ABBREV)));
	_txtBiology->setText(tr(UnitStats::getStatString(&UnitStats::biology, UnitStats::STATSTR_ABBREV)));
	_txtPhysics->setText(tr(UnitStats::getStatString(&UnitStats::physics, UnitStats::STATSTR_ABBREV)));

	if (_showMana)
		_txtMana->setText(tr(UnitStats::getStatString(&UnitStats::mana, UnitStats::STATSTR_ABBREV)));
	if (_showPsi)
		_txtPsiSkill->setText(tr(UnitStats::getStatString(&UnitStats::psiSkill, UnitStats::STATSTR_ABBREV)));

	_lstSoldiers->setColumns(11, 100, 20, 20, 20, 20, 20, 20, 20, 20, 20, 17);
	_lstSoldiers->setSelectable(true);
	_lstSoldiers->setHighContrast(true);
	_lstSoldiers->setBackground(_window);
	_lstSoldiers->setMargin(2);
	_lstSoldiers->onMouseClick((ActionHandler)&ExperienceOverviewState::lstSoldiersClick);

	_lstSoldiers->clearList();
	int row = 0;
	for (auto* soldier : *_game->getSavedGame()->getSavedBattle()->getUnits())
	{
		if (!soldier->getGeoscapeSoldier())
		{
			continue;
		}
		if (soldier->getStatus() == STATUS_DEAD || soldier->getStatus() == STATUS_IGNORE_ME)
		{
			continue;
		}

		const UnitStats* stats = soldier->getExpStats();

		std::ostringstream bravery;
		bravery << stats->bravery;
		std::ostringstream reactions;
		reactions << stats->reactions;
		std::ostringstream firing;
		firing << stats->firing;
		std::ostringstream throwing;
		throwing << stats->throwing;
		std::ostringstream melee;
		melee << stats->melee;
		std::ostringstream hacking;
		hacking << stats->hacking;
		std::ostringstream biology;
		biology << stats->biology;
		std::ostringstream physics;
		physics << stats->physics;
		std::ostringstream mana;
		std::ostringstream psiSkill;
		if (_showMana)
			mana << stats->mana;
		if (_showPsi)
			psiSkill << stats->psiSkill;

		_soldiers.push_back(soldier);
		_lstSoldiers->addRow(11,
			soldier->getName(_game->getLanguage()).c_str(),
			bravery.str().c_str(),
			reactions.str().c_str(),
			firing.str().c_str(),
			throwing.str().c_str(),
			melee.str().c_str(),
			hacking.str().c_str(),
			biology.str().c_str(),
			physics.str().c_str(),
			mana.str().c_str(),
			psiSkill.str().c_str(),
			"");
		for (int col = 1; col < 9; ++col)
		{
			if (_lstSoldiers->getCellText(row, col) != "0")
			{
				_lstSoldiers->setCellColor(row, col, _lstSoldiers->getSecondaryColor());
			}
		}
		row++;
	}
}

/**
 * Returns to the previous screen.
 * @param action Pointer to an action.
 */
void ExperienceOverviewState::btnOkClick(Action*)
{
	_game->popState();
}

/**
 * Selects the soldier if possible and returns to the previous screen.
 * @param action Pointer to an action.
 */
void ExperienceOverviewState::lstSoldiersClick(Action*)
{
	if (_parent->allowButtons())
	{
		auto index = _lstSoldiers->getSelectedRow();
		auto* bu = _soldiers.at(index);

		if (bu->isSelectable(_game->getSavedGame()->getSavedBattle()->getSide(), false, false))
		{
			// select
			_parent->getBattleGame()->cancelAllActions();
			_parent->getBattleGame()->primaryAction(bu->getPosition());
		}

		// center on position
		_parent->getMap()->getCamera()->centerOnPosition(bu->getPosition());

		_game->popState();
	}
}

}

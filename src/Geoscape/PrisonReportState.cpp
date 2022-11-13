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
#include "PrisonReportState.h"
#include "../Engine/Game.h"
#include "../Mod/Mod.h"
#include "../Interface/TextButton.h"
#include "../Interface/Window.h"
#include "../Interface/Text.h"
#include "../Mod/RuleResearch.h"
#include "../Ufopaedia/Ufopaedia.h"
#include "../Basescape/PrisonManagementState.h"
#include "../Basescape/SoldiersState.h"
#include "../Engine/Options.h"
#include "../Savegame/BasePrisoner.h"

namespace OpenXcom
{
/**
 * Creates the PrisonReportState for case of prisoner's death.
 * @param prisoner - current prisoner
 * @param base - current base
 */
PrisonReportState::PrisonReportState(BasePrisoner* prisoner, Base* base) :
		_prisoner(prisoner), _base(base), _reportCase(PRISONER_REPORT_CASE_DEATH)
{
	initUI();
}

/**
 * Creates the PrisonReportState for case of finished interrogation.
 * @param research - main interrogation research rule
 * @param bonus - bonus interrogation research rule
 * @param prisoner - current prisoner
 * @param base - current base
 */
PrisonReportState::PrisonReportState(const RuleResearch* research, const RuleResearch* bonus, BasePrisoner* prisoner, Base* base) :
		_research(research), _bonus(bonus), _prisoner(prisoner), _base(base), _reportCase(PRISONER_REPORT_CASE_INTERROGATION)
{
	initUI();
}

PrisonReportState::PrisonReportState(Soldier* soldier, BasePrisoner* prisoner, Base* base) :
		_soldier(soldier), _prisoner(prisoner), _base(base), _reportCase(PRISONER_REPORT_CASE_INTERROGATION)
{
	initUI();
}

/**
 * Initializes all the elements in the PrisonReportState screen.
 * @param reportCase - PrisonerReportCase for desired display.
 */
void PrisonReportState::initUI()
{
	_screen = false;

	// Create objects
	_window = new Window(this, 230, 140, 45, 30, POPUP_BOTH);
	_btnOk = new TextButton(80, 16, 64, 146);
	_btnDetails = new TextButton(80, 16, 176, 146);
	_txtTitle = new Text(230, 17, 45, 70);
	_txtDescription = new Text(230, 32, 45, 96);

	// Set palette
	std::string interface = "geoPrisonerReportNormal";
	if (_reportCase == PRISONER_REPORT_CASE_DEATH)
	{
		interface = "geoPrisonerReportDeath";
	}
	setInterface(interface);
	add(_window, "window", interface);
	add(_btnOk, "button", interface);
	add(_btnDetails, "button", interface);
	add(_txtTitle, "text1", interface);
	add(_txtDescription, "text2", interface);

	centerAllSurfaces();

	// Set up objects
	setWindowBackground(_window, interface);

	_btnOk->setText(tr("STR_OK"));
	_btnOk->onMouseClick((ActionHandler)&PrisonReportState::btnOkClick);
	_btnOk->onKeyboardPress((ActionHandler)&PrisonReportState::btnOkClick, Options::keyCancel);

	std::string btnName = "NONE", desription = "NONE";
	std::ostringstream ss;
	switch (_reportCase) {
	case PRISONER_REPORT_CASE_DEATH:
		btnName = "STR_GO_TO_BASE";
		ss << "STR_PRISONER_DIED_PT1";
		ss << _prisoner->getNameAndId();
		ss << "STR_PRISONER_DIED_PT2";
		desription = ss.str();
		break;
	case PRISONER_REPORT_CASE_INTERROGATION:
		btnName = "STR_VIEW_REPORTS";
		ss << "STR_PRISONER_INTERROGATED_PT1";
		ss << _prisoner->getNameAndId();
		ss << "STR_PRISONER_INTERROGATED_PT2";
		desription = ss.str();
		break;
	case PRISONER_REPORT_RECRUITING:
		btnName = "STR_VIEW_SOLDIER";
		ss << "STR_PRISONER_RECRUITING_PT1";
		ss << _prisoner->getNameAndId();
		ss << "STR_PRISONER_INTERROGATED_PT2";
		ss << _prisoner->getGeoscapeSoldier();
		ss << "STR_PRISONER_INTERROGATED_PT3";
		desription = ss.str();
		break;
	}
	_btnDetails->setText(tr(btnName));
	_btnDetails->onMouseClick((ActionHandler)&PrisonReportState::btnDetailsClick);
	_btnDetails->onKeyboardPress((ActionHandler)&PrisonReportState::btnDetailsClick, Options::keyOk);

	_txtTitle->setBig();
	_txtTitle->setAlign(ALIGN_CENTER);
	_txtTitle->setText(tr("STR_PRISON_REPORT"));

	_txtDescription->setAlign(ALIGN_CENTER);
	_txtDescription->setBig();
	_txtDescription->setWordWrap(true);
	_txtDescription->setText(tr(desription));
}

/**
 * return to the previous screen
 * @param action Pointer to an action.
 */
void PrisonReportState::btnOkClick(Action *)
{
	_game->popState();
}

/**
 * open the Ufopaedia to the entry about the Research.
 * @param action Pointer to an action.
 */
void PrisonReportState::btnDetailsClick(Action *)
{
	_game->popState();
	if (_reportCase == PRISONER_REPORT_CASE_DEATH)
	{
		_game->pushState(new PrisonManagementState(_base));
	}
	else if (_reportCase == PRISONER_REPORT_CASE_INTERROGATION)
	{
		std::string name;
		std::string bonusName;
		if (_bonus)
		{
			if (_bonus->getLookup().empty())
				bonusName = _bonus->getName();
			else
				bonusName = _bonus->getLookup();
			Ufopaedia::openArticle(_game, bonusName);
		}
		if (_research)
		{
			if (_research->getLookup().empty())
				name = _research->getName();
			else
				name = _research->getLookup();
			Ufopaedia::openArticle(_game, name);
		}
	}
	else if (_reportCase == PRISONER_REPORT_RECRUITING)
	{
		_game->pushState(new SoldiersState(_base));
	}
}

}

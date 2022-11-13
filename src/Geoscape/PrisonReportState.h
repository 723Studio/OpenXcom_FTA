#pragma once
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
#include "../Engine/State.h"

namespace OpenXcom
{
class Game;
class Window;
class TextButton;
class Text;
class Base;
class BasePrisoner;
class RuleResearch;
class Soldier;
class Globe;

enum PrisonerReportCase {PRISONER_REPORT_CASE_DEATH, PRISONER_REPORT_CASE_INTERROGATION, PRISONER_REPORT_RECRUITING};

/**
 * Window which report the player about some event with base prisoner (finished interrogation or recruiting, death, etc.)
 * Allow him to view information about the prisoner, recruited soldier or researched project (Ufopaedia).
 */
class PrisonReportState : public State
{
	Window *_window;
	Text *_txtTitle, *_txtDescription;
	TextButton *_btnDetails, *_btnOk;
	const RuleResearch * _research = nullptr, * _bonus = nullptr;
	BasePrisoner* _prisoner;
	Base* _base;
	Soldier* _soldier = nullptr;
	PrisonerReportCase _reportCase;
public:
	/// Creates the PrisonReportState for case of prisoner's death.
	PrisonReportState(BasePrisoner* prisoner, Base *base);
	/// Creates the PrisonReportState for case of finished interrogation.
	PrisonReportState(const RuleResearch * research, const RuleResearch *bonus, BasePrisoner* prisoner, Base* base);
	/// Creates the PrisonReportState for case of finished reqcruiting.
	PrisonReportState(Soldier *soldier, BasePrisoner* prisoner, Base* base);
	/// Inits the UI for the state.
	void initUI();
	/// Handler for clicking the OK button.
	void btnOkClick(Action *action);
	/// Handler for clicking the Report button.
	void btnDetailsClick(Action *action);
};

}

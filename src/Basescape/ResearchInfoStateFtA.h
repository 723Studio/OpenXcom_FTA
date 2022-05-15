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
class Window;
class TextButton;
class Text;
class TextList;
class Base;
class RuleResearch;
class ResearchProject;

/**
 * Window which allows changing of the number of assigned scientist to a project.
 */
class ResearchInfoStateFtA : public State
{
private:
	Base *_base;
	TextButton *_btnOk, *_btnCancel, *_btnAllocate, *_btnAbandon;
	Window *_window;
	Text *_txtTitle, *_txtAvailableScientist, *_txtAvailableSpace;
	Text *_txtStat1, *_txtStat2;
	ResearchProject *_project;
	RuleResearch *_rule;
	std::vector<Soldier *> _scientists;

	void getAssignedScientists();
	void buildUi();
	void setAssignedScientist();

public:
	/// Creates the ResearchProject state.
	ResearchInfoStateFtA(Base *base, RuleResearch *rule);
	ResearchInfoStateFtA(Base *base, ResearchProject *project);
	/// Cleans up the ResearchInfo state
	~ResearchInfoStateFtA();
	/// Handler for clicking the OK button.
	void btnOkClick(Action *action);
	/// Handler for clicking the Cancel button.
	void btnCancelClick(Action *action);
	/// Handler for clicking the Abandon button.
	void btnAbandonClick(Action *action);
};

}

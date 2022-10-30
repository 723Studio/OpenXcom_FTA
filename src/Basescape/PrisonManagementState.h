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

class TextButton;
class Window;
class Text;
class TextList;
class Base;

/**
 * PrisonManagement screen that lets the player manage prisoner actions.
 */
class PrisonManagementState : public State
{
private:
	Base *_base;
	TextButton *_btnOk;
	Window *_window;
	Text *_txtTitle, * _txtSpace, *_txtAvailable, * _txtAllocated, *_txtPrisoner, *_txtAgents, *_txtState;
	TextList *_lstPrisoners;

public:
	/// Creates the PrisonManagement state.
	PrisonManagementState(Base *base);
	/// Handler for clicking the OK button.
	void btnOkClick(Action *action);
	/// Handler for clicking the ResearchProject list.
	void onSelectPrisoner(Action *action);
	/// Fills the ResearchProject list with Base ResearchProjects.
	void fillPrisonList(size_t scrl);
	/// Updates the research list.
	void init() override;
};

}

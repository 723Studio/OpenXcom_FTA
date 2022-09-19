#pragma once
/*
 * Copyright 2010-2020 OpenXcom Developers.
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
* Covert operation screen that lets the player manage
* all the operations of a base.
*/
class IntelState : public State
{
private:
	Base *_base;
	TextButton *_btnPrisonCells, *_btnOk, *_btnAgents;
	Window *_window;
	Text *_txtTitle, *_txtAgentsAvailable, *_txtAgentsAllocated, *_txtIntelSpace, *_txtProject, *_txtAgents, *_txtProgress; //*_txtInterrogationSpaceAvailable,
	TextList *_lstProjects;
public:
	/// Creates the CovertOperation state.
	IntelState(Base* base);
	/// Cleans up the CovertOperation state.
	~IntelState();
	/// Handler for clicking the OK button.
	void btnOkClick(Action* action);
	/// Handler for clicking the New Operation button.
	void btnPrisonClick(Action* action);
	/// Handler for clicking the Agents button.
	void btnAgentsClick(Action* action);
	/// Handler for clicking the Intel Projects list.
	void onSelectProject(Action* action);
	void onProjectDetails(Action* action);
	/// Fills the CovertOperation list with Base CovertOperation.
	void fillProjectList(size_t scrl);
	/// Updates the CovertOperation list.
	void init() override;
};

}

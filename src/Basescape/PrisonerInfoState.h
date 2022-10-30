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
#include <set>
#include <utility>

namespace OpenXcom
{
class Soldier;
class Window;
class TextButton;
class Text;
class TextList;
class Base;
class RulePrisoner;
class BasePrisoner;
enum PrisonerState : int;

/**
 * Window which allows changing of the number of assigned scientist to a project.
 */
class PrisonerInfoState : public State
{
private:
	Base *_base;
	TextButton *_btnOk, *_btnCancel, *_btnAllocate, *_btnTerminate, *_btnHolder;
	TextButton *_btnInterrogate, *_btnRecruit, *_btnTorture, *_btnContain;
	TextButton* _group;
	Window *_window;
	Text *_txtTitle, *_txtAvailableAgents, *_txtAvailableSpace, *_txtActionsHeader;
	Text* _txtHealth, * _txtMorale, * _txtCooperation, * _txtAggressiveness;
	TextList *_lstAgents;
	BasePrisoner *_prisoner;
	const RulePrisoner*_rule;
	std::set<Soldier*> _agents;
	PrisonerState _display;
	
	int _workSpace;

public:
	/// Creates the ResearchProject state.
	PrisonerInfoState(Base *base, BasePrisoner *prisoner, const RulePrisoner*rule);
	/// Cleans up the ResearchInfo state
	~PrisonerInfoState();
	/// Handler for clicking the OK button.
	void btnOkClick(Action *action);
	/// Handler for clicking the Cancel button.
	void btnCancelClick(Action *action);
	/// Handler for clicking the Abandon button.
	void btnTerminateClick(Action *action);
	/// Handler for clicking the Allocate button.
	void btnAllocateClick(Action *action);
	/// Handler for clicking the Interrogate button.
	void btnInterrogateToggle(Action* action);
	/// Handler for clicking the Recruit button.
	void btnRecruitToggle(Action* action);
	/// Handler for clicking the Torture button.
	void btnTortureToggle(Action* action);
	/// Handler for clicking the Contain button.
	void btnContainToggle(Action* action);
	/// Fills the Scientists list with Assigned Scientists.
	void fillAgentsList(size_t scrl);
	/// Updates the research list.
	void init() override;
	void setAssignedAgents();
	std::set<Soldier*> getAgents() { return _agents; }
	void addAgent(Soldier * agent) { _agents.insert(agent); }
	void removeAgent(Soldier* agent) { _agents.erase(agent); }
	void setAgents(std::set<Soldier*> agent) { _agents = std::move(agent); }
	int getWorkspace() { return _workSpace; }
};

}

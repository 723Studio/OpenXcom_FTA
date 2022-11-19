#pragma once
/*
 * Copyright 2010-2019 OpenXcom Developers.
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
#include <map>

namespace OpenXcom
{

class TextButton;
class Window;
class Text;
class RuleEvent;
struct CustomAnswer;

/**
 * Displays info about a custom Geoscape event.
 */
class GeoscapeEventState : public State
{
private:
	Window *_window;
	Text *_txtTitle, *_txtMessage;
	TextButton *_btnOk;
	TextButton* _btnAnswerOne, * _btnAnswerTwo, * _btnAnswerThree, * _btnAnswerFour;
	Text* _txtTooltip;

	std::string _researchName;
	std::string _bonusResearchName;
	const RuleEvent &_eventRule;
	std::map<int, CustomAnswer> _customAnswers;
	std::string _currentTooltip;

	/// Helper performing event logic.
	void eventLogic();
	/// Function to spawn custom events based on the chosen button
	void spawnCustomEvents(int playerChoice);

public:
	/// Creates the GeoscapeEventState.
	GeoscapeEventState(const RuleEvent& eventRule);
	/// Cleans up the GeoscapeEventState.
	~GeoscapeEventState();
	/// Initializes the state.
	void init() override;
	/// Handler for clicking the OK button.
	void btnOkClick(Action *action);
	/// Handler for clicking the custom answer button.
	void btnAnswerOneClick(Action* action);
	/// Handler for clicking the custom answer button.
	void btnAnswerOneClickRight(Action* action);
	/// Handler for clicking the custom answer button.
	void btnAnswerTwoClick(Action* action);
	/// Handler for clicking the custom answer button.
	void btnAnswerTwoClickRight(Action* action);
	/// Handler for clicking the custom answer button.
	void btnAnswerThreeClick(Action* action);
	/// Handler for clicking the custom answer button.
	void btnAnswerThreeClickRight(Action* action);
	/// Handler for clicking the custom answer button.
	void btnAnswerFourClick(Action* action);
	/// Handler for clicking the custom answer button.
	void btnAnswerFourClickRight(Action* action);
	/// Handler for showing a mouseover tooltip
	void txtTooltipIn(Action* action);
	/// Handler for hiding a mouseover tooltip
	void txtTooltipOut(Action* action);
};

class GeoscapeEventAnswerInfoState : public State
{
private:
	Window* _window;
	Text* _txtDescription;
	TextButton* _btnOk;
public:
	/// Creates the GeoscapeEventState.
	GeoscapeEventAnswerInfoState(RuleEvent rule, std::string descr);
	/// Cleans up the GeoscapeEventState.
	~GeoscapeEventAnswerInfoState();
	/// Handler for clicking the OK button.
	void btnOkClick(Action* action);
};

}

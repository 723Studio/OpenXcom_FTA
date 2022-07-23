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

class Base;
class RuleResearch;
class Window;
class TextButton;
class Text;

/**
 * Screen which displays detailed info about manufacturing project.
 */
class ResearchProjectDetailsState : public State
{
private:
	Base *_base;
	const RuleResearch *_rule;
	Window *_window;
	TextButton *_btnOk;
	Text *_txtTitle, *_txtDifficulty, *_txtFunds, *_txtDestroyitem, *_txtReqStatsHeader, *_txtReqStats;
	std::string generateStatsList();
public:
	/// Creates the State.
	ResearchProjectDetailsState(Base *base, const RuleResearch *rule);
	/// Handler for the OK button.
	void btnOKClick(Action *action);
};

}

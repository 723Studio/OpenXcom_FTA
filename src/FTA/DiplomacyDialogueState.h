#pragma once
/*
 * Copyright 2010-2026 OpenXcom Developers.
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

#include <SDL.h>

namespace OpenXcom
{

class Base;
class DiplomacyFaction;
class InteractiveSurface;
class Text;
class TextList;
class RuleDiplomacyAction;

class DiplomacyDialogueState : public State
{
private:
	Base* _base;
	DiplomacyFaction* _faction;

	InteractiveSurface* _bg;
	Text* _txtReply;
	TextList* _lstActions;
	Text* _txtCloseHint;
	Text* _txtTooltip;

	std::vector<const RuleDiplomacyAction*> _visibleActions;
	bool _exitMode;
	SDL_Event* _exitModeIgnoreEvent;

	void setGreeting();
	void rebuildActionsList();
	bool isActionAvailable(const RuleDiplomacyAction& action) const;
	void applyAction(const RuleDiplomacyAction& action, Action* triggerAction);

	void closeIfExitMode(Action* action);
	void lstActionsMouseOver(Action* action);
	void lstActionsMouseOut(Action* action);
	void lstActionsClick(Action* action);

public:
	DiplomacyDialogueState(Base* base, DiplomacyFaction* faction);
	~DiplomacyDialogueState() override;
};

}

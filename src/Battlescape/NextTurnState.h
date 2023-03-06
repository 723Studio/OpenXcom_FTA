#pragma once
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
#include "../Engine/State.h"
#include "../Savegame/BattleUnit.h"
#include "../Mod/AlienDeployment.h"
#include "../Mod/RuleEnviroEffects.h"

namespace OpenXcom
{

class Window;
class Text;
class TextButton;
class SavedBattleGame;
class BattlescapeState;
class Timer;
class Surface;
class Unit;
class BattleUnit;
class Node;
class Position;

/**
 * Screen which announces the next turn.
 */
class NextTurnState : public State
{
private:
	static const int NEXT_TURN_DELAY = 500;
	Window *_window;
	Text *_txtTitle, *_txtTurn, *_txtSide, *_txtMessage, *_txtMessage2, *_txtMessage3, *_txtMessageReinforcements;
	TextButton *_btnBriefingReinforcements;
	SavedBattleGame *_battleGame;
	BattlescapeState *_state;
	Timer *_timer;
	Surface *_bg;
	int _currentTurn;
	bool _showBriefing;
	BriefingData _customBriefing;
	std::vector< std::vector<int> > _compliantBlocksMap;
	std::vector<Node*> _compliantNodesList;
	std::vector<Position> _compliantBlocksList;

	/// Applies a given environmental condition effects to a given faction.
	bool applyEnvironmentalConditionToFaction(UnitFaction faction, EnvironmentalCondition condition);
	/// Checks if bug hunt mode should be activated or not.
	void checkBugHuntMode();

	/// Runs reinforcements logic.
	bool determineReinforcements();
	/// Deploys the reinforcements, according to the alien reinforcements deployment rules.
	bool deployReinforcements(const ReinforcementsData &wave);
	/// Adds a reinforcement unit to the game.
	BattleUnit* addReinforcement(const ReinforcementsData &wave, Unit *rules, int alienRank, bool civilian);
	/// Finds a spot near a friend to spawn at.
	bool placeReinforcementNearFriend(BattleUnit *unit);
public:
	/// Creates the Next Turn state.
	NextTurnState(SavedBattleGame *battleGame, BattlescapeState *state);
	/// Cleans up the Next Turn state.
	~NextTurnState();
	/// Initialization
	void init() override;
	/// Handler for clicking anything.
	void handle(Action *action) override;
	/// Handles the timer.
	void think() override;
	/// Closes the window.
	void close();
	void resize(int &dX, int &dY) override;
	/// Handler for clicking the Reinforcements Briefing button.
	void btnBriefingReinforcementsClick(Action *action);
};

}

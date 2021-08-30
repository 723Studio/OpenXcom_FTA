#pragma once
/*
 * Copyright 2010-2021 OpenXcom Developers.
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


class InteractiveSurface;
class HackingView;
class Text;
class Bar;
class Timer;
//class BattleItem;
class BattleUnit;
class BattleObject;
class Tile;
class TileEngine;
struct BattleAction;
class HackingNode;
class ConsoleTextManager;

typedef void (State::* LogHandler)(HackingNode*);

/**
 * The Hacking Tool User Interface. Hacking tool is an item for hacking stationary computers and enemy drones.
 */
class HackingState : public State
{
	Surface* _bg;
	HackingView* _hackingView;
	Text* _consoleTxt;
	Text* _txtTimeUnits, * _txtHealth;
	Text* _numTimeUnits, * _numHealth;
	Bar* _barTimeUnits, * _barHealth;
	InteractiveSurface* _exitButton;
	Tile* _targetTile;
	BattleUnit* _targetUnit = nullptr;
	BattleObject* _targetObject = nullptr;
//	BattleItem* _item; // we may need it later
	BattleAction* _action;
	Timer* _timerAnimate;
	TileEngine* _tileEngine;
	
	ConsoleTextManager* _consoleManager;
	int _timeUnits{ 0 }, _maxTimeUnits{ 0 }, _health{ 0 }, _maxHealth{ 0 };
	int _tuBaseCost, _tuFirewallCost, _hpFirewallCost;


	/// Updates the hacking device info.
	void init() override;
	/// Handler for the exit button.
	void onExitClick(Action* action);
	/// Handler for the node click.
	void onNodeClick(Action* action);
	///  Shows nodes that are linked to the current node.
	void revealNeighbours(HackingNode* node);
	/// Log to the console if the node has special properties
	void notifyState(HackingNode* node);
	/// Updates the hacking tool interface.
	void update();
	/// Handles Minigame animation.
	void animate();
public:
	/// Creates the HackingState.
	//HackingState(BattleAction* action, BattleUnit* targetUnit, TileEngine* tileEngine);
	HackingState(BattleAction* action, Tile* targetTile, TileEngine* tileEngine);
	~HackingState();
	/// Handler for right-clicking anything.
	void handle(Action* action) override;
	/// Handles timers.
	void think() override;
};

} // namespace

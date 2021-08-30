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
#include "BattleState.h"

namespace OpenXcom
{

class BattlescapeGame;
class BattleUnit;
class BattleItem;
class Tile;
class Game;

/**
 * A hacking battle state.
 */
class HackingBState : public BattleState
{
private:
	Game* _game;
	BattleUnit* _unit;
	BattleItem* _item;
	Tile* _targetTile;
	bool _initialized;
public:
	/// Creates a new Hacking battle state.
	HackingBState(BattlescapeGame* parent, BattleAction action, Game* game);
	/// Cleans up the Hacking battle state.
	~HackingBState();
	/// Initializes the state.
	void init() override;
	/// Runs state functionality every cycle.
	void think() override;
};

}// namespace

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
#include "HackingBState.h"
#include "HackingState.h"
#include "BattlescapeGame.h"
#include "BattlescapeState.h"
#include "TileEngine.h"
#include "InfoboxState.h"
#include "Map.h"
#include "Camera.h"
#include "../Savegame/SavedGame.h"
#include "../Savegame/SavedBattleGame.h"
#include "../Savegame/Tile.h"
#include "../Engine/Game.h"
#include "../Engine/RNG.h"
#include "../Engine/Language.h"
#include "../Engine/Sound.h"
#include "../Mod/Mod.h"
#include "../Savegame/BattleUnitStatistics.h"

namespace OpenXcom
{

	/**
	 * Sets up a HackingBState.
	 */
	HackingBState::HackingBState(BattlescapeGame* parent, BattleAction action, Game* game) : BattleState(parent, action), _unit(0), _target(0), _item(0), _initialized(false), _game(game)
	{
	}

	/**
	 * Deletes the HackingBState.
	 */
	HackingBState::~HackingBState()
	{
	}

	/**
	 * Initializes the sequence:
	 * - checks if the action is valid,
	 * - adds a hacking minigame to the world.
	 */
	void HackingBState::init()
	{
		if (_initialized) return;
		_initialized = true;

		_item = _action.weapon;

		if (!_item) // can't do hacking without a hacking device
		{
			_parent->popState();
			return;
		}

		if (!_parent->getSave()->getTile(_action.target)) // invalid target position
		{
			_parent->popState();
			return;
		}

		_unit = _action.actor;

		int distanceSq = _action.actor->distance3dToPositionSq(_action.target);
		if (_action.weapon->getRules()->isOutOfRange(distanceSq))
		{
			// out of range
			_action.result = "STR_OUT_OF_RANGE";
			_parent->popState();
			return;
		}

		_target = _parent->getSave()->getTile(_action.target)->getUnit();

		if (!_target) // invalid target
		{
			_parent->popState();
			return;
		}

		if (!_action.spendTU(&_action.result)) // not enough time units
		{
			_parent->popState();
			return;
		}

		//int height = _target->getFloatHeight() + (_target->getHeight() / 2) - _parent->getSave()->getTile(_action.target)->getTerrainLevel();
		//Position voxel = _action.target.toVoxel() + Position(8, 8, height);
		//_parent->statePushFront(new ExplosionBState(_parent, voxel, BattleActionAttack{ _action.type, _action.actor, _action.weapon, _action.weapon }));
		TileEngine* tileEngine = _game->getSavedGame()->getSavedBattle()->getTileEngine();
		_game->pushState(new HackingState(&_action, _target, tileEngine));
	}


	/**
	 * After the explosion animation is done doing its thing,
	 * restore the camera/cursor.
	 */
	void HackingBState::think()
	{
		if (_action.cameraPosition.z != -1)
		{
			_parent->getMap()->getCamera()->setMapOffset(_action.cameraPosition);
			_parent->getMap()->invalidate();
		}
		if (_parent->getSave()->getSide() == FACTION_PLAYER || _parent->getSave()->getDebugMode())
		{
			_parent->setupCursor();
		}
		_parent->popState();
		_parent->cancelCurrentAction();
	}

}

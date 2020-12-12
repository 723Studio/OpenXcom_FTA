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
#include "BattleState.h"
#include "Position.h"

namespace OpenXcom
{

class BattlescapeGame;
class BattleUnit;
class BattleItem;
class Tile;
struct RuleDamageType;

/**
 * Explosion state not only handles explosions, but also bullet impacts!
 * Refactoring tip : ImpactBState.
 */
class ExplosionBState : public BattleState
{
private:
	int _explosionCounter;
	int _terrainMeleeTilePart;
	BattleActionAttack _attack;
	Position _center;
	const RuleDamageType *_damageType;
	Tile *_tile;
	BattleUnit *_targetPsiOrHit;
	int _power;
	int _radius;
	int _range;
	bool _areaOfEffect, _lowerWeapon, _hit, _psi;

	/// Calculates the effects of the explosion.
	void explode();
	/// Set new value to reference if new value is not equal -1.
	void optValue(int &oldValue, int newValue) const;
public:
	/// Creates a new ExplosionBState class.
	ExplosionBState(BattlescapeGame *parent, Position center, BattleActionAttack attack, Tile *tile = 0, bool lowerWeapon = false, int range = 0, int explosionCounter = 0, int terrainMeleeTilePart = 0);
	/// Cleans up the ExplosionBState.
	~ExplosionBState();
	/// Initializes the state.
	void init() override;
	/// Handles a cancel request.
	void cancel() override;
	/// Runs state functionality every cycle.
	void think() override;

};

}

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
#include "Position.h"

namespace OpenXcom
{

class PathfindingOpenSet;
struct OpenSetEntry;

/**
 * Cost of one step.
 * As this is 2d point value it is not totally ordered.
 */
struct PathfindingCost
{
	Sint16 time = 0;
	Sint16 energy = 0;

	/// Default constructor.
	PathfindingCost() = default;

	/// Constructor from ints.
	PathfindingCost(int t, int e) : time(t), energy(e) {}

	PathfindingCost operator+(PathfindingCost c) const
	{
		return {
			static_cast<Sint16>(time + c.time),
			static_cast<Sint16>(energy + c.energy),
		};
	}

	bool operator<=(PathfindingCost c) const
	{
		return time <= c.time && energy <= c.energy;
	}
	bool operator>=(PathfindingCost c) const
	{
		return time >= c.time && energy >= c.energy;
	}
	bool operator<(PathfindingCost c) const
	{
		return time < c.time && energy < c.energy;
	}
	bool operator>(PathfindingCost c) const
	{
		return time > c.time && energy > c.energy;
	}
};

/**
 * Result of one step in pathfinding algorithm.
 */
struct PathfindingStep
{
	/// Real cost of moving one step.
	PathfindingCost cost;
	/// Pathfinding penalty for given step.
	PathfindingCost penalty = { };
	/// Final position of one step.
	Position pos = { };
};

/**
 * A class that holds pathfinding info for a certain node on the map.
 */
class PathfindingNode
{
private:
	Position _pos;
	PathfindingCost _tuCost;
	PathfindingNode* _prevNode;
	int _prevDir;
	/// Approximate cost to reach goal position.
	Sint16 _tuGuess;
	/// Is best path find for this tile.
	bool _checked;
	// Invasive field needed by PathfindingOpenSet
	Uint8 _openentry;
	friend class PathfindingOpenSet;
public:
	/// Creates a new PathfindingNode class.
	PathfindingNode(Position pos);
	/// Cleans up the PathfindingNode.
	~PathfindingNode();
	/// Gets the node position.
	Position getPosition() const;
	/// Resets the node.
	void reset();
	/// Is checked?
	bool isChecked() const;
	/// Marks the node as checked.
	void setChecked() { _checked = true; }
	/// Gets the TU cost.
	PathfindingCost getTUCost(bool missile) const { return missile ? PathfindingCost{} : _tuCost; }
	/// Gets the previous node.
	PathfindingNode* getPrevNode() const;
	/// Gets the previous walking direction.
	int getPrevDir() const;
	/// Is this node already in a PathfindingOpenSet?
	bool inOpenSet() const { return (_openentry != 0); }
	/// Gets the approximate cost to reach the target position.
	int getTUGuess() const { return _tuGuess; }

	#ifdef __MORPHOS__
	#undef connect
	#endif

	/// Connects to previous node along the path.
	void connect(PathfindingCost cost, PathfindingNode* prevNode, int prevDir, Position target);
	/// Connects to previous node along a visit.
	void connect(PathfindingCost cost, PathfindingNode* prevNode, int prevDir);
};

/**
 * Compares PathfindingNode pointers based on TU cost.
 */
class MinNodeCosts
{
public:
	/**
	 * Compares nodes @a *a and @a *b.
	 * @param a Pointer to first node.
	 * @param b Pointer to second node.
	 * @return True if node @a *a must come before @a *b.
	 */
	bool operator()(const PathfindingNode *a, const PathfindingNode *b) const
	{
		return a->getTUCost(false) < b->getTUCost(false);
	}
};

}

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
#include <queue>
#include <SDL_stdinc.h>

namespace OpenXcom
{

class PathfindingNode;

struct OpenSetEntry
{
	PathfindingNode *_node;
	Sint16 _cost;
	Uint8 _openentry;
};

/**
 * Helper class to compare entries through pointers.
 */
class EntryCompare
{
public:
	/**
	 * Compares entries @a *a and @a *b.
	 * @param a Pointer to first entry.
	 * @param b Pointer to second entry.
	 * @return True if entry @a *b must come before @a *a.
	 */
	bool operator()(const OpenSetEntry& a, const OpenSetEntry& b) const
	{
		return b._cost < a._cost;
	}
};

/**
 * A class that holds references to the nodes to be examined in pathfinding.
 */
class PathfindingOpenSet
{
public:
	/// Cleans up the set and frees allocated memory.
	~PathfindingOpenSet();
	/// Gets the next node to check.
	PathfindingNode *pop();
	/// Adds a node to the set.
	void push(PathfindingNode *node);
	/// Is the set empty?
	bool empty() const { return _queue.empty(); }

private:
	std::priority_queue<OpenSetEntry, std::vector<OpenSetEntry>, EntryCompare> _queue;

	/// Removes reachable discarded entries.
	void removeDiscarded();
};

}

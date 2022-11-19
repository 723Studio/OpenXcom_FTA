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
#include <assert.h>
#include "PathfindingOpenSet.h"
#include "PathfindingNode.h"

namespace OpenXcom
{

/**
 * Cleans up all the entries still in set.
 */
PathfindingOpenSet::~PathfindingOpenSet()
{

}

/**
 * Keeps removing all discarded entries that have come to the top of the queue.
 */
void PathfindingOpenSet::removeDiscarded()
{
	while (!_queue.empty() && _queue.top()._node->_openentry != _queue.top()._openentry)
	{
		_queue.pop();
	}
}

/**
 * Gets the node with the least cost.
 * After this call, the node is no longer in the set. It is an error to call this when the set is empty.
 * @return A pointer to the node which had the least cost.
 */
PathfindingNode *PathfindingOpenSet::pop()
{
	assert(!empty());

	PathfindingNode *nd = _queue.top()._node;
	_queue.pop();
	nd->_openentry = 0;

	// Discarded entries might be visible now.
	removeDiscarded();
	return nd;
}

/**
 * Places the node in the set.
 * If the node was already in the set, the previous entry is discarded.
 * It is the caller's responsibility to never re-add a node with a worse cost.
 * @param node A pointer to the node to add.
 */
void PathfindingOpenSet::push(PathfindingNode *node)
{
	assert(node->_openentry != 255u);

	OpenSetEntry entry = {};
	entry._node = node;
	entry._cost = node->getTUCost(false).time * 4 + node->getTUGuess(); //HACK: this is not real cost, more rough approximation for algorithm, as bonus `getTUGuess` work more like gravity/potential than normal cost.
	entry._openentry = ++node->_openentry; // next unique number, used to check if old recode is still valid.
	_queue.push(entry);
}


}

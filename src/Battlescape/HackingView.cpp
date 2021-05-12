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

#include "HackingView.h"
#include "HackingState.h"
#include "../Engine/RNG.h"

namespace OpenXcom
{

struct Point
{
	Sint16 x, y;
	Point(Sint16 x, Sint16 y) : x(x), y(y) {}
};


constexpr int hackingGridStartX{ 66 };
constexpr int hackingGridStartY{ 29 };
constexpr int hackingNodeOffsetX{ 18 };
constexpr int hackingGridHeight{ 9 };
constexpr int hackingGridWidth{ 36 };
constexpr int gridLineColor{ 50 };



const int HackingNode::_nodeBlob[7][6] =
{
	{0,0,1,1,0,0},
	{0,1,2,2,1,0},
	{1,2,3,3,2,1},
	{1,3,5,5,3,1},
	{1,2,3,3,2,1},
	{0,1,2,2,1,0},
	{0,0,1,1,0,0}
};


void HackingNode::draw()
{
	// set the color of the node
	switch (_nodeState)
	{
	case NodeState::ACTIVATED:
	{
		_color = (Uint8)NodeColor::GREEN;
		break;
	}
	case NodeState::LOCKED:
	{
		_color = (Uint8)NodeColor::RED;
		break;
	}
	case NodeState::IMPENETRABLE:
	{
		_color = (Uint8)NodeColor::YELLOW;
		break;
	}
	case NodeState::TARGET:
	{
		_color = (Uint8)NodeColor::BLUE;
		break;
	}
	case NodeState::DISABLED:
	default:
		_color = (Uint8)NodeColor::GRAY;
	};

	// draw node blob
	for (int y = 0; y < 7; ++y)
	{
		for (int x = 0; x < 6; ++x)
		{
			Uint8 pixelOffset = _nodeBlob[y][x];
			if (pixelOffset == 0)
			{
				continue;
			}
			else
			{
				Uint8 color = _color - pixelOffset - _frame;
				setPixel(x, y, color);
			}
		}
	}
}

void HackingNode::animate()
{
	if (_nodeState == NodeState::LOCKED || _nodeState == NodeState::TARGET)
	{
		_frame++;
		if (_frame > 1)
		{
			_frame = 0;
		}
		_redraw = true;
	}
}

/**
 * Initializes the Hacking view.
 * @param w The HackingView width.
 * @param h The HackingView height.
 * @param x The HackingView x origin.
 * @param y The HackingView y origin.
 */
HackingView::HackingView(int w, int h, int x, int y) : Surface(w, h, x, y), _frame(0)
{
	_redraw = true;

	// Create node grid 
	for (int row = 0; row < std::size(_nodeArray); ++row)
		for (int col = 0; (col < std::size(_nodeArray[row]) - 1 + row % 2); ++col) // 1 less for even row and full length for odd
		{
			_nodeArray[row][col] = new HackingNode(hackingGridStartX + hackingGridWidth * col - hackingNodeOffsetX * (row % 2),
				hackingGridStartY + hackingGridHeight * row,
				row, col);
			_nodeArray[row][col]->setVisible(false);
		}
}

HackingView::~HackingView()
{
}

/**
 * Generates the starting state of the Hacking view.
 * Sets up starting point, objectives and defenses
 */
void HackingView::initField()
{
	// Set target node 
	int finishRow = RNG::generate(0, 5) * 2 + 1;
	_nodeArray[finishRow][4]->setState(NodeState::TARGET);

	// Set up defence
	for (int col = 1; col <= 3; ++col)
	{
		int rndOffset = RNG::generate(0, 3) * 2;
		_nodeArray[1 + rndOffset][col]->setState(NodeState::IMPENETRABLE);
		_nodeArray[3 + rndOffset][col]->setState(NodeState::LOCKED);
		_nodeArray[5 + rndOffset][col]->setState(NodeState::IMPENETRABLE);
	}

	// Set starting node 
	int startRow = RNG::generate(0, 1) * 2 + 5;
	_nodeArray[startRow][0]->setState(NodeState::ACTIVATED);
	_nodeArray[startRow][0]->setVisible(true);
	_nodeArray[startRow - 1][0]->setVisible(true);
	_nodeArray[startRow + 1][0]->setVisible(true);
}

/**
 * Draws the links between the nodes on the HackingView. Nodes are drawn separately
 */
void HackingView::draw()
{
	for (auto link : _linkArray)
	{
		drawLine(link.first.x, link.first.y, link.second.x, link.second.y, gridLineColor - _frame);
	}
}

/**
 * Updates the hacking view animation.
 */
void HackingView::animate()
{
	_frame++;
	if (_frame > 2)
	{
		_frame = 0;
	}
	_redraw = true;
	for (int row = 0; row < std::size(_nodeArray); ++row)
		for (int col = 0; (col < std::size(_nodeArray[row])); ++col) 
		{
			if (_nodeArray[row][col])
			{
				_nodeArray[row][col]->animate();
			}
		}
}

/**
 * Adds a single line to the array of links that connect centers of the nodes using provided coordinates
 * @param x1 
 * @param y1 
 * @param x2 
 * @param y2 
*/
void HackingView::addLink(Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2)
{
	Sint16 xOffset = getX() - 3;
	Sint16 yOffset = getY() - 3;
	if (x1 > x2) // lines are drawn wrong from left to right, so we make sure they are all drawn right to left
	{
		_linkArray.push_back(std::make_pair(Point(x1 - xOffset, y1 - yOffset), Point(x2 - xOffset, y2 - yOffset)));
	}
	else
	{
		_linkArray.push_back(std::make_pair(Point(x2 - xOffset, y2 - yOffset), Point(x1 - xOffset, y1 - yOffset)));
	}
}

/**
 * Adds links from the current node to the surrounding activated nodes.
 * @param node Pointer to the current node.
 */
void HackingView::addLinks(HackingNode* node)
{
	// Get node position
	int Row = node->getGridRow();
	int Col = node->getGridCol();
	int maxRow = std::size(_nodeArray) - 1;
	int maxCol = std::size(_nodeArray[0]) - 1;
	Col -= Row % 2; // adjust column position if we are on an odd row

	for (int currRow = Row - 1; currRow <= Row + 1 && currRow <= maxRow;)
	{
		if (currRow >= 0)
		{
			// check and link the node to the upper/lower left
			if (Col >= 0)
			{
				if (_nodeArray[currRow][Col] && _nodeArray[currRow][Col]->getState() == NodeState::ACTIVATED)
				{
					addLink(node->getX(), node->getY(), _nodeArray[currRow][Col]->getX(), _nodeArray[currRow][Col]->getY());
				}
			}
			// check and link the node to the upper/lower right
			if (Col < maxCol)
			{
				if (_nodeArray[currRow][Col + 1] && _nodeArray[currRow][Col + 1]->getState() == NodeState::ACTIVATED)
				{
					addLink(node->getX(), node->getY(), _nodeArray[currRow][Col + 1]->getX(), _nodeArray[currRow][Col + 1]->getY());
				}
			}
		}
		currRow += 2;
	}
}

/**
 * Activates the current node and connects it to the surrounding activated nodes with animated lines
 * @param node Current node
*/
void HackingView::activateNode(HackingNode* node)
{
	node->setState(NodeState::ACTIVATED);
	addLinks(node);
}

/**
 * Sets visible one node above and one below the current node.
 * Must only be used on the firewall center to show the whole firewall once it's been breached
 * @param node Current node
*/
void HackingView::revealFirewall(HackingNode* node)
{
	_nodeArray[node->getGridRow() - 2][node->getGridCol()]->setVisible(true);
	_nodeArray[node->getGridRow() + 2][node->getGridCol()]->setVisible(true);
}

} // namespace

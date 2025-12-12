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
#include "../Engine/InteractiveSurface.h"

namespace OpenXcom
{
// Classes forward declarations

struct Point
{
	Sint16 x, y;
	Point(Sint16 x, Sint16 y) : x(x), y(y) {}
};

enum class NodeState
{
	DISABLED,
	ACTIVATED,
	LOCKED,
	IMPENETRABLE,
	TARGET
};

enum class NodeColor
{
	GRAY = 7,
	RED = 40,
	GREEN = 54,
	YELLOW = 21,
	BLUE = 215
};

/**
 * Class that represents a clickable Node that is drawn on the Hacking view.
 * It can have one of 5 states described in NodeState and has a corresponding color
 * defined in NodeColor
 */
class HackingNode : public InteractiveSurface
{
	static const int _nodeBlob[7][6];
	Uint8 _color = (Uint8)NodeColor::GRAY;
	int _gridRow, _gridCol, _frame;
	NodeState _nodeState{ NodeState::DISABLED };

public:
	HackingNode(Sint16 x, Sint16 y, int gridX, int gridY) : InteractiveSurface(6, 7, x, y), _gridRow(gridX), _gridCol(gridY), _frame(0)
	{
		_redraw = true;
	};
	void draw() override;
	void animate();
	int getColor() const { return _color; }
	int getGridRow() const { return _gridRow; }
	int getGridCol() const { return _gridCol; }
	NodeState getState() const { return _nodeState; }
	void setState(NodeState state) { _nodeState = state; }
};

/**
 * Displays a view of hacking minigame. Contains a field where hacking nodes are placed in a grid pattern
 * and an array of animated lines that connect the nodes once they have been activated.
 * Class is responsible for drawing the hacking node grid and updating it according to player actions that are handled by HackingState
 */
class HackingView : public Surface
{
	std::vector<std::pair<Point, Point>> _linkArray{};
	HackingNode* _nodeArray[13][5]{ };
	int _frame;

	/// Add a line on the field connecting two points
	void addLink(Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2);
	/// Add lines from the current node to its neigbours that has already been activated
	void addLinks(HackingNode* node);
public:
	/// Create the HackingView
	HackingView(int w, int h, int x, int y);
	~HackingView();
	/// Generate the starting state of the hacking grid
	void initField();
	/// Draw the Hacking view
	void draw() override;
	/// Update the animation of the hacking view elements
	void animate();
	/// Set the node to active state and update its links
	void activateNode(HackingNode* node);
	/// Reveal the entire firewall structure after its weak point has been breached
	void revealFirewall(HackingNode* node);
	int getGridHeight() const { return std::size(_nodeArray); }
	int getGridWidth() const { return std::size(_nodeArray[0]); }
	HackingNode* const getNode(int row, int col) const { return _nodeArray[row][col]; }
	
};

} //namespace

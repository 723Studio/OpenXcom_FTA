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

#include "HackingState.h"
#include "HackingView.h"
//#include "BattlescapeGame.h"
#include "../Engine/InteractiveSurface.h"
#include "../Engine/Game.h"
#include "../Engine/Action.h"
#include "../Engine/Timer.h"
#include "../Engine/Screen.h"
#include "../Engine/Options.h"
#include "../Interface/Text.h"
//#include "../Savegame/BattleUnit.h"
#include "../Mod/Mod.h"
#include "../Savegame/SavedGame.h"
#include "../Savegame/SavedBattleGame.h"


namespace OpenXcom
{

constexpr int hackingGridStartX { 66 };
constexpr int hackingGridStartY { 29 };
constexpr int hackingNodeOffsetX { 18 };
constexpr int hackingGridHeight { 9 };
constexpr int hackingGridWidth { 36 };

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
	BLUE = 213
};

class HackingNode : public InteractiveSurface
{
	static const int _nodeBlob[7][6];
	Uint8 _color = 40;
	int _gridRow, _gridCol;
	NodeState _nodeState{ NodeState::DISABLED };

public:
	HackingNode(Sint16 x, Sint16 y, int gridX, int gridY);
	void draw() override;
	int getColor() const { return _color; }
	int getGridRow() const { return _gridRow; }
	int getGridCol() const { return _gridCol; }
	void setColor(Uint8 color) { _color = color; }
	NodeState getState() const { return _nodeState; }
	void setState(NodeState state) { _nodeState = state; }	
};

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


HackingNode::HackingNode(Sint16 x, Sint16 y, int gridX, int gridY) : InteractiveSurface(6, 7, x, y), _gridRow(gridX), _gridCol(gridY)
{
	_redraw = true;
}

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
				Uint8 color = _color - pixelOffset;
				setPixel(x, y, color);
			}
		}
	}
}

/**
 * Initializes the Hacking State.
 * @param game Pointer to the core game.
 * @param action Pointer to an action.
 */
HackingState::HackingState(BattleAction* action) : _action(action)
{
	if (Options::maximizeInfoScreens)
	{
		Options::baseXResolution = Screen::ORIGINAL_WIDTH;
		Options::baseYResolution = Screen::ORIGINAL_HEIGHT;
		_game->getScreen()->resetDisplay(false);
	}
	_bg = new Surface(320, 200);
	_hackingView = new HackingView(164, 121, 41, 26, _game);//, _action->actor);
	
	_exitButton = new InteractiveSurface(20, 20, 255, 157);
	_consoleTxt = new Text(55, 108, 215, 26);
	_consoleTxt->setWordWrap(true);
	_consoleTxt->setSmall();
	_consoleTxt->setText("It's a long text that I need to write to see how it fits on the console screen. I need more text here. And even more text.");
	

	if (_game->getScreen()->getDY() > 50)
	{
		_screen = false;
	}

	// Set palette
	_game->getSavedGame()->getSavedBattle()->setPaletteByDepth(this);

	add(_bg);
	add(_hackingView);
	
	add(_consoleTxt);
	//add(_exitButton, "buttonExit", "hackingTool", _bg); // TODO: add new interface to rulesets
	add(_exitButton);

	// Create node grid 
	for (int row = 0; row < std::size(_nodeArray); ++row)
		for (int col = 0; (col < std::size(_nodeArray[row])- 1 + row % 2) ; ++col) // 1 less for even row and full length for odd
		{
			_nodeArray[row][col] = new HackingNode(hackingGridStartX + hackingGridWidth * col - hackingNodeOffsetX *(row % 2),
												hackingGridStartY + hackingGridHeight * row,
												row, col);
			add(_nodeArray[row][col]);
			_nodeArray[row][col]->setVisible(false);
			_nodeArray[row][col]->onMouseClick((ActionHandler)&HackingState::onNodeClick);
		}

	// TODO: Set target node (random)
	_nodeArray[3][4]->setState(NodeState::TARGET);

	// TODO: Set up defence
	_nodeArray[5][1]->setState(NodeState::IMPENETRABLE);
	_nodeArray[1][1]->setState(NodeState::IMPENETRABLE);
	_nodeArray[3][1]->setState(NodeState::LOCKED);
	_nodeArray[9][2]->setState(NodeState::IMPENETRABLE);
	_nodeArray[5][2]->setState(NodeState::IMPENETRABLE);
	_nodeArray[7][2]->setState(NodeState::LOCKED);
	
	// TODO: set starting node (random)
	_nodeArray[3][0]->setVisible(true);
	_nodeArray[3][0]->setState(NodeState::ACTIVATED);
	showNeighbours(_nodeArray[3][0]);


	centerAllSurfaces();

	Surface* backgroundSprite = 0;

	// TODO: see if we need to use something like this:
	//if (!_item->getRules()->getMediKitCustomBackground().empty())
	//{
	//	backgroundSprite = _game->getMod()->getSurface(_item->getRules()->getMediKitCustomBackground(), false);
	//}
	if (!backgroundSprite)
	{
		backgroundSprite = _game->getMod()->getSurface("HackingUI");
	}

	backgroundSprite->blitNShade(_bg, 0, 0);

	_exitButton->onMouseClick((ActionHandler)&HackingState::onExitClick);
	_exitButton->onKeyboardPress((ActionHandler)&HackingState::onExitClick, Options::keyCancel);

	// TODO: enable animation handling when it's ready
	_timerAnimate = new Timer(125);
	_timerAnimate->onTimer((StateHandler)&HackingState::animate);
	_timerAnimate->start();

	update();
}

HackingState::~HackingState()
{
	delete _timerAnimate;
}

/**
 * Closes the window on right-click.
 * @param action Pointer to an action.
 */
void HackingState::handle(Action* action)
{
	State::handle(action);
	if (action->getDetails()->type == SDL_MOUSEBUTTONDOWN && _game->isRightClick(action))
	{
		onExitClick(action);
	}
}

/**
 * Updates hacking state.
 */
void HackingState::update()
{
	// TODO: update hacking interface
	_hackingView->invalidate();
}

/**
 * Animation handler. Updates the minigame view animation.
 */
void HackingState::animate()
{
	_hackingView->animate(); // TODO: uncomment when ready
}

/**
 * Handles timers.
 */
void HackingState::think()
{
	State::think();
	_timerAnimate->think(this, 0); // TODO: uncomment when ready
}

/**
 * Exits the screen.
 * @param action Pointer to an action.
 */
void HackingState::onExitClick(Action*)
{
	if (Options::maximizeInfoScreens)
	{
		Screen::updateScale(Options::battlescapeScale, Options::baseXBattlescape, Options::baseYBattlescape, true);
		_game->getScreen()->resetDisplay(false);
	}
	_game->popState();
}

void HackingState::onNodeClick(Action* action)
{
	HackingNode* node = dynamic_cast<HackingNode*>(action->getSender());
	if (!node) { return; }

	switch (node->getState())
	{
	case NodeState::TARGET:
	{
		onExitClick(0);
		break;
	}
	case NodeState::DISABLED:
	{
		node->setState(NodeState::ACTIVATED);
		showNeighbours(node);
		addLinks(node);
		_consoleTxt->setText(">Proceeding...");
		break;
	}
	case NodeState::LOCKED:
	{
		node->setState(NodeState::ACTIVATED);
		showNeighbours(node);
		addLinks(node);
		_consoleTxt->setText(">Breaking in...");
		break;
	}
	case NodeState::IMPENETRABLE:
	{
		_consoleTxt->setText(">Can't proceed!");
		break;
	}
	case NodeState::ACTIVATED:
	default:
		break;
	}
		
	update();
	
}

/**
 * Shows nodes that are linked to the current node.
 * @param node Pointer to the current node.
 */
void HackingState::showNeighbours(HackingNode* node)
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
			// show the node to the upper/lower left
			if (Col >= 0)
			{
				if (_nodeArray[currRow][Col])
				{
					_nodeArray[currRow][Col]->setVisible(true);
				}
			}
			// show the node to the upper/lower right
			if (Col < maxCol)
			{
				if (_nodeArray[currRow][Col + 1])
				{
					_nodeArray[currRow][Col + 1]->setVisible(true);
				}
			}
		}
		currRow += 2;
	}
}

/**
 * Adds links from the current node to the surrounding activated nodes.
 * @param node Pointer to the current node.
 */
void HackingState::addLinks(HackingNode* node)
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
					_hackingView->addLink(node->getX(), node->getY(), _nodeArray[currRow][Col]->getX(), _nodeArray[currRow][Col]->getY());
				}
			}
			// check and link the node to the upper/lower right
			if (Col < maxCol)
			{
				if (_nodeArray[currRow][Col + 1] && _nodeArray[currRow][Col + 1]->getState() == NodeState::ACTIVATED)
				{
					_hackingView->addLink(node->getX(), node->getY(), _nodeArray[currRow][Col + 1]->getX(), _nodeArray[currRow][Col + 1]->getY());
					
				}
			}
		}
		currRow += 2;
	}
}

} // namespace

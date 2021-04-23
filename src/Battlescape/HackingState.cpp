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
constexpr int hackingViewStartY { 29 };
constexpr int hackingNodeOffsetX { 18 };
constexpr int hackingGridHeight { 9 };
constexpr int hackingGridWidth { 36 };

enum class NodeState
{
	DISABLED,
	ACTIVATED,
	LOCKED,
	IMPENETRABLE
};

enum class NodeColor
{
	GRAY = 7,
	RED = 40,
	GREEN = 54,
	YELLOW = 21
};

class HackingNode : public InteractiveSurface
{
	static const int _nodeBlob[7][6];
	Uint8 _color = 40;
	//Sint16 _x, _y;
	NodeState _nodeState{ NodeState::DISABLED };
	std::array<HackingNode*, 4> _neighbourNodes{};
public:
	HackingNode(Sint16 x, Sint16 y);
	void draw() override;
//	void mouseClick(Action* action, State* state) override;
	int getColor() const { return _color; }
	void setColor(Uint8 color) { _color = color; }
	NodeState getState() { return _nodeState; }
	void setState(NodeState state)
	{
		if(_nodeState == state) return;
		else
		{
			_nodeState = state;
			_redraw = true;
		}
	}
	void setNeighbour(int index, HackingNode* node) { _neighbourNodes[index] = node; }
	void activateNeighbours();
	
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


HackingNode::HackingNode(Sint16 x, Sint16 y) : InteractiveSurface(6, 7, x, y)//, _x(x), _y(y)
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
void HackingNode::activateNeighbours()
{
	for (int i = 0; i < _neighbourNodes.size(); ++i)
		if (_neighbourNodes[i])
		{
			_neighbourNodes[i]->setVisible(true);
			_neighbourNodes[i]->invalidate();
		}
}

//void HackingNode::mouseClick(Action* action, State* state)
//{
//	_color += 17;
//	_redraw = true;
//}


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
	//_bg->drawRect(0, 0, 320, 200, 3);
	//add(_exitButton, "buttonExit", "hackingTool", _bg); // TODO: add new interface to rulesets
	add(_exitButton);

	// Create node grid 
	for (int i = 0; i < std::size(_nodeArray); ++i)
		for (int j = 0; (j < std::size(_nodeArray[i])- 1 + i % 2) ; ++j) // 1 less for even i and full length for odd
		{
			_nodeArray[i][j] = new HackingNode(hackingGridStartX + hackingGridWidth * j - hackingNodeOffsetX *(i % 2),
												hackingViewStartY + hackingGridHeight * i);
			add(_nodeArray[i][j]);
			_nodeArray[i][j]->setVisible(false);
			_nodeArray[i][j]->onMouseClick((ActionHandler)&HackingState::onNodeClick);
		}
	// Link nodes
	for(int i = 0; i < std::size(_nodeArray); ++i)
		for (int j = 0; j < std::size(_nodeArray[i]); ++j)
		{
			if (_nodeArray[i][j])
			{
				if (i > 0)
				{
					if (j >= i % 2)
					{
						_nodeArray[i][j]->setNeighbour(0, _nodeArray[i - 1][j - i % 2]);
					}
					if (j < std::size(_nodeArray[i]) - 1)
					{
						_nodeArray[i][j]->setNeighbour(1, _nodeArray[i - 1][j + 1 - i % 2]);
					}
				}
				if (i < std::size(_nodeArray) - 1)
				{
					if (j >= i % 2)
					{
						_nodeArray[i][j]->setNeighbour(2, _nodeArray[i + 1][j - i % 2]);
					}
					if (j < std::size(_nodeArray[i + 1]) - 1)
					{
						_nodeArray[i][j]->setNeighbour(3, _nodeArray[i + 1][j + 1 - i % 2]);
					}
				}
			}
		}
	// TODO: Set up defence
	_nodeArray[5][2]->setState(NodeState::IMPENETRABLE);
	_nodeArray[1][2]->setState(NodeState::IMPENETRABLE);
	_nodeArray[3][2]->setState(NodeState::LOCKED);
	
	// TODO: set starting node (random)
	_nodeArray[3][0]->setVisible(true);
	_nodeArray[3][0]->setState(NodeState::ACTIVATED);
	_nodeArray[3][0]->activateNeighbours();


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
	//_timerAnimate = new Timer(125);
	//_timerAnimate->onTimer((StateHandler)&HackingState::animate);
	//_timerAnimate->start();

	update();
}

HackingState::~HackingState()
{
//	delete _timerAnimate;
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
	//for (int i = 0; i < std::size(_nodeArray); ++i)
	//	for (int j = 0; (j < std::size(_nodeArray[i]) - 1 + i % 2); ++j)
	//	{
	//		_nodeArray[i][j]->invalidate();
	//	}
}

/**
 * Animation handler. Updates the minigame view animation.
 */
void HackingState::animate()
{
//	_hackingView->animate(); // TODO: uncomment when ready
}

/**
 * Handles timers.
 */
void HackingState::think()
{
	State::think();
//	_timerAnimate->think(this, 0); // TODO: uncomment when ready
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
	case NodeState::DISABLED:
	{
		node->setState(NodeState::ACTIVATED);
		node->activateNeighbours();
		_consoleTxt->setText(">Proceeding...");
		break;
	}
	case NodeState::LOCKED:
	{
		node->setState(NodeState::ACTIVATED);
		node->activateNeighbours();
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

} // namespace

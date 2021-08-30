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

#include<deque>
#include "HackingState.h"
#include "HackingView.h"
//#include "BattlescapeGame.h"
#include "TileEngine.h"
#include "../Engine/Action.h"
#include "../Engine/Game.h"
#include "../Engine/InteractiveSurface.h"
#include "../Engine/LocalizedText.h"
#include "../Engine/Options.h"
#include "../Engine/Screen.h"
#include "../Engine/Timer.h"
#include "../Interface/Bar.h"
#include "../Interface/Text.h"
#include "../Savegame/BattleUnit.h"
#include "../Savegame/BattleUnitStatistics.h"
#include "../Mod/Mod.h"
#include "../Mod/RuleInterface.h"
#include "../Savegame/BattleItem.h"
#include "../Savegame/SavedGame.h"
#include "../Savegame/SavedBattleGame.h"


namespace OpenXcom
{
/**
 * Helper class to manage message console text 
 */
class ConsoleTextManager
{
	std::deque<std::string> _messageLog { };
	Text* _consoleTxt;
	int _maxNumLines;
public:
	ConsoleTextManager(Text* txtField, int numLines = 14) : _consoleTxt(txtField), _maxNumLines(numLines) {};
	void addMessage(std::string msg);
};
void ConsoleTextManager::addMessage(std::string msg)
{
	std::ostringstream _text{ "" };
	_messageLog.push_back(msg);
	do
	{
		_text.str("");
		for (auto msgLine : _messageLog)
		{
			_text << ">" << msgLine << '\n';
		}
		_consoleTxt->setText(_text.str());
		if (_consoleTxt->getNumLines() > _maxNumLines)
		{
			_messageLog.pop_front();
		}
	} while (_consoleTxt->getNumLines() > _maxNumLines);
}

/**
 * Helper function that returns a string representation of a type (mainly used for numbers).
 * @param t The value to stringify.
 * @return A string representation of the value.
 */
template<typename type>
std::string toString(type t)
{
	std::ostringstream ss;
	ss << t;
	return ss.str();
}

/**
 * Initializes the Hacking State.
 * @param action Pointer to an action.
 * @param targetUnit Pointer to a target unit
 * @param tileEngine Pointer to a TileEngine
 */
//HackingState::HackingState(BattleAction* action, BattleUnit* targetUnit, TileEngine* tileEngine) : _action(action), _targetUnit(targetUnit), _tileEngine(tileEngine)
HackingState::HackingState(BattleAction* action, Tile* targetTile, TileEngine* tileEngine) : _action(action), _targetTile(targetTile), _tileEngine(tileEngine)
{
	if (Options::maximizeInfoScreens)
	{
		Options::baseXResolution = Screen::ORIGINAL_WIDTH;
		Options::baseYResolution = Screen::ORIGINAL_HEIGHT;
		_game->getScreen()->resetDisplay(false);
	}
	_targetUnit = _targetTile->getUnit();
	_targetObject = _targetTile->getBattleObject();

	_tuBaseCost = _game->getMod()->getHackingBaseTuCost();
	_tuFirewallCost = _game->getMod()->getHackingFirewallBaseTuCost();
	_hpFirewallCost = _game->getMod()->getHackingFirewallBaseHpCost();
	_maxTimeUnits = _action->weapon->getRules()->getHackingTU();
	_maxHealth = _action->weapon->getRules()->getHackingHP();

	_bg = new Surface(320, 200);
	_hackingView = new HackingView(164, 121, 41, 26);
	
	_exitButton = new InteractiveSurface(20, 20, 255, 157);
	_consoleTxt = new Text(55, 108, 215, 26);
	_consoleTxt->setWordWrap(true);
	_consoleTxt->setSmall();
	_consoleTxt->setHighContrast(true);
	_consoleManager = new ConsoleTextManager(_consoleTxt);
	_consoleManager->addMessage(tr("STR_HACKING_LOGIN"));
	_consoleManager->addMessage(tr("STR_HACKING_LOGIN_SUCCESS"));

	_txtTimeUnits = new Text(75, 9, 41, 158);
	_numTimeUnits = new Text(18, 9, 118, 158);
	_barTimeUnits = new Bar(69, 6, 136, 159);

	_txtHealth = new Text(75, 9, 41, 167);
	_numHealth = new Text(18, 9, 118, 167);
	_barHealth = new Bar(69, 6, 136, 168);


	if (_game->getScreen()->getDY() > 50)
	{
		_screen = false;
	}

	// Set palette
	setStandardPalette("PAL_BATTLESCAPE");

	add(_bg);
	add(_hackingView, "viewscreen", "hacking", _bg);
	add(_consoleTxt, "textConsole", "hacking", _bg);
	add(_exitButton, "exitButton", "hacking", _bg);
	add(_txtTimeUnits, "textTUs", "hacking", _bg);
	add(_numTimeUnits, "numTUs", "hacking", _bg);
	add(_barTimeUnits, "barTUs", "hacking", _bg);
	add(_txtHealth, "textHP", "hacking", _bg);
	add(_numHealth, "numHP", "hacking", _bg);
	add(_barHealth, "barHP", "hacking", _bg);

	_txtTimeUnits->setHighContrast(true);
	_txtTimeUnits->setText(tr("STR_TIME_UNITS"));
	_numTimeUnits->setHighContrast(true);
	_barTimeUnits->setScale((static_cast<double>(_barTimeUnits->getWidth()) - 1) / _maxTimeUnits);

	_txtHealth->setHighContrast(true);
	_txtHealth->setText(tr("STR_HEALTH"));
	_numHealth->setHighContrast(true);
	_barHealth->setScale((static_cast<double>(_barHealth->getWidth()) - 1) / _maxHealth);

	for (int row = 0; row < _hackingView->getGridHeight(); ++row)
		for (int col = 0; col < _hackingView->getGridWidth(); ++col)
		{
			HackingNode* node = _hackingView->getNode(row, col);
			if (node)
			{
				add(node, "node", "hacking", _hackingView);
				node->onMouseClick((ActionHandler)&HackingState::onNodeClick);
			}
		}

	centerAllSurfaces();

	Surface* backgroundSprite = _game->getMod()->getSurface("HackingUI");

	backgroundSprite->blitNShade(_bg, 0, 0);

	_exitButton->onMouseClick((ActionHandler)&HackingState::onExitClick);
	_exitButton->onKeyboardPress((ActionHandler)&HackingState::onExitClick, Options::keyCancel);

	// Set up animation
	_timerAnimate = new Timer(125);
	_timerAnimate->onTimer((StateHandler)&HackingState::animate);
	_timerAnimate->start();

}

HackingState::~HackingState()
{
	delete _timerAnimate;
}

/**
 * Initialise hacking device starting state
 */
void HackingState::init()
{
	State::init();

	_timeUnits = _maxTimeUnits;
	_barTimeUnits->setMax(_maxTimeUnits);

	_health = _maxHealth;
	_barHealth->setMax(_maxHealth);

	_hackingView->initField();
	update();
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
	_hackingView->invalidate();
	_numTimeUnits->setText(toString(_timeUnits));
	_barTimeUnits->setValue(_timeUnits);
	_numHealth->setText(toString(_health));
	_barHealth->setValue(_health);
}

/**
 * Animation handler. Updates the minigame view animation.
 */
void HackingState::animate()
{
	_hackingView->animate(); 
}

/**
 * Handles timers.
 */
void HackingState::think()
{
	State::think();
	_timerAnimate->think(this, 0); 
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

/**
 * Handles the mouse click on the node
 * Activates the nodes at the cost of _timeUnits and _health and pushes the feedback messages to the console
 * @param action User action
*/
void HackingState::onNodeClick(Action* action)
{
	HackingNode* node = dynamic_cast<HackingNode*>(action->getSender());
	if (!node) { return; }

	if (_health <= 0) { return; }

	switch (node->getState())
	{
	case NodeState::TARGET:
	{
		if (_timeUnits >= _tuBaseCost)
		{
			if (_targetUnit)
			{
				_tileEngine->hackAttack(*_action, _targetUnit);
			}
			if (_targetObject)
			{
				_tileEngine->hackObject(*_action, _targetObject);
			}
			onExitClick(0);
		}
		else
		{
			_consoleManager->addMessage(tr("STR_HACKING_NOT_ENOUGH_TU"));
		}
		break;
	}
	case NodeState::DISABLED:
	{
		if (_timeUnits >= _tuBaseCost)
		{
			_timeUnits -= _tuBaseCost;
			_consoleManager->addMessage(tr("STR_HACKING_PROCEED"));
			_hackingView->activateNode(node);
			revealNeighbours(node);
		}
		else
		{
			_consoleManager->addMessage(tr("STR_HACKING_NOT_ENOUGH_TU"));
		}
		break;
	}
	case NodeState::LOCKED:
	{
		if (_timeUnits >= _tuFirewallCost) 
		{
			_timeUnits -= _tuFirewallCost;
			_health -= _hpFirewallCost;
			if (_health > 0)
			{
				_consoleManager->addMessage(tr("STR_HACKING_FIREWALL_BREACHED"));
				_hackingView->activateNode(node);
				revealNeighbours(node);
				_hackingView->revealFirewall(node);  // Show the entire firewall once it's been breached
			}
			else
			{
				_health = 0;
				_consoleManager->addMessage(tr("STR_HACKING_VIRUS_TERMINATED"));
			}
		}
		else
		{
			if (_timeUnits < _tuFirewallCost)
			{
				_consoleManager->addMessage(tr("STR_HACKING_NOT_ENOUGH_TU"));
			}
		}
		break;
	}
	case NodeState::IMPENETRABLE:
	{
		_consoleManager->addMessage(tr("STR_HACKING_NODE_IMPENETRABLE"));
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
void HackingState::revealNeighbours(HackingNode* node)
{
	// Get node position
	int row = node->getGridRow();
	int col = node->getGridCol();
	// Get node grid boundaries
	int maxRow = _hackingView->getGridHeight() - 1;
	int maxCol = _hackingView->getGridWidth() - 1;
	col -= row % 2; // adjust column position if we are on an odd row

	for (int currRow = row - 1; currRow <= row + 1 && currRow <= maxRow;)
	{
		if (currRow >= 0)
		{
			// show the node to the upper/lower left
			if (col >= 0)
			{
				HackingNode* node = _hackingView->getNode(currRow, col);
				if (node && !node->getVisible())
				{
					node->setVisible(true);
					notifyState(node);
				}
			}
			// show the node to the upper/lower right
			if (col < maxCol)
			{
				HackingNode* node = _hackingView->getNode(currRow, col + 1);
				if (node && !node->getVisible())
				{
					node->setVisible(true);
					notifyState(node);
				}
			}
		}
		currRow += 2;
	}
}

/**
 * Logs to the console if the node has special properties.
 * @param node Pointer to the current node.
 */
void HackingState::notifyState(HackingNode* node)
{
	switch (node->getState())
	{
	case NodeState::IMPENETRABLE:
	{
		_consoleManager->addMessage(tr("STR_HACKING_FIREWALL_DETECTED"));
		break;
	}
	case NodeState::LOCKED:
	{
		_consoleManager->addMessage(tr("STR_HACKING_FIREWALL_BREACH_DETECTED"));
		break;
	}
	case NodeState::TARGET:
	{
		_consoleManager->addMessage(tr("STR_HACKING_OBJECTIVE_DETECTED"));
		break;
	}
	default:
		break;
	}
}

} // namespace

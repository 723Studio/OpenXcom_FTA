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

#include "HackingView.h"
#include "../Engine/Game.h"
//#include "../Mod/Mod.h"
//#include "../Mod/RuleInterface.h"
//#include "../Engine/SurfaceSet.h"
#include "../Engine/Action.h"
//#include "../Engine/Language.h"
//#include "../Savegame/BattleUnit.h"
//#include "../Interface/Text.h"

namespace OpenXcom
{

struct Point
{
	Sint16 x, y;
	Point(Sint16 x, Sint16 y) : x(x), y(y) {}
};
/**
 * Initializes the Hacking view.
 * @param w The HackingView width.
 * @param h The HackingView height.
 * @param x The HackingView x origin.
 * @param y The HackingView y origin.
 * @param game Pointer to the core game.
 * @param unit The current unit.
 */
//HackingView::HackingView(int w, int h, int x, int y, Game* game, BattleUnit* unit) : InteractiveSurface(w, h, x, y), _game(game), _unit(unit), _frame(0)
HackingView::HackingView(int w, int h, int x, int y, Game* game) : InteractiveSurface(w, h, x, y), _game(game), _frame(0)
{
	_redraw = true;
}

HackingView::~HackingView()
{
}
/**
 * Draws the HackingView view.
 */
void HackingView::draw()
{
	//SurfaceSet* set = _game->getMod()->getSurfaceSet("DETBLOB.DAT");
	//Surface* surface = 0;
	//surface = new Surface(5, 5, 0, 0);
	//HackingNode* node = new HackingNode();
	
	//this->lock();
	// TODO: draw the hacking view
	//clear();
	//drawRect(0, 0, this->getWidth(), this->getHeight(), 90);
	for (auto link : _linkArray)
	{
		drawLine(link.first.x, link.first.y, link.second.x, link.second.y, 50 - _frame);
	}
	//surface->drawRect(0,0,5,5,35);
	//surface->blitNShade(this, _x, _y, 0);
	//_node->draw();
	//_node->blitNShade(this, _x-2, _y-3, 0);
	//this->unlock();

}

void HackingView::drawGrid()
{
	
	

	
}

/**
 * Handles clicks on the medikit view.
 * @param action Pointer to an action.
 * @param state State that the action handlers belong to.
 */
void HackingView::mouseClick(Action* action, State* state)
{
	// We will need to get a set of nodes here
	// example:
	//SurfaceSet* set = _game->getMod()->getSurfaceSet("MEDIBITS.DAT");

	//_x = action->getRelativeXMouse() / action->getXScale();
	//_y = action->getRelativeYMouse() / action->getYScale();

	
	// Find what node we clicked on and process action logic
	// example:
/*	for (unsigned int i = 0; i < set->getTotalFrames(); i++)
	{
		Surface* surface = set->getFrame(i);
		if (surface->getPixel(x, y))
		{
			_selectedPart = i;
			_redraw = true;
			break;
		}
	}*/
}

/**
 * Updates the hacking view animation.
 */
void HackingView::animate()
{
	// TODO: rework animation frames if needed
	_frame++;
	if (_frame > 2)
	{
		_frame = 0;
	}
	_redraw = true;
}

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

} // namespace

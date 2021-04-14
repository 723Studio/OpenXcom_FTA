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

/**
 * Initializes the Scanner view.
 * @param w The ScannerView width.
 * @param h The ScannerView height.
 * @param x The ScannerView x origin.
 * @param y The ScannerView y origin.
 * @param game Pointer to the core game.
 * @param unit The current unit.
 */
HackingView::HackingView(int w, int h, int x, int y, Game* game, BattleUnit* unit) : InteractiveSurface(w, h, x, y), _game(game), _unit(unit), _frame(0)
{
	_redraw = true;
}

/**
 * Draws the HackingView view.
 */
void HackingView::draw()
{
	//SurfaceSet* set = _game->getMod()->getSurfaceSet("DETBLOB.DAT");
	Surface* surface = 0;

	clear();

	this->lock();
	// TODO: draw the hacking view
	// surface->blitNShade(this, x, y, 0);
	this->unlock();

}

/**
 * Handles clicks on the medikit view.
 * @param action Pointer to an action.
 * @param state State that the action handlers belong to.
 */
void HackingView::mouseClick(Action* action, State*)
{
	// We will need to get a set of nodes here
	// example:
	//SurfaceSet* set = _game->getMod()->getSurfaceSet("MEDIBITS.DAT");
	int x = action->getRelativeXMouse() / action->getXScale();
	int y = action->getRelativeYMouse() / action->getYScale();

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
	if (_frame > 1)
	{
		_frame = 0;
	}
	_redraw = true;
}

} // namespace

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
#include "../Engine/State.h"

namespace OpenXcom
{

class TextButton;
class Window;
class Text;
class GeoscapeState;

/**
 * A screen with links to the OXCE geoscape functionality.
 */
class ExtendedGeoscapeLinksState : public State
{
private:
	TextButton *_btnOk;
	TextButton *_btnFunding, *_btnTechTree, *_btnGlobalResearch, *_btnGlobalProduction, *_btnUfoTracker, *_btnPilotExp, *_btnNotes, *_btnMusic;
	TextButton *_btnTest;
	Window *_window;
	Text *_txtTitle;
	GeoscapeState *_parent;
	bool _ftaUi;
public:
	/// Creates the ExtendedGeoscapeLinks state.
	ExtendedGeoscapeLinksState(GeoscapeState* parent);
	/// Cleans up the ExtendedGeoscapeLinks state.
	~ExtendedGeoscapeLinksState() = default;
	/// Handlers for clicking the buttons.
	void btnFundingClick(Action* action);
	void btnTechTreeClick(Action* action);
	void btnGlobalResearchClick(Action* action);
	void btnGlobalProductionClick(Action* action);
	void btnUfoTrackerClick(Action* action);
	void btnPilotExpClick(Action* action);
	void btnNotesClick(Action* action);
	void btnMusicClick(Action* action);
	void btnTestClick(Action* action);
	void btnOkClick(Action* action);
};

}

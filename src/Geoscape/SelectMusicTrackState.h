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
#include <vector>
#include "../Engine/State.h"

namespace OpenXcom
{

enum SelectMusicTrackOrigin
{
	SMT_GEOSCAPE,
	SMT_BATTLESCAPE
};

class Window;
class Text;
class TextButton;
class TextList;
class Music;

/**
 * Select Music Track window that allows changing
 * of the currently played music track.
 */
class SelectMusicTrackState : public State
{
private:
	SelectMusicTrackOrigin _origin;
	Window *_window;
	Text *_txtTitle;
	TextButton *_btnCancel;
	TextList *_lstTracks;
	std::vector<Music*> _tracks;
	std::vector<std::string> _trackNames;
public:
	/// Creates the Select Music Track state.
	SelectMusicTrackState(SelectMusicTrackOrigin origin);
	/// Cleans up the Select Music Track state.
	~SelectMusicTrackState();
	/// Handler for clicking the Cancel button.
	void btnCancelClick(Action *action);
	/// Handler for clicking the Weapons list.
	void lstTrackClick(Action *action);
};

}

#pragma once
/*
 * Copyright 2010-2015 OpenXcom Developers.
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

class TechTreeViewerState;
class Window;
class Text;
class TextButton;
class TextEdit;
class TextList;

/**
 * Window which allows selecting a topic for the Tech Tree Viewer.
 */
class TechTreeSelectState : public State
{
private:
	TechTreeViewerState *_parent;
	TextButton *_btnOk;
	TextEdit *_btnQuickSearch;
	Window *_window;
	Text *_txtTitle;
	TextList *_lstTopics;
	std::vector<std::string> _availableTopics;
	size_t _firstManufacturingTopicIndex;
	size_t _firstFacilitiesTopicIndex;
	size_t _firstItemTopicIndex;
	size_t _firstCraftTopicIndex;
	void initLists();
	void onSelectTopic(Action *action);
public:
	/// Creates the TechTreeSelect state.
	TechTreeSelectState(TechTreeViewerState *parent);
	/// Cleans up the TechTreeSelect state
	~TechTreeSelectState();
	/// Initializes the state.
	void init() override;
	/// Handler for clicking the OK button.
	void btnOkClick(Action *action);
	/// Handlers for Quick Search.
	void btnQuickSearchToggle(Action *action);
	void btnQuickSearchApply(Action *action);
};
}

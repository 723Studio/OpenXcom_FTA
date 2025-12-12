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

class Window;
class Text;
class TextButton;
class TextList;

/**
 * Window which displays manufacture dependencies tree.
 */
class ManufactureDependenciesTreeState : public State
{
private:
	Window *_window;
	Text *_txtTitle;
	TextList *_lstTopics;
	TextButton *_btnOk, *_btnShowAll;
	std::string _selectedItem;
	bool _showAll, _ftaGame;
	void initList();
	void initFtAList();
  public:
	/// Creates the ManufactureDependenciesTree state.
	ManufactureDependenciesTreeState(const std::string &selectedItem);
	/// Cleans up the ManufactureDependenciesTree state
	~ManufactureDependenciesTreeState();
	/// Initializes the state.
	void init() override;
	/// Handler for clicking the OK button.
	void btnOkClick(Action *action);
	/// Handler for clicking the [Show All] button.
	void btnShowAllClick(Action *action);
};
}

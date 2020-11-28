#pragma once
/*
* Copyright 2010-2020 OpenXcom Developers.
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
#include <vector>
#include <string>

namespace OpenXcom
{

class TextButton;
class Window;
class Text;
class Globe;
class Base;
class Soldier;

/**
* Alternative report screen shown monthly to display, used in FtA
* changes in the player's performance, factions status, loyalty other important stuff.
*/
class AltMonthlyReportState : public State
{
private:
	TextButton* _btnOk, * _btnBigOk;
	Window* _window;
	Text* _txtTitle, * _txtMonth, * _txtRating;
	Text* _txtLoyalty, * _txtMaintenance, * _txtBalance, * _txtBonus;
	Text* _txtDesc, * _txtFailure;
	int _gameOver;
	int _ratingTotal, _fundingDiff, _lastMonthsRating, _loyalty, _lastMonthsLoyalty;
	std::string _stuffMessage, _factionMessage;
	std::vector<std::string> _happyList, _sadList, _pactList, _cancelPactList;
	Globe* _globe;
	std::vector<Soldier*> _soldiersMedalled;
	/// Builds a faction list string.
	std::string factionList(const std::vector<std::string>& factions, const std::string& singular, const std::string& plural);
public:
	/// Creates the Monthly Report state.
	AltMonthlyReportState(Globe* globe);
	/// Cleans up the Monthly Report state.
	~AltMonthlyReportState();
	/// Handler for clicking the OK button.
	void btnOkClick(Action* action);
	/// Calculate monthly updates.
	void calculateUpdates();
	/// Calculate monthly scores.
	void calculateChanges();
};

/**
* State that concludes alpha_game
* changes in the player's performance and funding.
*/
class AlphaGameVersionEnds : public State
{
private:
	TextButton* _btnOk;
	Window* _window;
	Text* _txtTitle;
	Text* _txtDesc;
	int _gameOver;
public:
	/// Creates the Monthly Report state.
	AlphaGameVersionEnds();
	/// Cleans up the Monthly Report state.
	~AlphaGameVersionEnds();
	/// Handler for clicking the OK button.
	void btnOkClick(Action* action);
	/// Calculate monthly scores.
};

}

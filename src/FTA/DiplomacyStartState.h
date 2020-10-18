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
#include "../Engine/State.h"

namespace OpenXcom
{
class TextButton;
class Window;
class Text;
class Surface;
class Frame;
class DiplomacyFaction;
class Action;
class Game;
class Base;

/**
* Enumeration for the type of trading operation.
*/
enum TradeOperation { OPERATION_BUYING, OPERATION_SELLING };

/**
 * Diplomacy main screen that shows diplomacy cards
 * for all discovered Factions and Faction main interfaces.
 */
class DiplomacyStartState : public State
{
private:
	TextButton *_btnOk;
	std::vector<TextButton*> _btnsInfo, _btnsTalk, _btnsPurchaise, _btnsSell;
	Window *_window;
	std::vector<Window*> _cards;
	Text* _txtTitle;
	std::vector<Text*> _txtsName, _txtsRep;
	Base* _base;
public:
	/// Creates the Diplomacy Start state.
	DiplomacyStartState(Base* base, bool geoscape = false);
	/// Cleans up the Diplomacy Start state.
	~DiplomacyStartState();
	/// Handler for clicking the OK button.
	void btnOkClick(Action *action);
	/// Handler for clicking the INFO buttons.
	void btnInfoClick(Action* action);
	/// Handler for clicking the NEGOTIATION buttons.
	void btnTalkClick(Action* action);
	/// Handler for clicking the BUY buttons.
	void btnPurchaiseClick(Action* action);
	/// Handler for clicking the SELL buttons.
	void btnSellClick(Action* action);
};
/**
 * Diplomacy Faction info state.
 * Showes main stats of diplomacy Faction
 */
class DiplomacyInfoState : public State
{
private:
	TextButton* _btnOk;
	Window* _window;
	Text* _txtTitle, *_txtDesc;
public:
	/// Creates the Diplomacy Start state.
	DiplomacyInfoState(const DiplomacyFaction* faction);
	/// Cleans up the Diplomacy Start state.
	~DiplomacyInfoState();
	/// Handler for clicking the OK button.
	void btnOkClick(Action* action);
	/// Handler for clicking the INFO buttons.
};

/**
 * State for selecting base
 * witch we want to use for trading.
 */
class DiplomacyChooseBaseState : public State
{
private:
	std::vector<TextButton*> _btnsBase;
	Window* _window;
	Text* _txtTitle;
	DiplomacyFaction *_faction;
	TradeOperation _opeation;
public:
	/// Creates the DiplomacyChooseBaseState.
	DiplomacyChooseBaseState(DiplomacyFaction* faction, TradeOperation opeation);
	/// Cleans up the DiplomacyChooseBaseState.
	~DiplomacyChooseBaseState();
	/// Handler for clicking the Base button.
	void btnBaseClick(Action* action);
};

}

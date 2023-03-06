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
#include "DiplomacyStartState.h"
#include <sstream>
#include "../Engine/Game.h"
#include "../Engine/Action.h"
#include "../Mod/Mod.h"
#include "../Engine/LocalizedText.h"
#include "../Engine/Unicode.h"
#include "../Interface/TextButton.h"
#include "../Interface/Window.h"
#include "../Interface/Text.h"
#include "../Interface/Frame.h"
#include "../Savegame/SavedGame.h"
#include "../Engine/Options.h"
#include "../Savegame/DiplomacyFaction.h"
#include "../Mod/RuleDiplomacyFaction.h"
#include "../Savegame/Base.h"
#include "../FTA/DiplomacySellState.h"
#include "../FTA/DiplomacyPurchaseState.h"

namespace OpenXcom
{
/**
 * Initializes all the elements in the Diplomacy screen.
 * @param geoscape to check for animation style.
 * @param game Pointer to the core game.
 */
DiplomacyStartState::DiplomacyStartState(Base* base, bool geoscape) : _base(base)
{
	_screen = false;
	std::string interfaceName = "diplomacyMainWindow";

	// Create objects
	if (geoscape) { _window = new Window(this, 320, 200, 0, 0, POPUP_BOTH); }
	else { _window = new Window(this, 320, 200, 0, 0, POPUP_NONE); }
	_btnOk = new TextButton(220, 14, 50, 178);
	_txtTitle = new Text(320, 17, 0, 8);

	// Set palette
	setInterface(interfaceName);
	add(_window, "window", interfaceName);
	add(_btnOk, "button", interfaceName);
	add(_txtTitle, "text1", interfaceName);


	std::vector<DiplomacyFaction*> factions = _game->getSavedGame()->getDiplomacyFactions();
	DiplomacyFaction* faction;
	int step = 0;
	int dX = 103;
	for (std::vector<DiplomacyFaction*>::iterator i = factions.begin(); i != factions.end(); ++i)
	{
		if (step > 2) { break; } //draw only 3 cards
		dX = 103 * step;
		faction = factions.at(step);
		Window* card = new Window(this, 98, 150, 8 + dX, 25, POPUP_NONE);
		Text* txtName = new Text(90, 17, 12 + dX, 31);
		TextButton* btnInfo = new TextButton(35, 14, 14 + dX, 108); //h = 18?
		Text* txtRep = new Text(90, 9, 12 + dX, 49);
		TextButton* btnTalk = new TextButton(86, 14, 14 + dX, 125);
		TextButton* btnPurchase = new TextButton(86, 14, 14 + dX, 140);
		TextButton* btnSell = new TextButton(86, 14, 14 + dX, 155);
		if (faction->isDiscovered())
		{
			add(card, "window", interfaceName);
			card->setBackground(_game->getMod()->getSurface(faction->getRules()->getCardBackground()));
			card->setVeryThinBorder();
			_cards.push_back(card);
			//name, reputation
			add(txtName, "name", interfaceName);
			txtName->setAlign(ALIGN_CENTER);
			txtName->setBig();
			txtName->setText(tr(faction->getRules()->getName()));
			_txtsName.push_back(txtName);
			add(txtRep, "name", interfaceName);
			txtRep->setAlign(ALIGN_CENTER);
			txtRep->setText(tr(faction->getReputationName()));
			txtRep->setAlign(ALIGN_CENTER);
			//Info, Negation
			add(btnInfo, "button", interfaceName);
			btnInfo->setText(tr("STR_INFO"));
			btnInfo->onMouseClick((ActionHandler)&DiplomacyStartState::btnInfoClick);
			_btnsInfo.push_back(btnInfo);
			_txtsRep.push_back(txtRep);
			add(btnTalk, "button", interfaceName);
			btnTalk->setText(tr("STR_NEGOTIATION"));
			btnTalk->onMouseClick((ActionHandler)&DiplomacyStartState::btnTalkClick);
			_btnsTalk.push_back(btnTalk);
			//trade buttons
			add(btnPurchase, "button", interfaceName);
			btnPurchase->setText(tr("STR_PURCHASE"));
			btnPurchase->onMouseClick((ActionHandler)&DiplomacyStartState::btnPurchaseClick);
			_btnsPurchase.push_back(btnPurchase);
			add(btnSell, "button", interfaceName);
			btnSell->setText(tr("STR_SELL_UC"));
			btnSell->onMouseClick((ActionHandler)&DiplomacyStartState::btnSellClick);
			_btnsSell.push_back(btnSell);
		}
		step++;
	}

	centerAllSurfaces();

	// Set up objects outside of cards
	setWindowBackground(_window, interfaceName);

	_btnOk->setText(tr("STR_OK"));
	_btnOk->onMouseClick((ActionHandler)&DiplomacyStartState::btnOkClick);
	_btnOk->onKeyboardPress((ActionHandler)&DiplomacyStartState::btnOkClick, Options::keyOk);
	_btnOk->onKeyboardPress((ActionHandler)&DiplomacyStartState::btnOkClick, Options::keyCancel);

	_txtTitle->setAlign(ALIGN_CENTER);
	_txtTitle->setBig();
	_txtTitle->setText(tr("STR_DIPLOMACY_RELATIONS"));

}

/**
 *
 */
DiplomacyStartState::~DiplomacyStartState()
{

}

/**
 * Returns to the previous screen.
 * @param action Pointer to an action.
 */
void DiplomacyStartState::btnOkClick(Action *)
{
	_game->popState();
}

/**
 * Displays Faction info state.
 * @param action Pointer to an action.
 */
void DiplomacyStartState::btnInfoClick(Action* action)
{
	for (size_t i = 0; i < _btnsInfo.size(); ++i)
	{
		if (action->getSender() == _btnsInfo.at(i))
		{
			DiplomacyFaction* faction = _game->getSavedGame()->getDiplomacyFactions().at(i);
			_game->pushState(new DiplomacyInfoState(faction));
			break;
		}
	}
}

/**
 * Displays Faction negotiation state.
 * @param action Pointer to an action.
 */
void DiplomacyStartState::btnTalkClick(Action* action)
{
	for (size_t i = 0; i < _btnsTalk.size(); ++i)
	{
		if (action->getSender() == _btnsTalk.at(i))
		{
			auto faction = _game->getSavedGame()->getDiplomacyFactions().at(i);
			Log(LOG_INFO) << "You clicked NEGOTIATION button of " << tr(faction->getRules()->getName()) << " faction! Sorry, it's not implemented yet!";
			//_game->pushState(new UfopaediaSelectState(_cats[_offset + i], _heightOffset, _windowOffset));
			break;
		}
	}
}

/**
 * Displays Faction buy state.
 * @param action Pointer to an action.
 */
void DiplomacyStartState::btnPurchaseClick(Action* action)
{
	for (size_t i = 0; i < _btnsPurchase.size(); ++i)
	{
		if (action->getSender() == _btnsPurchase.at(i))
		{
			DiplomacyFaction* faction = _game->getSavedGame()->getDiplomacyFactions().at(i);
			if (_base != 0)
			{
				_game->pushState(new DiplomacyPurchaseState(_base, faction));
			}
			else if (_game->getSavedGame()->getBases()->size() == 1)
			{
				_game->pushState(new DiplomacyPurchaseState(_game->getSavedGame()->getBases()->front(), faction));
			}
			else
			{
				_game->pushState(new DiplomacyChooseBaseState(faction, OPERATION_BUYING));
			}
			break;
		}
	}
}
/**
 * Displays Faction sell state.
 * @param action Pointer to an action.
 */
void DiplomacyStartState::btnSellClick(Action* action)
{
	for (size_t i = 0; i < _btnsSell.size(); ++i)
	{
		if (action->getSender() == _btnsSell.at(i))
		{
			auto faction = _game->getSavedGame()->getDiplomacyFactions().at(i);
			if (_base != 0)
			{
				_game->pushState(new DiplomacySellState(_base, faction, 0));
			}
			else if (_game->getSavedGame()->getBases()->size() == 1)
			{
				_game->pushState(new DiplomacySellState (_game->getSavedGame()->getBases()->front(), faction, 0));
			}
			else
			{
				_game->pushState(new DiplomacyChooseBaseState(faction, OPERATION_SELLING));
			}
			break;
		}
	}
}


/**
 * Initializes all the elements in the Diplomacy Info screen
 * for selected diplomacy Faction.
 * @param game Pointer to the core game.
 */
DiplomacyInfoState::DiplomacyInfoState(const DiplomacyFaction* faction)
{
	auto rules = faction->getRules();
		_screen = false;
	std::string interfaceName = "diplomacyMainWindow";
	_window = new Window(this, 250, 160, 35, 20, POPUP_BOTH);
	_btnOk = new TextButton(103, 14, 174, 158);
	_txtTitle = new Text(234, 17, 43, 28);
	_txtDesc = new Text(234, 107, 43, 48);

	// Set palette
	setInterface(interfaceName);

	add(_window, "window", interfaceName);
	add(_btnOk, "button", interfaceName);
	add(_txtTitle, "name", interfaceName);
	add(_txtDesc, "text1", interfaceName);

	centerAllSurfaces();

	// Set up objects
	setWindowBackground(_window, interfaceName);
	_window->setBackground(_game->getMod()->getSurface(rules->getBackground()));
	_txtTitle->setText(tr(rules->getName()));
	_txtTitle->setAlign(ALIGN_CENTER);
	_txtTitle->setBig();

	_txtDesc->setText(tr(rules->getDescription()));
	_txtDesc->setWordWrap(true);


	_btnOk->setText(tr("STR_OK"));
	_btnOk->onMouseClick((ActionHandler)&DiplomacyInfoState::btnOkClick);
	_btnOk->onKeyboardPress((ActionHandler)&DiplomacyInfoState::btnOkClick, Options::keyOk);
	_btnOk->onKeyboardPress((ActionHandler)&DiplomacyInfoState::btnOkClick, Options::keyCancel);
	
}

DiplomacyInfoState::~DiplomacyInfoState()
{
}
/**
 * Returns to the previous screen.
 * @param action Pointer to an action.
 */
void DiplomacyInfoState::btnOkClick(Action* action)
{
	_game->popState();
}

DiplomacyChooseBaseState::DiplomacyChooseBaseState(DiplomacyFaction* faction, TradeOperation operation) : _faction(faction), _operation(operation)
{
	_screen = false;
	std::string interfaceName = "diplomacyMainWindow";
	
	setInterface(interfaceName);
	int bCount = _game->getSavedGame()->getBases()->size();
	int btnH = 17;
	if (bCount > 6)	{ btnH = 15;}
	int step = 0;
	int winH = 28 + (btnH + 3) * bCount;
	int winY = (200 - winH) / 2;
	_window = new Window(this, 134, winH, 93, winY, POPUP_BOTH);
	_txtTitle = new Text(124, 18, 98, winY + 5);
	add(_window, "window", interfaceName);
	add(_txtTitle, "name", interfaceName);

	for (std::vector<Base*>::iterator i = _game->getSavedGame()->getBases()->begin(); i != _game->getSavedGame()->getBases()->end(); ++i)
	{
		int dY = (btnH + 3) * step;
		TextButton* btnBase = new TextButton(118, btnH, 101, winY + 23 + dY);
		add(btnBase, "button", interfaceName);
		btnBase->setText((*i)->getName());
		btnBase->onMouseClick((ActionHandler)&DiplomacyChooseBaseState::btnBaseClick);
		_btnsBase.push_back(btnBase);
		step++;
	}

	centerAllSurfaces();
	setWindowBackground(_window, interfaceName); //#FINNIKTODO: change background, its creepy!
	_txtTitle->setText(tr("STR_CHOOSE_BASE"));
	_txtTitle->setAlign(ALIGN_CENTER);
	_txtTitle->setWordWrap(true);
	_txtTitle->setVerticalAlign(ALIGN_MIDDLE);
}

DiplomacyChooseBaseState::~DiplomacyChooseBaseState()
{
}
/**
 * Opens trading with Faction.
 * Selling or buying is determined with TRADING constant
 * @param action Pointer to an action.
 */
void DiplomacyChooseBaseState::btnBaseClick(Action* action)
{
	for (size_t i = 0; i < _btnsBase.size(); ++i)
	{
		if (action->getSender() == _btnsBase.at(i))
		{
			Base* base = _game->getSavedGame()->getBases()->at(i);
			_game->popState();
			switch (_operation)
			{
			case OpenXcom::OPERATION_BUYING:
				_game->pushState(new DiplomacyPurchaseState(base, _faction));
				break;
			case OpenXcom::OPERATION_SELLING:
				_game->pushState(new DiplomacySellState(base, _faction, 0));
				break;
			default:
				break;
			}
			break;
		}
	}
}

}

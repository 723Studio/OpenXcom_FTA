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
#include "FinishedCoverOperationDetailsState.h"
#include "../Engine/Game.h"
#include "../Engine/Action.h"
#include "../Engine/LocalizedText.h"
#include "../Engine/Options.h"
#include "../Engine/Logger.h"
#include "../Interface/TextButton.h"
#include "../Interface/Window.h"
#include "../Interface/Text.h"
#include "../Interface/TextList.h"
#include "../Mod/Mod.h"
#include "../Mod/RuleCovertOperation.h"
#include "../Mod/RuleInterface.h"
#include "../Savegame/CovertOperation.h"
#include "../Savegame/Base.h"
#include "../Savegame/SavedBattleGame.h"
#include "../Savegame/Soldier.h"
#include "../Ufopaedia/Ufopaedia.h"

namespace OpenXcom
{

	/**
	 * Initializes all the elements in the FinishedCoverOperationState window.
	 * @param geoEvent Pointer to the event.
	 * @param result - comes true if sucess operation, false if it was failed.
	 */
	FinishedCoverOperationDetailsState::FinishedCoverOperationDetailsState(CovertOperation* operation) :
		_operation(operation), _pageNumber(0), _hasItems(false), _hasRep(false), _hasFunds(false), _hasScore(false), _hasSStatus(false), _hasMessage(false)
	{
		_screen = false;
		_results = _operation->getResults();
		// Create objects
		_window = new Window(this, 320, 200, 0, 0);
		_btnOk = new TextButton(40, 12, 16, 180);
		_btnPage = new TextButton(60, 12, 244, 180);
		_txtTitle = new Text(300, 17, 16, 8);

		_txtItem = new Text(180, 9, 16, 24);
		_lstRecoveredItems = new TextList(272, 48, 16, 32); // h=48 = 8 rows

		_txtReputation = new Text(180, 9, 16, 84);
		_lstReputation = new TextList(272, 24, 16, 92); //3 rows max

		_lstFunds = new TextList(290, 9, 16, 120);
		_lstScore = new TextList(290, 9, 16, 128);

		_txtSoldierStatus = new Text(180, 9, 16, 140);
		_lstSoldierStatus = new TextList(290, 16, 16, 148); //2 rows max

		_txtMessage = new Text(290, 9, 16, 168);

		// Second page (soldier stats)
		_txtSoldier = new Text(90, 9, 16, 24); //16..106 = 90
		_txtTU = new Text(18, 9, 106, 24); //106
		_txtStamina = new Text(18, 9, 124, 24); //124
		_txtHealth = new Text(18, 9, 142, 24); //142
		_txtBravery = new Text(18, 9, 160, 24); //160
		_txtReactions = new Text(18, 9, 178, 24); //178
		_txtFiring = new Text(18, 9, 196, 24); //196
		_txtThrowing = new Text(18, 9, 214, 24); //214
		_txtMelee = new Text(18, 9, 232, 24); //232
		_txtStrength = new Text(18, 9, 250, 24); //250
		_txtPsiStrength = new Text(18, 9, 268, 24); //268
		_txtPsiSkill = new Text(18, 9, 286, 24); //286..304 = 18

		_lstSoldierStats = new TextList(288, 144, 16, 32); // 18 rows

		_txtTooltip = new Text(200, 9, 64, 180);

		setInterface("covertOperationFinishDetails");
		//1st page
		add(_window, "window", "covertOperationFinishDetails");
		add(_txtTitle, "heading", "covertOperationFinishDetails");
		add(_btnOk, "button", "covertOperationFinishDetails");
		add(_btnPage, "button", "covertOperationFinishDetails");

		add(_txtItem, "text", "covertOperationFinishDetails");
		add(_lstRecoveredItems, "list", "covertOperationFinishDetails");
		add(_txtReputation, "text", "covertOperationFinishDetails");
		add(_lstReputation, "list", "covertOperationFinishDetails");

		add(_lstFunds, "totals", "covertOperationFinishDetails");
		add(_lstScore, "totals", "covertOperationFinishDetails");

		add(_txtSoldierStatus, "text", "covertOperationFinishDetails");
		add(_lstSoldierStatus, "list", "covertOperationFinishDetails");

		add(_txtMessage, "text", "covertOperationFinishDetails");

		//2ed page
		add(_txtSoldier, "text", "covertOperationFinishDetails");
		add(_txtTU, "text", "covertOperationFinishDetails");
		add(_txtStamina, "text", "covertOperationFinishDetails");
		add(_txtHealth, "text", "covertOperationFinishDetails");
		add(_txtBravery, "text", "covertOperationFinishDetails");
		add(_txtReactions, "text", "covertOperationFinishDetails");
		add(_txtFiring, "text", "covertOperationFinishDetails");
		add(_txtThrowing, "text", "covertOperationFinishDetails");
		add(_txtMelee, "text", "covertOperationFinishDetails");
		add(_txtStrength, "text", "covertOperationFinishDetails");
		add(_txtPsiStrength, "text", "covertOperationFinishDetails");
		add(_txtPsiSkill, "text", "covertOperationFinishDetails");
		add(_lstSoldierStats, "list", "covertOperationFinishDetails");

		add(_txtTooltip, "text", "debriefing");

		centerAllSurfaces();

		// Set up objects
		setWindowBackground(_window, "covertOperationFinishDetails");

		_txtTitle->setBig();
		_txtTitle->setText(tr(operation->getOperationName()));


		_btnOk->setText(tr("STR_OK"));
		_btnOk->onMouseClick((ActionHandler)&FinishedCoverOperationDetailsState::btnOkClick);
		_btnOk->onKeyboardPress((ActionHandler)&FinishedCoverOperationDetailsState::btnOkClick, Options::keyOk);

		_btnPage->setText(tr("STR_STATS"));
		_btnPage->onMouseClick((ActionHandler)&FinishedCoverOperationDetailsState::btnPageClick);

		_txtItem->setText(tr("STR_LIST_ITEM"));
		_lstRecoveredItems->setColumns(2, 254, 64);
		_lstRecoveredItems->setDot(true);

		_txtReputation->setText(tr("STR_REPUTATION_CHANGE"));
		_lstReputation->setColumns(2, 254, 64);
		_lstReputation->setDot(true);

		_lstFunds->setColumns(2, 254, 64);
		_lstFunds->setDot(true);
		_lstScore->setColumns(2, 254, 64);
		_lstScore->setDot(true);

		_txtSoldierStatus->setText(tr("STR_SOLDIERS_STATUS"));
		_lstSoldierStatus->setColumns(2, 254, 18);
		_lstSoldierStatus->setDot(true);

		_txtMessage->setText(tr(_results->getSpecialMessage()));

		// Second page
		_txtSoldier->setText(tr("STR_NAME_UC"));

		_txtTU->setAlign(ALIGN_CENTER);
		_txtTU->setText(tr("STR_TIME_UNITS_ABBREVIATION"));
		_txtTU->setTooltip("STR_TIME_UNITS");
		_txtTU->onMouseIn((ActionHandler)&FinishedCoverOperationDetailsState::txtTooltipIn);
		_txtTU->onMouseOut((ActionHandler)&FinishedCoverOperationDetailsState::txtTooltipOut);

		_txtStamina->setAlign(ALIGN_CENTER);
		_txtStamina->setText(tr("STR_STAMINA_ABBREVIATION"));
		_txtStamina->setTooltip("STR_STAMINA");
		_txtStamina->onMouseIn((ActionHandler)&FinishedCoverOperationDetailsState::txtTooltipIn);
		_txtStamina->onMouseOut((ActionHandler)&FinishedCoverOperationDetailsState::txtTooltipOut);

		_txtHealth->setAlign(ALIGN_CENTER);
		_txtHealth->setText(tr("STR_HEALTH_ABBREVIATION"));
		_txtHealth->setTooltip("STR_HEALTH");
		_txtHealth->onMouseIn((ActionHandler)&FinishedCoverOperationDetailsState::txtTooltipIn);
		_txtHealth->onMouseOut((ActionHandler)&FinishedCoverOperationDetailsState::txtTooltipOut);

		_txtBravery->setAlign(ALIGN_CENTER);
		_txtBravery->setText(tr("STR_BRAVERY_ABBREVIATION"));
		_txtBravery->setTooltip("STR_BRAVERY");
		_txtBravery->onMouseIn((ActionHandler)&FinishedCoverOperationDetailsState::txtTooltipIn);
		_txtBravery->onMouseOut((ActionHandler)&FinishedCoverOperationDetailsState::txtTooltipOut);

		_txtReactions->setAlign(ALIGN_CENTER);
		_txtReactions->setText(tr("STR_REACTIONS_ABBREVIATION"));
		_txtReactions->setTooltip("STR_REACTIONS");
		_txtReactions->onMouseIn((ActionHandler)&FinishedCoverOperationDetailsState::txtTooltipIn);
		_txtReactions->onMouseOut((ActionHandler)&FinishedCoverOperationDetailsState::txtTooltipOut);

		_txtFiring->setAlign(ALIGN_CENTER);
		_txtFiring->setText(tr("STR_FIRING_ACCURACY_ABBREVIATION"));
		_txtFiring->setTooltip("STR_FIRING_ACCURACY");
		_txtFiring->onMouseIn((ActionHandler)&FinishedCoverOperationDetailsState::txtTooltipIn);
		_txtFiring->onMouseOut((ActionHandler)&FinishedCoverOperationDetailsState::txtTooltipOut);

		_txtThrowing->setAlign(ALIGN_CENTER);
		_txtThrowing->setText(tr("STR_THROWING_ACCURACY_ABBREVIATION"));
		_txtThrowing->setTooltip("STR_THROWING_ACCURACY");
		_txtThrowing->onMouseIn((ActionHandler)&FinishedCoverOperationDetailsState::txtTooltipIn);
		_txtThrowing->onMouseOut((ActionHandler)&FinishedCoverOperationDetailsState::txtTooltipOut);

		_txtMelee->setAlign(ALIGN_CENTER);
		_txtMelee->setText(tr("STR_MELEE_ACCURACY_ABBREVIATION"));
		_txtMelee->setTooltip("STR_MELEE_ACCURACY");
		_txtMelee->onMouseIn((ActionHandler)&FinishedCoverOperationDetailsState::txtTooltipIn);
		_txtMelee->onMouseOut((ActionHandler)&FinishedCoverOperationDetailsState::txtTooltipOut);

		_txtStrength->setAlign(ALIGN_CENTER);
		_txtStrength->setText(tr("STR_STRENGTH_ABBREVIATION"));
		_txtStrength->setTooltip("STR_STRENGTH");
		_txtStrength->onMouseIn((ActionHandler)&FinishedCoverOperationDetailsState::txtTooltipIn);
		_txtStrength->onMouseOut((ActionHandler)&FinishedCoverOperationDetailsState::txtTooltipOut);

		_txtPsiStrength->setAlign(ALIGN_CENTER);
		if (_game->getMod()->isManaFeatureEnabled())
		{
			_txtPsiStrength->setText(tr("STR_MANA_ABBREVIATION"));
			_txtPsiStrength->setTooltip("STR_MANA_POOL");
		}
		else
		{
			_txtPsiStrength->setText(tr("STR_PSIONIC_STRENGTH_ABBREVIATION"));
			_txtPsiStrength->setTooltip("STR_PSIONIC_STRENGTH");
		}
		_txtPsiStrength->onMouseIn((ActionHandler)&FinishedCoverOperationDetailsState::txtTooltipIn);
		_txtPsiStrength->onMouseOut((ActionHandler)&FinishedCoverOperationDetailsState::txtTooltipOut);

		_txtPsiSkill->setAlign(ALIGN_CENTER);
		_txtPsiSkill->setText(tr("STR_PSIONIC_SKILL_ABBREVIATION"));
		_txtPsiSkill->setTooltip("STR_PSIONIC_SKILL");
		_txtPsiSkill->onMouseIn((ActionHandler)&FinishedCoverOperationDetailsState::txtTooltipIn);
		_txtPsiSkill->onMouseOut((ActionHandler)&FinishedCoverOperationDetailsState::txtTooltipOut);

		_lstSoldierStats->setColumns(13, 90, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 0);
		_lstSoldierStats->setAlign(ALIGN_CENTER);
		_lstSoldierStats->setAlign(ALIGN_LEFT, 0);
		_lstSoldierStats->setDot(true);

		std::map<std::string, int> items = _results->getItems();
		int rowItem = 0;
		if (!items.empty())
		{
			_hasItems = true;
			for (std::map<std::string, int>::const_iterator i = items.begin(); i != items.end(); ++i)
			{
				auto item = tr((*i).first);
				int qty = (*i).second;
				std::ostringstream ss;
				ss << Unicode::TOK_COLOR_FLIP << qty << Unicode::TOK_COLOR_FLIP;
				_lstRecoveredItems->addRow(2, item.c_str(), ss.str().c_str());
				++rowItem;
			}
		}

		std::map<std::string, int> factions = _results->getReputation();
		int rowReputation = 0;
		if (!factions.empty())
		{
			_hasRep = true;
			for (std::map<std::string, int>::const_iterator i = factions.begin(); i != factions.end(); ++i)
			{
				auto faction = tr((*i).first);
				int qty = (*i).second;
				std::ostringstream ss2;
				ss2 << Unicode::TOK_COLOR_FLIP << qty << Unicode::TOK_COLOR_FLIP;
				_lstReputation->addRow(2, faction.c_str(), ss2.str().c_str());
				++rowReputation;
			}
		}

		if (_results->getFunds() != 0)
		{
			std::ostringstream ss3;
			ss3 << _results->getFunds();
			_lstFunds->addRow(2, tr("STR_FUNDS_UC").c_str(), ss3.str().c_str());
			_hasFunds = true;
		}
		if (_results->getScore() != 0)
		{
			std::ostringstream ss4;
			ss4 << _results->getScore();
			_lstScore->addRow(2, tr("STR_SCORE_UC").c_str(), ss4.str().c_str());
			_hasScore = true;
		}

		int rowSoldierStatus = 0;
		std::map<std::string, int> soldierStatus = _results->getSoldierDamage();
		if (!soldierStatus.empty())
		{
			_hasSStatus = true;
			int wounded = 0, mia = 0;
			for (std::map<std::string, int>::const_iterator i = soldierStatus.begin(); i != soldierStatus.end(); ++i)
			{
				auto soldier = tr((*i).first);
				int damage = (*i).second;
				if (damage > 0)
					++wounded;
				if (damage < 0)
					++mia;
			}
			if (wounded > 0)
			{
				std::ostringstream ss5;
				ss5 << Unicode::TOK_COLOR_FLIP << wounded << Unicode::TOK_COLOR_FLIP;
				_lstSoldierStatus->addRow(2, tr("STR_XCOM_OPERATIVES_WOUNDED").c_str(), ss5.str().c_str());
				++rowSoldierStatus;
			}
			if (mia > 0)
			{
				std::ostringstream ss5;
				ss5 << Unicode::TOK_COLOR_FLIP << mia << Unicode::TOK_COLOR_FLIP;
				_lstSoldierStatus->addRow(2, tr("STR_XCOM_OPERATIVES_MISSING_IN_ACTION").c_str(), ss5.str().c_str());
				++rowSoldierStatus;
			}
		}

		if (!_results->getSpecialMessage().empty())
		{
			_hasMessage = true;
			_txtMessage->setText(tr(_results->getSpecialMessage()));
		}

		//ok, now we want adapt our UI to its content
		//first, we calculate offsets to know how much we need to move content
		int itemOffset = 0;
		if (rowItem == 0)
			itemOffset = 48 + 8; //full list size + header
		else if (rowItem > 7)
			itemOffset = 0; //we displayed maximum elements in this section, no offset
		else itemOffset = (48 - (rowItem * 8));

		int repOffset = 0;
		if (rowReputation == 0)
			repOffset = 24 + 8;
		else repOffset = (24 - (rowReputation * 8));

		int fundOffset = 0;
		if (!_hasFunds)
			fundOffset = 8;
		int scoreOffset = 0;
		if (!_hasScore)
			scoreOffset = 8;

		int statusOffset = 0;
		if (rowSoldierStatus == 0)
			statusOffset = 16 + 8; //full list size + header
		else
			statusOffset = (16 - (rowSoldierStatus * 8));

		//extra expanding of item list if we do not have a lot of things to show on the screen
		if (rowItem > 7)
		{
			int globalOffset = repOffset + fundOffset + scoreOffset + statusOffset;
			if (globalOffset > rowItem * 8)
			{
				int extraRowSpace = (rowItem - 6) * 8;
				_lstRecoveredItems->setHeight(_lstRecoveredItems->getHeight() + extraRowSpace);
				itemOffset -= extraRowSpace;
			}
			else
			{
				_lstRecoveredItems->setHeight(_lstRecoveredItems->getHeight() + (floor(globalOffset / 8) * 8));
				itemOffset -= globalOffset;
			}
		}
		//move UI elemnts by calculated offset
		_txtReputation->setY(_txtReputation->getY() - itemOffset);
		_lstReputation->setY(_lstReputation->getY() - itemOffset);

		_lstFunds->setY(_lstFunds->getY() - itemOffset - repOffset);
		_lstScore->setY(_lstScore->getY() - itemOffset - repOffset - fundOffset);

		_txtSoldierStatus->setY(_txtSoldierStatus->getY() - itemOffset - repOffset - fundOffset - scoreOffset);
		_lstSoldierStatus->setY(_lstSoldierStatus->getY() - itemOffset - repOffset - fundOffset - scoreOffset);

		_txtMessage->setY(_txtMessage->getY() - itemOffset - repOffset - fundOffset - scoreOffset - statusOffset);

		auto soldierStats = _results->getSoldierImprovement();
		int row = 0;
		for (std::vector<std::pair<std::string, UnitStats*>>::const_iterator i = soldierStats.begin(); i != soldierStats.end(); ++i)
		{
			auto soldierDamage = _results->getSoldierDamage();
			int damage = 0;
			for (std::map<std::string, int>::const_iterator j = soldierDamage.begin(); j != soldierDamage.end(); ++j)
			{
				if ((*i).first == (*j).first)
				{
					damage = (*j).second;
					std::ostringstream sName;
					if (damage < 0) //soldier MiA
					{
						_lstSoldierStats->setSmall();
						_lstSoldierStats->addRow(13, (*i).first.c_str(), "", "", "", "", "", "", "", "", "", "", "", "");
						_lstSoldierStats->setRowColor(row, _game->getMod()->getInterface("covertOperationFinishDetails")->getElement("damaged")->color);
						_lstSoldierStats->getTooltip();
					}
				}
			}
			if (damage >= 0)
			{
				auto tmp = (*i).second->psiStrength;
				if (_game->getMod()->isManaFeatureEnabled())
				{
					tmp = (*i).second->mana;
				}
				_lstSoldierStats->addRow(13, (*i).first.c_str(),
					makeSoldierString((*i).second->tu).c_str(),
					makeSoldierString((*i).second->stamina).c_str(),
					makeSoldierString((*i).second->health).c_str(),
					makeSoldierString((*i).second->bravery).c_str(),
					makeSoldierString((*i).second->reactions).c_str(),
					makeSoldierString((*i).second->firing).c_str(),
					makeSoldierString((*i).second->throwing).c_str(),
					makeSoldierString((*i).second->melee).c_str(),
					makeSoldierString((*i).second->strength).c_str(),
					makeSoldierString(tmp).c_str(),
					makeSoldierString((*i).second->psiSkill).c_str(),
					"");
			}
			row++;
		}

		//set up first page at startup
		hidePageUI();
		firstPage();
	}

	FinishedCoverOperationDetailsState::~FinishedCoverOperationDetailsState() {}

	/**
	 * Closes the window and shows a pedia article if needed.
	 * @param action Pointer to an action.
	 */
	void FinishedCoverOperationDetailsState::btnOkClick(Action*)
	{
		_game->popState();
	}

	/**
	 * Changes state page.
	 * @param action Pointer to an action.
	 */
	void FinishedCoverOperationDetailsState::btnPageClick(Action*)
	{
		hidePageUI();
		_pageNumber = (_pageNumber + 1) % 2;
		if (_pageNumber == 0)
		{
			_btnPage->setText(tr("STR_STATS"));
			firstPage();
		}
		else
		{
			_btnPage->setText(tr("STR_RESULTS"));
			secondPage();
		}
	}

	/**
	 * Draws soldier line.
	 * @param stat Stat increase value.
	 */
	std::string FinishedCoverOperationDetailsState::makeSoldierString(int stat)
	{
		if (stat == 0) return "";

		std::ostringstream ss;
		ss << Unicode::TOK_COLOR_FLIP << '+' << stat << Unicode::TOK_COLOR_FLIP;
		return ss.str();
	}

	/**
	 * Hides all paging UI elements.
	 */
	void FinishedCoverOperationDetailsState::hidePageUI()
	{
		_txtItem->setVisible(false);
		_lstRecoveredItems->setVisible(false);
		_txtReputation->setVisible(false);
		_lstReputation->setVisible(false);
		_lstFunds->setVisible(false);
		_lstScore->setVisible(false);
		_txtSoldierStatus->setVisible(false);
		_lstSoldierStatus->setVisible(false);
		_txtMessage->setVisible(false);

		_txtSoldier->setVisible(false);
		_txtTU->setVisible(false);
		_txtStamina->setVisible(false);
		_txtHealth->setVisible(false);
		_txtBravery->setVisible(false);
		_txtReactions->setVisible(false);
		_txtFiring->setVisible(false);
		_txtThrowing->setVisible(false);
		_txtMelee->setVisible(false);
		_txtStrength->setVisible(false);
		_txtPsiStrength->setVisible(false);
		_txtPsiSkill->setVisible(false);
		_lstSoldierStats->setVisible(false);
	}

	/**
	 * Draws all first page UI elements.
	 */
	void FinishedCoverOperationDetailsState::firstPage()
	{
		if (_hasItems)
		{
			_txtItem->setVisible(true);
			_lstRecoveredItems->setVisible(true);
		}

		if (_hasRep)
		{
			_txtReputation->setVisible(true);
			_lstReputation->setVisible(true);
		}

		if (_hasFunds)
		{
			_lstFunds->setVisible(true);
		}
		if (_hasScore)
		{
			_lstScore->setVisible(true);
		}

		if (_hasSStatus)
		{
			_txtSoldierStatus->setVisible(true);
			_lstSoldierStatus->setVisible(true);
		}

		if (_hasMessage)
		{
			_txtMessage->setVisible(true);
		}
	}

	/**
	 * Draws all second page UI elements.
	 */
	void FinishedCoverOperationDetailsState::secondPage()
	{
		_txtSoldier->setVisible(true);
		_txtTU->setVisible(true);
		_txtStamina->setVisible(true);
		_txtHealth->setVisible(true);
		_txtBravery->setVisible(true);
		_txtReactions->setVisible(true);
		_txtFiring->setVisible(true);
		_txtThrowing->setVisible(true);
		_txtMelee->setVisible(true);
		_txtStrength->setVisible(true);
		_txtPsiStrength->setVisible(true);
		_txtPsiSkill->setVisible(true);
		_lstSoldierStats->setVisible(true);


	}


	/**
	* Shows a tooltip for the appropriate text.
	* @param action Pointer to an action.
	*/
	void FinishedCoverOperationDetailsState::txtTooltipIn(Action* action)
	{
		_currentTooltip = action->getSender()->getTooltip();
		_txtTooltip->setText(tr(_currentTooltip));
	}

	/**
	* Clears the tooltip text.
	* @param action Pointer to an action.
	*/
	void FinishedCoverOperationDetailsState::txtTooltipOut(Action* action)
	{
		if (_currentTooltip == action->getSender()->getTooltip())
		{
			_txtTooltip->setText("");
		}
	}


}

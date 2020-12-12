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
#include "AltMonthlyReportState.h"
#include <climits>
#include <sstream>
#include "../Engine/Game.h"
#include "../Engine/Options.h"
#include "../Engine/Unicode.h"
#include "../Engine/LocalizedText.h"
#include "../Interface/TextButton.h"
#include "../Interface/Window.h"
#include "../Interface/Text.h"
#include "../Menu/SaveGameState.h"
#include "../Menu/StatisticsState.h"
#include "../Menu/CutsceneState.h"
#include "PsiTrainingState.h"
#include "TrainingState.h"
#include "Globe.h"
#include "../Savegame/SavedGame.h"
#include "../Savegame/Base.h"
#include "../Savegame/GameTime.h"
#include "../Savegame/Region.h"
#include "../Savegame/Country.h"
#include "../Savegame/DiplomacyFaction.h"
#include "../Savegame/Base.h"
#include "../Savegame/SoldierDiary.h"
#include "../Mod/Mod.h"
#include "../Mod/RuleCountry.h"
#include "../Mod/RuleDiplomacyFaction.h"
#include "../Mod/RuleInterface.h"
#include "../Mod/RuleVideo.h"
#include "../FTA/MasterMind.h"
#include "../Battlescape/CommendationState.h"

namespace OpenXcom
{
/**
* Initializes all the elements in the Alternative Monthly Report screen.
* @param game Pointer to the core game.
* @param psi Show psi training afterwards?
* @param globe Pointer to the globe.
*/
AltMonthlyReportState::AltMonthlyReportState(Globe* globe) : _gameOver(0), _ratingTotal(0), _fundingDiff(0), _lastMonthsRating(0), _happyList(0), _sadList(0), _pactList(0), _cancelPactList(0), _loyalty(0)
{
	_globe = globe;
	// Create objects
	_window = new Window(this, 320, 200, 0, 0);
	_btnOk = new TextButton(50, 12, 135, 180);
	_btnBigOk = new TextButton(120, 18, 100, 174);
	_txtTitle = new Text(300, 17, 16, 8);
	_txtMonth = new Text(130, 9, 16, 24);
	_txtRating = new Text(160, 9, 146, 24);
	_txtLoyalty = new Text(300, 9, 16, 32);
	_txtMaintenance = new Text(130, 9, 16, 40);
	_txtBalance = new Text(160, 9, 146, 40);
	_txtBonus = new Text(300, 9, 16, 48);
	_txtDesc = new Text(280, 124, 16, 56);
	_txtFailure = new Text(290, 160, 15, 10);

	// Set palette
	setInterface("monthlyReport");

	add(_window, "window", "monthlyReport");
	add(_btnOk, "button", "monthlyReport");
	add(_btnBigOk, "button", "monthlyReport");
	add(_txtTitle, "text1", "monthlyReport");
	add(_txtMonth, "text1", "monthlyReport");
	add(_txtRating, "text1", "monthlyReport");
	add(_txtLoyalty, "text1", "monthlyReport");
	add(_txtMaintenance, "text1", "monthlyReport");
	add(_txtBalance, "text1", "monthlyReport");
	add(_txtBonus, "text1", "monthlyReport");
	add(_txtDesc, "text2", "monthlyReport");
	add(_txtFailure, "text2", "monthlyReport");

	centerAllSurfaces();

	// Set up objects
	setWindowBackground(_window, "monthlyReport");

	_btnOk->setText(tr("STR_OK"));
	_btnOk->onMouseClick((ActionHandler)&AltMonthlyReportState::btnOkClick);
	_btnOk->onKeyboardPress((ActionHandler)&AltMonthlyReportState::btnOkClick, Options::keyOk);
	_btnOk->onKeyboardPress((ActionHandler)&AltMonthlyReportState::btnOkClick, Options::keyCancel);

	_btnBigOk->setText(tr("STR_OK"));
	_btnBigOk->onMouseClick((ActionHandler)&AltMonthlyReportState::btnOkClick);
	_btnBigOk->onKeyboardPress((ActionHandler)&AltMonthlyReportState::btnOkClick, Options::keyOk);
	_btnBigOk->onKeyboardPress((ActionHandler)&AltMonthlyReportState::btnOkClick, Options::keyCancel);
	_btnBigOk->setVisible(false);

	_txtTitle->setBig();
	_txtTitle->setText(tr("STR_XCOM_PROJECT_MONTHLY_REPORT"));

	_txtFailure->setBig();
	_txtFailure->setAlign(ALIGN_CENTER);
	_txtFailure->setVerticalAlign(ALIGN_MIDDLE);
	_txtFailure->setWordWrap(true);
	_txtFailure->setText(tr("STR_YOU_HAVE_FAILED"));
	_txtFailure->setVisible(false);

	std::string updates = calculateUpdates();

	int month = _game->getSavedGame()->getTime()->getMonth() - 1, year = _game->getSavedGame()->getTime()->getYear();
	if (month == 0)
	{
		month = 12;
		year--;
	}
	std::string m;
	switch (month)
	{
	case 1: m = "STR_JAN"; break;
	case 2: m = "STR_FEB"; break;
	case 3: m = "STR_MAR"; break;
	case 4: m = "STR_APR"; break;
	case 5: m = "STR_MAY"; break;
	case 6: m = "STR_JUN"; break;
	case 7: m = "STR_JUL"; break;
	case 8: m = "STR_AUG"; break;
	case 9: m = "STR_SEP"; break;
	case 10: m = "STR_OCT"; break;
	case 11: m = "STR_NOV"; break;
	case 12: m = "STR_DEC"; break;
	default: m = "";
	}
	_txtMonth->setText(tr("STR_MONTH").arg(tr(m)).arg(year));

	// Calculate rating
	int difficulty_threshold = _game->getMod()->getDefeatScore() + 100 * _game->getSavedGame()->getDifficultyCoefficient();
	std::string rating = tr("STR_RATING_TERRIBLE");
	if (_ratingTotal > difficulty_threshold - 300)
	{
		rating = tr("STR_RATING_POOR");
	}
	if (_ratingTotal > difficulty_threshold)
	{
		rating = tr("STR_RATING_OK");
	}
	if (_ratingTotal > 0)
	{
		rating = tr("STR_RATING_GOOD");
	}
	if (_ratingTotal > 500)
	{
		rating = tr("STR_RATING_EXCELLENT");
	}

	if (!_game->getMod()->getMonthlyRatings()->empty())
	{
		rating = "";
		int temp = INT_MIN;
		const std::map<int, std::string>* monthlyRatings = _game->getMod()->getMonthlyRatings();
		for (std::map<int, std::string>::const_iterator i = monthlyRatings->begin(); i != monthlyRatings->end(); ++i)
		{
			if (i->first > temp && i->first <= _ratingTotal)
			{
				temp = i->first;
				rating = tr(i->second);
			}
		}
	}

	_txtRating->setText(tr("STR_MONTHLY_RATING").arg(_ratingTotal).arg(rating));

	std::ostringstream ss;
	ss << tr("STR_LOYALTY") << "> " << Unicode::TOK_COLOR_FLIP << _game->getSavedGame()->getLoyalty();
	_txtLoyalty->setText(ss.str());

	std::ostringstream ss2;
	ss2 << tr("STR_MAINTENANCE") << "> " << Unicode::TOK_COLOR_FLIP << Unicode::formatFunding(_game->getSavedGame()->getBaseMaintenance());
	_txtMaintenance->setText(ss2.str());

	_txtBonus->setVisible(false);
	_txtDesc->setY(_txtBonus->getY());

	std::ostringstream ss3;
	ss3 << tr("STR_BALANCE") << "> " << Unicode::TOK_COLOR_FLIP << Unicode::formatFunding(_game->getSavedGame()->getFunds());
	_txtBalance->setText(ss3.str());

	_txtDesc->setWordWrap(true);

	// calculate satisfaction
	std::ostringstream ss5;
	std::string loyaltyString = tr("STR_COUNCIL_IS_DISSATISFIED");
	bool resetWarning = true;
	if (!_game->getMod()->getLoyaltyRatings()->empty())  //FtA - checks around loyalty
	{
		if (_lastMonthsLoyalty <= difficulty_threshold && _loyalty <= difficulty_threshold)
		{
			loyaltyString = tr("STR_YOU_HAVE_NOT_SUCCEEDED");
			_pactList.erase(_pactList.begin(), _pactList.end());
			_cancelPactList.erase(_cancelPactList.begin(), _cancelPactList.end());
			_happyList.erase(_happyList.begin(), _happyList.end());
			_sadList.erase(_sadList.begin(), _sadList.end());
			_gameOver = 1;
		}
		else //calculate rating message
		{
			int temp = INT_MIN;
			const std::map<int, std::string>* loyaltyRatings = _game->getMod()->getLoyaltyRatings();
			for (std::map<int, std::string>::const_iterator i = loyaltyRatings->begin(); i != loyaltyRatings->end(); ++i)
			{
				if (i->first > temp && i->first <= _loyalty)
				{
					temp = i->first;
					loyaltyString = tr(i->second);
				}
			}
		}
	}
	else //vanilla - checks around rating
	{
		if (_ratingTotal > difficulty_threshold)
		{
			loyaltyString = tr("STR_COUNCIL_IS_GENERALLY_SATISFIED");
		}
		if (_ratingTotal > 500)
		{
			loyaltyString = tr("STR_COUNCIL_IS_VERY_PLEASED");
		}
		if (_lastMonthsRating <= difficulty_threshold && _ratingTotal <= difficulty_threshold)
		{
			loyaltyString = tr("STR_YOU_HAVE_NOT_SUCCEEDED");
			_pactList.erase(_pactList.begin(), _pactList.end());
			_cancelPactList.erase(_cancelPactList.begin(), _cancelPactList.end());
			_happyList.erase(_happyList.begin(), _happyList.end());
			_sadList.erase(_sadList.begin(), _sadList.end());
			_gameOver = 1;
		}
	}

	ss5 << "\n";
	ss5 << loyaltyString;

	if (!_gameOver)
	{
		if (_game->getSavedGame()->getFunds() <= _game->getMod()->getDefeatFunds())
		{
			if (_game->getSavedGame()->getWarned())
			{
				ss5.str("");
				ss5 << tr("STR_YOU_HAVE_NOT_SUCCEEDED");
				_pactList.erase(_pactList.begin(), _pactList.end());
				_cancelPactList.erase(_cancelPactList.begin(), _cancelPactList.end());
				_happyList.erase(_happyList.begin(), _happyList.end());
				_sadList.erase(_sadList.begin(), _sadList.end());
				_gameOver = 2;
			}
			else
			{
				ss5 << "\n\n" << tr("STR_COUNCIL_REDUCE_DEBTS");
				_game->getSavedGame()->setWarned(true);
				resetWarning = false;
			}
		}
	}
	if (resetWarning && _game->getSavedGame()->getWarned())
	{
		_game->getSavedGame()->setWarned(false);
	}

	ss5 << updates;
	
	_txtDesc->setText(ss5.str());
}

/**
*
*/
AltMonthlyReportState::~AltMonthlyReportState()
{
}

/**
* Returns to the previous screen.
* @param action Pointer to an action.
*/
void AltMonthlyReportState::btnOkClick(Action*)
{
	if (!_gameOver)
	{
		_game->popState();
		// Award medals for service time
		// Iterate through all your bases
		for (std::vector<Base*>::iterator b = _game->getSavedGame()->getBases()->begin(); b != _game->getSavedGame()->getBases()->end(); ++b)
		{
			// Iterate through all your soldiers
			for (std::vector<Soldier*>::iterator s = (*b)->getSoldiers()->begin(); s != (*b)->getSoldiers()->end(); ++s)
			{
				Soldier* soldier = _game->getSavedGame()->getSoldier((*s)->getId());
				// Award medals to eligible soldiers
				soldier->getDiary()->addMonthlyService();
				if (soldier->getDiary()->manageCommendations(_game->getMod(), _game->getSavedGame()->getMissionStatistics()))
				{
					_soldiersMedalled.push_back(soldier);
				}
			}
		}
		if (!_soldiersMedalled.empty())
		{
			_game->pushState(new CommendationState(_soldiersMedalled));
		}

		bool psi = false;
		for (std::vector<Base*>::const_iterator b = _game->getSavedGame()->getBases()->begin(); b != _game->getSavedGame()->getBases()->end(); ++b)
		{
			psi = psi || (*b)->getAvailablePsiLabs();
		}
		if (psi && !Options::anytimePsiTraining)
		{
			_game->pushState(new PsiTrainingState);
		}
		// Autosave
		if (_game->getSavedGame()->isIronman())
		{
			_game->pushState(new SaveGameState(OPT_GEOSCAPE, SAVE_IRONMAN, _palette));
		}
		else if (Options::autosave)
		{
			_game->pushState(new SaveGameState(OPT_GEOSCAPE, SAVE_AUTO_GEOSCAPE, _palette));
		}
	}
	else
	{
		if (_txtFailure->getVisible())
		{
			_game->popState(); // in case the cutscene is not marked as "game over" (by accident or not) let's return to the geoscape

			std::string cutsceneId;
			if (_gameOver == 1)
				cutsceneId = _game->getMod()->getLoseRatingCutscene();
			else
				cutsceneId = _game->getMod()->getLoseMoneyCutscene();

			const RuleVideo* videoRule = _game->getMod()->getVideo(cutsceneId, true);
			if (videoRule->getLoseGame())
			{
				_game->getSavedGame()->setEnding(END_LOSE);
			}

			_game->pushState(new CutsceneState(cutsceneId));
			if (_game->getSavedGame()->isIronman())
			{
				_game->pushState(new SaveGameState(OPT_GEOSCAPE, SAVE_IRONMAN, _palette));
			}
		}
		else
		{
			_window->setColor(_game->getMod()->getInterface("monthlyReport")->getElement("window")->color2);
			_txtTitle->setVisible(false);
			_txtMonth->setVisible(false);
			_txtRating->setVisible(false);
			_txtLoyalty->setVisible(false);
			_txtMaintenance->setVisible(false);
			_txtBalance->setVisible(false);
			_txtBonus->setVisible(false);
			_txtDesc->setVisible(false);
			_btnOk->setVisible(false);
			_btnBigOk->setVisible(true);
			_txtFailure->setVisible(true);
			_game->getMod()->playMusic("GMLOSE");
		}
	}
}

/**
* Update all faction and loyalty conditions, prepare FtA-specific
* data to show in state and finally calculate our overall total score,
* with thanks to Volutar for the formulas.
*/
std::string AltMonthlyReportState::calculateUpdates()
{
	// initialize all our variables.
	SavedGame* save = _game->getSavedGame();
	_lastMonthsRating = 0;
	int xcomSubTotal = 0;
	int xcomTotal = 0;
	int alienTotal = 0;
	int monthOffset = save->getFundsList().size() - 2;
	int lastMonthOffset = save->getFundsList().size() - 3;
	if (lastMonthOffset < 0)
	{
		lastMonthOffset += 2;
	}
	std::ostringstream ss;

	// update activity meters, calculate a total score based on regional activity
	// and gather last month's score
	for (std::vector<Region*>::iterator k = save->getRegions()->begin(); k != save->getRegions()->end(); ++k)
	{
		(*k)->newMonth();
		if ((*k)->getActivityXcom().size() > 2)
		{
			_lastMonthsRating += (*k)->getActivityXcom().at(lastMonthOffset) - (*k)->getActivityAlien().at(lastMonthOffset);
		}
		xcomSubTotal += (*k)->getActivityXcom().at(monthOffset);
		alienTotal += (*k)->getActivityAlien().at(monthOffset);
	}

	// the council is more lenient after the first month
	if (save->getMonthsPassed() > 1)
	{
		save->getResearchScores().at(monthOffset) += 400;
	}

	xcomTotal = save->getResearchScores().at(monthOffset) + xcomSubTotal;

	if (save->getResearchScores().size() > 2)
	{
		_lastMonthsRating += save->getResearchScores().at(lastMonthOffset);
	}

	//calculate total.
	_ratingTotal = xcomTotal - alienTotal;

	// update factions
	std::vector<OpenXcom::DiplomacyFaction*> factions = _game->getSavedGame()->getDiplomacyFactions();
	if (!factions.empty())
	{
		ss << "\n\n";
		for (std::vector<DiplomacyFaction*>::iterator k = factions.begin(); k != factions.end(); ++k)
		{
			bool changed = _game->getMasterMind()->updateReputationLvl(*k);

			if ((*k)->isThisMonthDiscovered())
			{
				ss << tr("STR_WE_DISCOVERED_FACTION");
				ss << tr((*k)->getRules().getName());
				ss << ". ";
				ss << tr("STR_THEIR_ATTITUDE_TO_US");
				ss <<  tr((*k)->getReputationName());
				ss << ". ";
				ss << "\n\n";
			}
			else if (changed && (*k)->isDiscovered())
			{
				ss << tr((*k)->getRules().getName());
				ss << tr("STR_ATTITUDE_BECOME");
				ss << tr((*k)->getReputationName());
				ss << ". ";
				ss << "\n\n";
			}
		}

	}

	//handle loyalty updating
	_loyalty = save->getLoyalty();
	_lastMonthsLoyalty = save->getLastMonthsLoyalty();

	int funds = save->getFunds();
	if (funds < 0)
	{
		int noFundsV = _game->getMod()->getLoyaltyNoFundsValue();
		if (funds < noFundsV)
		{
			int	discontent = _game->getMod()->getLoyaltyNoFundsPenalty() * _game->getSavedGame()->getDifficultyCoefficient();
			auto stuffMessage = tr("STR_STUFF_NO_MONEY1");

			if (funds < noFundsV * 2)
			{
				discontent *= 2;
				stuffMessage = tr("STR_STUFF_NO_MONEY2");
			}
			if (funds < noFundsV * 5)
			{
				discontent *= 2;
				stuffMessage = tr("STR_STUFF_NO_MONEY5");
			}
			if (funds < noFundsV * 10)
			{
				discontent *= 2;
				stuffMessage = tr("STR_STUFF_NO_MONEY10");
			}
			if (funds < noFundsV * 20)
			{
				discontent *= 2;
				stuffMessage = tr("STR_STUFF_NO_MONEY20");
			}
			ss << stuffMessage;
			_game->getMasterMind()->updateLoyalty(discontent, XCOM_GEOSCAPE);
		}

	}
		//update loyalty data after it was loaded
	_game->getSavedGame()->setLastMonthsLoyalty(_loyalty);

	return ss.str();
	



}

/**
* Initializes all the elements in the Alpha Ends screen.
*/
AlphaGameVersionEnds::AlphaGameVersionEnds() : _gameOver(0)
{

	// Create objects
	_window = new Window(this, 320, 200, 0, 0);
	_btnOk = new TextButton(50, 12, 135, 180);
	_txtTitle = new Text(300, 34, 16, 8);
	_txtDesc = new Text(280, 130, 16, 46);

	// Set palette
	setInterface("monthlyReport");

	add(_window, "window", "monthlyReport");
	add(_btnOk, "button", "monthlyReport");
	add(_txtTitle, "text1", "monthlyReport");
	add(_txtDesc, "text2", "monthlyReport");

	centerAllSurfaces();

	// Set up objects
	setWindowBackground(_window, "monthlyReport");

	_btnOk->setText(tr("STR_OK"));
	_btnOk->onMouseClick((ActionHandler)&AlphaGameVersionEnds::btnOkClick);
	_btnOk->onKeyboardPress((ActionHandler)&AlphaGameVersionEnds::btnOkClick, Options::keyOk);
	_btnOk->onKeyboardPress((ActionHandler)&AlphaGameVersionEnds::btnOkClick, Options::keyCancel);


	_txtTitle->setBig();
	_txtTitle->setText(tr("STR_FTA_ALPHA_END"));
	_txtTitle->setAlign(ALIGN_CENTER);
	_txtTitle->setWordWrap(true);
	_txtDesc->setText(tr("STR_FTA_ALPHA_END_DESC"));
	_txtDesc->setWordWrap(true);
}

/**
*
*/
AlphaGameVersionEnds::~AlphaGameVersionEnds()
{
}

/**
* Returns to the previous screen.
* @param action Pointer to an action.
*/
void AlphaGameVersionEnds::btnOkClick(Action*)
{
	_game->getSavedGame()->setEnding(END_WIN);
	_game->pushState(new StatisticsState());
	_game->getMod()->playMusic("GMLOSE");
}

}

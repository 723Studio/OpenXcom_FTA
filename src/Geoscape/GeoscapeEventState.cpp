/*
 * Copyright 2010-2019 OpenXcom Developers.
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
#include "GeoscapeEventState.h"
#include <map>
#include "../Engine/Game.h"
#include "../Engine/LocalizedText.h"
#include "../Engine/RNG.h"
#include "../Engine/Action.h"
#include "../Interface/Text.h"
#include "../Interface/TextButton.h"
#include "../Interface/Window.h"
#include "../Mod/City.h"
#include "../Mod/Mod.h"
#include "../Mod/RuleEvent.h"
#include "../Mod/RuleRegion.h"
#include "../Savegame/Base.h"
#include "../Savegame/GeoscapeEvent.h"
#include "../Savegame/Region.h"
#include "../Savegame/SavedGame.h"
#include "../Savegame/Transfer.h"
#include "../Ufopaedia/Ufopaedia.h"
#include "../FTA/MasterMind.h"

namespace OpenXcom
{

/**
	* Initializes all the elements in the Geoscape Event window.
	* @param geoEvent Pointer to the event.
	*/
GeoscapeEventState::GeoscapeEventState(GeoscapeEvent* geoEvent) : _eventRule(geoEvent->getRules())
{
	_screen = false;

	// Create objects
	_window = new Window(this, 256, 176, 32, 12, POPUP_BOTH);
	_txtTitle = new Text(236, 32, 42, 26);
	_txtMessage = new Text(236, 94, 42, 61);
	_btnOk = new TextButton(100, 18, 110, 158);

	_btnAnswerOne = new TextButton(115, 18, 42, 158);
	_btnAnswerTwo = new TextButton(115, 18, 163, 158);
	_btnAnswerThree = new TextButton(236, 16, 42, 162);
	_btnAnswerFour = new TextButton(115, 16, 163, 162);

	// Set palette
	setInterface("geoscapeEvent");

	add(_window, "window", "geoscapeEvent");
	add(_txtTitle, "text1", "geoscapeEvent");
	add(_txtMessage, "text2", "geoscapeEvent");
	add(_btnOk, "button", "geoscapeEvent");

	add(_btnAnswerOne, "button", "geoscapeEvent");
	add(_btnAnswerTwo, "button", "geoscapeEvent");
	add(_btnAnswerThree, "button", "geoscapeEvent");
	add(_btnAnswerFour, "button", "geoscapeEvent");

	centerAllSurfaces();

	// Set up objects
	_window->setBackground(_game->getMod()->getSurface(_eventRule.getBackground()));

	_txtTitle->setAlign(ALIGN_CENTER);
	_txtTitle->setBig();
	_txtTitle->setWordWrap(true);
	_txtTitle->setText(tr(_eventRule.getName()));

	_txtMessage->setVerticalAlign(ALIGN_TOP);
	_txtMessage->setWordWrap(true);
	_txtMessage->setText(tr(_eventRule.getDescription()));

	_btnOk->setText(tr("STR_OK"));
	_btnOk->onMouseClick((ActionHandler)&GeoscapeEventState::btnOkClick);
	_btnOk->onKeyboardPress((ActionHandler)&GeoscapeEventState::btnOkClick, Options::keyOk);
	_btnOk->onKeyboardPress((ActionHandler)&GeoscapeEventState::btnOkClick, Options::keyCancel);

	_btnAnswerOne->setVisible(false);
	_btnAnswerTwo->setVisible(false);
	_btnAnswerThree->setVisible(false);
	_btnAnswerFour->setVisible(false);
	_btnAnswerOne->setTooltip("0");
	_btnAnswerTwo->setTooltip("1");
	_btnAnswerThree->setTooltip("2");
	_btnAnswerFour->setTooltip("3");

	auto customAnswers = _eventRule.getCustomAnswers();
	int answersN = customAnswers.size();
	if (answersN > 0)
	{
		_btnOk->setVisible(false);
		_btnAnswerOne->setText(tr(customAnswers[0].title));
		_btnAnswerTwo->setText(tr(customAnswers[1].title));
		_btnAnswerThree->setText(tr(customAnswers[2].title));
		_btnAnswerFour->setText(tr(customAnswers[3].title));
		if (answersN > 1) //case with answersN == 1 is not allowed on loading
		{
			_btnAnswerOne->setVisible(true);
			_btnAnswerTwo->setVisible(true);
			if (answersN > 2)
			{
				_btnAnswerThree->setVisible(true);
				_txtMessage->setHeight(78);
				_btnAnswerOne->setHeight(16);
				_btnAnswerTwo->setHeight(16);
				_btnAnswerOne->setY(142);
				_btnAnswerTwo->setY(142);
				if (answersN > 3)
				{
					_btnAnswerFour->setVisible(true);
					_btnAnswerThree->setWidth(115);
				}
			}
		}
	}

	_btnAnswerOne->onMouseClick((ActionHandler)&GeoscapeEventState::btnAnswerClick);
	_btnAnswerOne->onMouseClick((ActionHandler)&GeoscapeEventState::btnAnswerClick, SDL_BUTTON_RIGHT);
	_btnAnswerTwo->onMouseClick((ActionHandler)&GeoscapeEventState::btnAnswerClick);
	_btnAnswerTwo->onMouseClick((ActionHandler)&GeoscapeEventState::btnAnswerClick, SDL_BUTTON_RIGHT);
	_btnAnswerThree->onMouseClick((ActionHandler)&GeoscapeEventState::btnAnswerClick);
	_btnAnswerThree->onMouseClick((ActionHandler)&GeoscapeEventState::btnAnswerClick, SDL_BUTTON_RIGHT);
	_btnAnswerFour->onMouseClick((ActionHandler)&GeoscapeEventState::btnAnswerClick);
	_btnAnswerFour->onMouseClick((ActionHandler)&GeoscapeEventState::btnAnswerClick, SDL_BUTTON_RIGHT);


	eventLogic();
}

/**
	* Helper performing event logic.
	*/
void GeoscapeEventState::eventLogic()
{
	SavedGame* save = _game->getSavedGame();
	Base* hq = save->getBases()->front();
	const Mod* mod = _game->getMod();
	const RuleEvent& rule = _eventRule;

	RuleRegion* regionRule = nullptr;
	if (!rule.getRegionList().empty())
	{
		size_t pickRegion = RNG::generate(0, rule.getRegionList().size() - 1);
		auto regionName = rule.getRegionList().at(pickRegion);
		regionRule = _game->getMod()->getRegion(regionName, true);
		std::string place = tr(regionName);

		if (rule.isCitySpecific())
		{
			size_t cities = regionRule->getCities()->size();
			if (cities > 0)
			{
				size_t pickCity = RNG::generate(0, cities - 1);
				City* city = regionRule->getCities()->at(pickCity);
				place = city->getName(_game->getLanguage());
			}
		}

		std::string titlePlus = tr(rule.getName()).arg(place);
		_txtTitle->setText(titlePlus);

		std::string messagePlus = tr(rule.getDescription()).arg(place);
		_txtMessage->setText(messagePlus);
	}

	// 1. give/take score points
	if (regionRule)
	{
		for (auto region : *_game->getSavedGame()->getRegions())
		{
			if (region->getRules() == regionRule)
			{
				region->addActivityXcom(rule.getPoints());
				break;
			}
		}
	}
	else
	{
		save->addResearchScore(rule.getPoints());
	}

	// 2. give/take funds
	save->setFunds(save->getFunds() + rule.getFunds());

	// 3. spawn/transfer item into the HQ
	std::map<std::string, int> itemsToTransfer;

	for (auto& pair : rule.getEveryMultiItemList())
	{
		const RuleItem* itemRule = mod->getItem(pair.first, true);
		if (itemRule)
		{
			itemsToTransfer[itemRule->getType()] += pair.second;
		}
	}

	for (auto& itemName : rule.getEveryItemList())
	{
		const RuleItem* itemRule = mod->getItem(itemName, true);
		if (itemRule)
		{
			itemsToTransfer[itemRule->getType()] += 1;
		}
	}

	if (!rule.getRandomItemList().empty())
	{
		size_t pickItem = RNG::generate(0, rule.getRandomItemList().size() - 1);
		const RuleItem* randomItem = mod->getItem(rule.getRandomItemList().at(pickItem), true);
		if (randomItem)
		{
			itemsToTransfer[randomItem->getType()] += 1;
		}
	}

	if (!rule.getWeightedItemList().empty())
	{
		const RuleItem* randomItem = mod->getItem(rule.getWeightedItemList().choose(), true);
		if (randomItem)
		{
			itemsToTransfer[randomItem->getType()] += 1;
		}
	}

	for (auto& ti : itemsToTransfer)
	{
		Transfer* t = new Transfer(1);
		t->setItems(ti.first, ti.second);
		hq->getTransfers()->push_back(t);
	}

	// 4. give bonus research
	std::vector<const RuleResearch*> possibilities;

	for (auto rName : rule.getResearchList())
	{
		const RuleResearch* rRule = mod->getResearch(rName, true);
		if (!save->isResearched(rRule, false))
		{
			possibilities.push_back(rRule);
		}
	}

	if (!possibilities.empty())
	{
		size_t pickResearch = RNG::generate(0, possibilities.size() - 1);
		const RuleResearch* eventResearch = possibilities.at(pickResearch);
		save->addFinishedResearch(eventResearch, mod, hq, true);
		_researchName = eventResearch->getName();

		if (!eventResearch->getLookup().empty())
		{
			const RuleResearch* lookupResearch = mod->getResearch(eventResearch->getLookup(), true);
			save->addFinishedResearch(lookupResearch, mod, hq, true);
			_researchName = lookupResearch->getName();
		}
	}
}

/**
	*
	*/
GeoscapeEventState::~GeoscapeEventState()
{
	// Empty by design
}

/**
	* Initializes the state.
	*/
void GeoscapeEventState::init()
{
	State::init();

	if (!_eventRule.getMusic().empty())
	{
		_game->getMod()->playMusic(_eventRule.getMusic());
	}
}

/**
	* Closes the window and shows a pedia article if needed.
	* @param action Pointer to an action.
	*/
void GeoscapeEventState::btnOkClick(Action*)
{
	_game->popState();

	if (!_researchName.empty())
	{
		Ufopaedia::openArticle(_game, _researchName);
	}
}

void GeoscapeEventState::btnAnswerClick(Action* action)
{
	auto customAnswers = _eventRule.getCustomAnswers();
	std::string buttonTooltip = action->getSender()->getTooltip();
	int n = 0;
	if (buttonTooltip == "1") n = 1;
	if (buttonTooltip == "2") n = 2;
	if (buttonTooltip == "3") n = 3;

	if (action->getDetails()->button.button == SDL_BUTTON_RIGHT)
	{
		if (!customAnswers[n].description.empty())
		{
			_game->pushState(new GeoscapeEventAnswerInfoState(_eventRule, customAnswers[n].description));
		}
	}
	if (action->getDetails()->button.button == SDL_BUTTON_LEFT)
	{
		for (auto eventName : customAnswers[n].spawnEvent)
		{
			bool success = _game->getMasterMind()->spawnEvent(eventName);
		}

		_game->popState();
		if (!_researchName.empty())
		{
			Ufopaedia::openArticle(_game, _researchName);
		}
	}
}


/**
* Initializes all the elements in the GeoscapeEventAnswerInfoState window.
* @param rule Pointer to the event ruleset.
* @param descr string for state description.
*/
GeoscapeEventAnswerInfoState::GeoscapeEventAnswerInfoState(RuleEvent rule, std::string descr)
{
	_screen = false;

	// Create objects
	_window = new Window(this, 256, 135, 32, 31, POPUP_BOTH);
	_txtDescription = new Text(236, 94, 42, 42);
	_btnOk = new TextButton(100, 16, 110, 140);

	// Set palette
	setInterface("geoscapeEvent");

	add(_window, "window", "geoscapeEvent");
	add(_txtDescription, "text2", "geoscapeEvent");
	add(_btnOk, "button", "geoscapeEvent");

	centerAllSurfaces();

	// Set up objects
	_window->setBackground(_game->getMod()->getSurface(rule.getBackground()));

	_txtDescription->setVerticalAlign(ALIGN_MIDDLE);
	_txtDescription->setWordWrap(true);
	_txtDescription->setText(tr(descr));

	_btnOk->setText(tr("STR_OK"));
	_btnOk->onMouseClick((ActionHandler)&GeoscapeEventAnswerInfoState::btnOkClick);
	_btnOk->onKeyboardPress((ActionHandler)&GeoscapeEventAnswerInfoState::btnOkClick, Options::keyOk);
	_btnOk->onKeyboardPress((ActionHandler)&GeoscapeEventAnswerInfoState::btnOkClick, Options::keyCancel);

}

GeoscapeEventAnswerInfoState::~GeoscapeEventAnswerInfoState()
{
}

void GeoscapeEventAnswerInfoState::btnOkClick(Action*)
{
	_game->popState();
}

}

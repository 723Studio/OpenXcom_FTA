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
#include "GeoscapeState.h"
#include <map>
#include "../Basescape/SellState.h"
#include "../Engine/Game.h"
#include "../Engine/LocalizedText.h"
#include "../Engine/RNG.h"
#include "../Engine/Action.h"
#include "../Interface/Text.h"
#include "../Interface/TextList.h"
#include "../Interface/TextButton.h"
#include "../Interface/ToggleTextButton.h"
#include "../Interface/Window.h"
#include "../Menu/CutsceneState.h"
#include "../Menu/ErrorMessageState.h"
#include "../Mod/City.h"
#include "../Mod/Mod.h"
#include "../Mod/RuleEvent.h"
#include "../Mod/RuleInterface.h"
#include "../Mod/RuleRegion.h"
#include "../Mod/RuleSoldier.h"
#include "../Mod/RuleDiplomacyFaction.h"
#include "../Mod/RuleVideo.h"
#include "../Savegame/Base.h"
#include "../Savegame/ItemContainer.h"
#include "../Savegame/Region.h"
#include "../Savegame/SavedGame.h"
#include "../Savegame/Soldier.h"
#include "../Savegame/Transfer.h"
#include "../Savegame/DiplomacyFaction.h"
#include "../Ufopaedia/Ufopaedia.h"
#include "../FTA/MasterMind.h"

namespace OpenXcom
{

/**
 * Initializes all the elements in the Geoscape Event window.
 * @param geoEvent Pointer to the event.
 */
GeoscapeEventState::GeoscapeEventState(const RuleEvent& eventRule) : _eventRule(eventRule)
{
	_screen = false;

	// Create objects
	_window = new Window(this, 256, 176, 32, 12, POPUP_BOTH);
	_txtTitle = new Text(236, 32, 42, 23);
	_txtMessage = new Text(236, 96, 42, 58);
	_btnOk = new TextButton(108, 18, 48, 158);
	_btnItemsArriving = new ToggleTextButton(108, 18, 164, 158);
	_txtItem = new Text(114, 9, 44, 58);
	_txtQuantity = new Text(94, 9, 182, 58);
	_lstTransfers = new TextList(216, 80, 42, 69);

	_btnAnswerOne = new TextButton(115, 18, 42, 158);
	_btnAnswerTwo = new TextButton(115, 18, 163, 158);
	_btnAnswerThree = new TextButton(236, 16, 42, 162);
	_btnAnswerFour = new TextButton(115, 16, 163, 162);

	_txtTooltip = new Text(115, 10, 42, 148);

	// Set palette
	setInterface("geoscapeEvent");

	add(_window, "window", "geoscapeEvent");
	add(_txtTitle, "text1", "geoscapeEvent");
	add(_txtMessage, "text2", "geoscapeEvent");
	add(_btnOk, "button", "geoscapeEvent");
	add(_btnItemsArriving, "button", "geoscapeEvent");
	add(_txtItem, "text2", "geoscapeEvent");
	add(_txtQuantity, "text2", "geoscapeEvent");
	add(_lstTransfers, "list", "geoscapeEvent");

	add(_btnAnswerOne, "button", "geoscapeEvent");
	add(_btnAnswerTwo, "button", "geoscapeEvent");
	add(_btnAnswerThree, "button", "geoscapeEvent");
	add(_btnAnswerFour, "button", "geoscapeEvent");

	add(_txtTooltip, "text1", "geoscapeEvent");


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
	if (_eventRule.alignBottom())
	{
		_txtMessage->setVerticalAlign(ALIGN_BOTTOM);
	}
	_txtMessage->setScrollable(true);

	_btnOk->setText(tr("STR_OK"));
	_btnOk->onMouseClick((ActionHandler)&GeoscapeEventState::btnOkClick);
	_btnOk->onKeyboardPress((ActionHandler)&GeoscapeEventState::btnOkClick, Options::keyOk);
	_btnOk->onKeyboardPress((ActionHandler)&GeoscapeEventState::btnOkClick, Options::keyCancel);

	_btnAnswerOne->setVisible(false);
	_btnAnswerTwo->setVisible(false);
	_btnAnswerThree->setVisible(false);
	_btnAnswerFour->setVisible(false);

	_txtTooltip->setText("");

	bool bTooltipIsPresent = false;
	_customAnswers = _eventRule.getCustomAnswers();
	_btnOk->setVisible(_customAnswers.empty());

	if (_customAnswers.size() >= 2)
	{
		_btnAnswerOne->setText(tr(_customAnswers[0].title));
		_btnAnswerTwo->setText(tr(_customAnswers[1].title));
		if (!_customAnswers[0].description.empty())
		{
			_btnAnswerOne->setTooltip("STR_BUTTON_HINT");
			bTooltipIsPresent = true;
		}
		if (!_customAnswers[1].description.empty())
		{
			_btnAnswerTwo->setTooltip("STR_BUTTON_HINT");
			bTooltipIsPresent = true;
		}
		_btnAnswerOne->setVisible(true);
		_btnAnswerTwo->setVisible(true);
	}

	if (_customAnswers.size() >= 3)
	{
		_btnAnswerThree->setText(tr(_customAnswers[2].title));
		_btnAnswerThree->setVisible(true);
		if (!_customAnswers[2].description.empty())
		{
			_btnAnswerThree->setTooltip("STR_BUTTON_HINT");
			bTooltipIsPresent = true;
		}
		_txtMessage->setHeight(78);
		_btnAnswerOne->setHeight(16);
		_btnAnswerTwo->setHeight(16);
		_btnAnswerOne->setY(_btnAnswerOne->getY() - 16);
		_btnAnswerTwo->setY(_btnAnswerTwo->getY() - 16);
		_txtTooltip->setY(_txtTooltip->getY() - 16);
	}

	if (_customAnswers.size() >= 4)
	{
		_btnAnswerFour->setText(tr(_customAnswers[3].title));
		_btnAnswerFour->setVisible(true);
		if (!_customAnswers[3].description.empty())
		{
			_btnAnswerFour->setTooltip("STR_BUTTON_HINT");
			bTooltipIsPresent = true;
		}
		_btnAnswerThree->setWidth(115);
	}

	if (bTooltipIsPresent)
	{
		_txtMessage->setHeight(_txtMessage->getHeight() - _txtTooltip->getHeight());
	}

	_btnAnswerOne->onMouseClick((ActionHandler)&GeoscapeEventState::btnAnswerOneClick);
	_btnAnswerOne->onMouseClick((ActionHandler)&GeoscapeEventState::btnAnswerOneClickRight, SDL_BUTTON_RIGHT);
	_btnAnswerOne->onMouseIn((ActionHandler)&GeoscapeEventState::txtTooltipIn);
	_btnAnswerOne->onMouseOut((ActionHandler)&GeoscapeEventState::txtTooltipOut);
	_btnAnswerTwo->onMouseClick((ActionHandler)&GeoscapeEventState::btnAnswerTwoClick);
	_btnAnswerTwo->onMouseClick((ActionHandler)&GeoscapeEventState::btnAnswerTwoClickRight, SDL_BUTTON_RIGHT);
	_btnAnswerTwo->onMouseIn((ActionHandler)&GeoscapeEventState::txtTooltipIn);
	_btnAnswerTwo->onMouseOut((ActionHandler)&GeoscapeEventState::txtTooltipOut);
	_btnAnswerThree->onMouseClick((ActionHandler)&GeoscapeEventState::btnAnswerThreeClick);
	_btnAnswerThree->onMouseClick((ActionHandler)&GeoscapeEventState::btnAnswerThreeClickRight, SDL_BUTTON_RIGHT);
	_btnAnswerThree->onMouseIn((ActionHandler)&GeoscapeEventState::txtTooltipIn);
	_btnAnswerThree->onMouseOut((ActionHandler)&GeoscapeEventState::txtTooltipOut);
	_btnAnswerFour->onMouseClick((ActionHandler)&GeoscapeEventState::btnAnswerFourClick);
	_btnAnswerFour->onMouseClick((ActionHandler)&GeoscapeEventState::btnAnswerFourClickRight, SDL_BUTTON_RIGHT);
	_btnAnswerFour->onMouseIn((ActionHandler)&GeoscapeEventState::txtTooltipIn);
	_btnAnswerFour->onMouseOut((ActionHandler)&GeoscapeEventState::txtTooltipOut);
	_btnItemsArriving->setText(tr("STR_ITEMS_ARRIVING"));
	_btnItemsArriving->onMouseClick((ActionHandler)&GeoscapeEventState::btnItemsArrivingClick);

	_txtItem->setText(tr("STR_ITEM"));
	_txtQuantity->setText(tr("STR_QUANTITY_UC"));

	_lstTransfers->setColumns(2, 155, 41);
	_lstTransfers->setSelectable(true);
	_lstTransfers->setBackground(_window);
	_lstTransfers->setMargin(2);

	eventLogic();

	_txtItem->setVisible(false);
	_txtQuantity->setVisible(false);
	_lstTransfers->setVisible(false);

	if (_eventRule.getInvert())
	{
		_btnItemsArriving->setText(tr("STR_SUMMARY"));
	}
	else if (_lstTransfers->getTexts() == 0 || !Options::oxceGeoscapeEventsInstantDelivery || !_customAnswers.empty())
	{
		_btnOk->setX((_btnOk->getX() + _btnItemsArriving->getX()) / 2);
		_btnItemsArriving->setVisible(false);
	}
}

/**
* Helper performing event logic.
*/
void GeoscapeEventState::eventLogic()
{
	if (!_eventRule.getAdhocMissionScriptTags().empty())
	{
		auto* geo = _game->getGeoscapeState();
		geo->determineAlienMissions(false, &_eventRule);
	}

	SavedGame *save = _game->getSavedGame();
	Base *hq = save->getBases()->front();
	const Mod *mod = _game->getMod();
	const RuleEvent &rule = _eventRule;

	RuleRegion *regionRule = nullptr;
	City* city = nullptr;
	if (!rule.getRegionList().empty())
	{
		size_t pickRegion = RNG::generate(0, rule.getRegionList().size() - 1);
		auto& regionName = rule.getRegionList().at(pickRegion);
		regionRule = _game->getMod()->getRegion(regionName, true);
		std::string place = tr(regionName);

		if (rule.isCitySpecific())
		{
			size_t cities = regionRule->getCities()->size();
			if (cities > 0)
			{
				size_t pickCity = RNG::generate(0, cities - 1);
				city = regionRule->getCities()->at(pickCity);
				place = city->getName(_game->getLanguage());
			}
		}

		std::string titlePlus = tr(rule.getName()).arg(place);
		_txtTitle->setText(titlePlus);

		std::string messagePlus = tr(rule.getDescription()).arg(place);
		_txtMessage->setText(messagePlus);
	}

	// even if the event isn't city-specific, we'll still pick one city randomly to represent the region (and maybe even a country)
	if (regionRule)
	{
		if (!rule.isCitySpecific())
		{
			size_t cities = regionRule->getCities()->size();
			if (cities > 0)
			{
				size_t pickCity = RNG::generate(0, cities - 1);
				city = regionRule->getCities()->at(pickCity);
			}
		}
	}

	// 1. give/take score points
	int points = rule.getPoints();
	if (points != 0)
	{
		if (regionRule)
		{
			for (auto *region : *_game->getSavedGame()->getRegions())
			{
				if (region->getRules() == regionRule)
				{
					region->addActivityXcom(points);
					break;
				}
			}
		}
		else
		{
			save->addResearchScore(points);
		}
		_game->getMasterMind()->updateLoyalty(points, XCOM_GEOSCAPE);
	}


	// 2. give/take funds and loyalty
	save->setFunds(save->getFunds() + rule.getFunds());

	save->setLoyalty(save->getLoyalty() + rule.getLoyalty());

	// 3. saving removed covered operations list if exist
	std::vector<std::string>  removedCovertOperationsList = rule.getRemovedCovertOperationsList();
	if (!removedCovertOperationsList.empty())
	{
		for (auto it = removedCovertOperationsList.begin(); it < removedCovertOperationsList.end(); it++)
		{
			save->removePerformedCovertOperation((*it));
		}
	}

	// 4. spawn/transfer persons (soldiers, engineers, scientists, ...)
	const std::string& spawnedPersonType = rule.getSpawnedPersonType();
	if (rule.getSpawnedPersons() > 0 && !spawnedPersonType.empty())
	{
		if (spawnedPersonType == "STR_SCIENTIST")
		{
			Transfer* t = new Transfer(24);
			t->setScientists(rule.getSpawnedPersons());
			hq->getTransfers()->push_back(t);
		}
		else if (spawnedPersonType == "STR_ENGINEER")
		{
			Transfer* t = new Transfer(24);
			t->setEngineers(rule.getSpawnedPersons());
			hq->getTransfers()->push_back(t);
		}
		else
		{
			const RuleSoldier* ruleSoldier = mod->getSoldier(spawnedPersonType);
			if (ruleSoldier)
			{
				for (int i = 0; i < rule.getSpawnedPersons(); ++i)
				{
					Transfer* t = new Transfer(24);
					int nationality = _game->getSavedGame()->selectSoldierNationalityByLocation(_game->getMod(), ruleSoldier, city);
					Soldier* s = mod->genSoldier(save, ruleSoldier, nationality);
					YAML::YamlRootNodeReader reader(rule.getSpawnedSoldierTemplate(), "(spawned soldier template)");
					s->load(reader, mod, save, mod->getScriptGlobal(), true); // load from soldier template
					if (!rule.getSpawnedPersonName().empty())
					{
						s->setName(tr(rule.getSpawnedPersonName()));
					}
					else
					{
						s->genName();
					}
					t->setSoldier(s);
					hq->getTransfers()->push_back(t);
				}
			}
		}
	}

	// 5. spawn/transfer multiple soldiers into the HQ
	{
		std::map<const RuleSoldier*, int> soldiersToTransfer;

		for (auto& pair : rule.getEveryMultiSoldierList())
		{
			const RuleSoldier* soldierRule = mod->getSoldier(pair.first, true);
			if (soldierRule)
			{
				soldiersToTransfer[soldierRule] += pair.second;
			}
		}

		if (!rule.getRandomMultiSoldierList().empty())
		{
			size_t pickSoldier = RNG::generate(0, rule.getRandomMultiSoldierList().size() - 1);
			auto& sublist = rule.getRandomMultiSoldierList().at(pickSoldier);
			for (auto& pair : sublist)
			{
				const RuleSoldier* soldierRule = mod->getSoldier(pair.first, true);
				if (soldierRule)
				{
					soldiersToTransfer[soldierRule] += pair.second;
				}
			}
		}

		for (auto& ts : soldiersToTransfer)
		{
			for (int i = 0; i < ts.second; ++i)
			{
				Transfer* t = new Transfer(24);
				int nationality = _game->getSavedGame()->selectSoldierNationalityByLocation(_game->getMod(), ts.first, city);
				Soldier* s = mod->genSoldier(save, ts.first, nationality);
				YAML::YamlRootNodeReader reader(rule.getSpawnedSoldierTemplate(), "(spawned soldier template)");
				s->load(reader, mod, save, mod->getScriptGlobal(), true); // load from soldier template
				{
					// reset what may have been loaded
					s->genName();
				}
				t->setSoldier(s);
				hq->getTransfers()->push_back(t);
			}
		}
	}

	// 6. spawn/transfer item into the HQ
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

	if (!rule.getRandomMultiItemList().empty())
	{
		size_t pickItem = RNG::generate(0, rule.getRandomMultiItemList().size() - 1);
		auto& sublist = rule.getRandomMultiItemList().at(pickItem);
		for (auto& pair : sublist)
		{
			const RuleItem* itemRule = mod->getItem(pair.first, true);
			if (itemRule)
			{
				itemsToTransfer[itemRule->getType()] += pair.second;
			}
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
		if (rule.getInvert())
		{
			RuleItem* r = mod->getItem(ti.first, true);
			int removed = 0;
			for (auto* xbase : *save->getBases())
			{
				int bQty = xbase->getStorageItems()->getItem(r);
				if (bQty > 0)
				{
					int toRemove = std::min(bQty, ti.second);
					xbase->getStorageItems()->removeItem(r, toRemove);
					ti.second -= toRemove;
					removed += toRemove;
				}
				if (ti.second <= 0) break; // already removed enough
			}
			if (ti.second > 0)
			{
				for (auto* xbase : *save->getBases())
				{
					for (auto* xcraft : *xbase->getCrafts())
					{
						int cQty = xcraft->getItems()->getItem(r);
						if (cQty > 0 && xcraft->getStatus() != "STR_OUT")
						{
							int toRemove = std::min(cQty, ti.second);
							xcraft->getItems()->removeItem(r, toRemove);
							ti.second -= toRemove;
							removed += toRemove;
						}
						if (ti.second <= 0) break; // already removed enough
					}
					if (ti.second <= 0) break; // already removed enough
				}
			}

			std::ostringstream ss;
			ss << -removed;
			_lstTransfers->addRow(2, tr(ti.first).c_str(), ss.str().c_str());
		}
		else if (Options::oxceGeoscapeEventsInstantDelivery)
		{
			hq->getStorageItems()->addItem(mod->getItem(ti.first, true), ti.second);

			std::ostringstream ss;
			ss << ti.second;
			_lstTransfers->addRow(2, tr(ti.first).c_str(), ss.str().c_str());
		}
		else
		{
			Transfer* t = new Transfer(1);
			t->setItems(mod->getItem(ti.first, true), ti.second);
			hq->getTransfers()->push_back(t);
		}
	}

	// 6b. spawn craft into the HQ
	const RuleCraft* craftRule = mod->getCraft(rule.getSpawnedCraftType(), true);
	if (craftRule)
	{
		Craft* craft = new Craft(craftRule, hq, save->getId(craftRule->getType()));
		craft->initFixedWeapons(mod);
		if (Options::oxceGeoscapeEventsInstantDelivery)
		{
			// same as manufacture
			craft->checkup();
			hq->getCrafts()->push_back(craft);
		}
		else
		{
			// same as buy
			craft->setStatus("STR_REFUELLING");
			Transfer* t = new Transfer(1);
			t->setCraft(craft);
			hq->getTransfers()->push_back(t);
		}
	}

	// 7. give bonus research
	std::vector<const RuleResearch*> possibilities;

	for (auto* rRule : rule.getResearchList())
	{
		std::vector<const RuleResearch*> researches;
		for (const auto &rRule : rule.getResearchList())
		{
			researches.push_back(rRule);
		}
		_game->getMasterMind()->helpResearchDiscovery(researches, possibilities, hq, _researchName, _bonusResearchName);
	}

	// 8. handle counters
	for (auto& inc : rule.getIncreaseCounter())
	{
		_game->getSavedGame()->increaseCustomCounter(inc, rule.getCounterValue());
	}
	for (auto& dec : rule.getDecreaseCounter())
	{
		_game->getSavedGame()->decreaseCustomCounter(dec, rule.getCounterValue());
	}

	// 9. Add reputation
	auto reputationScore = _eventRule.getReputationScore();
	if (!reputationScore.empty())
	{
		for (std::map<std::string, int>::const_iterator i = reputationScore.begin(); i != reputationScore.end(); ++i)
		{
			for (std::vector<DiplomacyFaction*>::iterator j = save->getDiplomacyFactions().begin(); j != save->getDiplomacyFactions().end(); ++j)
			{
				std::string factionName = (*j)->getRules()->getName();
				std::string lookingName = (*i).first;
				if (factionName == lookingName)
				{
					(*j)->updateReputationScore((*i).second);
					break;
				}
			}
		}
	}

	// // Side effects:
	// // 1. remove obsolete research projects from all bases
	// // 2. handle items spawned by research
	// // 3. handle events spawned by research
	// save->handlePrimaryResearchSideEffects(topicsToCheck, mod, hq); #FINNIKTODO

	if (Options::oxceGeoscapeDebugLogMaxEntries > 0)
	{
		std::ostringstream ss;
		ss << "gameTime: " << save->getTime()->getFullString();
		ss << " eventPopup: " << rule.getName();
		save->getGeoscapeDebugLog().push_back(ss.str());
	}
}
/**
* Spawns custom events based on the chosen button.
* After that closes the window and shows a pedia article if needed.
* @param int playerChoice - an index of the pressed button
*/
void GeoscapeEventState::spawnCustomEvents(int playerChoice)
{
	for (const auto &eventName : _customAnswers[playerChoice].spawnEvents)
	{
		_game->getSavedGame()->spawnEvent(_game->getMod()->getEvent(eventName));
	}
}

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

	if (!_eventRule.getCutscene().empty())
	{
		_game->pushState(new CutsceneState(_eventRule.getCutscene()));
		if (_game->getSavedGame()->getEnding() == END_NONE)
		{
			const RuleVideo* videoRule = _game->getMod()->getVideo(_eventRule.getCutscene(), true);
			if (videoRule->getWinGame()) _game->getSavedGame()->setEnding(END_WIN);
			if (videoRule->getLoseGame()) _game->getSavedGame()->setEnding(END_LOSE);
		}
	}

	if (_game->getSavedGame()->getEnding() == END_NONE && !_game->getMod()->isFTAGame())
	{
		Base* base = _game->getSavedGame()->getBases()->front();
		if (_game->getSavedGame()->getMonthsPassed() > -1 && Options::storageLimitsEnforced && base != 0 && base->storesOverfull())
		{
			_game->pushState(new SellState(base, 0));
			_game->pushState(new ErrorMessageState(tr("STR_STORAGE_EXCEEDED").arg(base->getName()), _palette, _game->getMod()->getInterface("debriefing")->getElement("errorMessage")->color, "BACK01.SCR", _game->getMod()->getInterface("debriefing")->getElement("errorPalette")->color));
		}
	}

	if (!_bonusResearchName.empty())
	{
		Ufopaedia::openArticle(_game, _bonusResearchName);
	}

	if (!_researchName.empty())
	{
		Ufopaedia::openArticle(_game, _researchName);
	}
}

/**
* Calls spawning of events for custom button 1
* @param action Pointer to an action.
*/
void GeoscapeEventState::btnAnswerOneClick(Action* action)
{
	spawnCustomEvents(0);
	btnOkClick(action);
}

/**
 * Toggles the view between the description and the ItemsArriving list.
 * @param action Pointer to an action.
 */
void GeoscapeEventState::btnItemsArrivingClick(Action *)
{
	if (_btnItemsArriving->getPressed())
	{
		_txtMessage->setVisible(false);

		_txtItem->setVisible(true);
		_txtQuantity->setVisible(true);
		_lstTransfers->setVisible(true);
	}
	else
	{
		_txtItem->setVisible(false);
		_txtQuantity->setVisible(false);
		_lstTransfers->setVisible(false);

		_txtMessage->setVisible(true);
	}
}

/**
* Shows description for custom button 1 if present
* @param action Pointer to an action.
*/
void GeoscapeEventState::btnAnswerOneClickRight(Action* action)
{
	if (!_customAnswers[0].description.empty())
		{
			_game->pushState(new GeoscapeEventAnswerInfoState(_eventRule, _customAnswers[0].description));
		}
}

/**
* Calls spawning of events for custom button 2
* @param action Pointer to an action.
*/
void GeoscapeEventState::btnAnswerTwoClick(Action* action)
{
	spawnCustomEvents(1);
	btnOkClick(action);
}

/**
* Shows description for custom button 2 if present
* @param action Pointer to an action.
*/
void GeoscapeEventState::btnAnswerTwoClickRight(Action* action)
{
	if (!_customAnswers[1].description.empty())
	{
		_game->pushState(new GeoscapeEventAnswerInfoState(_eventRule, _customAnswers[1].description));
	}
}

/**
* Calls spawning of events for custom button 3
* @param action Pointer to an action.
*/
void GeoscapeEventState::btnAnswerThreeClick(Action* action)
{
	spawnCustomEvents(2);
	btnOkClick(action);
}

/**
* Shows description for custom button 3 if present
* @param action Pointer to an action.
*/
void GeoscapeEventState::btnAnswerThreeClickRight(Action* action)
{
	if (!_customAnswers[2].description.empty())
	{
		_game->pushState(new GeoscapeEventAnswerInfoState(_eventRule, _customAnswers[2].description));
	}
}

/**
* Calls spawning of events for custom button 4
* @param action Pointer to an action.
*/
void GeoscapeEventState::btnAnswerFourClick(Action* action)
{
	spawnCustomEvents(3);
	btnOkClick(action);
}

/**
* Shows description for custom button 4 if present
* @param action Pointer to an action.
*/
void GeoscapeEventState::btnAnswerFourClickRight(Action* action)
{
	if (!_customAnswers[3].description.empty())
	{
		_game->pushState(new GeoscapeEventAnswerInfoState(_eventRule, _customAnswers[3].description));
	}
}

/**
 * Shows a tooltip for the appropriate button.
 * @param action Pointer to an action.
 */
void GeoscapeEventState::txtTooltipIn(Action* action)
{
	_currentTooltip = action->getSender()->getTooltip();
	_txtTooltip->setText(tr(_currentTooltip));
}

/**
 * Clears the tooltip text.
 * @param action Pointer to an action.
 */
void GeoscapeEventState::txtTooltipOut(Action* action)
{
	if (_currentTooltip == action->getSender()->getTooltip())
		{
			_currentTooltip = "";
			_txtTooltip->setText("");
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

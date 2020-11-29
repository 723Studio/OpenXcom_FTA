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
#include "CovertOperationStartState.h"
#include "CovertOperationEquipmentState.h"
#include "CovertOperationSoldiersState.h"
#include "CovertOperationArmorState.h"
#include "ResearchState.h"
#include "ManufactureState.h"
#include <sstream>
#include <climits>
#include <iomanip>
#include <algorithm>
#include <locale>
#include "../fmath.h"
#include "../Interface/Window.h"
#include "../Interface/TextButton.h"
#include "../Interface/ToggleTextButton.h"
#include "../Interface/Text.h"
#include "../Interface/TextList.h"
#include "../Engine/SurfaceSet.h"
#include "../Engine/Game.h"
#include "../Engine/Action.h"
#include "../Engine/LocalizedText.h"
#include "../Engine/Options.h"
#include "../Engine/Timer.h"
#include "../Engine/Unicode.h"
#include "../Engine/Logger.h"
#include "../Engine/CrossPlatform.h"
#include "../Mod/Mod.h"
#include "../Mod/RuleItem.h"
#include "../Mod/RuleCovertOperation.h"
#include "../Mod/Armor.h"
#include "../Mod/RuleSoldier.h"
#include "../Mod/RuleInterface.h"
#include "../Savegame/Base.h"
#include "../Savegame/ItemContainer.h"
#include "../Savegame/CovertOperation.h"
#include "../Savegame/Soldier.h"
#include "../Savegame/SavedGame.h"
#include "../Menu/ErrorMessageState.h"

namespace OpenXcom
{

/**
* Initializes all the elements in the CoverOperation start screen.
* @param base Pointer to the base to get info from.
* @param rule RuleCovertOperation to start.
*/
CovertOperationStartState::CovertOperationStartState(Base* base, RuleCovertOperation* rule) : _base(base), _rule(rule), _chances(0), _cost(0),
																							  _scientists(0), _engeneers(0), _hasPsiItems(false), _hasPsionics(false)
{
	_items = new ItemContainer();

	_screen = false;
	int descrW = 73;
	int descrY = 24;
	int descrDY = descrW + descrY;
	_window = new Window(this, 320, 200, 0, 0, POPUP_BOTH);
	_txtTitle = new Text(310, 17, 5, 8);
	_txtDescription = new Text(304, descrW, 8, descrY);
	_btnCancel = new TextButton(148, 16, 8, 176);
	_btnStart = new TextButton(148, 16, 164, 176);

	int lablesDX = 90;
	int lableSpace = 3;
	_txtSoldiersRequired = new Text(lablesDX, 9, 8, descrDY + 1);
	_txtSoldiersAssigned = new Text(lablesDX, 9, 8, descrDY + 10);
	_txtOptionalSoldiers = new Text(lablesDX, 9, lablesDX + 8 + lableSpace,   descrDY + 1);
	_btnSoldiers = new TextButton(64, 16, 8, descrDY + 20);
	_btnEquipmet = new TextButton(64, 16, 8, descrDY + 37);
	_btnArmor = new TextButton(64, 16, 8, descrDY + 54);

	_crew = new Surface(220, 18, 77, descrDY + 19);
	_equip = new Surface(220, 18, 77, descrDY + 37);

	int secondColumnW = 304 - (2 * lablesDX + 2 * lableSpace);
	int secondColumnX = 312 - secondColumnW;
	int personellAssignBtnW = 62;
	int personellAssignBtnH = 16;
	int btnMargine = 1;
	int btnAssignSmallW = (personellAssignBtnW - 2 * btnMargine) / 3;
	_txtScientistsAssigned = new Text(secondColumnW, 9, secondColumnX, descrDY + 1);
	_btnAddScientist = new TextButton(btnAssignSmallW, personellAssignBtnH, 312 - personellAssignBtnW, descrDY + 11);
	_btnRemoveScientist = new TextButton(btnAssignSmallW, personellAssignBtnH, 312 - personellAssignBtnW + btnAssignSmallW + btnMargine, descrDY + 11);
	_btnResearchState = new TextButton(btnAssignSmallW, personellAssignBtnH, 312 - personellAssignBtnW + 2 * (btnAssignSmallW + btnMargine), descrDY + 11);

	_txtEngineersAssigned = new Text(secondColumnW, 9, secondColumnX, descrDY + 12 + personellAssignBtnH);
	_btnAddEngineer = new TextButton(btnAssignSmallW, personellAssignBtnH, 312 - personellAssignBtnW, descrDY + 22 + personellAssignBtnH);
	_btnRemoveEngineer = new TextButton(btnAssignSmallW, personellAssignBtnH, 312 - personellAssignBtnW + btnAssignSmallW + btnMargine, descrDY + 22 + personellAssignBtnH);
	_btnManufactureState = new TextButton(btnAssignSmallW, personellAssignBtnH, 312 - personellAssignBtnW + 2 * (btnAssignSmallW + btnMargine), descrDY + 22 + personellAssignBtnH);

	_txtDuration = new Text(304, 9, 8, 153);
	_txtChances = new Text(304, 9, 8, 164);

	//let's get some operation properties
	bool hasScientists = _rule->getScientistSlots() > 0;
	bool hasEngineers = _rule->getEngineerSlots() > 0;

	setInterface("newCovertOperationsMenu");
	add(_window, "window", "newCovertOperationsMenu");
	add(_txtTitle, "text", "newCovertOperationsMenu");
	add(_txtDescription, "text", "newCovertOperationsMenu");
	add(_btnCancel, "button", "newCovertOperationsMenu");
	add(_btnStart, "button", "newCovertOperationsMenu");

	add(_txtSoldiersRequired, "text", "newCovertOperationsMenu");
	add(_txtSoldiersAssigned, "text", "newCovertOperationsMenu");
	add(_txtOptionalSoldiers, "text", "newCovertOperationsMenu");
	add(_btnSoldiers, "button", "newCovertOperationsMenu");
	add(_btnEquipmet, "button", "newCovertOperationsMenu");
	add(_btnArmor, "button", "newCovertOperationsMenu");
	add(_crew);
	add(_equip);

	add(_txtScientistsAssigned, "text", "newCovertOperationsMenu");
	add(_btnAddScientist, "button", "newCovertOperationsMenu");
	add(_btnRemoveScientist, "button", "newCovertOperationsMenu");
	add(_btnResearchState, "button", "newCovertOperationsMenu");

	add(_txtEngineersAssigned, "text", "newCovertOperationsMenu");
	add(_btnAddEngineer, "button", "newCovertOperationsMenu");
	add(_btnRemoveEngineer, "button", "newCovertOperationsMenu");
	add(_btnManufactureState, "button", "newCovertOperationsMenu");

	add(_txtDuration, "text", "newCovertOperationsMenu");
	add(_txtChances, "text", "newCovertOperationsMenu");
	
	centerAllSurfaces();

	setWindowBackground(_window, "newCovertOperationsMenu");

	_txtTitle->setText(tr(_rule->getName()));
	_txtTitle->setBig();
	_txtTitle->setAlign(ALIGN_CENTER);
	_txtDescription->setText(tr(_rule->getDescription()));
	_txtDescription->setWordWrap(true);

	_btnStart->setText(tr("STR_START_OPERATION_US")); 
	_btnStart->onMouseClick((ActionHandler)&CovertOperationStartState::btnStartClick);
	_btnStart->onKeyboardPress((ActionHandler)&CovertOperationStartState::btnStartClick, Options::keyOk);

	_btnCancel->setText(tr("STR_CANCEL_UC"));
	_btnCancel->onMouseClick((ActionHandler)&CovertOperationStartState::btnCancelClick);
	_btnCancel->onKeyboardPress((ActionHandler)&CovertOperationStartState::btnCancelClick, Options::keyCancel);

	_txtSoldiersRequired->setText(tr("STR_SOLDIERS_REQUIRED").arg(rule->getSoldierSlots()));
	
	_txtOptionalSoldiers->setText(tr("STR_OPTIONAL_SOLDIERS").arg(rule->getOptionalSoldierSlots()));

	_btnSoldiers->setText(tr("STR_SOLDIERS_UC"));
	_btnSoldiers->onMouseClick((ActionHandler)&CovertOperationStartState::btnSoldiersClick);
	_btnEquipmet->setText(tr("STR_EQUIPMENT_UC"));
	_btnEquipmet->onMouseClick((ActionHandler)&CovertOperationStartState::btnEquipmetClick);
	_btnArmor->setText(tr("STR_ARMOR"));
	_btnArmor->onMouseClick((ActionHandler)&CovertOperationStartState::btnArmorClick);

	_txtScientistsAssigned->setAlign(ALIGN_RIGHT);
	_btnAddScientist->setText("+");
	_btnAddScientist->onMouseClick((ActionHandler)&CovertOperationStartState::btnAddScientistClick, SDL_BUTTON_LEFT);
	_btnAddScientist->onMouseClick((ActionHandler)&CovertOperationStartState::btnAddScientistClick, SDL_BUTTON_RIGHT);
	_btnRemoveScientist->setText("-");
	_btnRemoveScientist->onMouseClick((ActionHandler)&CovertOperationStartState::btnRemoveScientistClick, SDL_BUTTON_LEFT);
	_btnRemoveScientist->onMouseClick((ActionHandler)&CovertOperationStartState::btnRemoveScientistClick, SDL_BUTTON_RIGHT);
	_btnResearchState->setText("STR_LAB_ICON");
	_btnResearchState->onMouseClick((ActionHandler)&CovertOperationStartState::btnResearchStateClick);


	_txtEngineersAssigned->setAlign(ALIGN_RIGHT);
	_btnAddEngineer->setText("+");
	_btnAddEngineer->onMouseClick((ActionHandler)&CovertOperationStartState::btnAddEngineerClick, SDL_BUTTON_LEFT);
	_btnAddEngineer->onMouseClick((ActionHandler)&CovertOperationStartState::btnAddEngineerClick, SDL_BUTTON_RIGHT);
	_btnRemoveEngineer->setText("-");
	_btnRemoveEngineer->onMouseClick((ActionHandler)&CovertOperationStartState::btnRemoveEngineerClick, SDL_BUTTON_LEFT);
	_btnRemoveEngineer->onMouseClick((ActionHandler)&CovertOperationStartState::btnRemoveEngineerClick, SDL_BUTTON_RIGHT);
	_btnManufactureState->setText("STR_WORKSHOP_ICON");
	_btnManufactureState->onMouseClick((ActionHandler)&CovertOperationStartState::btnManufactureStateClick);

	_txtOptionalSoldiers->setVisible(_rule->getOptionalSoldierSlots() > 0);

	_txtScientistsAssigned->setVisible(hasScientists);
	_btnAddScientist->setVisible(hasScientists);
	_btnRemoveScientist->setVisible(hasScientists);
	_btnResearchState->setVisible(hasScientists);

	_txtEngineersAssigned->setVisible(hasEngineers);
	_btnAddEngineer->setVisible(hasEngineers);
	_btnRemoveEngineer->setVisible(hasEngineers);
	_btnManufactureState->setVisible(hasEngineers);

}

CovertOperationStartState::~CovertOperationStartState()
{
	delete _items;
}
/**
 * The operation start state info can change
 * after going into other screens.
 */
void CovertOperationStartState::init()
{
	State::init();

	_txtSoldiersAssigned->setText(tr("STR_SOLDIERS_ASSIGNED").arg(_soldiers.size()));

	std::ostringstream ss;
	ss << _scientists << "/" << _rule->getScientistSlots();
	_txtScientistsAssigned->setText(tr("STR_SCIENTISTS_REQUIRED").arg(ss.str()));

	std::ostringstream se;
	se << _engeneers << "/" << _rule->getEngineerSlots();
	_txtEngineersAssigned->setText(tr("STR_ENGINEERS_REQUIRED").arg(se.str()));

	bool mod = _game->getSavedGame()->getDebugMode();

	_txtDuration->setText(tr("STR_OPERATION_DURATION_UC").arg(tr(getOperationTimeString(mod))));
	_txtDuration->setAlign(ALIGN_RIGHT);
	_txtChances->setText(tr("STR_OPERATION_CHANCES_UC").arg(tr(getOperationOddsString(mod))));
	_txtChances->setAlign(ALIGN_RIGHT);

	_btnStart->setVisible(_soldiers.size() >= _rule->getSoldierSlots());
	_btnEquipmet->setVisible(_soldiers.size() > 0 || _items->getTotalQuantity() > 0);
	_btnArmor->setVisible(_soldiers.size() > 0);


	SurfaceSet* texture = _game->getMod()->getSurfaceSet("BASEBITS.PCK");
	_crew->clear();
	_equip->clear();

	Surface* frame1 = texture->getFrame(38);

	SurfaceSet* customArmorPreviews = _game->getMod()->getSurfaceSet("CustomArmorPreviews");
	int x = 0;
	for (std::vector<Soldier*>::iterator i = _soldiers.begin(); i != _soldiers.end(); ++i)
	{
		for (auto index : (*i)->getArmor()->getCustomArmorPreviewIndex())
		{
			Surface* customFrame1 = customArmorPreviews->getFrame(index);
			if (customFrame1)
			{
				// modded armor previews
				customFrame1->blitNShade(_crew, x, 0);
			}
			else
			{
				// vanilla
				frame1->blitNShade(_crew, x, 0);
			}
			x += 10;
		}
	}

	Surface* frame2 = texture->getFrame(40);
	SurfaceSet* customItemPreviews = _game->getMod()->getSurfaceSet("CustomItemPreviews");
	x = 0;


	Surface* frame3 = texture->getFrame(39);
	for (int i = 0; i < _items->getTotalQuantity(); i += 4, x += 10)
	{
		frame3->blitNShade(_equip, x, 0);
	}
}

/**
* Returns to previous screen.
* @param action A pointer to an Action.
*/
void CovertOperationStartState::btnCancelClick(Action*)
{
	// lets return all items back to base
	for (std::map<std::string, int>::iterator it = _items->getContents()->begin(); it != _items->getContents()->end(); ++it)
	{
		_base->getStorageItems()->addItem(it->first, it->second);
	}
	_game->popState();
	_base->setScientists(_base->getScientists() + _scientists);
	_base->setEngineers(_base->getEngineers() + _engeneers);
}

/**
* Commits operation and return to operations state.
* @param action A pointer to an Action.
*/
void CovertOperationStartState::btnStartClick(Action*)
{
	//update values one more time just in case.
	int chances = round(this->getOperationOdds());
	int cost = getOperationCost();

	CovertOperation* newOperation = new CovertOperation(_rule, _base, cost, chances);
	_base->addCovertOperation(newOperation);
	// lets update operation with items and personell and assign soldiers.
	for (std::map<std::string, int>::iterator it = _items->getContents()->begin(); it != _items->getContents()->end(); ++it)
	{
		newOperation->getItems()->addItem(it->first, it->second);
		RuleItem* item = _game->getMod()->getItem(it->first);
		if (item->getBattleType() == BT_PSIAMP) _hasPsiItems = true; //looks like this item can be used for psionic offence!
	}
	for (std::vector<Soldier*>::iterator i = _base->getSoldiers()->begin(); i != _base->getSoldiers()->end(); ++i)
	{
		bool matched = false;
		auto iter = std::find(std::begin(_soldiers), std::end(_soldiers), (*i));
		if (iter != std::end(_soldiers)) {
			matched = true;
		}
		if (matched)
		{
			(*i)->setCovertOperation(newOperation);
			(*i)->setCraft(0);
			bool wasMatTraining = false;
			bool wasPsiTraining = false;
			if ((*i)->isInTraining())
			{
				(*i)->setReturnToTrainingWhenOperationOver(MARTIAL_TRAINING);
				wasMatTraining = true;
			}
			if ((*i)->isInPsiTraining())
			{
				(*i)->setReturnToTrainingWhenOperationOver(PSI_TRAINING);
				wasPsiTraining = true;
			}
			if (wasMatTraining && wasPsiTraining)
			{
				(*i)->setReturnToTrainingWhenOperationOver(BOTH_TRAININGS);
			}
			(*i)->setPsiTraining(false);
			(*i)->setTraining(false);
			if ((*i)->getCurrentStats()->psiSkill > 0) _hasPsionics = true; //hey, this soldier we sending has psionic skills!
		}
	}
	newOperation->setAssignedScientists(_scientists);
	newOperation->setAssignedEngineers(_engeneers);

	if (_hasPsionics && _hasPsiItems && _game->getSavedGame()->isResearched(_game->getMod()->getPsiRequirements()))
	{ //operation that we are about to start has psionic offencive potential
		newOperation->setIsPsi(true);
	}
	// now we add this operation to list of performed operations to not let run this operation second time.
	_game->getSavedGame()->addPerformedCovertOperation(newOperation->getOperationName());

	//operation committed, close the state
	_game->popState();
	_game->popState();
}

/**
* Go to the Soldier screen.
* @param action A pointer to an Action.
*/
void CovertOperationStartState::btnSoldiersClick(Action* action)
{
	_game->pushState(new CovertOperationSoldiersState(_base, this));
}

/**
* Go to the Equipment screen.
* @param action A pointer to an Action.
*/
void CovertOperationStartState::btnEquipmetClick(Action* action)
{
	_game->pushState(new CovertOperationEquipmentState(_base, this));
}

/**
* Go to the Armor screen.
* @param action A pointer to an Action.
*/
void CovertOperationStartState::btnArmorClick(Action* action)
{
	_game->pushState(new CovertOperationArmorState(_base, this));
}

void CovertOperationStartState::btnAddScientistClick(Action* action)
{
	if (_scientists >= _rule->getScientistSlots()) return;
	int changeValue = 0;
	if (action->getDetails()->button.button == SDL_BUTTON_LEFT)
	{
		changeValue = 1;
	}
	else if (action->getDetails()->button.button == SDL_BUTTON_RIGHT)
	{
		if (_rule->getScientistSlots() < _base->getScientists()) changeValue = _rule->getScientistSlots();
		else changeValue = _base->getScientists();
		if (changeValue + _scientists > _rule->getScientistSlots()) changeValue = _rule->getScientistSlots() - _scientists;
	}
	if (_base->getScientists() > 0 && _rule->getScientistSlots() > 0)
	{
		_base->setScientists(_base->getScientists() - changeValue);
		_scientists = _scientists + changeValue;
	}
	else
	{
		_game->pushState(new ErrorMessageState(tr("STR_NOT_ENOUGH_SCIENTISTS"), _palette, _game->getMod()->getInterface("soldierInfo")->getElement("errorMessage")->color, "BACK05.SCR", _game->getMod()->getInterface("soldierInfo")->getElement("errorPalette")->color));
	}
	init();
}

void CovertOperationStartState::btnRemoveScientistClick(Action* action)
{
	if (_scientists == 0) return;
	int changeValue = 0;
	if (action->getDetails()->button.button == SDL_BUTTON_LEFT)
	{
		changeValue = 1;
	}
	else if (action->getDetails()->button.button == SDL_BUTTON_RIGHT)
	{
		changeValue = _scientists;
	}
	if (_scientists > 0)
	{
		_scientists = _scientists - changeValue;
		_base->setScientists(_base->getScientists() + changeValue);
	}
	init();
}

void CovertOperationStartState::btnResearchStateClick(Action* action)
{
	_game->pushState(new ResearchState(_base));
}

void CovertOperationStartState::btnAddEngineerClick(Action* action)
{
	if (_engeneers >= _rule->getEngineerSlots()) return;
	int changeValue = 0;
	if (action->getDetails()->button.button == SDL_BUTTON_LEFT)
	{
		changeValue = 1;
	}
	else if (action->getDetails()->button.button == SDL_BUTTON_RIGHT)
	{
		if (_rule->getEngineerSlots() < _base->getEngineers()) changeValue = _rule->getEngineerSlots();
		else changeValue = _base->getEngineers();
		if (changeValue + _engeneers > _rule->getScientistSlots()) changeValue = _rule->getScientistSlots() - _engeneers;
	}
	if (_base->getEngineers() > 0 && _rule->getEngineerSlots() > 0)
	{
		_base->setEngineers(_base->getEngineers() - changeValue);
		_engeneers = _engeneers + changeValue;
	}
	else
	{
		_game->pushState(new ErrorMessageState(tr("STR_NOT_ENOUGH_ENGINEERS"), _palette, _game->getMod()->getInterface("soldierInfo")->getElement("errorMessage")->color, "BACK17.SCR", _game->getMod()->getInterface("soldierInfo")->getElement("errorPalette")->color));
	}
	init();
}

void CovertOperationStartState::btnRemoveEngineerClick(Action* action)
{
	if (_engeneers == 0) return;
	int changeValue = 0;
	if (action->getDetails()->button.button == SDL_BUTTON_LEFT)
	{
		changeValue = 1;
	}
	else if (action->getDetails()->button.button == SDL_BUTTON_RIGHT)
	{
		changeValue = _engeneers;
	}
	if (_engeneers > 0)
	{
		_engeneers = _engeneers - changeValue;
		_base->setEngineers(_base->getEngineers() + changeValue);
	}
	init();
}

void CovertOperationStartState::btnManufactureStateClick(Action* action)
{
	_game->pushState(new ManufactureState(_base));
}


std::string CovertOperationStartState::getOperationTimeString(bool mod)
{
	int time = getOperationCost();
	if (!mod)
	{
		if (time > 45)
		{
			return ("STR_SEVERAL_MONTHS");
		}
		else if (time > 20)
		{
			return ("STR_A_MONTH");
		}
		else if (time > 10)
		{
			return ("STR_SEVERAL_WEEKS");
		}
		else if (time > 6)
		{
			return ("STR_WEEK");
		}
		else
		{
			return ("STR_SEVERAL_DAYS");
		}
	}
	else
	{
		return std::to_string(time);
	}

}

std::string CovertOperationStartState::getOperationOddsString(bool mod)
{
	int odds = round(getOperationOdds());
	if (!mod)
	{
		if (odds > 100)
		{
			return ("STR_GREAT");
		}
		else if (odds > 75)
		{
			return ("STR_GOOD");
		}
		else if (odds > 50)
		{
			return ("STR_AVERAGE");
		}
		else if (odds > 25)
		{
			return ("STR_POOR");
		}
		else if (odds > 0)
		{
			return ("STR_VERY_LOW");
		}
		else
		{
			return ("STR_NONE");
		}
	}
	else
	{
		return std::to_string(odds);
	}
}


/**
* Returns (re)calculated chances of success operation in form of double value.
* @return operation chances.
*/
double CovertOperationStartState::getOperationOdds()
{
	int startOdds = _rule->getBaseChances();
	_chances = startOdds;

	int requierdSoldiers = _rule->getSoldierSlots();
	int assignedSoldiersN = _soldiers.size();
	// lets process staff effectiveness
	int slots = 0;
	slots = _rule->getOptionalSoldierSlots();
	if (slots > 0)
	{
		_chances = _chances - (slots * static_cast<double>(_rule->getOptionalSoldierEffect()) * 0.9);
		int optionalSoldiers = assignedSoldiersN - requierdSoldiers;
		_chances = _chances + optionalSoldiers * static_cast<double>(_rule->getOptionalSoldierEffect());
		slots = 0;
	}
	slots = (_rule->getScientistSlots());
	if (slots > 0)
	{
		int effect = _rule->getScientistEffect();
		_chances = _chances - (slots * effect * 0.9) + (_scientists * effect);
		slots = 0;
	}
	slots = (_rule->getEngineerSlots());
	if (slots > 0)
	{
		int effect = _rule->getEngineerEffect();
		_chances = _chances - (slots * effect * 0.9) + (_engeneers * effect);
		slots = 0;
	}
	//lets see if we need some decrease because of required items
	if (!_rule->getRequiredItemList().empty())
	{
		std::map<std::string, int> reqItems = _rule->getRequiredItemList();
		int reqItemsN = 0;
		for (std::map<std::string, int>::iterator it = reqItems.begin(); it != reqItems.end(); ++it)
		{
			reqItemsN = reqItemsN + it->second;
			std::string itmeName = it->first;
			for (std::map<std::string, int>::iterator j = _items->getContents()->begin(); j != _items->getContents()->end(); ++j)
			{
				if (j->first == itmeName)
				{
					reqItemsN = reqItemsN - j->second;
				}
			}
		}
		int change = reqItemsN * 5;
		_chances = _chances - change;
	}
	//now lets check soldier armor if we have something about it in rules
	if (!_rule->getAllowedArmor().empty())
	{
		int armorlessSoldiers = assignedSoldiersN;
		for (auto& solIt : _soldiers)
		{
			std::string armorName = solIt->getArmor()->getType();
			for (auto& ruleArmor : _rule->getAllowedArmor())
			{
				if (ruleArmor == armorName)
				{
					--armorlessSoldiers;
				}
			}
		}
		_chances = _chances - armorlessSoldiers * static_cast<double>(_rule->getAllowedArmorEffect());
	}
	//lets process soldier stats as pure bonus
	if (assignedSoldiersN > 0)
	{
		int soldierMaxRank = 0;
		int soldiersTotalRank = 0;
		double soldierReactions = 0;
		double soldierBrav = 0;
		double soldiersPsi = 0;
		double soldiersTU = 0;
		double soldiersSta = 0;
		//extract soldier stats we need for processing
		for (auto& solIt : _soldiers) 
		{
			//lets get soldier effectivness first, if any
			int solEffectivness = 100;
			if (!_rule->getSoldierTypeEffectiveness().empty())
			{
				std::string solType = solIt->getRules()->getType();
				auto ruleType = _rule->getSoldierTypeEffectiveness();
				for (std::map<std::string, int>::iterator t = ruleType.begin(); t != ruleType.end(); ++t)
				{
					std::string ruleTypeName = t->first;
					if (ruleTypeName == solType)
					{
						solEffectivness = t->second;
					}
				}
			}
			_chances = _chances * (solEffectivness / 100);
			//lets make a bonus for having officer for field command and avg ranking
			int rank = solIt->getRank();
			if (rank > soldierMaxRank) soldierMaxRank = rank;
			soldiersTotalRank = soldiersTotalRank + rank;
			//now lets handle soldier stats
			double reacCalc = statEffectCalc(solIt->getStatsWithAllBonuses()->reactions, 2000, 2, 16, -9) ;
			soldierReactions = soldierReactions + reacCalc * (solEffectivness / 100);
			int brav = solIt->getStatsWithAllBonuses()->bravery / 10;
			soldierBrav = soldierBrav + (static_cast<double>(brav) * (solEffectivness / 100)) - 3;
			int psi = solIt->getStatsWithAllBonuses()->psiSkill;
			if (psi > 0 && _game->getSavedGame()->isResearched(_game->getMod()->getPsiRequirements())) //psi offencive bonus
			{
				float manaFactor = 1;
				if (_game->getMod()->isManaFeatureEnabled())
				{
					int mana = solIt->getStatsWithAllBonuses()->mana;
					manaFactor = (mana - solIt->getManaMissing()) / mana;
				}
				soldiersPsi = ((statEffectCalc(psi, 8000, 2.2, 8, 0) + statEffectCalc(solIt->getStatsWithAllBonuses()->psiStrength, 8000, 2.2, 8, 0)) / 2) * manaFactor * (solEffectivness / 100);
			}
			else
			{
				soldiersPsi = statEffectCalc(solIt->getStatsWithAllBonuses()->psiStrength, 8000, 2.2, 8, 0) / 3; //as soldier still has psi defence and some extrasence capabilities
			}
			double tuCalc = statEffectCalc(solIt->getStatsWithAllBonuses()->tu, 1400, 1.8, 15, -10);
			soldiersTU = soldiersTU + tuCalc * (solEffectivness / 100);
			double staCalc = statEffectCalc(solIt->getStatsWithAllBonuses()->stamina, 1600, 1.8, 10, -6);
			soldiersSta = soldiersSta + staCalc * (solEffectivness / 100);
		}

		double officerEffect = -0.2321 * (pow(soldierMaxRank, 2)) + 2.5036 * soldierMaxRank + 0.0357; // cute nonlinear function for field officer + avg rank bonus
		double rankEffect = (soldiersTotalRank / assignedSoldiersN);
		double bravEffect = (soldierBrav / assignedSoldiersN) * 5;
		double reactEffect = (soldierReactions / assignedSoldiersN);
		double tuEffect = (soldiersTU / assignedSoldiersN);
		double staEffect = (soldiersSta / assignedSoldiersN);
		double effect = rankEffect + bravEffect + reactEffect + soldiersPsi + tuEffect + staEffect;
		_chances = _chances + effect;

		// let's check if itemset has specific FTA's item categories
		bool isConcealed = true;
		int heavy = 0;
		double itemCatEffect = 0;
		for (std::map<std::string, int>::iterator i = _items->getContents()->begin(); i != _items->getContents()->end(); ++i)
		{
			RuleItem* item = _game->getMod()->getItem((*i).first);
			isConcealed = item->belongsToCategory("STR_CONCEALABLE");
			if (item->belongsToCategory("STR_HEAVY_WEAPONS")) ++heavy;
		}
		if (isConcealed)
		{
			itemCatEffect = 4 * static_cast<double>(assignedSoldiersN);
		}
		itemCatEffect = itemCatEffect - (heavy * 10 / assignedSoldiersN);
		_chances = _chances + itemCatEffect;
	}

	if (_chances > 200) // we dont want too high chances
	{
		_chances = 200;
	}
	return _chances;
}

int CovertOperationStartState::getOperationCost()
{
	_cost = _rule->getCosts(); //load initial rule value
	int reducedCost = _cost;
	if (_chances > 100)
	{
		double bonus = (((_chances - 100) / (_chances - 82)) * 24) / 100; //some cute nonlinear calculation
		reducedCost -= std::round(_cost * bonus);
	}
	return reducedCost;
}


/**
* Removes soldier from covert operation start state
* @param action A pointer to a Soldier.
*/
void CovertOperationStartState::removeSoldier(Soldier* soldier)
{
	auto iter = std::find(std::begin(_soldiers), std::end(_soldiers), soldier);
	for (int k = 0; k < _soldiers.size(); k++) {
		if (_soldiers[k] == soldier) {
			_soldiers.erase(_soldiers.begin() + k);
		}
	}

}

double CovertOperationStartState::statEffectCalc(int stat, float a, float b, float c, float d)
{
	double y0 = exp(pow(stat, b) / a) - 1;
	double y1 = c * (y0/(y0 + 1)) + d;
	return y1;
}

}

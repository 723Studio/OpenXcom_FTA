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
#include <iomanip>
#include <algorithm>
#include <locale>
#include "../fmath.h"
#include "../Interface/Window.h"
#include "../Interface/TextButton.h"
#include "../Interface/Text.h"
#include "../Engine/SurfaceSet.h"
#include "../Engine/Game.h"
#include "../Engine/Action.h"
#include "../Engine/LocalizedText.h"
#include "../Engine/Options.h"
#include "../Engine/Timer.h"
#include "../Engine/Unicode.h"
#include "../Mod/Mod.h"
#include "../Mod/RuleItem.h"
#include "../Mod/RuleCovertOperation.h"
#include "../Mod/Armor.h"
#include "../Mod/RuleSoldier.h"
#include "../Savegame/Base.h"
#include "../Savegame/ItemContainer.h"
#include "../Savegame/CovertOperation.h"
#include "../Savegame/Soldier.h"
#include "../Savegame/SavedGame.h"

namespace OpenXcom
{

/**
* Initializes all the elements in the CoverOperation start screen.
* @param base Pointer to the base to get info from.
* @param rule RuleCovertOperation to start.
*/
CovertOperationStartState::CovertOperationStartState(Base* base, RuleCovertOperation* rule) :
	_base(base), _rule(rule), _cost(0), _chances(0), _hasPsiItems(false), _hasPsionics(false)
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

	_txtSoldiersRequired = new Text(220, 9, 8, descrDY + 1);
	_txtSoldiersAssigned = new Text(220, 9, 8, descrDY + 10);
	_btnSoldiers = new TextButton(64, 16, 8, descrDY + 20);
	_btnEquipmet = new TextButton(64, 16, 8, descrDY + 37);
	_btnArmor = new TextButton(64, 16, 8, descrDY + 54);

	_crew = new Surface(220, 18, 77, descrDY + 19);
	_equip = new Surface(220, 18, 77, descrDY + 37);

	_txtCategories = new Text(110, 9, 202, 98);
	_txtCategoriesList = new Text(110, 27, 202, 108);
	_txtDuration = new Text(304, 9, 8, 153);
	_txtChances = new Text(304, 9, 8, 164);

	setInterface("newCovertOperationsMenu");
	add(_window, "window", "newCovertOperationsMenu");
	add(_txtTitle, "text", "newCovertOperationsMenu");
	add(_txtDescription, "text", "newCovertOperationsMenu");
	add(_btnCancel, "button", "newCovertOperationsMenu");
	add(_btnStart, "button", "newCovertOperationsMenu");

	add(_txtSoldiersRequired, "text", "newCovertOperationsMenu");
	add(_txtSoldiersAssigned, "text", "newCovertOperationsMenu");
	add(_btnSoldiers, "button", "newCovertOperationsMenu");
	add(_btnEquipmet, "button", "newCovertOperationsMenu");
	add(_btnArmor, "button", "newCovertOperationsMenu");
	add(_crew);
	add(_equip);

	add(_txtCategories, "text", "newCovertOperationsMenu");
	add(_txtCategoriesList, "text", "newCovertOperationsMenu");
	add(_txtDuration, "text", "newCovertOperationsMenu");
	add(_txtChances, "text", "newCovertOperationsMenu");
	
	centerAllSurfaces();

	setWindowBackground(_window, "newCovertOperationsMenu");

	_txtTitle->setText(tr(_rule->getName()));
	_txtTitle->setBig();
	_txtTitle->setAlign(ALIGN_CENTER);

	std::ostringstream sd;
	sd << tr(_rule->getDescription());
	if (!_rule->getRequiredItemList().empty())
	{
		sd << "\n";
		sd << tr("STR_REQUIRED_ITEMS_LC");
		for (auto item : _rule->getRequiredItemList())
		{
			sd << "\n";
			sd << tr(item.first->getType());
			sd << ">" << "\x01" << item.second << "\x01";
		}
	}
	_txtDescription->setText(sd.str());
	_txtDescription->setWordWrap(true);
	_txtDescription->setScrollable(true);

	_btnStart->setText(tr("STR_START_OPERATION_US")); 
	_btnStart->onMouseClick((ActionHandler)&CovertOperationStartState::btnStartClick);
	_btnStart->onKeyboardPress((ActionHandler)&CovertOperationStartState::btnStartClick, Options::keyOk);

	_btnCancel->setText(tr("STR_CANCEL_UC"));
	_btnCancel->onMouseClick((ActionHandler)&CovertOperationStartState::btnCancelClick);
	_btnCancel->onKeyboardPress((ActionHandler)&CovertOperationStartState::btnCancelClick, Options::keyCancel);

	std::ostringstream se;
	se << _rule->getSoldierSlotsMin() << "/" << _rule->getSoldierSlotsMax();
	_txtSoldiersRequired->setText(tr("STR_SOLDIERS_REQUIRED").arg(se.str()));

	_btnSoldiers->setText(tr("STR_SOLDIERS_UC"));
	_btnSoldiers->onMouseClick((ActionHandler)&CovertOperationStartState::btnSoldiersClick);
	_btnEquipmet->setText(tr("STR_EQUIPMENT_UC"));
	_btnEquipmet->onMouseClick((ActionHandler)&CovertOperationStartState::btnEquipmetClick);
	_btnArmor->setText(tr("STR_ARMOR"));
	_btnArmor->onMouseClick((ActionHandler)&CovertOperationStartState::btnArmorClick);

	auto cats = _rule->getCategories();
	if (cats.size() > 0)
	{
		_txtCategories->setText(tr("STR_COVERT_OPS_CATEGORIES"));
		_txtCategories->setAlign(ALIGN_RIGHT);
		size_t pos = 0;
		std::ostringstream ss;
		for (const auto &it : cats)
		{
			if (pos > 0)
				ss << ", ";
			ss << tr(it);
			pos++;
		}
		_txtCategoriesList->setText(ss.str());
		_txtCategoriesList->setAlign(ALIGN_RIGHT);
	}
	else
	{
		_txtCategories->setVisible(false);
		_txtCategoriesList->setVisible(false);
		_txtCategoriesList->setWordWrap(true);
	}
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

	bool debug = _game->getSavedGame()->getDebugMode();
	_txtDuration->setText(tr("STR_OPERATION_DURATION_UC").arg(tr(getOperationTimeString(debug))));
	_txtDuration->setAlign(ALIGN_RIGHT);
	_txtChances->setText(tr("STR_OPERATION_CHANCES_UC").arg(tr(getOperationOddsString(debug))));
	_txtChances->setAlign(ALIGN_RIGHT);

	_btnStart->setVisible(_soldiers.size() >= (size_t)_rule->getSoldierSlotsMin());
	auto reqItems = _rule->getRequiredItemList();
	if (!reqItems.empty())
	{
		int reqItemsN = 0;
		for (auto& [reqItem, reqValue] : reqItems)
		{
			reqItemsN += reqValue;
			for (const auto& [contentItem, contentValue] : *_items->getContents())
			{
				if (contentItem == reqItem)
				{
					reqItemsN -= contentValue;
				}
			}
		}
		
		_btnStart->setVisible(reqItemsN <= 0);
	}
	_btnEquipmet->setVisible(!_soldiers.empty() || _items->getTotalQuantity() > 0);
	_btnArmor->setVisible(!_soldiers.empty());


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
			if (Surface* customFrame1 = customArmorPreviews->getFrame(index))
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

	x = 0;
	Surface* frame2 = texture->getFrame(39);
	for (int i = 0; i < _items->getTotalQuantity(); i += 4, x += 10)
	{
		frame2->blitNShade(_equip, x, 0);
	}
}

/**
* Returns to previous screen.
* @param action A pointer to an Action.
*/
void CovertOperationStartState::btnCancelClick(Action*)
{
	// lets return all items back to base
	for (auto& item : *_items->getContents())
	{
		_base->getStorageItems()->addItem(item.first, item.second);
	}
	
	_game->popState();
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

	CovertOperation* newOperation = new CovertOperation(_rule, _game->getMod(), _base, cost, chances);
	_base->addCovertOperation(newOperation);
	// lets update operation with items and personell and assign soldiers.
	for (auto& item : *_items->getContents())
	{
		newOperation->getItems()->addItem(item.first, item.second);
		if (item.first->getBattleType() == BT_PSIAMP)
		{
			_hasPsiItems = true;
		}
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
			(*i)->clearBaseDuty();
			(*i)->setCovertOperation(newOperation);
			
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


	if (_hasPsionics && _hasPsiItems && _game->getSavedGame()->isResearched(_game->getMod()->getPsiRequirements()))
	{ //operation that we are about to start has psionic offensive potential
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


std::string CovertOperationStartState::getOperationTimeString(bool mod)
{
	int time = getOperationCost();
	if (!mod)
	{
		if (time > 45 * 24)
		{
			return ("STR_SEVERAL_MONTHS");
		}
		else if (time > 20 * 24)
		{
			return ("STR_A_MONTH");
		}
		else if (time > 10 * 24)
		{
			return ("STR_SEVERAL_WEEKS");
		}
		else if (time > 6 * 24)
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

std::string CovertOperationStartState::getOperationOddsString(bool debug)
{
	int odds = round(getOperationOdds());
	if (!debug)
	{
		if (odds > 100)
		{
			return ("STR_GREAT");
		}
		else if (odds > 70)
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
	bool investigation = false, infiltration = false, negotiation = false, deception = false, engineering = false, science = false;
	int statsNumber = 0;
	auto cats = _rule->getCategories();
	if (std::find(cats.begin(), cats.end(), "STR_INVESTIGATION") != cats.end())
	{
		investigation = true;
		statsNumber++;
	}
	if (std::find(cats.begin(), cats.end(), "STR_INFILTRATION") != cats.end())
	{
		infiltration = true;
		statsNumber++;
	}
	if (std::find(cats.begin(), cats.end(), "STR_NEGOTIATION") != cats.end())
	{
		negotiation = true;
		statsNumber++;
	}
	if (std::find(cats.begin(), cats.end(), "STR_DECEPTION") != cats.end())
	{
		deception = true;
		statsNumber++;
	}
	if (std::find(cats.begin(), cats.end(), "STR_ENGINEERING") != cats.end())
	{
		engineering = true;
	}
	if (std::find(cats.begin(), cats.end(), "STR_SCIENCE") != cats.end())
	{
		science = true;
	}
	if (!investigation && !infiltration && !negotiation && !deception)
	{
		investigation = true; // we use it as a default option for further calculations
		statsNumber = 1;
	}

	_chances = _rule->getBaseChances();

	GameDifficulty diff = _game->getSavedGame()->getDifficulty();
	int staffCoeff = 1;
	switch (diff)
	{
	case DIFF_BEGINNER:
		_chances *= 1.1;
		break;
	case DIFF_EXPERIENCED:
		break;
	case DIFF_VETERAN:
		_chances *= 0.95;
		break;
	case DIFF_GENIUS:
		_chances *= 0.9;
		break;
	case DIFF_SUPERHUMAN:
		_chances *= 0.85;
		staffCoeff = 2;
		break;
	}

	int assignedSoldiersN = _soldiers.size();
	if (assignedSoldiersN > _rule->getSoldierSlotsMin())
	{
		_chances += (assignedSoldiersN - _rule->getSoldierSlotsMin()) * _rule->getOptionalSoldierEffect();
	}
	//lets see if we need some decrease because of required items
	auto bonItems = _rule->getBonusItemList();
	if (!bonItems.empty())
	{
		int bonItemsN = 0;
		for (auto& it : bonItems)
		{
			bonItemsN += it.second;
			for (auto& j : *_items->getContents())
			{
				if (j.first == it.first && j.second > it.second)
				{
					bonItemsN -= j.second;
				}
			}
		}

		_chances -= (double)_rule->getBonusItemsEffect() * bonItemsN;
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
		_chances -= armorlessSoldiers * static_cast<double>(_rule->getAllowedArmorEffect());
	}
	//lets process soldier stats as pure bonus
	if (assignedSoldiersN > 0)
	{
		int scientistsN = 0;
		int engineersN = 0;
		int soldierMaxRank = 0;
		int soldiersTotalRank = 0;
		//extract soldier stats we need for processing
		for (auto& s : _soldiers) 
		{
			auto stats = s->getStatsWithAllBonuses();
			double soldierBonus = 0;

			if (s->getRoleRank(ROLE_SCIENTIST) > 0)
				scientistsN++;
			if (s->getRoleRank(ROLE_ENGINEER) > 0)
				engineersN++;

			//lets get soldier effectiveness first, if any
			double solEffectiveness = 100;
			if (!_rule->getSoldierTypeEffectiveness().empty())
			{
				std::string solType = s->getRules()->getType();
				auto ruleType = _rule->getSoldierTypeEffectiveness();
				for (auto& t : ruleType)
				{
					std::string ruleTypeName = t.first;
					if (ruleTypeName == solType)
					{
						solEffectiveness = t.second;
					}
				}
			}

			//lets make a bonus for having officer for field command and avg ranking
			int rank = s->getBestRoleRank().second;
			if (rank > soldierMaxRank)
			{
				soldierMaxRank = rank;
			}
			soldiersTotalRank += rank;

			//now lets handle soldier stats
			if (investigation)
			{
				soldierBonus += (double)stats->investigation / 5;
				soldierBonus += (double)stats->perception / 10;
				soldierBonus += (double)stats->interrogation / 10;
			}
			if (infiltration)
			{
				soldierBonus += (double)stats->stealth / 5;
				soldierBonus += (double)stats->perception / 10;
				soldierBonus += (double)stats->interrogation / 10;
				soldierBonus += statEffectCalc(stats->bravery, 1800, 2.1, 15, -8);
			}
			if (negotiation)
			{
				soldierBonus += (double)stats->charisma / 3;
			}
			if (deception)
			{
				soldierBonus += (double)stats->deception / 3;
				soldierBonus += statEffectCalc(stats->bravery, 1800, 2.1, 9, -5);
			}

			int psi = stats->psiSkill;
			if (psi > 0 && _game->getSavedGame()->isResearched(_game->getMod()->getPsiRequirements())) //psi offensive bonus
				soldierBonus += (statEffectCalc(psi, 7000, 2.2, 8, 0) + statEffectCalc(stats->psiStrength, 7000, 2.2, 8, 0)) / 2;
			else
				soldierBonus += statEffectCalc(stats->psiStrength, 8000, 2.2, 8, 0) / 4; //as soldier still has psi defence and some excrescence capabilities

			soldierBonus = soldierBonus * solEffectiveness / 100 / statsNumber;
			_chances += soldierBonus;
			soldierBonus = 0;
		}

		double officerEffect = 0;
		if (assignedSoldiersN > 3)
		{
			officerEffect = -0.2321 * pow(soldierMaxRank, 2) + 2.5036 * soldierMaxRank + 0.0357; // cute nonlinear function for field officer + avg rank bonus
		}
		double rankEffect = (double)soldiersTotalRank / assignedSoldiersN;
		_chances += officerEffect + rankEffect;

		// let's check if itemset has specific FTA's item categories
		if (!_rule->getAllowAllEquipment())
		{
			bool allConsealed = true;
			int heavy = 0;
			int itemConcealedBonusEffect = _rule->getConcealedItemsBonus();
			double itemCatEffect = 0;

			double diffCoeff = 1;
			switch (diff)
			{
			case DIFF_BEGINNER:
				diffCoeff = 0.90;
				break;
			case DIFF_EXPERIENCED:
				break;
			case DIFF_VETERAN:
				diffCoeff *= 1.1;
				break;
			case DIFF_GENIUS:
				diffCoeff *= 1.3;
				break;
			case DIFF_SUPERHUMAN:
				diffCoeff *= 1.7;
				break;
			}

			for (auto& items : *_items->getContents())
			{
				if (!items.first->belongsToCategory("STR_CONCEALABLE"))
					allConsealed = false;
				if (items.first->belongsToCategory("STR_HEAVY_WEAPONS") && !items.first->belongsToCategory("STR_CLIPS"))
					++heavy;
			}

			if (!allConsealed)
			{
				itemCatEffect = -itemConcealedBonusEffect * diffCoeff;
			}

			itemCatEffect = itemCatEffect - diffCoeff * heavy * itemConcealedBonusEffect * 4;
			_chances += itemCatEffect;
		}
	}

	double intelBonus = ((double)_base->getOperationBoost() * 100 ) / 100;
	_chances += intelBonus;

	if (_chances > 200) // we dont want too high chances
		_chances = 200;
	return _chances;
}

int CovertOperationStartState::getOperationCost()
{
	_cost = _rule->getCosts(); //load initial rule value
	int reducedCost = _cost;
	if (_chances > 100)
	{
		double bonus = (_chances - 100) / (_chances - 82) * 24 / 100; //some cute nonlinear calculation
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
	//auto iter = std::find(std::begin(_soldiers), std::end(_soldiers), soldier); //#FINNIKCHECK
	for (size_t k = 0; k < _soldiers.size(); k++) {
		if (_soldiers[k] == soldier) {
			_soldiers.erase(_soldiers.begin() + k);
		}
	}
}

/**
 * Expresses the effective influence of a soldier's characteristic on the result of an operation, based on the input coefficients
 * https://github.com/723Studio/OpenXcom_FTA/wiki/Covert-operations#effective-stat-impact
 * @param stat - soldier stat
 * @param a - shifts the inflection of the graph to the right
 * @param b - the slope of the inflection
 * @param c - limits values from above
 * @param d - initial value of the function at 0
 * @return - effective value
 */
double CovertOperationStartState::statEffectCalc(int stat, double a, double b, double c, double d)
{
	double y0 = exp(pow(stat, b) / a) - 1;
	return c * (y0 / (y0 + 1)) + d;
}

void CovertOperationStartState::addSoldier(Soldier *soldier)
{
	_soldiers.push_back(soldier);
	auto compareRole = [](const Soldier* lhs, const Soldier* rhs)
	{
		return lhs->getRoles()[0]->role < rhs->getRoles()[0]->role;
	};
	sort(_soldiers.begin(), _soldiers.end(), compareRole);
}

}

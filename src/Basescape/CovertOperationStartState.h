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
#include "SoldierSortUtil.h"

namespace OpenXcom
{

class Base;
class RuleCovertOperation;
class Window;
class TextButton;
class ToggleTextButton;
class Text;
class Surface;
class ItemContainer;

/**
* Screen which displays needed elements to start productions (items/required workshop state/cost to build a unit, ...).
*/
class CovertOperationStartState : public State
{
private:
	Base* _base;
	RuleCovertOperation* _rule;
	Window* _window;
	TextButton* _btnCancel, * _btnStart, * _btnSoldiers, * _btnEquipmet, * _btnArmor;
	TextButton* _btnAddScientist, * _btnRemoveScientist, * _btnAddEngineer, * _btnRemoveEngineer, * _btnResearchState, * _btnManufactureState;
	Text* _txtTitle, * _txtDescription, * _txtSoldiersRequired, * _txtSoldiersAssigned, * _txtOptionalSoldiers, * _txtScientistsAssigned, * _txtEngineersAssigned, * _txtDuration, * _txtChances;
	int _cost, _scientists, _engeneers;
	double _chances;
	bool _hasPsionics, _hasPsiItems;
	std::vector<Soldier *> _soldiers;
	ItemContainer* _items;
	Surface* _crew, * _equip;
public:
	/// Creates the State.
	CovertOperationStartState(Base* base, RuleCovertOperation* rule);
	~CovertOperationStartState();
	// *** UI Section ***
	/// Updates the operation info.
	void init() override;
	/// Handler for the Cancel button.
	void btnCancelClick(Action* action);
	/// Handler for the Start button.
	void btnStartClick(Action* action);
	/// Handler for the Soldiers button.
	void btnSoldiersClick(Action* action);
	/// Handler for the Equipmet button.
	void btnEquipmetClick(Action* action);
	/// Handler for the Armor button.
	void btnArmorClick(Action* action);
	/// Handler for the AddScientist button.
	void btnAddScientistClick(Action* action);
	/// Handler for the RemoveScientist button.
	void btnRemoveScientistClick(Action* action);
	/// Handler for the ResearchState button.
	void btnResearchStateClick(Action* action);
	/// Handler for the AddEngineer button.
	void btnAddEngineerClick(Action* action);
	/// Handler for the RemoveEngineer button.
	void btnRemoveEngineerClick(Action* action);
	/// Handler for the ManufactureState button.
	void btnManufactureStateClick(Action* action);
	/// Gets string that discribes prediction of timeframe that needs to finish the operation.
	std::string getOperationTimeString(bool mod = false);
	/// Gets string that discribes operations odds of success.
	std::string getOperationOddsString(bool mod = false);

	// *** Logic Section ***
	/// Gets operation success chances.
	double getOperationOdds();
	/// Gets operation's final cost.
	int getOperationCost();
	/// Gets operation rules
	const RuleCovertOperation* getRule() { return _rule; }
	/// Gets items assigned for this planned operation.
	ItemContainer* getItems() { return _items; };
	/// Sets item set for this planned operation.
	void setItems(ItemContainer* items) { _items = items; };
	/// Gets soldiers assigned for this planned operation.
	std::vector<Soldier*> getSoldiers() { return _soldiers; };
	void addSoldier(Soldier* soldier) { _soldiers.push_back(soldier); }
	void removeSoldier(Soldier* soldier);
	/// Sets soldiers assigned for this planned operation.
	void setSoldiers(std::vector<Soldier*> soldiers) { _soldiers = soldiers; };
	/// Gets if there are assigned psionic agents on this operation.
	int getHasPsiAssigned() const { return _hasPsionics; };
	/// Sets if there are assigned psionic agents on this operation.
	void setHasPsiAssigned(int hasPsionics) { _hasPsionics = hasPsionics; };
	/// Gets if there is equipped psionic equipment on this operation.
	int getHasPsiItems() const { return _hasPsiItems; };
	/// Sets if there is equipped psionic equipment on this operation.
	void setHasPsiItems(int hasPsiItems) { _hasPsiItems = hasPsiItems; };
	/// Gets effective value for imputed soldier stat.
	double statEffectCalc(int stat, float a, float b, float c, float d);
};

}

#pragma once
/*
 * Copyright 2010-2022 OpenXcom Developers.
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

class BasescapeState;
class Base;
class Surface;
class MiniBaseView;
class TextButton;
class TextEdit;
class Text;
class Bar;

/**
 * Base Info screen that shows all the
 * stats of a base from the Basescape.
 */
class BaseInfoStateFtA : public State
{
private:
	Base *_base;
	BasescapeState *_state;

	Surface *_bg;
	MiniBaseView *_mini;
	TextButton *_btnOk, *_btnTransfers, *_btnStores, *_btnMonthlyCosts, *_btnDispose;
	TextEdit *_edtBase;

	Text *_txtPersonnel, *_txtSoldiers, *_txtQuarters, *_txtStores, *_txtLaboratories, *_txtWorkshops, *_txtGym, *_txtInterrogation;
	Text *_numSoldiers, *_numQuarters, *_numStores, *_numLaboratories, *_numWorkshops, *_numGym, *_numInterrogation;
	Bar *_barSoldiers,*_barQuarters, *_barStores, *_barLaboratories, *_barWorkshops, *_barGym, *_barInterrogation;

	Text *_txtPrison, *_txtMonsters, *_txtContainment, *_txtPower, *_txtDefense, *_txtRadar, *_txtGlobalDetection;
	Text *_numPrison, *_numMonsters, *_numContainment, *_numPower, *_numDefense, *_numRadar, *_numGlobalDetection;
	Bar *_barPrison, *_barMonsters, *_barContainment, *_barPower, *_barDefense, *_barRadar, *_barGlobalDetection;
public:
	/// Creates the Base Info state.
	BaseInfoStateFtA(Base *base, BasescapeState *state);
	/// Cleans up the Base Info state.
	~BaseInfoStateFtA();
	/// Updates the base stats.
	void init() override;
	/// Handler for changing the text on the Name edit.
	void edtBaseChange(Action *action);
	/// Handler for clicking the mini base view.
	void miniClick(Action *action);
	/// Handler for selecting bases.
	void handleKeyPress(Action *action);
	/// Handler for clicking the OK button.
	void btnOkClick(Action *action);
	/// Handler for clicking the Transfers button.
	void btnTransfersClick(Action *action);
	/// Handler for clicking the Stores button.
	void btnStoresClick(Action *action);
	/// Handler for clicking the Dispose button.
	void btnDisposeClick(Action* action);
	/// Handler for clicking the Monthly Costs button.
	void btnMonthlyCostsClick(Action *action);
};

}

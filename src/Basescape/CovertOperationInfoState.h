#pragma once
/*
 * Copyright 2010-2015 OpenXcom Developers.
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
#include <vector>
#include <map>
#include <string>
#include <unordered_set>

namespace OpenXcom
{

	class TextButton;
	class Window;
	class Text;
	class TextList;
	class CovertOperation;
	class RuleCovertOperation;

	/**
	 * Displays general info about ongoing covert operation, allows player to terminate operations.
	 */
	class CovertOperationInfoState : public State
	{
	private:
		TextButton* _btnOk, * _btnTerminate;
		Window* _window;
		Text* _txtTitle, * _txtScientists, * _txtEngineers, * _txtDungeonLevel, * _txtProgress, * _txtSoldiers, * _txtAditionalInfo;
		TextList* _lstSoldiers, * _lstAditionalInfo;
		CovertOperation* _operation;
		RuleCovertOperation* _rule;
	public:
		/// Creates the Tech Tree Viewer state.
		CovertOperationInfoState(CovertOperation* operation);
		/// Cleans up the Tech Tree Viewer state.
		~CovertOperationInfoState();
		/// Handler for clicking the OK button.
		void btnOkClick(Action* action);
		/// Handler for clicking the New button.
		void btnTerminateClick(Action* action);
		// init class
		void init() override;

		void fillSoldiersList();
		void fillAditionalInfoList();
	};

	class CovertOperationConfirmTerminateState : public State
	{
	private:
		TextButton* _btnOk, * _btnCancel;
		Window* _window;
		Text* _txtText;
		CovertOperation* _operation;
	public:
		/// Creates the Tech Tree Viewer state.
		CovertOperationConfirmTerminateState(CovertOperation* operation);
		/// Cleans up the Tech Tree Viewer state.
		~CovertOperationConfirmTerminateState();
		/// Handler for clicking the OK button.
		void btnOkClick(Action* action);
		/// Handler for clicking the New button.
		void btnCancelClick(Action* action);
	};



}

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

namespace OpenXcom
{

	class TextButton;
	class Window;
	class Text;
	class TextList;
	class CovertOperation;
	class CovertOperationResults;
	class CommendationLateState;

	/**
	 * Displays info about complete Covert Operations.
	 */
	class FinishedCoverOperationDetailsState : public State
	{
	private:
		Window* _window;
		Text* _txtTitle, *_txtItem, *_txtReputation, * _txtSoldierStatus, *_txtMessage;
		TextList* _lstRecoveredItems, *_lstReputation, *_lstFunds, * _lstScore, * _lstSoldierStatus, * _lstSoldierStats;
		std::string _currentTooltip;
		TextButton* _btnOk, * _btnPage;
		bool _hasItems, _hasRep, _hasFunds, _hasScore, _hasSStatus, _hasMessage, _hasMIA;
		std::string _researchName;
		CovertOperation* _operation;
		int _pageNumber;
		CovertOperationResults* _results;

	public:
		/// Creates the FinishedCoverOperationState.
		FinishedCoverOperationDetailsState(CovertOperation* operation);
		/// Cleans up the FinishedCoverOperationState.
		~FinishedCoverOperationDetailsState();
		/// Creates a string for the soldier stats table from a stat difference value
		std::string makeSoldierString(int stat);
		/// Handler for clicking the OK button.
		void btnOkClick(Action* action);
		/// Handler for clicking the DETAILS button.
		void btnPageClick(Action* action);
		/// Hide all page UI elements
		void hidePageUI();
		/// Drawing first page
		void firstPage();
		/// Drawing second page
		void secondPage();
	};

}

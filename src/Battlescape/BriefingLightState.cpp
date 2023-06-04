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
#include "BriefingLightState.h"
#include <vector>
#include "../Engine/Game.h"
#include "../Engine/LocalizedText.h"
#include "../Interface/TextButton.h"
#include "../Interface/Text.h"
#include "../Interface/TextList.h"
#include "../Interface/ToggleTextButton.h"
#include "../Interface/Window.h"
#include "../Mod/Mod.h"
#include "../Mod/AlienDeployment.h"
#include "../Mod/ArticleDefinition.h"
#include "../Mod/RuleStartingCondition.h"
#include "../Savegame/SavedGame.h"
#include "../Engine/Options.h"
#include "../Engine/Screen.h"
#include "../Engine/Unicode.h"

namespace OpenXcom
{

/**
 * Initializes all the elements in the BriefingLight screen.
 * @param deployment Pointer to the mission deployment.
 */
BriefingLightState::BriefingLightState(AlienDeployment *deployment)
{
	_screen = true;
	// Create objects
	_window = new Window(this, 320, 200, 0, 0);
	_btnOk = new TextButton(140, 18, 164, 164);
	_btnArmors = new ToggleTextButton(140, 18, 16, 164);
	_txtTitle = new Text(300, 32, 16, 24);
	_txtBriefing = new Text(288, 104, 16, 56);
	_txtArmors = new Text(288, 25, 16, 56);
	_lstArmors = new TextList(280, 80, 8, 81);

	std::string title = deployment->getType();
	std::string desc = deployment->getAlertDescription();

	BriefingData data = deployment->getBriefingData();
	setStandardPalette("PAL_GEOSCAPE", data.palette);
	_window->setBackground(_game->getMod()->getSurface(data.background));

	add(_window, "window", "briefing");
	add(_btnOk, "button", "briefing");
	add(_btnArmors, "button", "briefing");
	add(_txtTitle, "text", "briefing");
	add(_txtBriefing, "text", "briefing");
	add(_txtArmors, "text", "briefing");
	add(_lstArmors, "text", "briefing");

	centerAllSurfaces();

	// Set up objects
	_btnOk->setText(tr("STR_OK"));
	_btnOk->onMouseClick((ActionHandler)&BriefingLightState::btnOkClick);
	_btnOk->onKeyboardPress((ActionHandler)&BriefingLightState::btnOkClick, Options::keyOk);
	_btnOk->onKeyboardPress((ActionHandler)&BriefingLightState::btnOkClick, Options::keyCancel);

	_btnArmors->setText(tr("STR_WHAT_CAN_I_WEAR"));
	_btnArmors->onMouseClick((ActionHandler)&BriefingLightState::btnArmorsClick);
	_btnArmors->setVisible(false);

	_txtTitle->setBig();
	_txtTitle->setText(tr(title));

	_txtBriefing->setWordWrap(true);
	_txtBriefing->setText(tr(desc));

	_txtArmors->setWordWrap(true);
	_txtArmors->setVisible(false);

	_lstArmors->setColumns(2, 148, 116);
	_lstArmors->setSelectable(true);
	_lstArmors->setBackground(_window);
	_lstArmors->setMargin(8);
	_lstArmors->setVisible(false);

	checkStartingCondition(deployment);
}

/**
* Checks the starting condition.
*/
void BriefingLightState::checkStartingCondition(AlienDeployment *deployment)
{
	const RuleStartingCondition *startingCondition = _game->getMod()->getStartingCondition(deployment->getStartingCondition());
	if (startingCondition != 0)
	{
		auto& list = startingCondition->getForbiddenArmors().empty() ? startingCondition->getAllowedArmors() : startingCondition->getForbiddenArmors();
		const std::string messageCode = startingCondition->getForbiddenArmors().empty() ? "STR_STARTING_CONDITION_ARMORS_ALLOWED" : "STR_STARTING_CONDITION_ARMORS_FORBIDDEN";
		if (!list.empty())
		{
			_txtArmors->setText(tr(messageCode).arg("")); // passing empty argument, because it is obsolete since a list display was introduced
			_btnArmors->setVisible(true);

			std::vector<std::string> armorNameList;
			for (auto& armorType : list)
			{
				ArticleDefinition* article = _game->getMod()->getUfopaediaArticle(armorType, false);
				if (article && _game->getSavedGame()->isResearched(article->_requires))
				{
					std::string translation = tr(armorType);
					armorNameList.push_back(translation);
				}
			}
			if (armorNameList.empty())
			{
				// no suitable armor yet
				std::string translation = tr("STR_UNKNOWN");
				armorNameList.push_back(translation);
			}
			std::sort(armorNameList.begin(), armorNameList.end(), [&](std::string& a, std::string& b) { return Unicode::naturalCompare(a, b); });
			if (armorNameList.size() % 2 != 0)
			{
				armorNameList.push_back(""); // just padding, we want an even number of items in the list
			}
			size_t halfSize = armorNameList.size() / 2;
			for (size_t i = 0; i < halfSize; ++i)
			{
				_lstArmors->addRow(2, armorNameList[i].c_str(), armorNameList[i + halfSize].c_str());
			}
		}
	}
}

/**
 *
 */
BriefingLightState::~BriefingLightState()
{

}

/**
 * Closes the window.
 * @param action Pointer to an action.
 */
void BriefingLightState::btnOkClick(Action *)
{
	_game->popState();
}

/**
 * Shows allowed armors.
 * @param action Pointer to an action.
 */
void BriefingLightState::btnArmorsClick(Action *)
{
	_txtArmors->setVisible(_btnArmors->getPressed());
	_lstArmors->setVisible(_btnArmors->getPressed());
	_txtBriefing->setVisible(!_btnArmors->getPressed());
}

}

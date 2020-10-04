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
#include "../Engine/Game.h"
#include "../Engine/LocalizedText.h"
#include "../Interface/TextButton.h"
#include "../Interface/Text.h"
#include "../Interface/ToggleTextButton.h"
#include "../Interface/Window.h"
#include "../Mod/Mod.h"
#include "../Mod/AlienDeployment.h"
#include "../Mod/ArticleDefinition.h"
#include "../Mod/RuleStartingCondition.h"
#include "../Savegame/SavedGame.h"
#include "../Engine/Options.h"
#include "../Engine/Screen.h"

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
	_txtArmors = new Text(288, 104, 16, 56);

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

	centerAllSurfaces();

	// Set up objects
	_btnOk->setText(tr("STR_OK"));
	_btnOk->onMouseClick((ActionHandler)&BriefingLightState::btnOkClick);
	_btnOk->onKeyboardPress((ActionHandler)&BriefingLightState::btnOkClick, Options::keyOk);
	_btnOk->onKeyboardPress((ActionHandler)&BriefingLightState::btnOkClick, Options::keyCancel);

	_btnArmors->setText(tr("STR_WHAT_CAN_I_WEAR"));
	_btnArmors->onMouseClick((ActionHandler)&BriefingLightState::btnArmorsClick);

	_txtTitle->setBig();
	_txtTitle->setText(tr(title));

	_txtBriefing->setWordWrap(true);
	_txtBriefing->setText(tr(desc));

	_txtArmors->setWordWrap(true);
	_txtArmors->setVisible(false);

	std::string message = checkStartingCondition(deployment);
	if (!message.empty())
	{
		_txtArmors->setText(message);
	}
	else
	{
		_btnArmors->setVisible(false);
	}
}

/**
* Checks the starting condition.
*/
std::string BriefingLightState::checkStartingCondition(AlienDeployment *deployment)
{
	const RuleStartingCondition *startingCondition = _game->getMod()->getStartingCondition(deployment->getStartingCondition());
	if (startingCondition != 0)
	{
		auto list = startingCondition->getForbiddenArmors();
		std::string messageCode = "STR_STARTING_CONDITION_ARMORS_FORBIDDEN";
		if (list.empty())
		{
			list = startingCondition->getAllowedArmors();
			messageCode = "STR_STARTING_CONDITION_ARMORS_ALLOWED";
		}
		if (list.empty())
		{
			// everything is allowed
			return "";
		}
		std::ostringstream ss;
		int i = 0;
		for (std::vector<std::string>::const_iterator it = list.begin(); it != list.end(); ++it)
		{
			ArticleDefinition *article = _game->getMod()->getUfopaediaArticle((*it), false);
			if (article && _game->getSavedGame()->isResearched(article->requires))
			{
				if (i > 0)
					ss << ", ";
				ss << tr(*it);
				i++;
			}
		}
		std::string argument = ss.str();
		if (argument.empty())
		{
			// no suitable armor yet
			argument = tr("STR_UNKNOWN");
		}
		return tr(messageCode).arg(argument);
	}
	else
	{
		// everything is allowed
		return "";
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
	_txtBriefing->setVisible(!_btnArmors->getPressed());
}

}

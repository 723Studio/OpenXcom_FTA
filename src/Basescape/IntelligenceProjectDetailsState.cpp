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
#include "IntelligenceProjectDetailsState.h"
#include <sstream>
#include "../Interface/Window.h"
#include "../Interface/TextButton.h"
#include "../Interface/Text.h"
#include "../Engine/Game.h"
#include "../Engine/LocalizedText.h"
#include "../Engine/Options.h"
#include "../Mod/Mod.h"
#include "../Mod/RuleIntelProject.h"
#include "../Savegame/SavedGame.h"
#include "../Mod/RuleInterface.h"

namespace OpenXcom
{

/**
 * Initializes all the elements in the productions start screen.
 * @param rule The RuleIntelProject.
 */
IntelligenceProjectDetailsState::IntelligenceProjectDetailsState(const RuleIntelProject* rule) : _rule(rule)
{
	_screen = false;

	_window = new Window(this, 168, 200, 152, 0);
	_btnOk = new TextButton(146, 16, 163, 175);
	_txtTitle = new Text(154, 17, 159, 7);
	_txtSpecialType = new Text(154, 9, 159, 27);
	_txtReqStatsHeader = new Text(154, 9, 159, 37);
	_txtReqStats = new Text(154, 19, 159, 47);
	_txtDescriptionHeader = new Text(154, 9, 159, 67);
	_txtDescription = new Text(154, 95, 159, 77);

	// Set palette
	setInterface("intelDetailsMenu");

	add(_window, "window", "intelDetailsMenu");
	add(_txtTitle, "text", "intelDetailsMenu");
	add(_txtSpecialType, "text", "intelDetailsMenu");
	add(_txtReqStatsHeader, "text", "intelDetailsMenu");
	add(_txtReqStats, "text", "intelDetailsMenu");
	add(_txtDescriptionHeader, "text", "intelDetailsMenu");
	add(_txtDescription, "text", "intelDetailsMenu");
	add(_btnOk, "button", "intelDetailsMenu");

	centerAllSurfaces();

	setWindowBackground(_window, "intelDetailsMenu");

	_txtTitle->setText(tr("STR_INTEL_STATE_INFO"));
	_txtTitle->setBig();
	_txtTitle->setAlign(ALIGN_CENTER);

	std::string typeName;
	switch (_rule->getSpecialRule()) {
	case INTEL_NONE:
		typeName = "STR_INTEL_NONE";
		break;
	case INTEL_UFO_TRACKING:
		typeName = "STR_INTEL_UFO_TRACKING";
		break;
	case INTEL_COVERT_OPERATIONS:
		typeName = "STR_INTEL_COVERT_OPERATIONS";
		break;
	case INTEL_DEPLOYMENT_HINTS:
		typeName = "STR_INTEL_DEPLOYMENT_HINTS";
		break;
	default: ;
	}

	_txtSpecialType->setText(tr("STR_PROJECT_TYPE").arg(tr(typeName)));

	
	_txtReqStatsHeader->setText(tr("STR_REQUIRED_STATS"));

	_txtReqStats->setText(generateStatsList());
	_txtReqStats->setWordWrap(true);
	_txtReqStats->setColor(_game->getMod()->getInterface("intelDetailsMenu")->getElement("text")->color2);

	_txtDescriptionHeader->setText(tr("STR_PROJECT_DESCRIPTION"));

	_txtDescription->setText(tr(_rule->getDescription()));
	_txtDescription->setWordWrap(true);
	_txtDescription->setScrollable(true);
	_txtDescription->setColor(_game->getMod()->getInterface("intelDetailsMenu")->getElement("text")->color2);

	_btnOk->setText(tr("STR_OK"));
	_btnOk->onMouseClick((ActionHandler)&IntelligenceProjectDetailsState::btnOKClick);
	_btnOk->onKeyboardPress((ActionHandler)&IntelligenceProjectDetailsState::btnOKClick, Options::keyOk);
	_btnOk->onKeyboardPress((ActionHandler)&IntelligenceProjectDetailsState::btnOKClick, Options::keyCancel);
}

/**
 * Returns to previous screen.
 * @param action A pointer to an Action.
 */
void IntelligenceProjectDetailsState::btnOKClick(Action *)
{
	_game->popState();
}


std::string IntelligenceProjectDetailsState::generateStatsList()
{
	std::ostringstream ss;

	auto stats = _rule->getStats();
	std::map<int, std::string> statMap;

	if (stats.data > 0)
		statMap.insert(std::make_pair(stats.data, tr(UnitStats::getStatString(&UnitStats::data, UnitStats::STATSTR_LC))));
	if (stats.computers > 0)
		statMap.insert(std::make_pair(stats.computers, tr(UnitStats::getStatString(&UnitStats::computers, UnitStats::STATSTR_LC))));
	if (stats.hacking > 0)
		statMap.insert(std::make_pair(stats.hacking, tr(UnitStats::getStatString(&UnitStats::hacking, UnitStats::STATSTR_LC))));
	if (stats.investigation > 0)
		statMap.insert(std::make_pair(stats.investigation, tr(UnitStats::getStatString(&UnitStats::investigation, UnitStats::STATSTR_LC))));

	if (!statMap.empty())
	{
		size_t pos = 0;
		std::pair<int, std::string> result;
		for (auto it = statMap.rbegin(); it != statMap.rend(); ++it)
		{
			if (pos > 0)
			{
				ss << ", ";
			}
			ss << (*it).second;
			pos++;
		}
		ss << ".";
	}

	return ss.str();
}

}

/*
 * Copyright 2010-2026 OpenXcom Developers.
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
#include "DiplomacyDialogueState.h"
#include "DiplomacyHirePersonnelState.h"
#include "DiplomacyStartState.h"
#include <algorithm>
#include "../Engine/Game.h"
#include "../Engine/Action.h"
#include "../Engine/Logger.h"
#include "../Engine/InteractiveSurface.h"
#include "../Engine/RNG.h"
#include "../Interface/Text.h"
#include "../Interface/TextList.h"
#include "../Mod/Mod.h"
#include "../Mod/RuleDiplomacyFaction.h"
#include "../Mod/RuleDiplomacyAction.h"
#include "../Savegame/SavedGame.h"
#include "../Savegame/DiplomacyFaction.h"
#include "../Savegame/Base.h"
#include "../FTA/MasterMind.h"
#include "../Geoscape/GeoscapeState.h"

namespace OpenXcom
{

namespace
{
	DiplomacyFaction* findFactionByRules(SavedGame& save, const RuleDiplomacyFaction* rules)
	{
		if (!rules)
		{
			return nullptr;
		}
		for (auto* f : save.getDiplomacyFactions())
		{
			if (f && f->getRules() == rules)
			{
				return f;
			}
		}
		return nullptr;
	}

	bool isCpalSurfaceName(const std::string& name)
	{
		return name.find("_CPAL") != std::string::npos;
	}

} // namespace

DiplomacyDialogueState::DiplomacyDialogueState(Base* base, DiplomacyFaction* faction)
	: _base(base), _faction(faction), _bg(nullptr), _txtReply(nullptr), _lstActions(nullptr), _txtCloseHint(nullptr), _txtTooltip(nullptr), _exitMode(false), _exitModeIgnoreEvent(nullptr)
{
	_screen = true;

	// UI skeleton (no window, no standard buttons)
	_bg = new InteractiveSurface(320, 200, 0, 0);
	_txtReply = new Text(160, 98, 2, 2);
	_lstActions = new TextList(160, 96, 2, 102);
	_txtCloseHint = new Text(160, 96, 2, 102);
	_txtTooltip = new Text(149, 44, 169, 154);

	setInterface("diplomacyDialogue");

	add(_bg);
	add(_txtReply, "reply", "diplomacyDialogue");
	add(_lstActions, "actions", "diplomacyDialogue");
	add(_txtCloseHint, "reply", "diplomacyDialogue");
	add(_txtTooltip, "tooltip", "diplomacyDialogue");

	// Background
	if (_faction && _faction->getRules())
	{
		auto rules = _faction->getRules();
		std::string bgName = rules->getDiplomacyBackground();
		if (bgName.empty())
		{
			bgName = rules->getBackground();
		}
		const auto* surf = _game->getMod()->getSurface(bgName);
		if (surf)
		{
			surf->blitNShade(_bg, 0, 0);
			if (isCpalSurfaceName(bgName))
			{
				setCustomPalette(surf->getPalette(), Mod::GEOSCAPE_CURSOR);
				_bg->setPalette(_palette);
				_txtReply->setPalette(_palette);
				_lstActions->setPalette(_palette);
				_txtTooltip->setPalette(_palette);
			}
		}
	}

	// Reply
	_txtReply->setWordWrap(true);
	_txtReply->setScrollable(true);

	// Actions list
	// Reserve some space for the built-in scrollbar/arrow buttons (they are drawn outside the list surface).
	_lstActions->setColumns(1, 152); // 160 - 8 for scrollbar
	_lstActions->setSelectable(true);
	_lstActions->setBackground(_bg);
	_lstActions->setMargin(2);
	_lstActions->setWordWrap(true);
	// Keep scroll controls in the gap between actions and tooltip.
	_lstActions->setScrolling(true, -6);
	_lstActions->setSelectorOffsetBlock(+12);
	_lstActions->onMouseOver((ActionHandler)&DiplomacyDialogueState::lstActionsMouseOver);
	_lstActions->onMouseOut((ActionHandler)&DiplomacyDialogueState::lstActionsMouseOut);
	_lstActions->onMouseClick((ActionHandler)&DiplomacyDialogueState::lstActionsClick);

	// Close hint (shown only in exit-mode)
	_txtCloseHint->setWordWrap(true);
	_txtCloseHint->setScrollable(false);
	_txtCloseHint->setVisible(false);

	// Tooltip
	_txtTooltip->setWordWrap(true);
	_txtTooltip->setScrollable(false);
	_txtTooltip->setVerticalAlign(ALIGN_BOTTOM);

	// Exit-mode click/any-key catcher (only closes when _exitMode is true)
	_bg->onMouseClick((ActionHandler)&DiplomacyDialogueState::closeIfExitMode, SDL_BUTTON_LEFT);
	_bg->onMouseClick((ActionHandler)&DiplomacyDialogueState::closeIfExitMode, SDL_BUTTON_RIGHT);
	_bg->onKeyboardPress((ActionHandler)&DiplomacyDialogueState::closeIfExitMode);
	_bg->setFocus(true);

	setGreeting();
	rebuildActionsList();
	centerAllSurfaces();
}

DiplomacyDialogueState::~DiplomacyDialogueState() = default;

void DiplomacyDialogueState::setGreeting()
{
	if (!_faction || !_faction->getRules())
	{
		_txtReply->setText("");
		return;
	}

	const auto& greetings = _faction->getRules()->getGreetings();
	int repLevel = _faction->getReputationLevel();
	auto it = greetings.find(repLevel);
	if (it == greetings.end() || it->second.empty())
	{
		// Fallback: try nearest lower rep level, else nearest higher.
		auto lower = greetings.lower_bound(repLevel);
		// lower_bound points to first element with key >= repLevel
		if (lower != greetings.begin())
		{
			// Prefer the previous element (key < repLevel) if it exists.
			--lower;
			if (!lower->second.empty())
			{
				it = lower;
			}
		}

		if (it == greetings.end() || it->second.empty())
		{
			// Otherwise try first element with key >= repLevel.
			auto higher = greetings.lower_bound(repLevel);
			if (higher != greetings.end() && !higher->second.empty())
			{
				it = higher;
			}
		}
	}

	if (it == greetings.end() || it->second.empty())
	{
		_txtReply->setText("");
		return;
	}

	const auto& list = it->second;
	const int idx = RNG::generate(0, static_cast<int>(list.size()) - 1);
	_txtReply->setText(tr(list.at(static_cast<size_t>(idx))));
}

bool DiplomacyDialogueState::isActionAvailable(const RuleDiplomacyAction& action) const
{
	if (!_faction)
	{
		return false;
	}
	SavedGame* save = _game->getSavedGame();
	if (!save)
	{
		return false;
	}

	const auto& cond = action.getConditions();

	if (_faction->isActionOnCooldown(action.getType()))
	{
		Log(LOG_DEBUG) << "DiplomacyAction " << action.getType() << " hidden: cooldown " << _faction->getActionCooldown(action.getType());
		return false;
	}

	for (const auto& [factionName, minLvl] : cond.reputationLevelMin)
	{
		auto* f = findFactionByRules(*save, factionName);
		if (!f || f->getReputationLevel() < minLvl)
		{
			Log(LOG_DEBUG) << "DiplomacyAction " << action.getType() << " hidden: repLevelMin failed";
			return false;
		}
	}
	for (const auto& [factionName, maxLvl] : cond.reputationLevelMax)
	{
		auto* f = findFactionByRules(*save, factionName);
		if (!f || f->getReputationLevel() >= maxLvl)
		{
			Log(LOG_DEBUG) << "DiplomacyAction " << action.getType() << " hidden: repLevelMax failed";
			return false;
		}
	}

	if (cond.loyaltyMin && save->getLoyalty() < *cond.loyaltyMin)
	{
		Log(LOG_DEBUG) << "DiplomacyAction " << action.getType() << " hidden: loyaltyMin";
		return false;
	}
	if (cond.loyaltyMax && save->getLoyalty() >= *cond.loyaltyMax)
	{
		Log(LOG_DEBUG) << "DiplomacyAction " << action.getType() << " hidden: loyaltyMax";
		return false;
	}

	if (cond.fundsMin && save->getFunds() < *cond.fundsMin)
	{
		Log(LOG_DEBUG) << "DiplomacyAction " << action.getType() << " hidden: fundsMin";
		return false;
	}
	if (cond.fundsMax && save->getFunds() >= *cond.fundsMax)
	{
		Log(LOG_DEBUG) << "DiplomacyAction " << action.getType() << " hidden: fundsMax";
		return false;
	}

	for (const auto& [research, mustHave] : cond.researchTriggers)
	{
		bool has = save->isResearched(research);
		if (mustHave != has)
		{
			Log(LOG_DEBUG) << "DiplomacyAction " << action.getType() << " hidden: researchTrigger failed";
			return false;
		}
	}

	for (const auto& [item, mustHave] : cond.itemTriggers)
	{
		bool has = item ? save->isItemObtained(item->getType(), _game->getMod()) : false;
		if (mustHave != has)
		{
			Log(LOG_DEBUG) << "DiplomacyAction " << action.getType() << " hidden: itemTrigger failed";
			return false;
		}
	}

	for (const auto& [treatyId, mustHave] : cond.treaties)
	{
		bool has = _faction->hasTreaty(treatyId);
		if (mustHave != has)
		{
			Log(LOG_DEBUG) << "DiplomacyAction " << action.getType() << " hidden: treaty failed for " << treatyId;
			return false;
		}
	}

	return true;
}

void DiplomacyDialogueState::rebuildActionsList()
{
	// Normal mode: show options list, hide exit-mode hint.
	if (_txtCloseHint)
	{
		_txtCloseHint->setVisible(false);
	}
	if (_lstActions)
	{
		_lstActions->setVisible(true);
		_lstActions->setSelectable(true);
	}

	_visibleActions.clear();
	_lstActions->clearList();
	_txtTooltip->setText("");

	if (!_faction || !_faction->getRules())
	{
		return;
	}

	const std::string& myFactionName = _faction->getRules()->getName();
	auto* mod = _game->getMod();
	if (!mod)
	{
		return;
	}

	auto list = mod->getDiplomacyActionList();
	if (!list)
	{
		return;
	}

	std::vector<const RuleDiplomacyAction*> actions;
	actions.reserve(list->size());

	for (const auto& type : *list)
	{
		auto* action = mod->getDiplomacyAction(type);
		if (!action)
		{
			continue;
		}
		if (action->getFactionRules() != _faction->getRules())
		{
			continue;
		}
		if (!isActionAvailable(*action))
		{
			continue;
		}

		actions.push_back(action);
	}

	std::stable_sort(actions.begin(), actions.end(),
		[](const RuleDiplomacyAction* a, const RuleDiplomacyAction* b)
		{
			if (a == b)
			{
				return false;
			}
			const bool aExit = a && a->getEffects().specialNavigation == RuleDiplomacyAction::NAV_EXIT;
			const bool bExit = b && b->getEffects().specialNavigation == RuleDiplomacyAction::NAV_EXIT;
			if (aExit != bExit)
			{
				return !aExit; // EXIT always last
			}
			const int ao = a ? a->getListOrder() : 0;
			const int bo = b ? b->getListOrder() : 0;
			if (ao != bo)
			{
				return ao < bo;
			}
			// keep ruleset order for ties
			return false;
		});

	_visibleActions = std::move(actions);
	for (const auto* action : _visibleActions)
	{
		if (!action)
		{
			continue;
		}
		const std::string rowText = tr(action->getText());
		_lstActions->addRow(1, const_cast<char*>(rowText.c_str()));
	}

	Log(LOG_DEBUG) << "DiplomacyDialogueState: visible actions for " << myFactionName << ": " << _visibleActions.size();
}

void DiplomacyDialogueState::applyAction(const RuleDiplomacyAction& action, Action* triggerAction)
{
	SavedGame* save = _game->getSavedGame();
	auto* mod = _game->getMod();
	auto* mind = _game->getMasterMind();
	if (!save || !mod || !mind || !_faction)
	{
		return;
	}

	Log(LOG_DEBUG) << "DiplomacyDialogueState: applying action " << action.getType();

	const auto& eff = action.getEffects();
	const auto& cond = action.getConditions();

	if (!eff.responseText.empty())
	{
		_txtReply->setText(tr(eff.responseText));
	}

	for (const auto& [factionRules, delta] : eff.reputationChanges)
	{
		auto* f = findFactionByRules(*save, factionRules);
		if (!f)
		{
			Log(LOG_DEBUG) << "DiplomacyDialogueState: reputationChanges skipped, faction not found";
			continue;
		}
		f->updateReputationScore(delta);
		mind->updateReputationLvl(f, false);
	}

	if (!Mod::isEmptyRuleName(action.getEventScriptName()))
	{
		mind->eventScriptProcessor({action.getEventScriptName()}, OTHER_SCRIPT);
	}

	if (eff.funds)
	{
		save->setFunds(save->getFunds() + *eff.funds);
	}

	if (eff.loyalty)
	{
		save->setLoyalty(save->getLoyalty() + *eff.loyalty);
	}

	if (!Mod::isEmptyRuleName(action.getSpawnMissionName()))
	{
		GeoscapeState* gs = _game->getGeoscapeState();
		Base* firstBase = (save->getBases() && !save->getBases()->empty()) ? save->getBases()->front() : nullptr;
		if (gs && gs->getGlobe() && firstBase)
		{
			mind->spawnAlienMission(action.getSpawnMissionName(), *gs->getGlobe(), firstBase);
		}
		else
		{
			Log(LOG_DEBUG) << "DiplomacyDialogueState: spawnMission skipped (missing GeoscapeState/globe/base)";
		}
	}

	for (const auto& [treatyId, enabled] : eff.changeTreaty)
	{
		_faction->setTreaty(treatyId, enabled);
	}

	if (cond.timerCooldown && *cond.timerCooldown > 0)
	{
		_faction->setActionCooldown(action.getType(), *cond.timerCooldown);
	}

	switch (eff.specialNavigation)
	{
	case RuleDiplomacyAction::NAV_EXIT:
		_exitMode = true;
		_exitModeIgnoreEvent = triggerAction ? triggerAction->getDetails() : nullptr;
		_lstActions->setVisible(false);
		_lstActions->setSelectable(false);
		_txtCloseHint->setText(tr("STR_DIPLOMACY_TALK_CLOSE"));
		_txtCloseHint->setVisible(true);
		_txtTooltip->setText("");
		return;
	case RuleDiplomacyAction::NAV_HIRE_SOLDIERS:
		if (_base != 0)
		{
			_game->pushState(new DiplomacyHirePersonnelState(_base, _faction));
		}
		else if (_game->getSavedGame()->getBases()->size() == 1)
		{
			_game->pushState(new DiplomacyHirePersonnelState(_game->getSavedGame()->getBases()->front(), _faction));
		}
		else
		{
			_game->pushState(new DiplomacyChooseBaseState(_faction, OPERATION_HIRING));
		}
		break;
	case RuleDiplomacyAction::NAV_NONE:
	default:
		break;
	}

	rebuildActionsList();
}

void DiplomacyDialogueState::closeIfExitMode(Action* action)
{
	if (_exitMode)
	{
		if (_exitModeIgnoreEvent && action && action->getDetails() == _exitModeIgnoreEvent)
		{
			// Ignore the same click/keypress that switched us into exit mode.
			_exitModeIgnoreEvent = nullptr;
			return;
		}
		_game->popState();
	}
}

void DiplomacyDialogueState::lstActionsMouseOver(Action*)
{
	if (_exitMode)
	{
		return;
	}
	unsigned int row = _lstActions->getSelectedRow();
	if (row < _visibleActions.size())
	{
		const auto* action = _visibleActions.at(row);
		_txtTooltip->setText(tr(action->getTooltip()));
	}
	else
	{
		_txtTooltip->setText("");
	}
}

void DiplomacyDialogueState::lstActionsMouseOut(Action*)
{
	if (_exitMode)
	{
		return;
	}
	_txtTooltip->setText("");
}

void DiplomacyDialogueState::lstActionsClick(Action* action)
{
	if (_exitMode)
	{
		return;
	}

	unsigned int row = _lstActions->getSelectedRow();
	if (row < _visibleActions.size())
	{
		applyAction(*_visibleActions.at(row), action);
	}
}

}

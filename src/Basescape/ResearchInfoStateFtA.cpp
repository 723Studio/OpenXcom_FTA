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
#include "ResearchInfoStateFtA.h"
#include "ResearchAllocateScientistsState.h"
#include "../Engine/Action.h"
#include "../Engine/Game.h"
#include "../Engine/LocalizedText.h"
#include "../Engine/Options.h"
#include "../Engine/RNG.h"
#include "../Interface/Text.h"
#include "../Interface/TextButton.h"
#include "../Interface/TextList.h"
#include "../Interface/Window.h"
#include "../Mod/Mod.h"
#include "../Mod/RuleResearch.h"
#include "../Savegame/Base.h"
#include "../Savegame/ItemContainer.h"
#include "../Savegame/ResearchProject.h"
#include "../Savegame/SavedGame.h"
#include "../Savegame/Soldier.h"

namespace OpenXcom
{

/**
 * Initializes all the elements in the ResearchProject screen.
 * @param game Pointer to the core game.
 * @param base Pointer to the base to get info from.
 * @param rule A RuleResearch which will be used to create a new ResearchProject
 */
ResearchInfoStateFtA::ResearchInfoStateFtA(Base *base, const RuleResearch *rule) : _base(base), _rule(rule)
{
	_newProject = true;
	buildUi();

}

/**
 * Initializes all the elements in the ResearchProject screen.
 * @param game Pointer to the core game.
 * @param base Pointer to the base to get info from.
 * @param project A ResearchProject to modify
 */
ResearchInfoStateFtA::ResearchInfoStateFtA(Base *base, ResearchProject *project) : _base(base), _project(project)
{
	_newProject = false;
	buildUi();
}



/**
 * Builds dialog.
 */
void ResearchInfoStateFtA::buildUi()
{
	_screen = false;

	_window = new Window(this, 320, 156, 0, 22);
	_txtTitle = new Text(302, 17, 9, 31);

	_txtAvailableScientist = new Text(166, 9, 9, 52);
	_txtAvailableSpace = new Text(166, 9, 9, 62);
	_txtGrade = new Text(32, 9, 125, 80);
	_txtStat1 = new Text(18, 9, 157, 80);
	_txtStat2 = new Text(18, 9, 157, 80);
	_txtStat3 = new Text(18, 9, 157, 80);
	_txtStat4 = new Text(18, 9, 157, 80);
	_txtStat5 = new Text(18, 9, 157, 80);
	_txtStat6 = new Text(18, 9, 157, 80);
	_txtInsight = new Text(18, 9, 157, 80); //x offset later

	_btnOk = new TextButton(148, 16, 165, 153);
	_btnCancel = new TextButton(148, 16, 9, 153);
	_btnAllocate = new TextButton(110, 16, 9, 72);
	_btnAbandon = new TextButton(115, 16, 188, 56);
	_lstScientists = new TextList(301, 63, 9, 89);

	// Set palette
	setInterface("allocateResearch");

	add(_window, "window", "allocateResearch");
	add(_btnOk, "button2", "allocateResearch");
	add(_btnCancel, "button2", "allocateResearch");
	add(_btnAllocate, "button2", "allocateResearch");
	add(_btnAbandon, "button3", "allocateResearch");
	add(_txtTitle, "text", "allocateResearch");
	add(_txtAvailableScientist, "text", "allocateResearch");
	add(_txtAvailableSpace, "text", "allocateResearch");
	add(_txtGrade, "text", "allocateResearch");
	add(_txtStat1, "text", "allocateResearch");
	add(_txtStat2, "text", "allocateResearch");
	add(_txtStat3, "text", "allocateResearch");
	add(_txtStat4, "text", "allocateResearch");
	add(_txtStat5, "text", "allocateResearch");
	add(_txtStat6, "text", "allocateResearch");
	add(_txtInsight, "text", "allocateResearch");
	add(_lstScientists, "list", "allocateResearch");

	centerAllSurfaces();

	// Set up objects
	setWindowBackground(_window, "allocateResearch");

	_txtTitle->setBig();
	_txtTitle->setAlign(ALIGN_CENTER);
	_txtTitle->setText(tr(getResearchRules()->getName()));

	_btnAllocate->setText(tr("STR_ALLOCATE_SCIENTISTS"));
	_btnAllocate->onMouseClick((ActionHandler)&ResearchInfoStateFtA::btnAllocateClick);

	_btnOk->onMouseClick((ActionHandler)&ResearchInfoStateFtA::btnOkClick);
	_btnOk->onKeyboardPress((ActionHandler)&ResearchInfoStateFtA::btnOkClick, Options::keyOk);

	if (_newProject)
	{
		_btnOk->setText(tr("STR_START_PROJECT"));
		_btnAbandon->setVisible(false);
	}
	else
	{
		_btnOk->setText(tr("STR_OK"));
	}

	_btnCancel->setText(tr("STR_CANCEL_UC"));
	_btnCancel->onMouseClick((ActionHandler)&ResearchInfoStateFtA::btnCancelClick);
	_btnCancel->onKeyboardPress((ActionHandler)&ResearchInfoStateFtA::btnCancelClick, Options::keyCancel);

	_btnAbandon->setText(tr("STR_ABANDON_PROJECT"));
	_btnAbandon->onMouseClick((ActionHandler)&ResearchInfoStateFtA::btnAbandonClick);

	_txtGrade->setText(tr("GRADE"));

	setAssignedScientists();

	int x = _txtStat1->getX();
	const unsigned int offset = 18;
	int stat = getStatString(0).first;
	if (stat > 0)
	{
		_txtStat1->setText(tr(getStatString(0).second));
		_txtStat1->setX(x);
		x += offset;
	}

	stat = getStatString(1).first;
	if (stat > 0)
	{
		_txtStat2->setText(tr(getStatString(1).second));
		_txtStat2->setX(x);
		x += offset;
	}

	stat = getStatString(2).first;
	if (stat > 0)
	{
		_txtStat3->setText(tr(getStatString(2).second));
		_txtStat3->setX(x);
		x += offset;
	}

	stat = getStatString(3).first;
	if (stat > 0)
	{
		_txtStat4->setText(tr(getStatString(3).second));
		_txtStat4->setX(x);
		x += offset;
	}

	stat = getStatString(4).first;
	if (stat > 0)
	{
		_txtStat5->setText(tr(getStatString(4).second));
		_txtStat5->setX(x);
		x += offset;
	}

	stat = getStatString(5).first;
	if (stat > 0)
	{
		_txtStat6->setText(tr(getStatString(5).second));
		_txtStat6->setX(x);
		x += offset;
	}

	_txtInsight->setText(tr(UnitStats::getStatString(&UnitStats::insight, UnitStats::STATSTR_SHORT)));
	_txtInsight->setX(x);

	_lstScientists->setColumns(9, 116, 32, 18, 18, 18, 18, 18, 18, 18);
	_lstScientists->setAlign(ALIGN_CENTER);
	_lstScientists->setAlign(ALIGN_LEFT, 0);
	_lstScientists->setBackground(_window);
	//_lstScientists->setMargin(2);
	_lstScientists->setWordWrap(true);

	if (_newProject)
	{
		// mark new as normal
		if (_game->getSavedGame()->isResearchRuleStatusNew(getResearchRules()->getName()))
		{
			_game->getSavedGame()->setResearchRuleStatus(getResearchRules()->getName(), RuleResearch::RESEARCH_STATUS_NORMAL);
		}
	}
}

/**
 * Frees up memory that's not automatically cleaned on exit
 */
ResearchInfoStateFtA::~ResearchInfoStateFtA()
{
}

/**
 * Updates the research list
 * after going to other screens.
 */
void ResearchInfoStateFtA::init()
{
	State::init();
	fillScientistsList(0);
}


const RuleResearch* ResearchInfoStateFtA::getResearchRules()
{
	if (_newProject)
	{
		return _rule;
	}
	else
	{
		return _project->getRules();
	}
}

int ResearchInfoStateFtA::GetStatValue(Soldier &s, const std::string &desc)
{
	
	const UnitStats *sStats = s.getCurrentStats();
	if (desc == UnitStats::getStatString(&UnitStats::physics, UnitStats::STATSTR_SHORT))
		return sStats->physics;
	if (desc == UnitStats::getStatString(&UnitStats::chemistry, UnitStats::STATSTR_SHORT))
		return sStats->chemistry;
	if (desc == UnitStats::getStatString(&UnitStats::biology, UnitStats::STATSTR_SHORT))
		return sStats->biology;
	if (desc == UnitStats::getStatString(&UnitStats::data, UnitStats::STATSTR_SHORT))
		return sStats->data;
	if (desc == UnitStats::getStatString(&UnitStats::computers, UnitStats::STATSTR_SHORT))
		return sStats->computers;
	if (desc == UnitStats::getStatString(&UnitStats::tactics, UnitStats::STATSTR_SHORT))
		return sStats->tactics;
	if (desc == UnitStats::getStatString(&UnitStats::materials, UnitStats::STATSTR_SHORT))
		return sStats->materials;
	if (desc == UnitStats::getStatString(&UnitStats::designing, UnitStats::STATSTR_SHORT))
		return sStats->designing;
	if (desc == UnitStats::getStatString(&UnitStats::alienTech, UnitStats::STATSTR_SHORT))
		return sStats->alienTech;
	if (desc == UnitStats::getStatString(&UnitStats::psionics, UnitStats::STATSTR_SHORT))
		return sStats->psionics;
	if (desc == UnitStats::getStatString(&UnitStats::xenolinguistics, UnitStats::STATSTR_SHORT))
		return sStats->xenolinguistics;

	Log(LOG_ERROR) << "Link to undefined stat: " << desc;
	return 0;
}

void ResearchInfoStateFtA::fillScientistsList(size_t scrl)
{
	_lstScientists->clearList();
	std::vector<std::string> stats(7, "");
	for (auto s : _scientists)
	{
		std::ostringstream ss;
		ss << s->getRoleRank(ROLE_SCIENTIST);
		size_t i = 0;
		for (auto stat : _researchStats)
		{
			stats[i++] = std::to_string(GetStatValue(*s, stat.second));
		}
		stats[i] = std::to_string(s->getCurrentStats()->insight);
		_lstScientists->addRow(9, s->getName().c_str(), ss.str().c_str(), stats[0].c_str(), stats[1].c_str(), stats[2].c_str(),
							   stats[3].c_str(), stats[4].c_str(), stats[5].c_str(), stats[6].c_str());
	}
}


/**
 * Returns to the previous screen.
 * @param action Pointer to an action.
 */
void ResearchInfoStateFtA::btnOkClick(Action *)
{
	if (_newProject)
	{
		int rng = RNG::generate(50, 150);
		int randomizedCost = getResearchRules()->getCost() * rng / 100;
		if (getResearchRules()->getCost() > 0)
		{
			randomizedCost = std::max(1, randomizedCost);
		}
		_project = new ResearchProject(getResearchRules(), randomizedCost);
		_base->addResearch(_project);
		if (getResearchRules()->needItem() && getResearchRules()->destroyItem())
		{
			_base->getStorageItems()->removeItem(getResearchRules()->getName(), 1);
		}
	}

	for (auto s : _scientists)
	{
		s->clearBaseDuty();
		s->setResearchProject(_project);
	}

	_game->popState();
}

/**
 * Returns to the previous screen, removing the current project from the active
 * research list.
 * @param action Pointer to an action.
 */
void ResearchInfoStateFtA::btnCancelClick(Action *)
{
	_game->popState();
}

/**
 * Returns to the previous screen, removing the current project from the active
 * research list.
 * @param action Pointer to an action.
 */
void ResearchInfoStateFtA::btnAbandonClick(Action *)
{
	for (auto s : _scientists)
	{
		s->setResearchProject(0);
	}
	_base->removeResearch(_project);
	_game->popState();
}

/**
 * Opens Allocate Scientists state.
 * @param action Pointer to an action.
 */
void ResearchInfoStateFtA::btnAllocateClick(Action *action)
{
	_game->pushState(new ResearchAllocateScientistsState(_base, this));
}

/**
 * Updates count of assigned/free scientists and available lab space.
 */
void ResearchInfoStateFtA::setAssignedScientists()
{
	size_t freeScientists = 0;
	auto recovery = _base->getSumRecoveryPerDay();
	bool isBusy = false, isFree = false;

	for (auto s : _base->getPersonnel(ROLE_SCIENTIST))
	{
		s->getCurrentDuty(_game->getLanguage(), recovery, isBusy, isFree);
		if (s->getResearchProject() && s->getResearchProject()->getRules() == this->getResearchRules())
		{
			_scientists.insert(s);
		}
		else if (!isBusy && isFree)
		{
			if (_scientists.find(s) != _scientists.end())
			{ }
			else
				freeScientists++;
		}
	}
	_txtAvailableScientist->setText(tr("STR_SCIENTISTS_AVAILABLE_UC").arg(freeScientists));

	size_t teamSize = _scientists.size();
	for (auto s : _scientists)
	{
		if (s->getResearchProject() && s->getResearchProject()->getRules() != this->getResearchRules())
		{
			teamSize--;
		}
	}
	_workSpace = _base->getFreeLaboratories(true, _project) - teamSize;
	_txtAvailableSpace->setText(tr("STR_LABORATORY_SPACE_AVAILABLE_UC").arg(_workSpace));
}

std::pair<int, std::string> ResearchInfoStateFtA::getStatString(size_t position)
{
	auto stats = getResearchRules()->getStats();
	std::map<int, std::string> statMap;

	if (stats.physics > 0)
	{
		statMap.insert(std::make_pair(stats.physics, tr(UnitStats::getStatString(&UnitStats::physics, UnitStats::STATSTR_SHORT))));
		_researchStats.insert(std::make_pair(stats.physics, UnitStats::getStatString(&UnitStats::physics, UnitStats::STATSTR_SHORT)));
	}
	if (stats.chemistry > 0)
	{
		statMap.insert(std::make_pair(stats.chemistry, tr(UnitStats::getStatString(&UnitStats::chemistry, UnitStats::STATSTR_SHORT))));
		_researchStats.insert(std::make_pair(stats.chemistry, UnitStats::getStatString(&UnitStats::chemistry, UnitStats::STATSTR_SHORT)));
	}
	if (stats.biology > 0)
	{
		statMap.insert(std::make_pair(stats.biology, tr(UnitStats::getStatString(&UnitStats::biology, UnitStats::STATSTR_SHORT))));
		_researchStats.insert(std::make_pair(stats.biology, UnitStats::getStatString(&UnitStats::biology, UnitStats::STATSTR_SHORT)));
	}
	if (stats.data > 0)
	{
		statMap.insert(std::make_pair(stats.data, tr(UnitStats::getStatString(&UnitStats::data, UnitStats::STATSTR_SHORT))));
		_researchStats.insert(std::make_pair(stats.data, UnitStats::getStatString(&UnitStats::data, UnitStats::STATSTR_SHORT)));
	}
	if (stats.computers > 0)
	{
		statMap.insert(std::make_pair(stats.computers, tr(UnitStats::getStatString(&UnitStats::computers, UnitStats::STATSTR_SHORT))));
		_researchStats.insert(std::make_pair(stats.computers, UnitStats::getStatString(&UnitStats::computers, UnitStats::STATSTR_SHORT)));
	}
	if (stats.tactics > 0)
	{
		statMap.insert(std::make_pair(stats.tactics, tr(UnitStats::getStatString(&UnitStats::tactics, UnitStats::STATSTR_SHORT))));
		_researchStats.insert(std::make_pair(stats.tactics, UnitStats::getStatString(&UnitStats::tactics, UnitStats::STATSTR_SHORT)));
	}
	if (stats.materials > 0)
	{
		statMap.insert(std::make_pair(stats.materials, tr(UnitStats::getStatString(&UnitStats::materials, UnitStats::STATSTR_SHORT))));
		_researchStats.insert(std::make_pair(stats.materials, UnitStats::getStatString(&UnitStats::materials, UnitStats::STATSTR_SHORT)));
	}
	if (stats.designing > 0)
	{
		statMap.insert(std::make_pair(stats.designing, tr(UnitStats::getStatString(&UnitStats::designing, UnitStats::STATSTR_SHORT))));
		_researchStats.insert(std::make_pair(stats.designing, UnitStats::getStatString(&UnitStats::designing, UnitStats::STATSTR_SHORT)));
	}
	if (stats.alienTech > 0)
	{
		statMap.insert(std::make_pair(stats.alienTech, tr(UnitStats::getStatString(&UnitStats::alienTech, UnitStats::STATSTR_SHORT))));
		_researchStats.insert(std::make_pair(stats.alienTech, UnitStats::getStatString(&UnitStats::alienTech, UnitStats::STATSTR_SHORT)));
	}
	if (stats.psionics > 0)
	{
		statMap.insert(std::make_pair(stats.psionics, tr(UnitStats::getStatString(&UnitStats::psionics, UnitStats::STATSTR_SHORT))));
		_researchStats.insert(std::make_pair(stats.psionics, UnitStats::getStatString(&UnitStats::psionics, UnitStats::STATSTR_SHORT)));
	}
	if (stats.xenolinguistics > 0)
	{
		statMap.insert(std::make_pair(stats.xenolinguistics, tr(UnitStats::getStatString(&UnitStats::xenolinguistics, UnitStats::STATSTR_SHORT))));
		_researchStats.insert(std::make_pair(stats.xenolinguistics, UnitStats::getStatString(&UnitStats::xenolinguistics, UnitStats::STATSTR_SHORT)));
	}

	size_t pos = 0;
	std::pair<int, std::string> result;
	for (auto it = statMap.rbegin(); it != statMap.rend(); ++it)
	{
		if (pos == position)
		{
			result = std::make_pair((*it).first, (*it).second);
			break;
		}
		pos++;
	}

	return result;
}

}

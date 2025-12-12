#include "SoldierSortUtil.h"
#include "../Mod/RuleSoldier.h"

#define GET_ATTRIB_STAT_FN(attrib) \
	int OpenXcom::attrib##Stat(const Game *game, const Soldier *s) { return s->getStatsWithAllBonuses()->attrib; }
GET_ATTRIB_STAT_FN(tu)
GET_ATTRIB_STAT_FN(stamina)
GET_ATTRIB_STAT_FN(health)
GET_ATTRIB_STAT_FN(bravery)
GET_ATTRIB_STAT_FN(reactions)
GET_ATTRIB_STAT_FN(firing)
GET_ATTRIB_STAT_FN(throwing)
GET_ATTRIB_STAT_FN(strength)
int OpenXcom::manaStat(const Game* game, const Soldier* s)
{
	// don't reveal mana before it would otherwise be known
	if (game->getSavedGame()->isManaUnlocked(game->getMod()))
	{
		return s->getStatsWithAllBonuses()->mana;
	}
	return 0;
}
int OpenXcom::psiStrengthStat(const Game *game, const Soldier *s)
{
	// don't reveal psi strength before it would otherwise be known
	if (s->getCurrentStats()->psiSkill > 0
		|| (Options::psiStrengthEval
		&& game->getSavedGame()->isResearched(game->getMod()->getPsiRequirements())))
	{
		return s->getStatsWithAllBonuses()->psiStrength;
	}
	return 0;
}
int OpenXcom::psiSkillStat(const Game *game, const Soldier *s)
{
	// when Options::anytimePsiTraining is turned on, psiSkill can actually have a negative value
	if (s->getCurrentStats()->psiSkill > 0)
	{
		return s->getStatsWithAllBonuses()->psiSkill;
	}
	return 0;
}
GET_ATTRIB_STAT_FN(melee)
GET_ATTRIB_STAT_FN(maneuvering)
GET_ATTRIB_STAT_FN(missiles)
GET_ATTRIB_STAT_FN(dogfight)
GET_ATTRIB_STAT_FN(tracking)
GET_ATTRIB_STAT_FN(cooperation)
GET_ATTRIB_STAT_FN(beams)
GET_ATTRIB_STAT_FN(synaptic)
GET_ATTRIB_STAT_FN(gravity)
GET_ATTRIB_STAT_FN(physics)
GET_ATTRIB_STAT_FN(chemistry)
GET_ATTRIB_STAT_FN(biology)
GET_ATTRIB_STAT_FN(insight)
GET_ATTRIB_STAT_FN(data)
GET_ATTRIB_STAT_FN(computers)
GET_ATTRIB_STAT_FN(tactics)
GET_ATTRIB_STAT_FN(materials)
GET_ATTRIB_STAT_FN(designing)
GET_ATTRIB_STAT_FN(alienTech)
GET_ATTRIB_STAT_FN(psionics)
GET_ATTRIB_STAT_FN(xenolinguistics)
GET_ATTRIB_STAT_FN(weaponry)
GET_ATTRIB_STAT_FN(explosives)
GET_ATTRIB_STAT_FN(efficiency)
GET_ATTRIB_STAT_FN(microelectronics)
GET_ATTRIB_STAT_FN(metallurgy)
GET_ATTRIB_STAT_FN(processing)
GET_ATTRIB_STAT_FN(hacking)
GET_ATTRIB_STAT_FN(robotics)
GET_ATTRIB_STAT_FN(diligence)
GET_ATTRIB_STAT_FN(reverseEngineering)
GET_ATTRIB_STAT_FN(stealth)
GET_ATTRIB_STAT_FN(perception)
GET_ATTRIB_STAT_FN(charisma)
GET_ATTRIB_STAT_FN(investigation)
GET_ATTRIB_STAT_FN(deception)
GET_ATTRIB_STAT_FN(interrogation)
#undef GET_ATTRIB_STAT_FN

#define GET_ATTRIB_STAT_FN(attrib) \
	int OpenXcom::attrib##StatBase(const Game *game, const Soldier *s) { return s->getCurrentStats()->attrib; }
GET_ATTRIB_STAT_FN(tu)
GET_ATTRIB_STAT_FN(stamina)
GET_ATTRIB_STAT_FN(health)
GET_ATTRIB_STAT_FN(bravery)
GET_ATTRIB_STAT_FN(reactions)
GET_ATTRIB_STAT_FN(firing)
GET_ATTRIB_STAT_FN(throwing)
GET_ATTRIB_STAT_FN(strength)
int OpenXcom::manaStatBase(const Game* game, const Soldier* s)
{
	// don't reveal mana before it would otherwise be known
	if (game->getSavedGame()->isManaUnlocked(game->getMod()))
	{
		return s->getCurrentStats()->mana;
	}
	return 0;
}
int OpenXcom::psiStrengthStatBase(const Game *game, const Soldier *s)
{
	// don't reveal psi strength before it would otherwise be known
	if (s->getCurrentStats()->psiSkill > 0
		|| (Options::psiStrengthEval
		&& game->getSavedGame()->isResearched(game->getMod()->getPsiRequirements())))
	{
		return s->getCurrentStats()->psiStrength;
	}
	return 0;
}
int OpenXcom::psiSkillStatBase(const Game *game, const Soldier *s)
{
	// when Options::anytimePsiTraining is turned on, psiSkill can actually have a negative value
	if (s->getCurrentStats()->psiSkill > 0)
	{
		return s->getCurrentStats()->psiSkill;
	}
	return 0;
}
GET_ATTRIB_STAT_FN(melee)
GET_ATTRIB_STAT_FN(maneuvering)
GET_ATTRIB_STAT_FN(missiles)
GET_ATTRIB_STAT_FN(dogfight)
GET_ATTRIB_STAT_FN(tracking)
GET_ATTRIB_STAT_FN(cooperation)
GET_ATTRIB_STAT_FN(beams)
GET_ATTRIB_STAT_FN(synaptic)
GET_ATTRIB_STAT_FN(gravity)
GET_ATTRIB_STAT_FN(physics)
GET_ATTRIB_STAT_FN(chemistry)
GET_ATTRIB_STAT_FN(biology)
GET_ATTRIB_STAT_FN(insight)
GET_ATTRIB_STAT_FN(data)
GET_ATTRIB_STAT_FN(computers)
GET_ATTRIB_STAT_FN(tactics)
GET_ATTRIB_STAT_FN(materials)
GET_ATTRIB_STAT_FN(designing)
GET_ATTRIB_STAT_FN(alienTech)
GET_ATTRIB_STAT_FN(psionics)
GET_ATTRIB_STAT_FN(xenolinguistics)
GET_ATTRIB_STAT_FN(weaponry)
GET_ATTRIB_STAT_FN(explosives)
GET_ATTRIB_STAT_FN(efficiency)
GET_ATTRIB_STAT_FN(microelectronics)
GET_ATTRIB_STAT_FN(metallurgy)
GET_ATTRIB_STAT_FN(processing)
GET_ATTRIB_STAT_FN(hacking)
GET_ATTRIB_STAT_FN(robotics)
GET_ATTRIB_STAT_FN(diligence)
GET_ATTRIB_STAT_FN(reverseEngineering)
GET_ATTRIB_STAT_FN(stealth)
GET_ATTRIB_STAT_FN(perception)
GET_ATTRIB_STAT_FN(charisma)
GET_ATTRIB_STAT_FN(investigation)
GET_ATTRIB_STAT_FN(deception)
GET_ATTRIB_STAT_FN(interrogation)
#undef GET_ATTRIB_STAT_FN

#define GET_ATTRIB_STAT_FN(attrib) \
	int OpenXcom::attrib##StatPlus(const Game *game, const Soldier *s) { return s->getStatsWithSoldierBonusesOnly()->attrib; }
GET_ATTRIB_STAT_FN(tu)
GET_ATTRIB_STAT_FN(stamina)
GET_ATTRIB_STAT_FN(health)
GET_ATTRIB_STAT_FN(bravery)
GET_ATTRIB_STAT_FN(reactions)
GET_ATTRIB_STAT_FN(firing)
GET_ATTRIB_STAT_FN(throwing)
GET_ATTRIB_STAT_FN(strength)
int OpenXcom::manaStatPlus(const Game* game, const Soldier* s)
{
	// don't reveal mana before it would otherwise be known
	if (game->getSavedGame()->isManaUnlocked(game->getMod()))
	{
		return s->getStatsWithSoldierBonusesOnly()->mana;
	}
	return 0;
}
int OpenXcom::psiStrengthStatPlus(const Game *game, const Soldier *s)
{
	// don't reveal psi strength before it would otherwise be known
	if (s->getCurrentStats()->psiSkill > 0
		|| (Options::psiStrengthEval
		&& game->getSavedGame()->isResearched(game->getMod()->getPsiRequirements())))
	{
		return s->getStatsWithSoldierBonusesOnly()->psiStrength;
	}
	return 0;
}
int OpenXcom::psiSkillStatPlus(const Game *game, const Soldier *s)
{
	// when Options::anytimePsiTraining is turned on, psiSkill can actually have a negative value
	if (s->getCurrentStats()->psiSkill > 0)
	{
		return s->getStatsWithSoldierBonusesOnly()->psiSkill;
	}
	return 0;
}
GET_ATTRIB_STAT_FN(melee)
GET_ATTRIB_STAT_FN(maneuvering)
GET_ATTRIB_STAT_FN(missiles)
GET_ATTRIB_STAT_FN(dogfight)
GET_ATTRIB_STAT_FN(tracking)
GET_ATTRIB_STAT_FN(cooperation)
GET_ATTRIB_STAT_FN(beams)
GET_ATTRIB_STAT_FN(synaptic)
GET_ATTRIB_STAT_FN(gravity)
GET_ATTRIB_STAT_FN(physics)
GET_ATTRIB_STAT_FN(chemistry)
GET_ATTRIB_STAT_FN(biology)
GET_ATTRIB_STAT_FN(insight)
GET_ATTRIB_STAT_FN(data)
GET_ATTRIB_STAT_FN(computers)
GET_ATTRIB_STAT_FN(tactics)
GET_ATTRIB_STAT_FN(materials)
GET_ATTRIB_STAT_FN(designing)
GET_ATTRIB_STAT_FN(alienTech)
GET_ATTRIB_STAT_FN(psionics)
GET_ATTRIB_STAT_FN(xenolinguistics)
GET_ATTRIB_STAT_FN(weaponry)
GET_ATTRIB_STAT_FN(explosives)
GET_ATTRIB_STAT_FN(efficiency)
GET_ATTRIB_STAT_FN(microelectronics)
GET_ATTRIB_STAT_FN(metallurgy)
GET_ATTRIB_STAT_FN(processing)
GET_ATTRIB_STAT_FN(hacking)
GET_ATTRIB_STAT_FN(robotics)
GET_ATTRIB_STAT_FN(diligence)
GET_ATTRIB_STAT_FN(reverseEngineering)
GET_ATTRIB_STAT_FN(stealth)
GET_ATTRIB_STAT_FN(perception)
GET_ATTRIB_STAT_FN(charisma)
GET_ATTRIB_STAT_FN(investigation)
GET_ATTRIB_STAT_FN(deception)
GET_ATTRIB_STAT_FN(interrogation)
#undef GET_ATTRIB_STAT_FN


#define GET_SOLDIER_STAT_FN(attrib, camelCaseAttrib) \
	int OpenXcom::attrib##Stat(const Game *game, const Soldier *s) { return s->get##camelCaseAttrib(); }
GET_SOLDIER_STAT_FN(id, Id)
int OpenXcom::nameStat(const Game *game, const Soldier *s)
{
	return 0;
}
int OpenXcom::craftIdStat(const Game *game, const Soldier *s)
{
	if (s->getCraft())
	{
		return s->getCraft()->getId();
	}
	return 0;
}
int OpenXcom::typeStat(const Game *game, const Soldier *s)
{
	return s->getRules()->getListOrder();
}
GET_SOLDIER_STAT_FN(rank, Rank)
int OpenXcom::roleStat(const Game *game, const Soldier *s)
{
	return static_cast<int>(s->getBestRoleRank().first);
}
int OpenXcom::roleRankStat(const Game *game, const Soldier *s)
{
	return s->getBestRoleRank().second;
}

GET_SOLDIER_STAT_FN(missions, Missions)
GET_SOLDIER_STAT_FN(kills, Kills)
//GET_SOLDIER_STAT_FN(woundRecovery, WoundRecovery)
int OpenXcom::woundRecoveryStat(const Game *game, const Soldier *s)
{
	return s->getWoundRecovery(0.0f, 0.0f);
}
GET_SOLDIER_STAT_FN(manaMissing, ManaMissing)
int OpenXcom::idleDaysStat(const Game *game, const Soldier *s)
{
	return game->getSavedGame()->getSoldierIdleDays(s);
}
#undef GET_SOLDIER_STAT_FN

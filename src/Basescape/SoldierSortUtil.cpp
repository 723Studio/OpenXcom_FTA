#include "SoldierSortUtil.h"
#include "../Mod/RuleSoldier.h"

#define GET_ATTRIB_STAT_FN(attrib) \
	int OpenXcom::attrib##Stat(Game *game, Soldier *s) { return s->getStatsWithAllBonuses()->attrib; }
GET_ATTRIB_STAT_FN(tu)
GET_ATTRIB_STAT_FN(stamina)
GET_ATTRIB_STAT_FN(health)
GET_ATTRIB_STAT_FN(bravery)
GET_ATTRIB_STAT_FN(reactions)
GET_ATTRIB_STAT_FN(firing)
GET_ATTRIB_STAT_FN(throwing)
GET_ATTRIB_STAT_FN(strength)
int OpenXcom::manaStat(Game* game, Soldier* s)
{
	// don't reveal mana before it would otherwise be known
	if (game->getSavedGame()->isManaUnlocked(game->getMod()))
	{
		return s->getStatsWithAllBonuses()->mana;
	}
	return 0;
}
int OpenXcom::psiStrengthStat(Game *game, Soldier *s)
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
int OpenXcom::psiSkillStat(Game *game, Soldier *s)
{
	// when Options::anytimePsiTraining is turned on, psiSkill can actually have a negative value
	if (s->getCurrentStats()->psiSkill > 0)
	{
		return s->getStatsWithAllBonuses()->psiSkill;
	}
	return 0;
}
GET_ATTRIB_STAT_FN(melee)
GET_ATTRIB_STAT_FN(manuvering)
GET_ATTRIB_STAT_FN(missiles)
GET_ATTRIB_STAT_FN(dogfight)
GET_ATTRIB_STAT_FN(tracking)
GET_ATTRIB_STAT_FN(tactics)
#undef GET_ATTRIB_STAT_FN

#define GET_ATTRIB_STAT_FN(attrib) \
	int OpenXcom::attrib##StatBase(Game *game, Soldier *s) { return s->getCurrentStats()->attrib; }
GET_ATTRIB_STAT_FN(tu)
GET_ATTRIB_STAT_FN(stamina)
GET_ATTRIB_STAT_FN(health)
GET_ATTRIB_STAT_FN(bravery)
GET_ATTRIB_STAT_FN(reactions)
GET_ATTRIB_STAT_FN(firing)
GET_ATTRIB_STAT_FN(throwing)
GET_ATTRIB_STAT_FN(strength)
int OpenXcom::manaStatBase(Game* game, Soldier* s)
{
	// don't reveal mana before it would otherwise be known
	if (game->getSavedGame()->isManaUnlocked(game->getMod()))
	{
		return s->getCurrentStats()->mana;
	}
	return 0;
}
int OpenXcom::psiStrengthStatBase(Game *game, Soldier *s)
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
int OpenXcom::psiSkillStatBase(Game *game, Soldier *s)
{
	// when Options::anytimePsiTraining is turned on, psiSkill can actually have a negative value
	if (s->getCurrentStats()->psiSkill > 0)
	{
		return s->getCurrentStats()->psiSkill;
	}
	return 0;
}
GET_ATTRIB_STAT_FN(melee)
GET_ATTRIB_STAT_FN(manuvering)
GET_ATTRIB_STAT_FN(missiles)
GET_ATTRIB_STAT_FN(dogfight)
GET_ATTRIB_STAT_FN(tracking)
GET_ATTRIB_STAT_FN(tactics)
#undef GET_ATTRIB_STAT_FN

#define GET_ATTRIB_STAT_FN(attrib) \
	int OpenXcom::attrib##StatPlus(Game *game, Soldier *s) { return s->getStatsWithSoldierBonusesOnly()->attrib; }
GET_ATTRIB_STAT_FN(tu)
GET_ATTRIB_STAT_FN(stamina)
GET_ATTRIB_STAT_FN(health)
GET_ATTRIB_STAT_FN(bravery)
GET_ATTRIB_STAT_FN(reactions)
GET_ATTRIB_STAT_FN(firing)
GET_ATTRIB_STAT_FN(throwing)
GET_ATTRIB_STAT_FN(strength)
int OpenXcom::manaStatPlus(Game* game, Soldier* s)
{
	// don't reveal mana before it would otherwise be known
	if (game->getSavedGame()->isManaUnlocked(game->getMod()))
	{
		return s->getStatsWithSoldierBonusesOnly()->mana;
	}
	return 0;
}
int OpenXcom::psiStrengthStatPlus(Game *game, Soldier *s)
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
int OpenXcom::psiSkillStatPlus(Game *game, Soldier *s)
{
	// when Options::anytimePsiTraining is turned on, psiSkill can actually have a negative value
	if (s->getCurrentStats()->psiSkill > 0)
	{
		return s->getStatsWithSoldierBonusesOnly()->psiSkill;
	}
	return 0;
}
GET_ATTRIB_STAT_FN(melee)
GET_ATTRIB_STAT_FN(manuvering)
GET_ATTRIB_STAT_FN(missiles)
GET_ATTRIB_STAT_FN(dogfight)
GET_ATTRIB_STAT_FN(tracking)
GET_ATTRIB_STAT_FN(tactics)
#undef GET_ATTRIB_STAT_FN


#define GET_SOLDIER_STAT_FN(attrib, camelCaseAttrib) \
	int OpenXcom::attrib##Stat(Game *game, Soldier *s) { return s->get##camelCaseAttrib(); }
GET_SOLDIER_STAT_FN(id, Id)
int OpenXcom::nameStat(Game *game, Soldier *s)
{
	return 0;
}
int OpenXcom::typeStat(Game *game, Soldier *s)
{
	return s->getRules()->getListOrder();
}
GET_SOLDIER_STAT_FN(rank, Rank)
GET_SOLDIER_STAT_FN(missions, Missions)
GET_SOLDIER_STAT_FN(kills, Kills)
//GET_SOLDIER_STAT_FN(woundRecovery, WoundRecovery)
int OpenXcom::woundRecoveryStat(Game *game, Soldier *s)
{
	return s->getWoundRecovery(0.0f, 0.0f);
}
GET_SOLDIER_STAT_FN(manaMissing, ManaMissing)
int OpenXcom::idleDaysStat(Game *game, Soldier *s)
{
	return game->getSavedGame()->getSoldierIdleDays(s);
}
#undef GET_SOLDIER_STAT_FN

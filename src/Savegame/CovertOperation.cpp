/*
 * Copyright 2010-2019 OpenXcom Developers.
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
#include "CovertOperation.h"
#include "SerializationHelper.h"
#include "../fmath.h"
#include "../Engine/Language.h"
#include "../Engine/Game.h"
#include "../Engine/RNG.h"
#include "../Engine/Logger.h"
#include "../Geoscape/Globe.h"
#include "../Battlescape/BattlescapeGenerator.h"
#include "../Battlescape/BriefingState.h"
#include "../Battlescape/DebriefingState.h"
#include "../Savegame/SavedGame.h"
#include "../Savegame/Base.h"
#include "../Savegame/Region.h"
#include "../Savegame/ItemContainer.h"
#include "../Savegame/Soldier.h"
#include "../Savegame/DiplomacyFaction.h"
#include "../Savegame/AlienMission.h"
#include "../Savegame/SavedBattleGame.h"
#include "../Savegame/AlienBase.h"
#include "../Mod/Mod.h"
#include "../Mod/RuleCovertOperation.h"
#include "../Mod/RuleSoldier.h"
#include "../Mod/RuleDiplomacyFaction.h"
#include "../Mod/RuleAlienMission.h"
#include "../Mod/RuleRegion.h"
#include "../Mod/AlienDeployment.h"
#include "../Mod/AlienRace.h"
#include "../Mod/Unit.h"
#include "../FTA/MasterMind.h"

namespace OpenXcom
{
CovertOperation::CovertOperation(const RuleCovertOperation* rule, const Mod* mod, Base* base, int cost, int chances) :
	_rule(rule), _mod(mod), _base(base), _spent(0), _cost(cost), _successChance(chances),
	_results(0), _inBattlescape(false), _hasBattlescapeResolve(false), _over(false), _hasPsi(false), _progressEventSpawned(false)
{
	_items = new ItemContainer();
	if (base != 0)
	{
		setBase(base);
	}
}

CovertOperation::~CovertOperation()
{
	delete _items;
}

/**
* Loads the event from YAML.
* @param node The YAML node containing the data.
*/
void CovertOperation::load(const YAML::YamlNodeReader& reader)
{
	reader.tryRead("spent", _spent);
	reader.tryRead("cost", _cost);
	reader.tryRead("successChance", _successChance);
	reader.tryRead("inBattlescape", _inBattlescape);
	reader.tryRead("hasBattlescapeResolve", _hasBattlescapeResolve);
	reader.tryRead("hasPsi", _hasPsi);
	reader.tryRead("over", _over);
	reader.tryRead("progressEventSpawned", _progressEventSpawned);
	_items->load(reader["items"], _mod);
}

/**
 	* Saves the Operation to YAML.
 	* @return YAML node.
 	*/
void CovertOperation::save(YAML::YamlNodeWriter writer) const
{
	writer.setAsMap();
	writer.write("name", getRules()->getName());
	writer.write("spent", _spent);
	writer.write("cost", _cost);
	writer.write("successChance", _successChance);
	writer.write("hasPsi", _hasPsi);
	writer.write("inBattlescape", _inBattlescape);
	writer.write("hasBattlescapeResolve", _hasBattlescapeResolve);
	writer.write("progressEventSpawned", _progressEventSpawned);
	if (_over)
	{
		writer.write("over", _over);
	}
	_items->save(writer["items"]);
}


void CovertOperation::setBase(Base* base)
{
	_base = base;
}

/**
 * Loads a Covert Operation name from rules.
 * @return operation name.
 */
std::string CovertOperation::getOperationName()
{
	return _rule->getName();
}


/**
 * Return a vector of pointers to a Soldier realizations that assigned to this operation.
 * @return a vector of pointers to a Soldier class.
 */
std::vector<Soldier*> CovertOperation::getSoldiers()
{
	std::vector<Soldier*> soldiers;
	for (auto& soldier : *_base->getSoldiers())
	{
		if (soldier->getCovertOperation() != 0 && soldier->getCovertOperation()->getOperationName() == this->getOperationName())
		{
			soldiers.push_back(soldier);
		}
	}
	return soldiers;
}

/**
 * Return a string describing CovertOperation success odds.
 * @par chance is input to calculate chance name.
 * @return a string describing CovertOperation success odds.
 */
std::string CovertOperation::getOddsName()
{
	if (_successChance > 100)
		return ("STR_GREAT");
	else if (_successChance > 70)
		return ("STR_GOOD");
	else if (_successChance > 50)
		return ("STR_AVERAGE");
	else if (_successChance > 25)
		return ("STR_POOR");
	else if (_successChance > 0)
		return ("STR_VERY_LOW");
	else
		return ("STR_NONE");

}

/**
 * Return a string describing approximate time before operation results.
 * @return a string describing time left.
 */
std::string CovertOperation::getTimeLeftName()
{
	int time = _cost - _spent;
	if (time > 45 * 24)
		return ("STR_SEVERAL_MONTHS");
	else if (time > 20 * 24)
		return ("STR_A_MONTH");
	else if (time > 10 * 24)
		return ("STR_SEVERAL_WEEKS");
	else if (time > 6 * 24)
		return ("STR_WEEK");
	else
		return ("STR_SEVERAL_DAYS");
}

/**
* Handle Covert Operation daily logic.
* @param engine game engine.
* @param globe
*/
bool CovertOperation::think(Game& engine, const Globe& globe)
{
	const Mod& mod = *engine.getMod();
	SavedGame& save = *engine.getSavedGame();
	if (_over)
	{
		finishOperation(); //case we should over it ASAP
		return false;
	}

	// are we there yet?
	if (_spent < _cost)
	{
		++_spent;
		//should we spawn ongoing event?
		std::string progressEvent = _rule->chooseProgressEvent();
		if (!progressEvent.empty())
		{
			if (!_rule->getRepeatProgressEvent() && _progressEventSpawned)
				return false;
			else
			{
				if (RNG::percent(_rule->getProgressEventChance()))
				{
					_progressEventSpawned = save.spawnEvent(mod.getEvent(progressEvent));
				}
			}
		}
		return false;
	}

	// ok, the time has come to resolve covert operation
	GameDifficulty diff = save.getDifficulty(); //first, we understand values based on game difficulty
	int critFailCoef = 45;
	int woundOdds = 20;
	int deathOdds = 10;
	switch (diff)
	{
	case OpenXcom::DIFF_BEGINNER:
		critFailCoef += 5;
		woundOdds -= 5;
		deathOdds -= 8;
		break;
	case OpenXcom::DIFF_EXPERIENCED:
		critFailCoef += 2;
		woundOdds -= 2;
		deathOdds -= 4;
		break;
	case OpenXcom::DIFF_VETERAN:
		break;
	case OpenXcom::DIFF_GENIUS:
		critFailCoef -= 2;
		woundOdds += 5;
		deathOdds += 2;
		break;
	case OpenXcom::DIFF_SUPERHUMAN:
		critFailCoef -= 5;
		woundOdds += 10;
		deathOdds += 10;
		break;
	}
	//now, we roll for operation results and init some vars
	int roll = RNG::generate(0, 99);
	_finishedResult = _successChance > roll;
	bool criticalFail = roll > (_successChance + critFailCoef);
	int score = 0;
	int loyalty = 0;
	int funds = 0;
	std::string eventName;
	std::vector<std::string> researchList;
	std::map<const RuleItem*, int> itemsToAdd;
	std::string missionName;
	std::string deploymentName;
	std::map<std::string, int> reputationScore;

	_results = new CovertOperationResults(this->getOperationName(), _finishedResult, "0"); //#FINNIKTODO date
	if (_finishedResult) //define success operation results
	{
		score = _rule->getSuccessScore();
		loyalty = _rule->getSuccessLoyalty();
		funds = _rule->getSuccessFunds();
		researchList = _rule->getSuccessResearchList();
		eventName = _rule->getSuccessEvent();
		reputationScore = _rule->getSuccessReputationScoreList();
		missionName = _rule->chooseGenSuccessMissionType();
		deploymentName = _rule->chooseGenInstantSuccessDeploymentType();
		if (!_rule->getSuccessItems().empty())
		{
			int totalWeight = 0;
			for (const auto& itemSet : _rule->getSuccessItems())
			{
				totalWeight += itemSet.first;
			}

			int runningTotal = 0;
			for (const auto& itemSet : _rule->getSuccessItems())
			{
				runningTotal += itemSet.first;
				if (runningTotal >= RNG::generate(1, totalWeight))
				{
					for (const auto& i : itemSet.second)
					{
						itemsToAdd[i.first] += i.second;
					}
					break; // break outer loop
				}
			}
		}
	}
	else //define failure operation results
	{
		score = _rule->getFailureScore();
		if (criticalFail) score = score - 300;
		loyalty = _rule->getFailureLoyalty();
		funds = _rule->getFailureFunds();
		researchList = _rule->getFailureResearchList();
		eventName = _rule->getFailureEvent();
		reputationScore = _rule->getFailureReputationScoreList();
		missionName = _rule->chooseGenFailureMissionType();
		deploymentName = _rule->chooseGenInstantTrapDeploymentType();
		if (!_rule->getFailureItems().empty())
		{
			int totalWeight = 0;
			for (const auto& itemSet : _rule->getFailureItems())
			{
				totalWeight += itemSet.first;
			}
			int runningTotal = 0;
			for (const auto& itemSet : _rule->getFailureItems())
			{
				runningTotal += itemSet.first;
				if (runningTotal >= RNG::generate(1, totalWeight))
				{
					for (const auto& i : itemSet.second)
					{
						itemsToAdd[i.first] += i.second;
					}
					break; // break outer loop
				}
			}
		}
	}
	// lets process operation results
	if (score != 0)
	{
		save.addResearchScore(score);
		_results->addScore(score);
		engine.getMasterMind()->updateLoyalty(score, XCOM_GEOSCAPE);
	}

	if (loyalty != 0)
	{
		engine.getMasterMind()->updateLoyalty(loyalty, ABSOLUTE_COEF);
	}

	if (funds != 0)
	{
		save.setFunds(save.getFunds() + funds);
		_results->addFunds(funds);
	}

	if (!itemsToAdd.empty())
	{
		for (auto& addItems : itemsToAdd)
		{
			this->getItems()->addItem(addItems.first, addItems.second);
			_results->addItem(addItems.first->getType(), addItems.second);
		}
	}

	if (!eventName.empty())
	{
		save.spawnEvent(mod.getEvent(eventName));
	}

	if (!researchList.empty())
	{
		std::vector<const RuleResearch*> possibilities;

		for (const auto& rName : researchList)
		{
			const RuleResearch* rRule = mod.getResearch(rName, true);
			if (!save.isResearched(rRule, false))
			{
				possibilities.push_back(rRule);
			}
		}

		if (!possibilities.empty())
		{
			const RuleResearch* eventResearch = possibilities.at(0);
			save.addFinishedResearch(eventResearch, &mod, _base, true);
			_researchName = eventResearch->getName();
			_results->setSpecialMessage("STR_NEW_DATA_ACQUIRED");

			if (!eventResearch->getLookup().empty())
			{
				const RuleResearch* lookupResearch = mod.getResearch(eventResearch->getLookup(), true);
				save.addFinishedResearch(lookupResearch, &mod, _base, true);
				_researchName = lookupResearch->getName();
			}
			if (auto bonus = save.selectGetOneFree(eventResearch))
			{
				save.addFinishedResearch(bonus, &mod, _base, true);
				if (!bonus->getLookup().empty())
				{
					save.addFinishedResearch(mod.getResearch(bonus->getLookup(), true), &mod, _base, true);
				}
			}
			// check and interrupt alien missions if necessary (based on unlocked research)
			for (auto am : save.getAlienMissions())
			{
				auto interruptResearchName = am->getRules().getInterruptResearch();
				if (!interruptResearchName.empty())
				{
					if (interruptResearchName == eventResearch->getName())
					{
						am->setInterrupted(true);
					}
				}
			}
		}
	}

	if (!reputationScore.empty())
	{
		for (const auto& i : reputationScore)
		{
			for (auto& j : save.getDiplomacyFactions())
			{
				std::string factionName = j->getRules()->getName();
				std::string lookingName = i.first;
				if (factionName == lookingName)
				{
					j->updateReputationScore(i.second);
					_results->addReputation(factionName, i.second);
					break;
				}
			}
		}
	}

	if (!_rule->getRequiredItemList().empty())
	{
		bool removeItems = false;

		if (_finishedResult)
		{
			removeItems = _rule->getRemoveRequiredItemsOnSuccess();
		}
		else
		{
			removeItems = _rule->getRemoveRequiredItemsOnFailure();
		}

		if (removeItems)
		{
			auto items = _rule->getRequiredItemList();
			for (auto &item : items)
			{
				_items->removeItem(item.first, item.second);
			}
		}
	}

	auto addSoldiers = _rule->getSoldiersToAdd();
	for (auto &kv : addSoldiers)
	{
		for (int i = 0; i < kv.second; ++i)
		{
			int nationality = engine.getSavedGame()->selectSoldierNationalityByLocation(&mod, kv.first, nullptr); //#FINNIKTODO more logic for defining nationality
			Soldier* soldier = mod.genSoldier(&save, kv.first, nationality);
			_base->getSoldiers()->push_back(soldier);
			soldier->setCovertOperation(this);
			soldier->setJustSaved(true);
		}
	}

	if (!missionName.empty())
	{
		// let's define variables for alien mission first
		int month = save.getMonthsPassed();
		const RuleAlienMission* missionRules = mod.getAlienMission(missionName);
		if (missionRules == 0)
		{
			throw Exception("Error processing alien mission generation for covert operation named: " + this->getOperationName() + ", mission type: " + missionName + " is not defined");
		}
		std::string targetRegion;
		std::string missionRace;
		int targetZone = missionRules->getSpawnZone();

		bool targetBase = RNG::percent(missionRules->getTargetBaseOdds());
		bool placed = false;
		bool hasBase = true;
		bool hasZone = true;
		int tries = 0;

		while (!placed || tries < 50)
		{
			if (missionRules->hasRegionWeights())
			{
				if (tries > 35) month = 0; //lets check all regions
				targetRegion = missionRules->generateRegion(month);
			}
			else //case to pick at random as alien mission rules has no region defined
			{
				targetRegion = mod.getRegionsList().at(RNG::generate(0, mod.getRegionsList().size() - 1));
			}

			RuleRegion* region = mod.getRegion(targetRegion, true);
			if (!region)
			{
				throw Exception("Error processing alien mission named: " + missionName + ", region named: " + targetRegion + " is not defined");
			}
			if ((int)(region->getMissionZones().size()) > targetZone)
			{
				hasZone = true;
			}

			if (targetBase) // we should choose region that has any xcom base
			{
				hasBase = false;
				std::string baseRegion;
				if (tries < 2)
				{
					baseRegion = save.locateRegion(this->getBase()->getLongitude(), this->getBase()->getLatitude())->getRules()->getType();
					if (baseRegion == targetRegion)
					{
						hasBase = true;
					}
				}
				else
				{
					for (std::vector<Base*>::iterator i = save.getBases()->begin(); i != save.getBases()->end(); ++i)
					{
						baseRegion = save.locateRegion((*i)->getLongitude(), (*i)->getLatitude())->getRules()->getType();
						if (baseRegion == targetRegion)
						{
							hasBase = true;
						}
					}
				}
			}
			tries++;
			if (hasBase && hasZone) // all checks passed!
			{
				placed = true;
			}
		}

		if(!placed)
		{
			Log(LOG_ERROR) << "An error occurred during the processing of the result of a covert operation:  " << this->getOperationName() << " ! Failed to choose right region for mission: " << missionName <<
				". Some alien mission rules could be ignored!";
		}

		missionRace = missionRules->generateRace(month);
		if (missionRace.empty())
		{
			if (mod.isFTAGame())
			{
				missionRace = "STR_MIB";
				Log(LOG_ERROR) << "An error occurred during the processing of the result of a covert operation:  " << this->getOperationName() << " ! In the rules of the alien mission " << missionName <<
					" no alien race has been set! As we run FTAGame race set to " << missionRace;
			}
			else
			{
				Log(LOG_ERROR) << "An error occurred during the processing of the result of a covert operation:  " << this->getOperationName() << " ! In the rules of the alien mission " << missionName <<
					" no alien race has been set, so it will be defined at random!";
				auto raceList = mod.getAlienRacesList();
				int pick = RNG::generate(0, raceList.size() - 1);
				missionRace = raceList.at(pick);
			}
		}
		if (mod.getAlienRace(missionRace) == 0)
		{
			throw Exception("Error processing alien mission named: " + missionName + ", race: " + missionRace + " is not defined");
		}

		//now we are ready to set up new alien mission
		AlienMission* mission = new AlienMission(*missionRules);
		mission->setRace(missionRace);
		mission->setId(save.getId("ALIEN_MISSIONS"));
		mission->setRegion(targetRegion, mod);
		mission->setMissionSiteZoneArea(targetZone);
		mission->start(engine, globe, 0);
		save.getAlienMissions().push_back(mission);
	}

	if (!_rule->getSpecialRule().empty())//processing of yet hardcoded fta story arc
	{
		std::string specRule = _rule->getSpecialRule();
		if (specRule == "STR_REGIONAL_HQ_DISCOVERY")
		{
			for (auto& i : *save.getAlienBases())
			{
				auto baseName = i->getDeployment()->getType();
				if (baseName == "STR_INITIAL_REGIONAL_HQ")
				{
					i->setDiscovered(true);

					_results->setSpecialMessage("STR_REGIONAL_HQ_FOUND");
				}
			}
		}
	}

	//simulating operation
	if (this->getRules()->getDanger() > 0)
	{
		backgroundSimulation(engine, _finishedResult, criticalFail, woundOdds, deathOdds);
	}

	if (!deploymentName.empty())
	{
		bool process = true;
		if (!_finishedResult) //if operation failed lets see if we get into a trap!
		{
			int trapRoll = _rule->getTrapChance();
			if (criticalFail && trapRoll > 0) trapRoll = trapRoll + 35;
			process = RNG::generate(0, 99) < trapRoll;
		}
		if (process && !_inBattlescape)
		{//oh, boy, we are going to generate battlescape to resolve our covert operation!
			AlienDeployment* deployment = mod.getDeployment(deploymentName);
			if (deployment != 0)
			{
				SavedBattleGame* bgame = new SavedBattleGame(engine.getMod(), engine.getLanguage());
				bgame->setMissionType(deploymentName);
				save.setBattleGame(bgame);
				BattlescapeGenerator bgen(&engine);
				bgen.setCovertOperation(this);
				_hasBattlescapeResolve = true;
				_inBattlescape = true;
				bgen.run();
				engine.pushState(new BriefingState());
			}
			else
			{
				throw Exception("No deployment defined for operation: " + this->getOperationName() +
					" ! It is reffering to alienDeployment named: " + deploymentName);
			}
		}
	}
	else //we do not push any battlescape for our operation or anything like that, so we can return to our base!
	{
		for (auto& item : *_items->getContents())
		{
			_base->getStorageItems()->addItem(item.first, item.second);
		}
	}

	return true;
}

/**
* Handle background simulation for Covert Operation.
* Award exp to soldiers, simulate critical fail repercussion.
* @param engine game engine.
* @param operationResult and criticalFail operation results.
* @param woundOdds and deathOdds stats from game difficulty.
*/
void CovertOperation::backgroundSimulation(Game& engine, bool operationResult, bool criticalFail, int woundOdds, int deathOdds)
{
	SavedGame& save = *engine.getSavedGame();

	int danger = this->getRules()->getDanger(); //only dangerous operations train battle stats

	//first, we calculate how much experience we can award for the operation (expRolls)
	int effCost = (int)ceil(this->getRules()->getCosts() / 15);
	int expRolls = (effCost * engine.getMod()->getCovertOpsExpFactor() / 100) + 1;
	expRolls += RNG::generate(-2, 2); //add more random
	if (save.getDifficulty() == DIFF_SUPERHUMAN)
		expRolls -= RNG::generate(0, 5);

	//processing soldiers change before returning home
	std::vector<Soldier*> soldiersToKill;
	int operationSoldierN = 0;
	std::vector<Soldier*> soldiers = getSoldiers();
	bool engagement = false;
	int avgStealth = 0;
	if (danger > 0)
	{
		for (auto s : soldiers)
		{
			avgStealth += s->getCurrentStats()->stealth;
		}

		avgStealth /= soldiers.size();
		int engChance = danger * 5 - avgStealth;
		if (!operationResult)
		{
			engChance *= 2;
		}
		engagement = RNG::percent(engChance);
	}

	for (auto& soldier : soldiers)
	{
		bool dead = false;
		int wound = 0;
		++operationSoldierN;
		UnitStats* exp = new UnitStats();

		if (danger > 0)
		{
			int damage = 0;
			int damageRolls = danger * (ceil(save.getDifficultyCoefficient() / 2) + 1);
			if (criticalFail)
			{
				damageRolls *= RNG::generate(1, 3);
			}

			for (size_t j = 0; j < damageRolls; j++)
			{
				if (RNG::generate(0, 99) < woundOdds && RNG::generate(0, 99) > soldier->getStatsWithAllBonuses()->psiSkill)
				{
					++wound;
				}
			}
			if (wound > 0)
			{
				damage = RNG::generate(wound, wound * 6);
			}
			if (damage < soldier->getCurrentStats()->health)
			{
				soldier->setWoundRecovery(damage);
				_results->addSoldierDamage(soldier->getName(), damage);
			}
			else
			{
				dead = true; //ouch, too much damage rolled!
			}

			if (!dead && criticalFail)
			{ //OMG, Finger of Death for soldier on critical failed operation!!!
				dead = RNG::generate(0, 99) < deathOdds + ceil(danger / 3);
			}
			if (dead)
			{
				//Check for divine protection
				int protection = soldier->getBestRoleRank().second - 2;
				//lets add save if we have psi. Btw, there is a place for additional perks
				if (_hasPsi)
					protection += 3;
				int requiredProtection = RNG::generate(1, 7 + save.getDifficultyCoefficient());
				if (criticalFail)
					requiredProtection += 3;
				if (requiredProtection > protection)
				{ //RIP...
					soldiersToKill.push_back(soldier);
				}
				else
				{
					dead = false;
					if (soldier->getStatsWithAllBonuses()->bravery <= 20 || RNG::percent(5))
						exp->bravery++;
				}
			}
		}

		UnitStats origStat = *soldier->getCurrentStats();
		//soldiers can improve stats based on virtual experience they take
		if (!dead && expRolls > 0)
		{
			const UnitStats caps = soldier->getRules()->getStatCaps();
			// at first, we process combat stats
			if (engagement)
			{
				//TU and Energy
				if (origStat.tu < caps.tu)
					exp->tu += RNG::generate(-3, expRolls);
				if (origStat.stamina < caps.stamina)
					exp->stamina = RNG::generate(-3, expRolls);

				//other stats would be rolled to be improved
				int statID = 0;
				int expGain = 0;
				for (size_t j = 0; j < (size_t)expRolls; j++)
				{
					statID = RNG::generate(1, 6);  //choose non-psionic combat stat
					expGain = RNG::generate(1, 4); //choose how many experience it would be
					if (expGain == 4)
						expGain = 1;
					switch (statID)
					{
					case 0:
						if (origStat.health < caps.health)
							exp->health += expGain;
						break;
					case 1:
						if (origStat.bravery < caps.bravery && !exp->bravery)
						{
							int braveryRoll = 1;
							if (wound > 0)
							{
								braveryRoll += 1;
							}
							if (RNG::generate(0, 14) > braveryRoll)
								exp->bravery += expGain;
						}
						break;
					case 2:
						if (origStat.reactions < caps.reactions)
							exp->reactions += expGain;
						break;
					case 3:
						if (origStat.firing < caps.firing)
							exp->firing += expGain;
						break;
					case 4:
						if (origStat.throwing < caps.throwing)
							exp->throwing += expGain;
						break;
					case 5:
						if (origStat.melee < caps.melee)
							exp->melee += expGain;
						break;
					case 6:
						if (origStat.strength < caps.strength)
							exp->strength += expGain;
						break;
					default:
						break;
					}
				}
				expRolls /= 2;
			}
			else if (danger > 0 && !engagement)
			{
				exp->stealth += RNG::generate(0, 4);
				exp->perception += RNG::generate(0, 2);
			}

			auto cats = _rule->getCategories();
			if (std::find(cats.begin(), cats.end(), "STR_INVESTIGATION") != cats.end())
			{
				exp->investigation += RNG::generate(1, std::max(2, expRolls));
				exp->perception += RNG::generate(0, std::max(1, expRolls / 2));
				exp->interrogation += RNG::generate(0, std::max(1, expRolls / 2));
			}
			if (std::find(cats.begin(), cats.end(), "STR_INFILTRATION") != cats.end())
			{
				exp->stealth += RNG::generate(1, std::max(2, expRolls));
				exp->perception += RNG::generate(0, std::max(1, expRolls / 2));
				exp->interrogation += RNG::generate(0, std::max(1, expRolls / 2));
			}
			if (std::find(cats.begin(), cats.end(), "STR_NEGOTIATION") != cats.end())
			{
				exp->charisma += RNG::generate(1, std::max(3, static_cast<int>(expRolls * 1.5)));
			}
			if (std::find(cats.begin(), cats.end(), "STR_DECEPTION") != cats.end())
			{
				exp->deception += RNG::generate(1, std::max(3, static_cast<int>(expRolls * 1.5)));
			}
		}

		if (soldier->isRookieAgent())
		{
			soldier->addExperience(ROLE_AGENT, RNG::generate(5, 10), getOperationName()); // extra role experience for rookie agents
			soldier->setRookieAgent(false);
		}

		// Ordinary covert operations must not train psionic stats or mana.
		// Dedicated psionics mechanics should handle these stats separately.
		exp->psiStrength = 0;
		exp->psiSkill = 0;
		exp->mana = 0;

		soldier->improvePrimaryStats(exp, ROLE_AGENT);
		//also improve secondary stats
		int rate = 0;
		soldier->getCurrentStatsEditable()->tu += Soldier::improveStat(exp->tu, rate, false);
		soldier->getCurrentStatsEditable()->stamina += Soldier::improveStat(exp->stamina, rate, false);

		UnitStats improvement = *soldier->getCurrentStats() - origStat;
		_results->addSoldierImprovement(soldier->getName(), improvement);

		if (!dead && wound != 0)
		{
			soldier->setReturnToTrainingWhenOperationOver(NONE);
		}
		delete exp;
	}

	//if needed kill soldiers from doomed list
	if (!soldiersToKill.empty())
	{
		int it = 0;
		int killN = soldiersToKill.size();
		bool loneSaved = false;
		int chosenID = 0;
		if (killN == operationSoldierN) //we want to keep at least one soldier alive, that would say operation failure results
		{
			loneSaved = true;
			chosenID = RNG::generate(0, killN - 1);
		}
		for (auto& j : soldiersToKill)
		{
			if (loneSaved && chosenID == it)
			{
				int health = j->getCurrentStats()->health;
				int genDamage = (int)RNG::generate(health * 0.5, health * 0.9);
				j->setWoundRecovery(genDamage);
				_results->addSoldierDamage(j->getName(), genDamage);
			}
			else
			{
				_results->addSoldierDamage(j->getName(), -10);
				save.killSoldier(true, j); //RIP
			}
			++it;
		}
	}
}

/**
 * Makes sure we over operation
 */
void CovertOperation::finishOperation()
{
	auto soldiers = getSoldiers();
	for (auto& soldier : soldiers)
	{
		//remove soldier from operation
		soldier->setCovertOperation(0);
		soldier->setJustSaved(false);

		//if soldier was not hurt we return him or her to training, if settings allows it
		if (soldier->getHealthMissing() == 0)
		{
			ReturnToTrainings trainings = soldier->getReturnToTrainingsWhenOperationOver();
			if (trainings == MARTIAL_TRAINING || trainings == BOTH_TRAININGS)
			{
				if (_base->getUsedTraining() < _base->getAvailableTraining())
				{
					soldier->setTraining(true);
				}
			}
			if (trainings == PSI_TRAINING || trainings == BOTH_TRAININGS)
			{
				if ((_base->getUsedPsiLabs() < _base->getAvailablePsiLabs()) && Options::anytimePsiTraining)
				{
					soldier->setPsiTraining(true);
				}
			}
		}
	}
	_over = true;
	_base->removeCovertOperation(this);
}

}
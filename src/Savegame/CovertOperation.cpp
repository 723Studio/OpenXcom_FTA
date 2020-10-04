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
#include <assert.h>
#include "../fmath.h"
#include "../Engine/Language.h"
#include "../Engine/Game.h"
#include "../Engine/RNG.h"
#include "../Engine/Logger.h"
#include "../Geoscape/FinishedCoverOperationState.h"
#include "../Geoscape/Globe.h"
#include "../Battlescape/BattlescapeGenerator.h"
#include "../Battlescape/BriefingState.h"
#include "../Battlescape/DebriefingState.h"
#include "../Savegame/SavedGame.h"
#include "../Savegame/Base.h"
#include "../Savegame/Region.h"
#include "../Savegame/ItemContainer.h"
#include "../Savegame/Soldier.h"
#include "../Savegame/SoldierDiary.h"
#include "../Savegame/BattleUnit.h"
#include "../Savegame/GeoscapeEvent.h"
#include "../Savegame/DiplomacyFaction.h"
#include "../Savegame/AlienMission.h"
#include "../Savegame/SavedBattleGame.h"
#include "../Savegame/ItemContainer.h"
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
CovertOperation::CovertOperation(const RuleCovertOperation* rule, Base* base, int cost, int chances) :
	_rule(rule), _base(base), _spent(0), _cost(cost), _successChance(chances), _assignedScientists(0), _assignedEngineers(0),
	_inBattlescape(false), _hasBattlescapeResolve(false), _over(false), _hasPsi(false), _progressEventSpawned(false), _results(0)
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
void CovertOperation::load(const YAML::Node& node)
{
	_spent = node["spent"].as<int>(_spent);
	_cost = node["cost"].as<int>(_cost);
	_successChance = node["successChance"].as<int>(_successChance);
	_assignedScientists = node["assignedScientists"].as<int>(_assignedScientists);
	_assignedEngineers = node["assignedEngineers"].as<int>(_assignedEngineers);
	_inBattlescape = node["inBattlescape"].as<bool>(_inBattlescape);
	_hasBattlescapeResolve = node["hasBattlescapeResolve"].as<bool>(_hasBattlescapeResolve);
	_hasPsi =  node["hasPsi"].as<bool>(_hasPsi);
	_over = node["over"].as<bool>(_over);
	_progressEventSpawned = node["progressEventSpawned"].as<bool>(_progressEventSpawned);
	_items->load(node["items"]);
}

/**
	* Saves the Operation to YAML.
	* @return YAML node.
	*/
YAML::Node CovertOperation::save() const
{
	YAML::Node node;
	node["name"] = getRules()->getName();
	node["spent"] = _spent;
	node["cost"] = _cost;
	node["successChance"] = _successChance;
	node["assignedScientists"] = _assignedScientists;
	node["assignedEngineers"] = _assignedEngineers;
	node["hasPsi"] = _hasPsi;
	node["inBattlescape"] = _inBattlescape;
	node["hasBattlescapeResolve"] = _hasBattlescapeResolve;
	node["progressEventSpawned"] = _progressEventSpawned;
	if (_over)
	{
		node["over"] = _over;
	}
	node["items"] = _items->save();

	return node;
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
 * Return a vector of pointers to a Soldier realisations that assigned to this operation.
 * @return a vector of pointers to a Soldier class.
 */
std::vector<Soldier*> CovertOperation::getSoldiers()
{
	std::vector<Soldier*> soldiers;
	for (std::vector<Soldier*>::iterator i = _base->getSoldiers()->begin(); i != _base->getSoldiers()->end(); ++i)
	{
		if ((*i)->getCovertOperation() != 0 && (*i)->getCovertOperation()->getOperationName() == this->getOperationName())
		{
			soldiers.push_back(*i);
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
	else if (_successChance > 75)
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
	if (time > 45)
		return ("STR_SEVERAL_MONTHS");
	else if (time > 20)
		return ("STR_MONTH");
	else if (time > 10)
		return ("STR_SEVERAL_WEEKS");
	else if (time > 6)
		return ("STR_WEEK");
	else
		return ("STR_SEVERAL_DAYS");
}

/**
* Handle Covert Operation daily logic.
* @param Game game engine.
* @param ThinkPeriond - timestep to determine think process
*/
bool CovertOperation::think(Game& engine, const Globe& globe)
{
	const Mod& mod = *engine.getMod();
	SavedGame& save = *engine.getSavedGame();
	// if finished, don't do anything
	if (_over)
		return false;

	// are we there yet?
	if (_spent <= _cost)
	{
		++_spent;
		//should we spawn ongoing event?
		std::string progressEvent = _rule->getProgressEvent();
		if (!progressEvent.empty())
		{
			bool spawn = false;
			if (!_rule->getRepeatProgressEvent() && _progressEventSpawned)
				return false;
			else
			{
				if (RNG::percent(_rule->getProgressEventChance()))
				{
					_progressEventSpawned = engine.getMasterMind()->spawnEvent(_rule->getProgressEvent());
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
		deathOdds -= 5;
		break;
	case OpenXcom::DIFF_VETERAN:
		break;
	case OpenXcom::DIFF_GENIUS:
		critFailCoef -= 2;
		woundOdds += 2;
		deathOdds += 2;
		break;
	case OpenXcom::DIFF_SUPERHUMAN:
		critFailCoef -= 5;
		woundOdds += 5;
		deathOdds += 4;
		break;
	}
	//now, we roll for operation results and init some vars
	int roll = RNG::generate(0, 99);
	bool operationResult = _successChance > roll;
	bool criticalFail = roll > (_successChance + critFailCoef);
	int score = 0;
	int funds = 0;
	std::string eventName;
	std::vector<std::string> researchList;
	std::map<std::string, int> itemsToAdd;
	std::string missionName;
	std::string deploymentName;
	std::map<std::string, int> reputationScore;
	//before we go process operation, let's return stuff to its base
	int sci = this->getAssignedScientists();
	int eng = this->getAssignedEngineers();
	if (sci > 0) _base->setScientists(_base->getScientists() + sci);
	if (eng > 0) _base->setEngineers(_base->getEngineers() + eng);
	_results = new CovertOperationResults(this->getOperationName(), operationResult, "0"); //TODO date
	//load results of operation
	if (operationResult)
	{
		score = _rule->getSuccessScore();
		funds = _rule->getSuccessFunds();
		researchList = _rule->getSuccessResearchList();
		eventName = _rule->getSuccessEvent();
		reputationScore = _rule->getSuccessReputationScoreList();
		missionName = _rule->chooseGenSuccessMissionType();
		deploymentName = _rule->chooseGenInstantSuccessDeploumentType();
		for (auto& pair : _rule->getSuccessEveryItemList())
		{
			const RuleItem* itemRule = mod.getItem(pair.first, true);
			if (itemRule)
			{
				itemsToAdd[itemRule->getType()] += pair.second;
			}
		}
		if (!_rule->getSuccessWeightedItemList().empty())
		{
			const RuleItem* weightedItem = mod.getItem(_rule->getSuccessWeightedItemList().choose(), true);
			if (weightedItem)
			{
				itemsToAdd[weightedItem->getType()] += 1;
			}
		}
	}
	else
	{
		score = _rule->getFailureScore() * (-1);
		if (criticalFail) score = score - 300;
		funds = _rule->getFailureFunds();
		researchList = _rule->getFailureResearchList();
		eventName = _rule->getFailureEvent();
		reputationScore = _rule->getFailureReputationScoreList();
		missionName = _rule->chooseGenFailureMissionType();
		deploymentName = _rule->chooseGenInstantTrapDeploumentType();
		for (auto& pair : _rule->getFailureEveryItemList())
		{
			const RuleItem* itemRule = mod.getItem(pair.first, true);
			if (itemRule)
			{
				itemsToAdd[itemRule->getType()] += pair.second;
			}
		}
		if (!_rule->getFailureWeightedItemList().empty())
		{
			const RuleItem* weightedItem = mod.getItem(_rule->getFailureWeightedItemList().choose(), true);
			if (weightedItem)
			{
				itemsToAdd[weightedItem->getType()] += 1;
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
			_results->addItem(addItems.first, addItems.second);
		}
	}

	if (!eventName.empty())
	{
		bool spawnEvent = engine.getMasterMind()->spawnEvent(eventName);
	}

	if (!researchList.empty())
	{
		std::vector<const RuleResearch*> possibilities;

		for (auto rName : researchList)
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
		}
	}
	
	if (!reputationScore.empty())
	{
		for (std::map<std::string, int>::const_iterator i = reputationScore.begin(); i != reputationScore.end(); ++i)
		{
			for (std::vector<DiplomacyFaction*>::iterator j = save.getDiplomacyFactions().begin(); j != save.getDiplomacyFactions().end(); ++j)
			{
				std::string factionName = (*j)->getRules().getName();
				std::string lookingName = (*i).first;
				if (factionName == lookingName)
				{
					(*j)->setReputation((*j)->getReputation() + (*i).second);
					_results->addReputation(factionName, (*i).second);
					break;
				}
			}
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

		bool isSiteType = missionRules->getObjective() == OBJECTIVE_SITE;
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
			if (mod.getIsFTAGame())
			{
				missionRace = "STR_ADVENT";
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
		mission->setMissionSiteZone(targetZone);
		mission->start(engine, globe, 0);
		save.getAlienMissions().push_back(mission);
	}

	if (!_rule->getSpecialRule().empty())//processing of yet hardcoded fta story arc
	{
		std::string specRule = _rule->getSpecialRule();
		if (specRule == "STR_REGIONAL_HQ_DISCOVERY")
		{
			for (std::vector<OpenXcom::AlienBase*>::iterator i = save.getAlienBases()->begin(); i != save.getAlienBases()->end(); ++i)
			{
				auto baseName = (*i)->getDeployment()->getType();
				if (baseName == "STR_INITIAL_REGIONAL_HQ")
				{
					(*i)->setDiscovered(true);

					_results->setSpecialMessage("STR_REGIONAL_HQ_FOUND");
				}
			}
		}
	}

	if (!deploymentName.empty())
	{
		bool process = true;
		if (!operationResult) //if operation failed lets see if we get into a trap!
		{
			int trapRoll = _rule->getTrapChance();
			if (criticalFail && trapRoll > 0) trapRoll = trapRoll + 35;
			process = RNG::generate(0, 99) < trapRoll;
		}
		if (process) 
		{//oh, boy, we are going to generate battlescape to resolve our covert opration!
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
		//simulating operation
		backgroundSimulation(engine, operationResult, criticalFail, woundOdds, deathOdds);
		// lets return items from operation to the base
		for (std::map<std::string, int>::iterator it = _items->getContents()->begin(); it != _items->getContents()->end(); ++it)
		{
			_base->getStorageItems()->addItem(it->first, it->second);
		}
		//now we can finish operation
		engine.pushState(new FinishedCoverOperationState(this, operationResult));
	}
	return true;
}

/**
* Handle backgound simulation for Covert Operation.
* Award exp to soldiers, simulate critical fail repercussion.
* @param Game game engine.
* @param operationResult and criticalFail operation results.
* @param woundOdds and deathOdds stats from game difficulty.
*/
void CovertOperation::backgroundSimulation(Game& engine, bool operationResult, bool criticalFail, int woundOdds, int deathOdds)
{
	const Mod& mod = *engine.getMod();
	SavedGame& save = *engine.getSavedGame();

	//first, we calculate how much expirience we can award for the operation (expRolls)
	int ruleCost = this->getRules()->getCosts();
	int danger = this->getRules()->getDanger();
	int effCost = 1;
	if (ruleCost < 20) effCost = ceil(ruleCost / 8);
	else if (ruleCost < 40) effCost = ceil(ruleCost / 8.5);
	else if (ruleCost < 60) effCost = ceil(ruleCost / 9.86);
	else if (effCost < 80) 	effCost = ceil(ruleCost / 11.54);
	else effCost = ceil(ruleCost / 12.52);

	int expRolls = 1 + effCost + ceil(danger / 3);
	//limit expirience gain
	if (expRolls > 10 && expRolls <= 13) expRolls = 10;
	if (expRolls > 13 && expRolls <= 16) expRolls = 11;
	if (expRolls > 16) expRolls = 12;

	if (save.getMonthsPassed() > 10) expRolls++; //bonus for lategame
	int expRollsRandom = RNG::generate(-3, 3);
	expRolls += expRollsRandom; //add more random
	//expirience is reduced for fail and critical fail
	if (!operationResult && !criticalFail) expRolls = round(expRolls / 3);
	else if (criticalFail) expRolls = round(expRolls / 4) - 2;

	//processing soldiers change before returning home
	std::vector<Soldier*> soldiersToKill;
	int operationSoldierN = 0;
	std::vector<Soldier*> soldiers = getSoldiers();
	for (std::vector<Soldier*>::iterator i = soldiers.begin(); i != soldiers.end(); ++i)
	{
		bool dead = false;
		int wound = 0;
		int damage = 0;
		bool saved = false;
		++operationSoldierN;
		UnitStats* stats = (*i)->getCurrentStats();
		const UnitStats caps = (*i)->getRules()->getStatCaps();
		UnitStats* improvement = new UnitStats();
		int tuExp = 0, staminaExp = 0;
		int healthExp = 0, braveryExp = 0, reactionsExp = 0, firingExp = 0, throwingExp = 0, meleeExp = 0, streingthExp = 0;
		int psiSkillExp = 0, psiStrExp = 0, manaExp = 0;
		//our dudes did something very wrong, so we hurt them back
		if (criticalFail && danger > 0)
		{
			for (size_t j = 0; j < danger; j++)
			{
				bool hit = RNG::generate(0, 99) < woundOdds;
				if (hit)
				{
					bool miss = RNG::generate(0, 99) < ceil((*i)->getStatsWithAllBonuses()->reactions * 0.7);
					if (miss)
					{
						++reactionsExp;
					}
					else
					{
						++wound;
					}
				}
			}
			if (wound > 0)
			{
				damage = (int)RNG::generate((wound * 8), (wound * 12));
			}
			if (damage < (*i)->getCurrentStats()->health)
			{
				(*i)->setHealthMissing(damage);
				_results->addSoldierDamage((*i)->getName(), damage);
			}
			else
			{
				dead = true; //ouch, too much damage rolled!
			}
			if (!dead)
			{ //OMG, Finger of Death for soldier on critical failed operation!!!
				dead = RNG::generate(0, 99) < deathOdds + ceil(danger / 4);
			}
			if (dead)
			{
				//Check for divine protection
				int protection = (*i)->getRank();
				protection += (((*i)->getDiary()->getKillTotal() / 20) + ((*i)->getDiary()->getStunTotal() / 15)); //there is a place for additional perks
				//lets add save if we have psi
				if (_hasPsi)
					protection += 2;
				int requiredProtection = 15 + danger;
				if (requiredProtection > protection)
				{//RIP...
					soldiersToKill.push_back(*i);
				}
				else
				{
					dead = false;
					saved = true;
					++braveryExp;
				}
			}
		}
		//soldiers can improve stats based on virtual expirience they take
		
		if (!dead && expRolls > 0)
		{
			//TU and Energy is increased most time
			if (stats->tu < caps.tu)
				tuExp = RNG::generate(-2, expRolls); //negative roll makes small additional chance for improveStat return 0
			if (stats->stamina < caps.stamina)
				staminaExp = RNG::generate(-2, expRolls);
			//other stats would be rolled to be improved
			int statID = 0;
			int expGain = 0;
			bool trainPsiSkill = (stats->psiSkill > 0 && _hasPsi);
			bool trainPsiStr = false;
			if (trainPsiSkill && Options::allowPsiStrengthImprovement)
				trainPsiStr = true; //in case we have this special property
			bool trainingManaPri = false;
			if (trainPsiSkill && mod.isManaTrainingPrimary())
				trainingManaPri = true;
			bool trainingManaSec = false;
			if (mod.isManaTrainingSecondary())
				trainingManaSec = true;
			for (size_t j = 0; j < expRolls; j++)
			{
				statID = RNG::generate(1, 8); //choose stat
				expGain = RNG::generate(1, 4); //choose how many expirience it would be
				if (expGain == 4)
					expGain = 1;
				switch (statID)
				{
				case 0:
					if (stats->health < caps.health)
						healthExp += expGain;
					break;
				case 1:
					if (stats->bravery < caps.bravery && !braveryExp)
					{
						int braveryRoll = 9;
						if (wound > 0)
							braveryRoll -= 1;
						if (saved || wound > 4)
							braveryRoll -= 2;
						if (RNG::generate(0, 10) > braveryRoll)
							braveryExp += expGain;
						else
							++expRolls; //re-roll as it all was only to reduce bravery training
					}
					break;
				case 2:
					if (stats->reactions < caps.reactions)
						reactionsExp += expGain;
					break;
				case 3:
					if (stats->firing < caps.firing)
						firingExp += expGain;
					break;
				case 4:
					if (stats->throwing < caps.throwing)
						throwingExp += expGain;
					break;
				case 5:
					if (stats->melee < caps.melee)
						meleeExp += expGain;
					break;
				case 6:
					if (stats->strength < caps.strength)
						streingthExp += expGain;
					break;
				case 7:
					if (stats->psiSkill < caps.psiSkill && trainPsiSkill)
					{
						psiSkillExp += expGain;
						if (stats->psiStrength < caps.psiStrength && trainPsiStr)
							psiStrExp += expGain;
						if (stats->mana < caps.mana && trainingManaPri)
							manaExp += expGain;
					}
					else if (!trainPsiSkill) ++expRolls; //re-roll as we assume soldier used other tools to achieave his or her goals
					break;
				case 8: //special case for separate non-psi mana using, like XCF
					if (stats->mana < caps.mana && trainingManaSec)
						manaExp += expGain;
					else if (!trainingManaSec)
						++expRolls;
					break;
				default:
					break;
				}
			}
		}
		if (dead || wound != 0)
		{
			(*i)->setReturnToTrainingWhenOperationOver(NONE);
		}
		//we want to remember stat improvement for later use
		improvement->tu = improveStat(tuExp);
		improvement->stamina = improveStat(staminaExp);
		improvement->health = improveStat(healthExp);
		if (braveryExp > 0)
			improvement->bravery = 10;
		improvement->reactions = improveStat(reactionsExp);
		improvement->firing = improveStat(firingExp);
		improvement->throwing = improveStat(throwingExp);
		improvement->melee = improveStat(meleeExp);
		improvement->strength = improveStat(streingthExp);
		improvement->psiSkill = improveStat(psiSkillExp);
		improvement->psiStrength = improveStat(psiStrExp);
		improvement->mana = improveStat(manaExp);
		_results->addSoldierImprovement((*i)->getName(), improvement);

		//ok, now lets actually improve soldier stats!
		stats->tu += improvement->tu;
		stats->stamina += improvement->stamina;
		stats->health += improvement->health;
		stats->bravery += improvement->bravery;
		stats->reactions += improvement->reactions;
		stats->firing += improvement->firing;
		stats->throwing += improvement->throwing;
		stats->melee += improvement->melee;
		stats->strength += improvement->strength;
		stats->psiSkill += improvement->psiSkill;
		stats->psiStrength += improvement->psiStrength;
		stats->mana += improvement->mana;
	}

	//if needed kill soldiers from doomed list
	if (!soldiersToKill.empty())
	{
		int it = 0;
		int killN = soldiersToKill.size();
		bool loneSaved = false;
		int chosenID = 0;
		if (killN == operationSoldierN) //we want to keep at least one soldier alive, that would say operation filure results
		{
			loneSaved = true;
			chosenID = RNG::generate(0, killN - 1);
		} 
		for (std::vector<Soldier*>::iterator j = soldiersToKill.begin(); j != soldiersToKill.end(); ++j)
		{
			if (loneSaved && chosenID == it)
			{
				Log(LOG_INFO) << "All soldiers on covert operation named: " << this->getOperationName() <<
					" should be dead, but soldier named: " << (*j)->getName() << " was chosen to be the last survived.";
			}
			else
			{
				_results->addSoldierDamage((*j)->getName(), -10);
				save.killSoldier(*j); //RIP
			}
			++it;
		}
	}
}


/**
 * Converts the number of experience to the stat increase.
 * @param Experience counter.
 * @return Stat increase.
 */
int CovertOperation::improveStat(int exp) const
{
	if (exp > 10) return RNG::generate(2, 6);
	else if (exp > 5)  return RNG::generate(1, 4);
	else if (exp > 2)  return RNG::generate(1, 3);
	else if (exp > 0)  return RNG::generate(0, 1);
	else               return 0;
}

/**
 * Makes sure we over operation
 */
void CovertOperation::finishOperation()
{
	auto soldiers = getSoldiers();
	for (std::vector<Soldier*>::iterator i = soldiers.begin(); i != soldiers.end(); ++i)
	{
		//remove soldier from operation
		(*i)->setCovertOperation(0);

		//if soldier was not hurt we return him or her to trainig, if settings allows it
		if ((*i)->getHealthMissing() == 0)
		{
			ReturnToTrainings trainings = (*i)->getReturnToTrainingsWhenOperationOver();
			if (trainings == MARTIAL_TRAINING || trainings == BOTH_TRAININGS)
			{
				if (_base->getUsedTraining() < _base->getAvailableTraining())
				{
					(*i)->setTraining(true);
				}
			}
			if (trainings == PSI_TRAINING || trainings == BOTH_TRAININGS)
			{
				if ((_base->getUsedPsiLabs() < _base->getAvailablePsiLabs()) && Options::anytimePsiTraining)
				{
					(*i)->setPsiTraining(true);
				}
			}
		}
	}
	_over = true;
	_base->removeCovertOperation(this);
}

}



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
#include <vector>
#include <string>
#include <yaml-cpp/yaml.h>
#include "../Savegame/WeightedOptions.h"
#include "RuleBaseFacilityFunctions.h"

namespace OpenXcom
{

class Mod;

/**
 * Represents ruleset for a specific covert operation.
 * Contains constant info about a covert operation like
 * name, requirements, etc.
 * @sa CovertOperation
 */
class RuleCovertOperation
{
private:
	std::string _name, _description, _successDescription, _failureDescription, _successBackground, _failureBackground, _successMusic, _failureMusic, _successEvent,  _failureEvent, _specialRule;
	std::vector<std::string> _requires, _canceledBy, _allowedArmor, _successResearchList, _failureResearchList;
	RuleBaseFacilityFunctions _requiresBaseFunc;
	int _soldierSlots, _optionalSoldierSlots, _scientistSlots, _engineerSlots, _optionalSoldierEffect, _scientistEffect, _engineerEffect, _itemSpaceEffect, _armorEffect;
	double _itemSpaceLimit;
	int _baseChances, _costs, _successScore, _failureScore, _successFunds, _failureFunds, _progressEventChance, _trapChance, _danger, _concealedItemsBonus, _requiredItemsEffect;
	bool _repeatProgressEvent, _allowAllEquipment;
	WeightedOptions _successMissions, _failureMissions, _successWeightedItemList, _failureWeightedItemList, _instantTrapDeployment, _instantSuccessDeployment, _progressEvent;
	std::map<std::string, int> _requiredReputationLvl, _successReputationScore, _failureReputationScore, _successEveryItemList, _failureEveryItemList, _requiredItems, _soldierTypeEffectiveness; //TODO processing of _requiredReputationLvl
	int _listOrder;
public:
	/// Creates a blank craft ruleset.
	RuleCovertOperation(const std::string& name);
	/// Cleans up the craft ruleset.
	~RuleCovertOperation();
	/// Loads craft data from YAML.
	void load(const YAML::Node& node, Mod* mod, int listOrder);
	/// Cross link with other rules.
	void afterLoad(const Mod* mod);
	/// Gets the operation's name.
	const std::string& getName() const { return _name; };
	/// Gets the operation's description.
	const std::string& getDescription() const { return _description; };
	/// Gets the operation's success results description.
	const std::string& getSuccessDescription() const { return _successDescription; };
	/// Gets the operation's failure results description.
	const std::string& getFailureDescription() const { return _failureDescription; };
	/// Gets the operation's success results background image.
	const std::string& getSuccessBackground() const { return _successBackground; };
	/// Gets the operation's failure results background image.
	const std::string& getFailureBackground() const { return _failureBackground; };
	/// Gets the operation's success results background image.
	const std::string& getSuccessMusic() const { return _successMusic; };
	/// Gets the operation's failure results background image.
	const std::string& getFailureMusic() const { return _failureMusic; };
	/// Gets the operation's requirements.
	const std::vector<std::string>& getRequirements() const { return _requires; };
	/// Gets the base functions required to start operation.
	RuleBaseFacilityFunctions getRequiresBaseFunc() const { return _requiresBaseFunc; }
	/// Gets the research name that would make this operation impossible to run.
	//const std::vector<std::string>& getCanceledBy() const { return _canceledBy; };
	/// Gets the research name that would make this operation impossible to run.
	const std::vector<std::string>& getCanceledBy() const { return _canceledBy; };
	/// Gets the event name that would be spawned on success operation result.
	const std::string& getSuccessEvent() const { return _successEvent; };
	/// Gets the event name that would be spawned on failure operation result.
	const std::string& getFailureEvent() const { return _failureEvent; };
	/// Gets the event name that would be spawned while operation is in progress.
	const WeightedOptions& getProgressEvent() const { return _progressEvent; };
	/// Gets the operation's list of projects that would be discovered on success resolve.
	const std::vector<std::string>& getSuccessResearchList() const { return _successResearchList; };
	/// Gets the operation's list of projects that would be discovered on failure.
	const std::vector<std::string>& getFailureResearchList() const { return _failureResearchList; };
	/// Gets the soldiers number that required by this operations.
	int getSoldierSlots() const { return _soldierSlots; };
	/// Gets the optional soldiers slots for this operations.
	int getOptionalSoldierSlots() const { return _optionalSoldierSlots; };
	/// Gets the optional scientist slots for this operations.
	int getScientistSlots() const { return _scientistSlots; };
	/// Gets the optional engineer slots for this operations.
	int getEngineerSlots() const { return _engineerSlots; };
	/// Gets the optional soldiers slots effectiveness for this operations.
	int getOptionalSoldierEffect() const { return _optionalSoldierEffect; };
	/// Gets the optional scientist slots effectiveness for this operations.
	int getScientistEffect() const { return _scientistEffect; };
	/// Gets the optional engineer slots effectiveness for this operations.
	int getEngineerEffect() const { return _engineerEffect; };
	/// Gets the operation's base chances of success result.
	int getBaseChances() const { return _baseChances; };
	/// Gets the operation's time costs.
	int getCosts() const { return _costs; };
	/// Gets the operation's global score, that is awarded on success result.
	int getSuccessScore() const { return _successScore; };
	/// Gets the operation's global score, that is awarded on failure.
	int getFailureScore() const { return _failureScore; };
	/// Gets the operation's funds, that is awarded on success result.
	int getSuccessFunds() const { return _successFunds; };
	/// Gets the operation's funds, that is awarded on failure.
	int getFailureFunds() const { return _failureFunds; };
	/// Gets chance of spawning event in progress of running covert operation.
	int getProgressEventChance() const { return _progressEventChance; };
	/// Gets if progress event should be repeated in progress of running covert operation.
	bool getRepeatProgressEvent() const { return _repeatProgressEvent; };
	/// Gets operation item space limit.
	double getItemSpaceLimit() const { return _itemSpaceLimit; }
	/// Gets operation item space limit.
	int getItemSpaceEffect() const { return _itemSpaceEffect; }
	/// Gets operations trap chance.
	int getTrapChance() const { return _trapChance; }
	/// Gets operations danger level.
	int getDanger() const { return _danger; }
	/// Gets chosen alien mission to run as success resolve of covert operation.
	std::string chooseGenSuccessMissionType() const;
	/// Gets chosen alien mission to run as failure resolve of covert operation.
	std::string chooseGenFailureMissionType() const;
	/// Gets chosen deployment that would run as failure resolve of covert operation.
	std::string chooseGenInstantTrapDeploumentType() const;
	/// Gets chosen deployment that would run as success resolve of covert operation.
	std::string chooseGenInstantSuccessDeploumentType() const;
	/// Gets chosen geoscape event that would run in progress of covert operation.
	std::string chooseProgressEvent() const;
	/// Gets the faction's required reputation list for this operation.
	const std::map<std::string, int>& getRequiredReputationLvlList() const { return _requiredReputationLvl; }
	/// Gets the factions reputation award list for this operation on success result.
	const std::map<std::string, int>& getSuccessReputationScoreList() const { return _successReputationScore; }
	/// Gets the factions reputation award list for this operation on failure.
	const std::map<std::string, int>& getFailureReputationScoreList() const { return _failureReputationScore; }
	/// Gets list of items, all of it would be added to this operation on success result.
	const std::map<std::string, int>& getSuccessEveryItemList() const { return _successEveryItemList; }
	/// Gets list of items, all of it would be added to this operation on failure.
	const std::map<std::string, int>& getFailureEveryItemList() const { return _failureEveryItemList; }
	/// Gets a list of items; one of them is randomly selected (considering weights) and would be added to this operation on success result.
	const WeightedOptions& getSuccessWeightedItemList() const { return _successWeightedItemList; }
	/// Gets a list of items; one of them is randomly selected (considering weights) and  would be added to this operation on failure.
	const WeightedOptions& getFailureWeightedItemList() const { return _failureWeightedItemList; }
	/// Gets the operation's required item list.
	const std::map<std::string, int>& getRequiredItemList() const { return _requiredItems; }
	/// Gets bonus for having required items for this operation.
	int getRequiredItemsEffect() const { return _requiredItemsEffect; };
	/// Gets if this covert operation does not have any effects for concealed or heavy weapons.
	bool getAllowAllEquipment() const { return _allowAllEquipment; };
	/// Gets bonus for concealed-only weapons for this operation.
	int getConcealedItemsBonus() const { return _concealedItemsBonus; }; 
	/// Gets the operation's allowed armor.
	const std::vector<std::string>& getAllowedArmor() const { return _allowedArmor; };
	/// Gets the operation's allowed armor effect.
	int getAllowedArmorEffect() const { return _armorEffect; };
	/// Gets the effectiveness by soldier type for this operations.
	const std::map<std::string, int>& getSoldierTypeEffectiveness() const { return _soldierTypeEffectiveness; }
	/// Gets the operation's special campaign rule.
	const std::string& getSpecialRule() const { return _specialRule; };
	/// Gets the operation's list order.
	int getListOrder() const { return _listOrder; }
};
}

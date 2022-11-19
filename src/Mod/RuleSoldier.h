#pragma once
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
#include <string>
#include <yaml-cpp/yaml.h>
#include "Unit.h"
#include "RuleBaseFacilityFunctions.h"
#include "../Engine/Script.h"

namespace OpenXcom
{

class Mod;
class ModScript;
class SoldierNamePool;
class StatString;
class RuleItem;
class RuleSkill;
class RulePrisoner;
class Armor;
/// Soldier roles for FtA game
enum SoldierRole : int { ROLE_SOLDIER = 0, ROLE_PILOT = 1, ROLE_AGENT = 2, ROLE_SCIENTIST = 3, ROLE_ENGINEER = 4 };

struct SoldierRoleRanksRequirments
{
	SoldierRole role;
	std::map<int, int> requirments;

	/// Loads stats from YAML.
	void load(const YAML::Node &node)
	{
		role = (SoldierRole)node["role"].as<int>(role);
		requirments = node["requirments"].as<std::map<int, int>>(requirments);
	}
};

struct SoldierRoleRanksStrings
{
	SoldierRole role;
	std::map<int, std::string> strings;

	/// Loads stats from YAML.
	void load(const YAML::Node &node)
	{
		role = (SoldierRole)node["role"].as<int>(role);
		strings = node["strings"].as<std::map<int, std::string>>(strings);
	}
};

/**
 * Represents the creation data for an X-COM unit.
 * This info is copied to either Soldier for Geoscape or BattleUnit for Battlescape.
 * @sa Soldier BattleUnit
 */
class RuleSoldier
{
public:
	/// Number of bits for soldier gender.
	static constexpr int LookGenderBits = 1;
	/// Number of bits for soldier look.
	static constexpr int LookBaseBits = 2;
	/// Number of bits for soldier lookVariants.
	static constexpr int LookVariantBits = 6;
	/// Max number of soldier lookVariant.s
	static constexpr int LookVariantMax = (1 << LookVariantBits);
	/// Mask for soldier lookVariants.
	static constexpr int LookVariantMask = LookVariantMax - 1;
	/// Mask for all possible looks types for soldier.
	static constexpr int LookTotalMask = (1 << (LookVariantBits + LookBaseBits + LookGenderBits)) - 1;

	/// Name of class used in script.
	static constexpr const char *ScriptName = "RuleSoldier";
	/// Register all useful function used by script.
	static void ScriptRegister(ScriptParserBase* parser);

private:
	std::string _type;
	std::vector<SoldierRole> _roles;
	int _listOrder;
	std::vector<std::string> _requires;
	RuleBaseFacilityFunctions _requiresBuyBaseFunc;
	std::vector<SoldierRoleRanksRequirments*> _roleExpRequirments;
	std::vector<SoldierRoleRanksStrings*> _roleRankStrings;
	UnitStats _minStats, _maxStats, _statCaps, _trainingStatCaps, _dogfightExperience;
	std::string _armorName;
	const Armor* _armor;
	std::string _prisonerName;
	const RulePrisoner* _prisoner = nullptr;
	std::string _specWeaponName;
	const RuleItem* _specWeapon;
	int _monthlyBuyLimit;
	int _costBuy, _costSalary, _costSalarySquaddie, _costSalarySergeant, _costSalaryCaptain, _costSalaryColonel, _costSalaryCommander;
	int _standHeight, _kneelHeight, _floatHeight;
	int _femaleFrequency, _value, _transferTime, _moraleLossWhenKilled;
	int _manaMissingWoundThreshold = -1;
	int _healthMissingWoundThreshold = -1;
	std::vector<int> _deathSoundMale, _deathSoundFemale;
	std::vector<int> _panicSoundMale, _panicSoundFemale, _berserkSoundMale, _berserkSoundFemale;
	std::vector<int> _selectUnitSoundMale, _selectUnitSoundFemale;
	std::vector<int> _startMovingSoundMale, _startMovingSoundFemale;
	std::vector<int> _selectWeaponSoundMale, _selectWeaponSoundFemale;
	std::vector<int> _annoyedSoundMale, _annoyedSoundFemale;
	std::vector<SoldierNamePool*> _names;
	int _totalSoldierNamePoolWeight;
	std::string _armorForAvatar;
	int _avatarOffsetX, _avatarOffsetY, _flagOffset;
	bool _allowPromotion, _allowPiloting, _showTypeInInventory;
	std::vector<StatString*> _statStrings;
	std::vector<std::string> _rankStrings, _pilotRankStrings, _agentRankStrings, _scientistRankStrings, _engineerRankStrings;
	int _rankSprite, _rankSpriteBattlescape, _rankSpriteTiny;
	int _pilotRankSprite, _pilotRankSpriteBattlescape, _pilotRankSpriteTiny;
	int _agentRankSprite, _agentRankSpriteBattlescape, _agentRankSpriteTiny;
	int _scientistRankSprite, _scientistRankSpriteBattlescape, _scientistRankSpriteTiny;
	int _engineerRankSprite, _engineerRankSpriteBattlescape, _engineerRankSpriteTiny;
	int _skillIconSprite;
	std::vector<std::string> _skillNames;
	std::vector<const RuleSkill*> _skills;
	ScriptValues<RuleSoldier> _scriptValues;

	void loadRoles(const std::vector<int> &r);

	void addSoldierNamePool(const std::string &namFile);
public:
	/// Creates a blank soldier ruleset.
	RuleSoldier(const std::string &type);
	/// Cleans up the soldier ruleset.
	~RuleSoldier();
	/// Loads the soldier data from YAML.
	void load(const YAML::Node& node, Mod *mod, int listOrder, const ModScript &parsers);
	/// Cross link with other rules.
	void afterLoad(const Mod* mod);
	/// Gets the soldier's type.
	const std::string& getType() const;
	/// Gets the soldier's role.
	std::vector<SoldierRole> getRoles() const { return _roles; }
	/// Gets whether or not the soldier type should be displayed in the inventory.
	bool getShowTypeInInventory() const { return _showTypeInInventory; }
	/// Gets the list/sort order of the soldier's type.
	int getListOrder() const;
	/// Gets the soldier's requirements.
	const std::vector<std::string> &getRequirements() const;
	/// Gets the base functions required to buy solder.
	RuleBaseFacilityFunctions getRequiresBuyBaseFunc() const { return _requiresBuyBaseFunc; }
	/// Gets the minimum stats for the random stats generator.
	UnitStats getMinStats() const;
	/// Gets the maximum stats for the random stats generator.
	UnitStats getMaxStats() const;
	/// Gets the stat caps.
	UnitStats getStatCaps() const;
	/// Gets the training stat caps.
	UnitStats getTrainingStatCaps() const;
	/// Gets the improvement chances for pilots (after dogfight).
	UnitStats getDogfightExperience() const;
	/// Gets the monthly hiring limit.
	int getMonthlyBuyLimit() const { return _monthlyBuyLimit; }
	/// Gets the cost of the soldier.
	int getBuyCost() const;
	/// Does salary depend on rank?
	bool isSalaryDynamic() const;
	/// Is a skill menu defined for this soldier type?
	bool isSkillMenuDefined() const;
	/// Gets the list of defined skills.
	const std::vector<const RuleSkill*> &getSkills() const;
	/// Returns the sprite index for the skill icon sprite.
	int getSkillIconSprite() const;
	/// Gets the monthly salary of the soldier (for a given rank).
	int getSalaryCost(int rank) const;
	/// Gets the height of the soldier when it's standing.
	int getStandHeight() const;
	/// Gets the height of the soldier when it's kneeling.
	int getKneelHeight() const;
	/// Gets the elevation of the soldier when it's flying.
	int getFloatHeight() const;
	/// Gets the default-equipped armor.
	Armor* getDefaultArmor() const;
	/// Gets the prisoner type.
	const RulePrisoner* getPrisoner() const { return _prisoner; }
	/// Gets the armor for avatar display.
	const std::string& getArmorForAvatar() const;
	/// Gets the X offset used for avatar.
	int getAvatarOffsetX() const;
	/// Gets the Y offset used for avatar.
	int getAvatarOffsetY() const;
	/// Gets the flag offset.
	int getFlagOffset() const;
	/// Gets the special weapon.
	const RuleItem* getSpecialWeapon() const { return _specWeapon; }
	/// Gets the allow promotion flag.
	bool getAllowPromotion() const;
	/// Gets the allow piloting flag.
	bool getAllowPiloting() const;
	/// Gets the female appearance ratio.
	int getFemaleFrequency() const;
	/// Gets the soldier's male death sounds.
	const std::vector<int> &getMaleDeathSounds() const;
	/// Gets the soldier's female death sounds.
	const std::vector<int> &getFemaleDeathSounds() const;
	/// Gets the soldier's male panic sounds.
	const std::vector<int> &getMalePanicSounds() const;
	/// Gets the soldier's female panic sounds.
	const std::vector<int> &getFemalePanicSounds() const;
	/// Gets the soldier's male berserk sounds.
	const std::vector<int> &getMaleBerserkSounds() const;
	/// Gets the soldier's female berserk sounds.
	const std::vector<int> &getFemaleBerserkSounds() const;
	/// Gets the soldier's male "select unit" sounds.
	const std::vector<int> &getMaleSelectUnitSounds() const { return _selectUnitSoundMale; }
	/// Gets the soldier's female "select unit" sounds.
	const std::vector<int> &getFemaleSelectUnitSounds() const { return _selectUnitSoundFemale; }
	/// Gets the soldier's male "start moving" sounds.
	const std::vector<int> &getMaleStartMovingSounds() const { return _startMovingSoundMale; }
	/// Gets the soldier's female "start moving" sounds.
	const std::vector<int> &getFemaleStartMovingSounds() const { return _startMovingSoundFemale; }
	/// Gets the soldier's male "select weapon" sounds.
	const std::vector<int> &getMaleSelectWeaponSounds() const { return _selectWeaponSoundMale; }
	/// Gets the soldier's female "select weapon" sounds.
	const std::vector<int> &getFemaleSelectWeaponSounds() const { return _selectWeaponSoundFemale; }
	/// Gets the soldier's male "annoyed" sounds.
	const std::vector<int> &getMaleAnnoyedSounds() const { return _annoyedSoundMale; }
	/// Gets the soldier's female "annoyed" sounds.
	const std::vector<int> &getFemaleAnnoyedSounds() const { return _annoyedSoundFemale; }
	/// Gets the pool list for soldier names.
	const std::vector<SoldierNamePool*> &getNames() const;
	/// Gets the total weight of all the soldier name pools.
	int getTotalSoldierNamePoolWeight() const { return _totalSoldierNamePoolWeight; }
	/// Gets the value - for score calculation.
	int getValue() const;
	/// Gets the soldier's transfer time.
	int getTransferTime() const;
	/// Percentage modifier for morale loss when this unit is killed.
	int getMoraleLossWhenKilled() const { return _moraleLossWhenKilled; }
	/// Gets the list of StatStrings.
	const std::vector<StatString *> &getStatStrings() const;
	/// Gets the list of strings for ranks.
	const std::vector<std::string> &getRankStrings() const;
	/// Gets the offset of the rank sprite in BASEBITS.PCK.
	int getRankSprite() const;
	/// Gets the offset of the rank sprite in SMOKE.PCK.
	int getRankSpriteBattlescape() const;
	/// Gets the offset of the rank sprite in TinyRanks.
	int getRankSpriteTiny() const;
	/// Getters for FtA's roles sprite IDs.
	int getPilotRankSprite() const { return _pilotRankSprite; }
	int getPilotRankSpriteBattlescape() const { return _pilotRankSpriteBattlescape; }
	int getPilotRankSpriteTiny() const { return _pilotRankSpriteTiny; }
	int getAgentRankSprite() const { return _agentRankSprite; }
	int getAgentRankSpriteBattlescape() const { return _agentRankSpriteBattlescape; }
	int getAgentRankSpriteTiny() const { return _agentRankSpriteTiny; }
	int getScientistRankSprite() const { return _scientistRankSprite; }
	int getScientistSpriteBattlescape() const { return _scientistRankSpriteBattlescape; }
	int getScientistSpriteTiny() const { return _scientistRankSpriteTiny; }
	int getEngineerRankSprite() const { return _engineerRankSprite; }
	int getEngineerRankSpriteBattlescape() const { return _engineerRankSpriteBattlescape; }
	int getEngineerRankSpriteTiny() const { return _engineerRankSpriteTiny; }
	std::vector<SoldierRoleRanksRequirments *> getRoleExpRequirments() const { return _roleExpRequirments; }
	std::vector<SoldierRoleRanksStrings *> getRoleRankStrings() const { return _roleRankStrings; }

	/// Get all script values.
	const ScriptValues<RuleSoldier> &getScriptValuesRaw() const { return _scriptValues; }

	/// How much missing mana will act as "fatal wounds" and prevent the soldier from going into battle.
	int getManaWoundThreshold() const { return _manaMissingWoundThreshold; }
	/// How much missing health will act as "fatal wounds" and prevent the soldier from going into battle.
	int getHealthWoundThreshold() const { return _healthMissingWoundThreshold; }
};

}

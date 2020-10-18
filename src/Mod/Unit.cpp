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
#include "Unit.h"
#include "../Engine/Exception.h"
#include "../Engine/ScriptBind.h"
#include "Mod.h"

namespace OpenXcom
{

/**
 * Creates a certain type of unit.
 * @param type String defining the type.
 */
Unit::Unit(const std::string &type) :
	_type(type), _showFullNameInAlienInventory(-1), _armor(nullptr), _standHeight(0), _kneelHeight(0), _floatHeight(0), _value(0),
	_moraleLossWhenKilled(100), _aggroSound(-1), _moveSound(-1), _intelligence(0), _aggression(0),
	_spotter(0), _sniper(0), _energyRecovery(30), _specab(SPECAB_NONE), _livingWeapon(false),
	_psiWeapon("ALIEN_PSI_WEAPON"), _capturable(true), _canSurrender(false), _autoSurrender(false),
	_isLeeroyJenkins(false), _waitIfOutsideWeaponRange(false), _pickUpWeaponsMoreActively(-1)
{
}

/**
 *
 */
Unit::~Unit()
{

}

/**
 * Loads the unit from a YAML file.
 * @param node YAML node.
 * @param mod Mod for the unit.
 */
void Unit::load(const YAML::Node &node, Mod *mod)
{
	if (const YAML::Node &parent = node["refNode"])
	{
		load(parent, mod);
	}
	_type = node["type"].as<std::string>(_type);
	_civilianRecoveryType = node["civilianRecoveryType"].as<std::string>(_civilianRecoveryType);
	_race = node["race"].as<std::string>(_race);
	_showFullNameInAlienInventory = node["showFullNameInAlienInventory"].as<int>(_showFullNameInAlienInventory);
	_rank = node["rank"].as<std::string>(_rank);
	_stats.merge(node["stats"].as<UnitStats>(_stats));
	_statsRandom.merge(node["statsRandom"].as<UnitStats>(_statsRandom));
	_armorName = node["armor"].as<std::string>(_armorName);
	_standHeight = node["standHeight"].as<int>(_standHeight);
	_kneelHeight = node["kneelHeight"].as<int>(_kneelHeight);
	_floatHeight = node["floatHeight"].as<int>(_floatHeight);
	if (_floatHeight + _standHeight > 25)
	{
		throw Exception("Error with unit "+ _type +": Unit height may not exceed 25");
	}
	_value = node["value"].as<int>(_value);
	_moraleLossWhenKilled = node["moraleLossWhenKilled"].as<int>(_moraleLossWhenKilled);
	_intelligence = node["intelligence"].as<int>(_intelligence);
	_aggression = node["aggression"].as<int>(_aggression);
	_spotter = node["spotter"].as<int>(_spotter);
	_sniper = node["sniper"].as<int>(_sniper);
	_energyRecovery = node["energyRecovery"].as<int>(_energyRecovery);
	_specab = (SpecialAbility)node["specab"].as<int>(_specab);
	if (const YAML::Node& spawn = node["spawnUnit"])
	{
		_spawnUnitName = spawn.as<std::string>();
	}
	_livingWeapon = node["livingWeapon"].as<bool>(_livingWeapon);
	_canSurrender = node["canSurrender"].as<bool>(_canSurrender);
	_autoSurrender = node["autoSurrender"].as<bool>(_autoSurrender);
	_isLeeroyJenkins = node["isLeeroyJenkins"].as<bool>(_isLeeroyJenkins);
	_waitIfOutsideWeaponRange = node["waitIfOutsideWeaponRange"].as<bool>(_waitIfOutsideWeaponRange);
	_pickUpWeaponsMoreActively = node["pickUpWeaponsMoreActively"].as<int>(_pickUpWeaponsMoreActively);
	_meleeWeapon = node["meleeWeapon"].as<std::string>(_meleeWeapon);
	_psiWeapon = node["psiWeapon"].as<std::string>(_psiWeapon);
	_capturable = node["capturable"].as<bool>(_capturable);
	_altRecoveredUnit = node["altRecoveredUnit"].as<std::string>(_altRecoveredUnit);
	_specialObjectiveType = node["specialObjectiveType"].as<std::string>(_specialObjectiveType);
	_builtInWeaponsNames = node["builtInWeaponSets"].as<std::vector<std::vector<std::string> > >(_builtInWeaponsNames);
	if (node["builtInWeapons"])
	{
		_builtInWeaponsNames.push_back(node["builtInWeapons"].as<std::vector<std::string> >());
	}

	mod->loadSoundOffset(_type, _deathSound, node["deathSound"], "BATTLE.CAT");
	mod->loadSoundOffset(_type, _panicSound, node["panicSound"], "BATTLE.CAT");
	mod->loadSoundOffset(_type, _berserkSound, node["berserkSound"], "BATTLE.CAT");
	mod->loadSoundOffset(_type, _aggroSound, node["aggroSound"], "BATTLE.CAT");

	mod->loadSoundOffset(_type, _selectUnitSound, node["selectUnitSound"], "BATTLE.CAT");
	mod->loadSoundOffset(_type, _startMovingSound, node["startMovingSound"], "BATTLE.CAT");
	mod->loadSoundOffset(_type, _selectWeaponSound, node["selectWeaponSound"], "BATTLE.CAT");
	mod->loadSoundOffset(_type, _annoyedSound, node["annoyedSound"], "BATTLE.CAT");
	mod->loadSoundOffset(_type, _spawnedUnitSound, node["spawnedUnitSound"], "BATTLE.CAT");

	mod->loadSoundOffset(_type, _moveSound, node["moveSound"], "BATTLE.CAT");
}

/**
 * Cross link with other rules
 */
void Unit::afterLoad(const Mod* mod)
{
	mod->linkRule(_armor, _armorName);
	mod->linkRule(_spawnUnit, _spawnUnitName);
	mod->linkRule(_builtInWeapons, _builtInWeaponsNames);
	mod->linkRule(_altUnit, _altRecoveredUnit);

	/*if (!_altRecoveredUnit.empty() && !mod->getUnit(_altRecoveredUnit))
	{
		throw Exception("Unit type: '" + this->getType() + "' has broken link in altRecoveredUnit: '" + _altRecoveredUnit +"'!");
	}*/
	
}

/**
 * Returns the language string that names
 * this unit. Each unit type has a unique name.
 * @return The unit's name.
 */
const std::string& Unit::getType() const
{
	return _type;
}

/**
* Gets the type of staff (soldier/engineer/scientists) or type of item to be recovered when a civilian is saved.
* @return The type of staff/item to recover.
*/
std::string Unit::getCivilianRecoveryType() const
{
	return _civilianRecoveryType;
}

/**
 * Returns the unit's stats data object.
 * @return The unit's stats.
 */
UnitStats *Unit::getStats()
{
	return &_stats;
}

/**
 * Returns the unit's random stats data object.
 * @return The unit's random stats.
 */
UnitStats* Unit::getRandomStats()
{
	return &_statsRandom;
}

/**
 * Returns the unit's height at standing.
 * @return The unit's height.
 */
int Unit::getStandHeight() const
{
	return _standHeight;
}

/**
 * Returns the unit's height at kneeling.
 * @return The unit's kneeling height.
 */
int Unit::getKneelHeight() const
{
	return _kneelHeight;
}

/**
 * Returns the unit's floating elevation.
 * @return The unit's floating height.
 */
int Unit::getFloatHeight() const
{
	return _floatHeight;
}

/**
 * Gets the unit's armor type.
 * @return The unit's armor type.
 */
Armor* Unit::getArmor() const
{
	return const_cast<Armor*>(_armor); //TODO: fix this function usage to remove const cast
}

/**
 * Gets the alien's race.
 * @return The alien's race.
 */
std::string Unit::getRace() const
{
	return _race;
}

/**
 * Gets the unit's rank.
 * @return The unit's rank.
 */
std::string Unit::getRank() const
{
	return _rank;
}

/**
 * Gets the unit's value - for scoring.
 * @return The unit's value.
 */
int Unit::getValue() const
{
	return _value;
}

/**
* Get the unit's death sounds.
* @return List of sound IDs.
*/
const std::vector<int> &Unit::getDeathSounds() const
{
	return _deathSound;
}

/**
 * Gets the unit's panic sounds.
 * @return List of sound IDs.
 */
const std::vector<int> &Unit::getPanicSounds() const
{
	return _panicSound;
}

/**
 * Gets the unit's berserk sounds.
 * @return List of sound IDs.
 */
const std::vector<int> &Unit::getBerserkSounds() const
{
	return _berserkSound;
}

/**
 * Gets the unit's move sound.
 * @return The id of the unit's move sound.
 */
int Unit::getMoveSound() const
{
	return _moveSound;
}

/**
 * Gets the intelligence. This is the number of turns the AI remembers your troop positions.
 * @return The unit's intelligence.
 */
int Unit::getIntelligence() const
{
	return _intelligence;
}

/**
 * Gets the aggression. Determines the chance of revenge and taking cover.
 * @return The unit's aggression.
 */
int Unit::getAggression() const
{
	return _aggression;
}

/**
 * Gets the spotter score. Determines how many turns sniper AI units can act on this unit seeing your troops.
 * @return The unit's spotter value.
 */
int Unit::getSpotterDuration() const
{
	// Lazy balance - use -1 to make this the same as intelligence value
	return (_spotter == -1) ? _intelligence : _spotter;
}

/**
 * Gets the sniper score. Determines the chances of firing from out of LOS on spotted units.
 * @return The unit's sniper value.
 */
int Unit::getSniperPercentage() const
{
	return _sniper;
}

/**
 * Gets the unit's special ability.
 * @return The unit's specab.
 */
int Unit::getSpecialAbility() const
{
	return (int)_specab;
}

/**
 * Gets the unit that is spawned when this one dies.
 * @return The unit's spawn unit.
 */
const Unit *Unit::getSpawnUnit() const
{
	return _spawnUnit;
}

/**
 * Gets the unit's war cry.
 * @return The id of the unit's aggro sound.
 */
int Unit::getAggroSound() const
{
	return _aggroSound;
}

/**
 * How much energy does this unit recover per turn?
 * @return energy recovery amount.
 */
int Unit::getEnergyRecovery() const
{
	return _energyRecovery;
}

/**
 * Checks if this unit is a living weapon.
 * a living weapon ignores any loadout that may be available to
 * its rank and uses the one associated with its race.
 * @return True if this unit is a living weapon.
 */
bool Unit::isLivingWeapon() const
{
	return _livingWeapon;
}

/**
 * What is this unit's built in melee weapon (if any).
 * @return the name of the weapon.
 */
const std::string &Unit::getMeleeWeapon() const
{
	return _meleeWeapon;
}

/**
* What is this unit's built in psi weapon (if any).
* @return the name of the weapon.
*/
const std::string &Unit::getPsiWeapon() const
{
	return _psiWeapon;
}

/**
 * What weapons does this unit have built in?
 * this is a vector of strings representing any
 * weapons that may be inherent to this creature.
 * note: unlike "livingWeapon" this is used in ADDITION to
 * any loadout or living weapon item that may be defined.
 * @return list of weapons that are integral to this unit.
 */
const std::vector<std::vector<const RuleItem*> > &Unit::getBuiltInWeapons() const
{
	return _builtInWeapons;
}

/**
* Gets whether the alien can be captured alive.
* @return a value determining whether the alien can be captured alive.
*/
bool Unit::getCapturable() const
{
	return _capturable;
}

/**
* Checks if this unit can surrender.
* @return True if this unit can surrender.
*/
bool Unit::canSurrender() const
{
	return _canSurrender || _autoSurrender;
}

/**
* Checks if this unit surrenders automatically, if all other units surrendered too.
* @return True if this unit auto-surrenders.
*/
bool Unit::autoSurrender() const
{
	return _autoSurrender;
}

/**
 * Should alien inventory show full name (e.g. Sectoid Leader) or just the race (e.g. Sectoid)?
 * @return True if full name can be shown.
 */
bool Unit::getShowFullNameInAlienInventory(Mod *mod) const
{
	if (_showFullNameInAlienInventory != -1)
	{
		return _showFullNameInAlienInventory == 0 ? false : true;
	}
	return mod->getShowFullNameInAlienInventory();
}


////////////////////////////////////////////////////////////
//					Script binding
////////////////////////////////////////////////////////////

namespace
{

void getTypeScript(const Unit* r, ScriptText& txt)
{
	if (r)
	{
		txt = { r->getType().c_str() };
		return;
	}
	else
	{
		txt = ScriptText::empty;
	}
}

std::string debugDisplayScript(const Unit* unit)
{
	if (unit)
	{
		std::string s;
		s += Unit::ScriptName;
		s += "(name: \"";
		s += unit->getType();
		s += "\")";
		return s;
	}
	else
	{
		return "null";
	}
}

} // namespace

void Unit::ScriptRegister(ScriptParserBase* parser)
{
	Bind<Unit> un = { parser };

	un.add<&getTypeScript>("getType");

	un.addDebugDisplay<&debugDisplayScript>();
}
/**
 * Register StatAdjustment in script parser.
 * @param parser Script parser.
 */
void StatAdjustment::ScriptRegister(ScriptParserBase* parser)
{
	Bind<StatAdjustment> sa = { parser };

	UnitStats::addGetStatsScript<&StatAdjustment::statGrowth>(sa, "", true);
}

} // namespace OpenXcom

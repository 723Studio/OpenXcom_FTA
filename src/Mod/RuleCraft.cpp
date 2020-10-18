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
#include <algorithm>
#include "RuleCraft.h"
#include "RuleTerrain.h"
#include "../Engine/Exception.h"
#include "Mod.h"

namespace OpenXcom
{

/**
 * Creates a blank ruleset for a certain
 * type of craft.
 * @param type String defining the type.
 */
RuleCraft::RuleCraft(const std::string &type) :
	_type(type), _sprite(-1), _marker(-1), _weapons(0), _soldiers(0), _pilots(0), _vehicles(0),
	_costBuy(0), _costRent(0), _costSell(0), _repairRate(1), _refuelRate(1),
	_transferTime(24), _score(0), _battlescapeTerrainData(0),
	_keepCraftAfterFailedMission(false), _allowLanding(true), _spacecraft(false), _notifyWhenRefueled(false), _autoPatrol(false),
	_listOrder(0), _maxItems(0), _maxAltitude(-1), _maxStorageSpace(0.0), _stats(),
	_shieldRechargeAtBase(1000),
	_mapVisible(true), _forceShowInMonthlyCosts(false)
{
	for (int i = 0; i < WeaponMax; ++ i)
	{
		for (int j = 0; j < WeaponTypeMax; ++j)
			_weaponTypes[i][j] = 0;
	}
	_stats.radarRange = 672;
	_stats.radarChance = 100;
	_stats.sightRange = 1696;
	_weaponStrings[0] = "STR_WEAPON_ONE";
	_weaponStrings[1] = "STR_WEAPON_TWO";
}

/**
 *
 */
RuleCraft::~RuleCraft()
{
	delete _battlescapeTerrainData;
}

/**
 * Loads the craft from a YAML file.
 * @param node YAML node.
 * @param mod Mod for the craft.
 * @param modIndex A value that offsets the sounds and sprite values to avoid conflicts.
 * @param listOrder The list weight for this craft.
 */
void RuleCraft::load(const YAML::Node &node, Mod *mod, int listOrder)
{
	if (const YAML::Node &parent = node["refNode"])
	{
		load(parent, mod, listOrder);
	}
	_type = node["type"].as<std::string>(_type);

	//requires
	mod->loadUnorderedNames(_type, _requires, node["requires"]);
	mod->loadBaseFunction(_type, _requiresBuyBaseFunc, node["requiresBuyBaseFunc"]);

	if (node["sprite"])
	{
		// used in
		// Surface set (baseOffset):
		//   BASEBITS.PCK (33)
		//   INTICON.PCK (11)
		//   INTICON.PCK (0)
		//
		// Final index in surfaceset is `baseOffset + sprite + (sprite > 4 ? modOffset : 0)`
		_sprite = mod->getOffset(node["sprite"].as<int>(_sprite), 4);
	}
	_stats.load(node);
	if (node["marker"])
	{
		_marker = mod->getOffset(node["marker"].as<int>(_marker), 8);
	}
	_weapons = node["weapons"].as<int>(_weapons);
	_soldiers = node["soldiers"].as<int>(_soldiers);
	_pilots = node["pilots"].as<int>(_pilots);
	_vehicles = node["vehicles"].as<int>(_vehicles);
	_costBuy = node["costBuy"].as<int>(_costBuy);
	_costRent = node["costRent"].as<int>(_costRent);
	_costSell = node["costSell"].as<int>(_costSell);
	_refuelItem = node["refuelItem"].as<std::string>(_refuelItem);
	_repairRate = node["repairRate"].as<int>(_repairRate);
	_refuelRate = node["refuelRate"].as<int>(_refuelRate);
	_transferTime = node["transferTime"].as<int>(_transferTime);
	_score = node["score"].as<int>(_score);
	if (const YAML::Node &terrain = node["battlescapeTerrainData"])
	{
		RuleTerrain *rule = new RuleTerrain(terrain["name"].as<std::string>());
		rule->load(terrain, mod);
		_battlescapeTerrainData = rule;
		if (const YAML::Node &craftInventoryTile = node["craftInventoryTile"])
		{
			_craftInventoryTile = craftInventoryTile.as<std::vector<int> >(_craftInventoryTile);
		}
	}
	_deployment = node["deployment"].as< std::vector< std::vector<int> > >(_deployment);
	_keepCraftAfterFailedMission = node["keepCraftAfterFailedMission"].as<bool>(_keepCraftAfterFailedMission);
	_allowLanding = node["allowLanding"].as<bool>(_allowLanding);
	_spacecraft = node["spacecraft"].as<bool>(_spacecraft);
	_notifyWhenRefueled = node["notifyWhenRefueled"].as<bool>(_notifyWhenRefueled);
	_autoPatrol = node["autoPatrol"].as<bool>(_autoPatrol);
	_listOrder = node["listOrder"].as<int>(_listOrder);
	if (!_listOrder)
	{
		_listOrder = listOrder;
	}
	_maxAltitude = node["maxAltitude"].as<int>(_maxAltitude);
	_maxItems = node["maxItems"].as<int>(_maxItems);
	_maxStorageSpace = node["maxStorageSpace"].as<double>(_maxStorageSpace);

	if (const YAML::Node &types = node["weaponTypes"])
	{
		for (int i = 0; (size_t)i < types.size() &&  i < WeaponMax; ++i)
		{
			const YAML::Node t = types[i];
			if (t.IsScalar())
			{
				for (int j = 0; j < WeaponTypeMax; ++j)
					_weaponTypes[i][j] = t.as<int>();
			}
			else if (t.IsSequence() && t.size() > 0)
			{
				for (int j = 0; (size_t)j < t.size() && j < WeaponTypeMax; ++j)
					_weaponTypes[i][j] = t[j].as<int>();
				for (int j = t.size(); j < WeaponTypeMax; ++j)
					_weaponTypes[i][j] = _weaponTypes[i][0];
			}
			else
			{
				throw Exception("Invalid weapon type in craft " + _type + ".");
			}
		}
	}
	if (const YAML::Node &str = node["weaponStrings"])
	{
		for (int i = 0; (size_t)i < str.size() &&  i < WeaponMax; ++i)
			_weaponStrings[i] = str[i].as<std::string>();
	}
	_shieldRechargeAtBase = node["shieldRechargedAtBase"].as<int>(_shieldRechargeAtBase);
	_mapVisible = node["mapVisible"].as<bool>(_mapVisible);
	_forceShowInMonthlyCosts = node["forceShowInMonthlyCosts"].as<bool>(_forceShowInMonthlyCosts);
}

/**
 * Gets the language string that names
 * this craft. Each craft type has a unique name.
 * @return The craft's name.
 */
const std::string &RuleCraft::getType() const
{
	return _type;
}

/**
 * Gets the list of research required to
 * acquire this craft.
 * @return The list of research IDs.
 */
const std::vector<std::string> &RuleCraft::getRequirements() const
{
	return _requires;
}

/**
 * Gets the ID of the sprite used to draw the craft
 * in the Basescape and Equip Craft screens.
 * @return The Sprite ID.
 */
int RuleCraft::getSprite() const
{
	return _sprite;
}

/**
 * Returns the globe marker for the craft type.
 * @return Marker sprite, -1 if none.
 */
int RuleCraft::getMarker() const
{
	return _marker;
}

/**
 * Gets the maximum fuel the craft can contain.
 * @return The fuel amount.
 */
int RuleCraft::getMaxFuel() const
{
	return _stats.fuelMax;
}

/**
 * Gets the maximum damage (damage the craft can take)
 * of the craft.
 * @return The maximum damage.
 */
int RuleCraft::getMaxDamage() const
{
	return _stats.damageMax;
}

/**
 * Gets the maximum speed of the craft flying
 * around the Geoscape.
 * @return The speed in knots.
 */
int RuleCraft::getMaxSpeed() const
{
	return _stats.speedMax;
}

/**
 * Gets the acceleration of the craft for
 * taking off / stopping.
 * @return The acceleration.
 */
int RuleCraft::getAcceleration() const
{
	return _stats.accel;
}

/**
 * Gets the maximum number of weapons that
 * can be equipped onto the craft.
 * @return The weapon capacity.
 */
int RuleCraft::getWeapons() const
{
	return _weapons;
}

/**
 * Gets the maximum number of soldiers that
 * the craft can carry.
 * @return The soldier capacity.
 */
int RuleCraft::getSoldiers() const
{
	return _soldiers;
}

/**
* Gets the number of pilots that the craft requires in order to take off.
* @return The number of pilots.
*/
int RuleCraft::getPilots() const
{
	return _pilots;
}

/**
 * Gets the maximum number of vehicles that
 * the craft can carry.
 * @return The vehicle capacity.
 */
int RuleCraft::getVehicles() const
{
	return _vehicles;
}

/**
 * Gets the cost of this craft for
 * purchase/rent (0 if not purchasable).
 * @return The cost.
 */
int RuleCraft::getBuyCost() const
{
	return _costBuy;
}

/**
 * Gets the cost of rent for a month.
 * @return The cost.
 */
int RuleCraft::getRentCost() const
{
	return _costRent;
}

/**
 * Gets the sell value of this craft
 * Rented craft should use 0.
 * @return The sell value.
 */
int RuleCraft::getSellCost() const
{
	return _costSell;
}

/**
 * Gets what item is required while
 * the craft is refuelling.
 * @return The item ID or "" if none.
 */
const std::string &RuleCraft::getRefuelItem() const
{
	return _refuelItem;
}

/**
 * Gets how much damage is removed from the
 * craft while repairing.
 * @return The amount of damage.
 */
int RuleCraft::getRepairRate() const
{
	return _repairRate;
}

/**
 * Gets how much fuel is added to the
 * craft while refuelling.
 * @return The amount of fuel.
 */
int RuleCraft::getRefuelRate() const
{
	return _refuelRate;
}

/**
 * Gets the craft's radar range
 * for detecting UFOs.
 * @return The range in nautical miles.
 */
int RuleCraft::getRadarRange() const
{
	return _stats.radarRange;
}

/**
 * Gets the craft's radar chance
 * for detecting UFOs.
 * @return The chance in percentage.
 */
int RuleCraft::getRadarChance() const
{
	return _stats.radarChance;
}

/**
 * Gets the craft's sight range
 * for detecting bases.
 * @return The range in nautical miles.
 */
int RuleCraft::getSightRange() const
{
	return _stats.sightRange;
}

/**
 * Gets the amount of time this item
 * takes to arrive at a base.
 * @return The time in hours.
 */
int RuleCraft::getTransferTime() const
{
	return _transferTime;
}

/**
 * Gets the number of points you lose
 * when this craft is destroyed.
 * @return The score in points.
 */
int RuleCraft::getScore() const
{
	return _score;
}

/**
 * Gets the terrain data needed to draw the Craft in the battlescape.
 * @return The terrain data.
 */
RuleTerrain *RuleCraft::getBattlescapeTerrainData() const
{
	return _battlescapeTerrainData;
}

/**
 * Checks if this craft is lost after a failed mission or not.
 * @return True if this craft is NOT lost (e.g. paratroopers).
 */
bool RuleCraft::keepCraftAfterFailedMission() const
{
	return _keepCraftAfterFailedMission;
}

/**
 * Checks if this craft is capable of landing (on missions).
 * @return True if this ship is capable of landing (on missions).
 */
bool RuleCraft::getAllowLanding() const
{
	return _allowLanding;
}

/**
 * Checks if this ship is capable of going to mars.
 * @return True if this ship is capable of going to mars.
 */
bool RuleCraft::getSpacecraft() const
{
	return _spacecraft;
}

/**
 * Checks if a notification should be displayed when the craft is refueled.
 * @return True if notification should appear.
 */
bool RuleCraft::notifyWhenRefueled() const
{
	return _notifyWhenRefueled;
}

/**
* Checks if the craft supports auto patrol feature.
* @return True if auto patrol is supported.
*/
bool RuleCraft::canAutoPatrol() const
{
	return _autoPatrol;
}

/**
 * Gets the list weight for this research item.
 * @return The list weight.
 */
int RuleCraft::getListOrder() const
{
	return _listOrder;
}

/**
 * Gets the deployment layout for this craft.
 * @return The deployment layout.
 */
const std::vector<std::vector<int> > &RuleCraft::getDeployment() const
{
	return _deployment;
}

/**
* Gets the craft inventory tile position.
* @return The tile position.
*/
const std::vector<int> &RuleCraft::getCraftInventoryTile() const
{
	return _craftInventoryTile;
}

/**
 * Gets the maximum amount of items this craft can store.
 * @return number of items.
 */
int RuleCraft::getMaxItems() const
{
	return _maxItems;
}

/**
 * Gets the maximum storage space for items in this craft.
 * @return storage space limit.
 */
double RuleCraft::getMaxStorageSpace() const
{
	return _maxStorageSpace;
}

/**
 * Test for possibility of usage of weapon type in weapon slot.
 * @param slot value less than WeaponMax.
 * @param weaponType weapon type of weapon that we try insert.
 * @return True if can use.
 */
bool RuleCraft::isValidWeaponSlot(int slot, int weaponType) const
{
	for (int j = 0; j < WeaponTypeMax; ++j)
	{
		if (_weaponTypes[slot][j] == weaponType)
			return true;
	}
	return false;
}

int RuleCraft::getWeaponTypesRaw(int slot, int subslot) const
{
	return _weaponTypes[slot][subslot];
}

/**
 * Return string ID of weapon slot name for geoscape craft state.
 * @param slot value less than WeaponMax.
 * @return String ID for translation.
 */
const std::string &RuleCraft::getWeaponSlotString(int slot) const
{
	return _weaponStrings[slot];
}
/**
 * Gets basic statistic of craft.
 * @return Basic stats of craft.
 */
const RuleCraftStats& RuleCraft::getStats() const
{
	return _stats;
}

/**
 * Gets the maximum altitude this craft can dogfight to.
 * @return max altitude (0-4).
 */
int RuleCraft::getMaxAltitude() const
{
	return _maxAltitude;
}

/**
 * If the craft is underwater, it can only dogfight over polygons.
 * TODO: Replace this with its own flag.
 * @return underwater or not
 */
bool RuleCraft::isWaterOnly() const
{
	return _maxAltitude > -1;
}

/**
 * Gets how many shield points are recharged when landed at base per hour
 * @return shield recharged per hour
 */
int RuleCraft::getShieldRechargeAtBase() const
{
	return _shieldRechargeAtBase;
}

/**
 * Gets whether or not the craft map should be visible at the beginning of the battlescape
 * @return visible or not?
 */
bool RuleCraft::isMapVisible() const
{
	return _mapVisible;
}

/**
 * Gets whether or not the craft type should be displayed in Monthly Costs even if not present in the base.
 * @return visible or not?
 */
bool RuleCraft::forceShowInMonthlyCosts() const
{
	return _forceShowInMonthlyCosts;
}

/**
 * Calculates the theoretical range of the craft
 * This depends on when you launch the craft as fuel is consumed only on exact 10 minute increments
 * @param type Which calculation should we do? 0 = maximum, 1 = minimum, 2 = average of the two
 * @return The calculated range
 */
int RuleCraft::calculateRange(int type)
{
	// If the craft uses an item to refuel, the tick rate is one fuel unit per 10 minutes
	int totalFuelTicks = _stats.fuelMax;

	// If no item is used to refuel, the tick rate depends on speed
	if (_refuelItem.empty())
	{
		// Craft with less than 100 speed don't consume fuel and therefore have infinite range
		if (_stats.speedMax < 100)
		{
			return -1;
		}

		totalFuelTicks = _stats.fuelMax / (_stats.speedMax / 100);
	}

	// Six ticks per hour, factor return trip and speed for total range
	int range;
	switch (type)
	{
		// Min range happens when the craft is sent at xx:x9:59, as a unit of fuel is immediately consumed, so we subtract an extra 'tick' of fuel in this case
		case 1:
			range = (totalFuelTicks - 1) * _stats.speedMax / 12;
			break;
		case 2:
			range = (2 * totalFuelTicks - 1) * _stats.speedMax / 12 / 2;
			break;
		default :
			range = totalFuelTicks * _stats.speedMax / 12;
			break;
	}

	return range;
}

}


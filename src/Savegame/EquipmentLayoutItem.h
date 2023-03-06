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
#include "../Mod/RuleItem.h"

namespace OpenXcom
{

class BattleItem;

/**
 * Represents a soldier-equipment layout item which is used
 * on the beginning of the Battlescape.
 */
class EquipmentLayoutItem
{
private:
	std::string _itemType;
	std::string _slot;
	int _slotX, _slotY;
	std::string _ammoItem[RuleItem::AmmoSlotMax];
	int _fuseTimer;
	bool _fixed;
	int _stackSize;
public:
	/// Creates a new soldier-equipment layout item and loads its contents from YAML.
	EquipmentLayoutItem(const YAML::Node& node);
	/// Creates a new soldier-equipment layout item.
	EquipmentLayoutItem(const BattleItem* item);
	/// Cleans up the soldier-equipment layout item.
	~EquipmentLayoutItem();
	/// Gets the item's type which has to be in a slot
	const std::string& getItemType() const;
	/// Gets the slot to be occupied
	const std::string& getSlot() const;
	/// Gets the slotX to be occupied
	int getSlotX() const;
	/// Gets the slotY to be occupied
	int getSlotY() const;
	/// Gets the ammo item
	const std::string& getAmmoItemForSlot(int i) const;
	/// Gets the turn until explosion
	int getFuseTimer() const;
	/// Is this a fixed weapon entry?
	bool isFixed() const;
	// Gets Stack size
	int stackSize() const;
	/// Loads the soldier-equipment layout item from YAML.
	void load(const YAML::Node& node);
	/// Saves the soldier-equipment layout item to YAML.
	YAML::Node save() const;
};

}

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
#include "ItemContainer.h"
#include "../Mod/Mod.h"
#include "../Mod/RuleItem.h"

namespace OpenXcom
{

/**
 * Initializes an item container with no contents.
 */
ItemContainer::ItemContainer()
{
}

/**
 *
 */
ItemContainer::~ItemContainer()
{
}

/**
 * Loads the item container from a YAML file.
 * @param node YAML node.
 */
void ItemContainer::load(const YAML::Node &node)
{
	_qty = node.as< std::map<std::string, int> >(_qty);
}

/**
 * Saves the item container to a YAML file.
 * @return YAML node.
 */
YAML::Node ItemContainer::save() const
{
	YAML::Node node;
	node = _qty;
	return node;
}

/**
 * Adds an item amount to the container.
 * @param id Item ID.
 * @param qty Item quantity.
 */
void ItemContainer::addItem(const std::string &id, int qty)
{
	if (Mod::isEmptyRuleName(id))
	{
		return;
	}
	_qty[id] += qty;
}

/**
 * Adds an item amount to the container.
 * @param id Item ID.
 * @param qty Item quantity.
 */
void ItemContainer::addItem(const RuleItem* item, int qty)
{
	if (item)
	{
		addItem(item->getType(), qty);
	}
}

/**
 * Removes an item amount from the container.
 * @param id Item ID.
 * @param qty Item quantity.
 */
void ItemContainer::removeItem(const std::string &id, int qty)
{
	if (Mod::isEmptyRuleName(id))
	{
		return;
	}
	auto it = _qty.find(id);
	if (it == _qty.end())
	{
		return;
	}

	if (qty < it->second)
	{
		it->second -= qty;
	}
	else
	{
		_qty.erase(it);
	}
}

/**
 * Removes an item amount from the container.
 * @param id Item ID.
 * @param qty Item quantity.
 */
void ItemContainer::removeItem(const RuleItem* item, int qty)
{
	if (item)
	{
		removeItem(item->getType(), qty);
	}
}

/**
 * Returns the quantity of an item in the container.
 * @param id Item ID.
 * @return Item quantity.
 */
int ItemContainer::getItem(const std::string &id) const
{
	if (Mod::isEmptyRuleName(id))
	{
		return 0;
	}

	auto it = _qty.find(id);
	if (it == _qty.end())
	{
		return 0;
	}
	else
	{
		return it->second;
	}
}

/**
 * Returns the quantity of an item in the container.
 * @param id Item ID.
 * @return Item quantity.
 */
int ItemContainer::getItem(const RuleItem* item) const
{
	if (item)
	{
		return getItem(item->getType());
	}
	else
	{
		return 0;
	}
}

/**
 * Returns the total quantity of the items in the container.
 * @return Total item quantity.
 */
int ItemContainer::getTotalQuantity() const
{
	int total = 0;
	for (const auto& pair : _qty)
	{
		total += pair.second;
	}
	return total;
}

/**
 * Returns the total size of the items in the container.
 * @param mod Pointer to mod.
 * @return Total item size.
 */
double ItemContainer::getTotalSize(const Mod *mod) const
{
	double total = 0;
	for (const auto& pair : _qty)
	{
		total += mod->getItem(pair.first, true)->getSize() * pair.second;
	}
	return total;
}

/**
 * Returns all the items currently contained within.
 * @return List of contents.
 */
std::map<std::string, int> *ItemContainer::getContents()
{
	return &_qty;
}

}

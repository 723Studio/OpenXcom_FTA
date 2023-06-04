#pragma once
/*
 * Copyright 2010-2015 OpenXcom Developers.
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

namespace OpenXcom
{

class Mod;

/**
 * Represents an item category.
 * Contains info about list order.
 */
class RuleItemCategory
{
private:
	std::string _type, _replaceBy;
	bool _hidden;
	int _listOrder;
public:
	/// Creates a blank item category ruleset.
	RuleItemCategory(const std::string &type, int listOrder);
	/// Cleans up the item category ruleset.
	~RuleItemCategory();
	/// Loads item data from YAML.
	void load(const YAML::Node& node, Mod *mod);
	/// Gets the item category type.
	const std::string &getType() const;
	/// Gets the item category type, which should be used instead of this one.
	const std::string &getReplaceBy() const;
	/// Indicates whether the category is hidden or visible.
	bool isHidden() const;
	/// Get the list weight for this item category.
	int getListOrder() const;

};

}

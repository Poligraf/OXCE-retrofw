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
#include "RuleItemCategory.h"
#include "Mod.h"

namespace OpenXcom
{

/**
 * Creates a blank ruleset for a certain item category.
 * @param type String defining the item category.
 */
RuleItemCategory::RuleItemCategory(const std::string &type) : _type(type), _hidden(false), _listOrder(0)
{
}

/**
 *
 */
RuleItemCategory::~RuleItemCategory()
{
}

/**
 * Loads the item category from a YAML file.
 * @param node YAML node.
 * @param mod Mod for the item.
 * @param listOrder The list weight for this item.
 */
void RuleItemCategory::load(const YAML::Node &node, Mod *mod, int listOrder)
{
	if (const YAML::Node &parent = node["refNode"])
	{
		load(parent, mod, listOrder);
	}
	_type = node["type"].as<std::string>(_type);
	_replaceBy = node["replaceBy"].as<std::string>(_replaceBy);
	_hidden = node["hidden"].as<bool>(_hidden);

	_listOrder = node["listOrder"].as<int>(_listOrder);

	if (!_listOrder)
	{
		_listOrder = listOrder;
	}
}

/**
 * Gets the item category type. Each category has a unique type.
 * @return The category's type.
 */
const std::string &RuleItemCategory::getType() const
{
	return _type;
}

/**
* Gets the item category type, which should be used instead of this one.
* @return The replacement category's type.
*/
const std::string &RuleItemCategory::getReplaceBy() const
{
	return _replaceBy;
}

/**
* Indicates whether the category is hidden or visible.
* @return True if hidden, false if visible.
*/
bool RuleItemCategory::isHidden() const
{
	return _hidden;
}

/**
 * Gets the list weight for this item category.
 * @return The list weight.
 */
int RuleItemCategory::getListOrder() const
{
	return _listOrder;
}

}

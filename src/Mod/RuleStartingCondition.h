#pragma once
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
#include <map>
#include <vector>
#include <string>
#include <yaml-cpp/yaml.h>

namespace OpenXcom
{
class Mod;
class Armor;
class Craft;

/**
 * Represents a specific Starting Condition.
 */
class RuleStartingCondition
{
private:
	std::string _type;
	std::map<std::string, std::map<std::string, int> > _defaultArmor;
	std::vector<std::string> _allowedArmors, _forbiddenArmors;
	std::vector<std::string> _allowedVehicles, _forbiddenVehicles;
	std::vector<std::string> _allowedItems, _forbiddenItems;
	std::vector<std::string> _allowedItemCategories, _forbiddenItemCategories;
	std::vector<std::string> _allowedCraft, _forbiddenCraft;
	std::vector<std::string> _allowedSoldierTypes, _forbiddenSoldierTypes;
	std::map<std::string, int> _requiredItems;
	bool _destroyRequiredItems;
public:
	/// Creates a blank Starting Conditions ruleset.
	RuleStartingCondition(const std::string& type);
	/// Cleans up the Starting Conditions ruleset.
	~RuleStartingCondition();
	/// Loads Starting Conditions data from YAML.
	void load(const YAML::Node& node, Mod *mod);
	/// Gets the Starting Conditions's type.
	const std::string& getType() const { return _type; }
	/// Gets the allowed armor types.
	const std::vector<std::string>& getAllowedArmors() const { return _allowedArmors; }
	/// Gets the forbidden armor types.
	const std::vector<std::string>& getForbiddenArmors() const { return _forbiddenArmors; }
	/// Gets the allowed craft types.
	const std::vector<std::string>& getAllowedCraft() const { return _allowedCraft; }
	/// Gets the forbidden craft types.
	const std::vector<std::string>& getForbiddenCraft() const { return _forbiddenCraft; }
	/// Gets the allowed soldier types.
	const std::vector<std::string>& getAllowedSoldierTypes() const { return _allowedSoldierTypes; }
	/// Gets the forbidden soldier types.
	const std::vector<std::string>& getForbiddenSoldierTypes() const { return _forbiddenSoldierTypes; }
	/// Gets the required items.
	const std::map<std::string, int>& getRequiredItems() const { return _requiredItems; }
	/// Should the required items be destroyed when the mission starts?
	bool getDestroyRequiredItems() const { return _destroyRequiredItems; }
	/// Checks if the craft type is permitted.
	bool isCraftPermitted(const std::string& craftType) const;
	/// Checks if the soldier type is permitted.
	bool isSoldierTypePermitted(const std::string& soldierType) const;
	/// Gets the replacement armor.
	std::string getArmorReplacement(const std::string& soldierType, const std::string& armorType) const;
	/// Checks if the vehicle type is permitted.
	bool isVehiclePermitted(const std::string& vehicleType) const;
	/// Checks if the item type is permitted.
	bool isItemPermitted(const std::string& itemType, Mod* mod, Craft* craft) const;
};

}

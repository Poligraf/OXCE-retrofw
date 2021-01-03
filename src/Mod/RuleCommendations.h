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
#include <yaml-cpp/yaml.h>

namespace OpenXcom
{

class Mod;
class RuleSoldierBonus;

/**
 * Represents a specific type of commendation.
 * Contains constant info about a commendation like
 * award criteria, sprite, description, etc.
 * @sa Commendation
 */
class RuleCommendations
{
private:
	std::map<std::string, std::vector<int> > _criteria;
	std::vector<std::vector<std::pair<int, std::vector<std::string> > > > _killCriteria;
	std::string _description;
	int _sprite;
	std::vector<std::string> _soldierBonusTypesNames;
	std::vector<const RuleSoldierBonus*> _soldierBonusTypes;

public:
	/// Creates a blank commendation ruleset.
	RuleCommendations();
	/// Cleans up the commendation ruleset.
	~RuleCommendations();
	/// Loads commendation data from YAML.
	void load(const YAML::Node& node);
	/// Cross link with other rules.
	void afterLoad(const Mod* mod);
	/// Get the commendation's description.
	std::string getDescription() const;
	/// Get the commendation's award criteria.
	std::map<std::string, std::vector<int> > *getCriteria();
	/// Get the commendation's award kill related criteria.
	std::vector<std::vector<std::pair<int, std::vector<std::string> > > > *getKillCriteria();
	/// Get the commendation's sprite.
	int getSprite() const;
	/// Gets the soldier bonus type corresponding to the commendation's decoration level.
	const RuleSoldierBonus *getSoldierBonus(int decorationLevel) const;

};

}

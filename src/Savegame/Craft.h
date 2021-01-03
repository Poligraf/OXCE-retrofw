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
#include "MovingTarget.h"
#include <utility>
#include <vector>
#include <string>
#include "../Mod/RuleCraft.h"
#include "../Engine/Script.h"

namespace OpenXcom
{

typedef std::pair<std::string, int> CraftId;

class RuleCraft;
class RuleItem;
class Base;
class Soldier;
class CraftWeapon;
class ItemContainer;
class Mod;
class SavedGame;
class Vehicle;
class RuleStartingCondition;
class ScriptParserBase;
class ScriptGlobal;

enum UfoDetection : int;

/**
 * Represents a craft stored in a base.
 * Contains variable info about a craft like
 * position, fuel, damage, etc.
 * @sa RuleCraft
 */
class Craft : public MovingTarget
{
public:
	/// Name of class used in script.
	static constexpr const char *ScriptName = "Craft";
	/// Register all useful function used by script.
	static void ScriptRegister(ScriptParserBase* parser);


private:
	const RuleCraft *_rules;
	Base *_base;
	int _fuel, _damage, _shield, _interceptionOrder, _takeoff;
	std::vector<CraftWeapon*> _weapons;
	ItemContainer *_items;
	std::vector<Vehicle*> _vehicles;
	std::string _status;
	bool _lowFuel, _mission, _inBattlescape, _inDogfight;
	double _speedMaxRadian;
	RuleCraftStats _stats;
	bool _isAutoPatrolling;
	double _lonAuto, _latAuto;
	std::vector<int> _pilots;
	int _skinIndex;
	ScriptValues<Craft> _scriptValues;

	void recalcSpeedMaxRadian();

	using MovingTarget::load;
public:
	/// Creates a craft of the specified type.
	Craft(const RuleCraft *rules, Base *base, int id = 0);
	/// Cleans up the craft.
	~Craft();
	/// Loads the craft from YAML.
	void load(const YAML::Node& node, const ScriptGlobal *shared, const Mod *mod, SavedGame *save);
	/// Finishes loading the craft from YAML (called after all other XCOM craft are loaded too).
	void finishLoading(const YAML::Node& node, SavedGame *save);
	/// Saves the craft to YAML.
	YAML::Node save(const ScriptGlobal *shared) const;
	/// Loads a craft ID from YAML.
	static CraftId loadId(const YAML::Node &node);
	/// Gets the craft's type.
	std::string getType() const override;
	/// Gets the craft's ruleset.
	const RuleCraft *getRules() const;
	/// Sets the craft's ruleset.
	void changeRules(RuleCraft *rules);
	/// Gets the craft's default name.
	std::string getDefaultName(Language *lang) const override;
	/// Gets the craft's marker sprite.
	int getMarker() const override;
	/// Gets the craft's base.
	Base *getBase() const;
	/// Sets the craft's base.
	void setBase(Base *base, bool move = true);
	/// Gets the craft's status.
	std::string getStatus() const;
	/// Sets the craft's status.
	void setStatus(const std::string &status);
	/// Gets the craft's altitude.
	std::string getAltitude() const;
	/// Sets the craft's destination.
	void setDestination(Target *dest) override;
	/// Gets whether the craft is on auto patrol.
	bool getIsAutoPatrolling() const;
	/// Sets whether the craft is on auto patrol.
	void setIsAutoPatrolling(bool isAuto);
	/// Gets the auto patrol longitude.
	double getLongitudeAuto() const;
	/// Sets the auto patrol longitude.
	void setLongitudeAuto(double lon);
	/// Gets the auto patrol latitude.
	double getLatitudeAuto() const;
	/// Sets the auto patrol latitude.
	void setLatitudeAuto(double lat);
	/// Gets the craft's amount of weapons.
	int getNumWeapons(bool onlyLoaded = false) const;
	/// Gets the craft's amount of soldiers.
	int getNumSoldiers() const;
	/// Gets the craft's amount of equipment.
	int getNumEquipment() const;
	/// Gets the craft's amount of vehicles.
	int getNumVehicles() const;
	/// Gets the craft's weapons.
	std::vector<CraftWeapon*> *getWeapons();
	/// Gets the craft's items.
	ItemContainer *getItems();
	/// Gets the craft's vehicles.
	std::vector<Vehicle*> *getVehicles();

	/// Gets the total storage size of all items in the craft. Including vehicles+ammo and craft weapons+ammo.
	double getTotalItemStorageSize(const Mod* mod) const;
	/// Gets the total number of items of a given type in the craft. Including vehicles+ammo and craft weapons+ammo.
	int getTotalItemCount(const RuleItem* item) const;

	/// Update the craft's stats.
	void addCraftStats(const RuleCraftStats& s);
	/// Gets the craft's stats.
	const RuleCraftStats& getCraftStats() const;
	/// Gets the craft's max amount of fuel.
	int getFuelMax() const;
	/// Gets the craft's amount of fuel.
	int getFuel() const;
	/// Sets the craft's amount of fuel.
	void setFuel(int fuel);
	/// Gets the craft's percentage of fuel.
	int getFuelPercentage() const;
	/// Gets the craft's max amount of damage.
	int getDamageMax() const;
	/// Gets the craft's amount of damage.
	int getDamage() const;
	/// Sets the craft's amount of damage.
	void setDamage(int damage);
	/// Gets the craft's percentage of damage.
	int getDamagePercentage() const;
	/// Gets the craft's max shield capacity
	int getShieldCapacity () const;
	/// Gets the craft's shield remaining
	int getShield() const;
	/// Sets the craft's shield remaining
	void setShield(int shield);
	/// Gets the percent shield remaining
	int getShieldPercentage() const;
	/// Gets whether the craft is running out of fuel.
	bool getLowFuel() const;
	/// Sets whether the craft is running out of fuel.
	void setLowFuel(bool low);
	/// Gets whether the craft has just finished a mission.
	bool getMissionComplete() const;
	/// Sets whether the craft has just finished a mission.
	void setMissionComplete(bool mission);
	/// Gets the craft's distance from its base.
	double getDistanceFromBase() const;
	/// Gets the craft's fuel consumption at a certain speed.
	int getFuelConsumption(int speed, int escortSpeed) const;
	/// Gets the craft's minimum fuel limit.
	int getFuelLimit() const;
	/// Gets the craft's minimum fuel limit to go to a base.
	int getFuelLimit(Base *base) const;

	double getBaseRange() const;
	/// Returns the craft to its base.
	void returnToBase();
	/// Returns the crew to their base (using transfers).
	void evacuateCrew(const Mod *mod);
	/// Checks if a target is detected by the craft's radar.
	UfoDetection detect(const Ufo *target, bool alreadyTracked) const;
	/// Handles craft logic.
	bool think();
	/// Does a craft full checkup.
	void checkup();
	/// Consumes the craft's fuel.
	void consumeFuel(int escortSpeed);
	/// Calculates the time to repair
	unsigned int calcRepairTime();
	/// Calculates the time to refuel
	unsigned int calcRefuelTime();
	/// Calculates the time to rearm
	unsigned int calcRearmTime();
	/// Repairs the craft.
	void repair();
	/// Refuels the craft.
	void refuel();
	/// Rearms the craft.
	const RuleItem* rearm();
	/// Sets the craft's battlescape status.
	void setInBattlescape(bool inbattle);
	/// Gets if the craft is in battlescape.
	bool isInBattlescape() const;
	/// Gets if craft is destroyed during dogfights.
	bool isDestroyed() const;
	/// Gets the amount of space available inside a craft.
	int getSpaceAvailable() const;
	/// Gets the amount of space used inside a craft.
	int getSpaceUsed() const;
	/// Checks if there are only permitted soldier types onboard.
	bool areOnlyPermittedSoldierTypesOnboard(const RuleStartingCondition* sc);
	/// Checks if there are enough required items onboard.
	bool areRequiredItemsOnboard(const std::map<std::string, int>& requiredItems);
	/// Destroys given required items.
	void destroyRequiredItems(const std::map<std::string, int>& requiredItems);
	/// Checks if there are enough pilots onboard.
	bool arePilotsOnboard();
	/// Checks if a pilot is already on the list.
	bool isPilot(int pilotId);
	/// Adds a pilot to the list.
	void addPilot(int pilotId);
	/// Removes all pilots from the list.
	void removeAllPilots();
	/// Gets the list of craft pilots.
	const std::vector<Soldier*> getPilotList(bool autoAdd);
	/// Calculates the accuracy bonus based on pilot skills.
	int getPilotAccuracyBonus(const std::vector<Soldier*> &pilots, const Mod *mod) const;
	/// Calculates the dodge bonus based on pilot skills.
	int getPilotDodgeBonus(const std::vector<Soldier*> &pilots, const Mod *mod) const;
	/// Calculates the approach speed modifier based on pilot skills.
	int getPilotApproachSpeedModifier(const std::vector<Soldier*> &pilots, const Mod *mod) const;
	/// Gets the craft's vehicles of a certain type.
	int getVehicleCount(const std::string &vehicle) const;
	/// Sets the craft's dogfight status.
	void setInDogfight(const bool inDogfight);
	/// Gets if the craft is in dogfight.
	bool isInDogfight() const;
	/// Sets interception order (first craft to leave the base gets 1, second 2, etc.).
	void setInterceptionOrder(const int order);
	/// Gets interception number.
	int getInterceptionOrder() const;
	/// Gets the craft's unique id.
	CraftId getUniqueId() const;
	/// Unloads the craft.
	void unload();
	/// Reuses a base item.
	void reuseItem(const RuleItem* item);
	/// Gets the attraction value of the craft for alien hunter-killers.
	int getHunterKillerAttraction(int huntMode) const;
	/// Gets the craft's skin index.
	int getSkinIndex() const { return _skinIndex; }
	/// Sets the craft's skin index.
	void setSkinIndex(int skinIndex) { _skinIndex = skinIndex; }
	/// Gets the craft's skin sprite ID.
	int getSkinSprite() const;
};

}

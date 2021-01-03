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
#include <vector>
#include "../Mod/RuleTerrain.h"
#include "../Mod/MapScript.h"

namespace OpenXcom
{

class SavedBattleGame;
class Mod;
class Craft;
class RuleCraft;
class Ufo;
class BattleItem;
class MapBlock;
class Vehicle;
class Tile;
class RuleInventory;
class RuleItem;
class Unit;
class AlienRace;
class RuleEnviroEffects;
class RuleStartingCondition;
class AlienDeployment;
class Game;
class Base;
class MissionSite;
class AlienBase;
class BattleUnit;
class Texture;
class Position;

/**
 * A utility class that generates the initial battlescape data. Taking into account mission type, craft and ufo involved, terrain type,...
 */
class BattlescapeGenerator
{
private:
	Game *_game;
	SavedBattleGame *_save;
	Mod *_mod;
	RuleInventory *_inventorySlotGround = nullptr;
	Craft *_craft;
	const RuleCraft *_craftRules;
	Ufo *_ufo;
	Base *_base;
	MissionSite *_mission;
	AlienBase *_alienBase;
	RuleTerrain *_terrain, *_baseTerrain, *_globeTerrain, *_alternateTerrain;
	int _mapsize_x, _mapsize_y, _mapsize_z;
	Texture *_missionTexture, *_globeTexture;
	int _worldShade;
	int _unitSequence;
	Tile *_craftInventoryTile;
	std::string _alienRace;
	const AlienDeployment *_alienCustomDeploy, *_alienCustomMission;
	int _alienItemLevel;
	int _ufoDamagePercentage;
	bool _allowAutoLoadout, _baseInventory, _generateFuel, _craftDeployed, _ufoDeployed;
	int _craftZ;
	SDL_Rect _craftPos;
	std::vector<SDL_Rect> _ufoPos;
	std::vector<int> _ufoZ;
	int _markAsReinforcementsBlock;
	int _blocksToDo;
	std::vector< std::vector<MapBlock*> > _blocks;
	std::vector< std::vector<bool> > _landingzone;
	std::vector< std::vector<int> > _segments, _drillMap;
	MapBlock *_dummy;
	std::vector<SDL_Rect> _placedBlockRects;
	std::vector<VerticalLevel> _verticalLevels;
	std::map<RuleTerrain*, int> _loadedTerrains;
	std::vector<std::pair<MapBlock*, Position> > _verticalLevelSegments;

	/// sets the map size and associated vars
	void init(bool resetTerrain);
	/// Generates a new battlescape map.
	void generateMap(const std::vector<MapScript*> *script, const std::string &customUfoName);
	/// Adds a vehicle to the game.
	BattleUnit *addXCOMVehicle(Vehicle *v);
	/// Adds a soldier to the game.
	BattleUnit *addXCOMUnit(BattleUnit *unit);
	/// Adds an alien to the game.
	BattleUnit *addAlien(Unit *rules, int alienRank, bool outside);
	/// Adds a civilian to the game.
	BattleUnit *addCivilian(Unit *rules, int nodeRank);
	/// Places an item on a soldier based on equipment layout.
	bool placeItemByLayout(BattleItem *item, const std::vector<BattleItem*> &itemList);
	/// Loads an XCom MAP file.
	int loadMAP(MapBlock *mapblock, int xoff, int yoff, int zoff, RuleTerrain *terrain, int objectIDOffset, bool discovered = false, bool craft = false, int ufoIndex = -1);
	/// Loads an XCom RMP file.
	void loadRMP(MapBlock *mapblock, int xoff, int yoff, int zoff, int segment);
	/// Checks a terrain requested by a command and loads it if necessary
	int loadExtraTerrain(RuleTerrain *terrain);
	/// Fills power sources with an alien fuel object.
	void fuelPowerSources();
	/// Possibly explodes ufo power sources.
	void explodePowerSources();
	/// Deploys the XCOM units on the mission.
	void deployXCOM(const RuleStartingCondition* startingCondition, const RuleEnviroEffects* enviro);
	/// Runs necessary checks before physically setting the position.
	bool canPlaceXCOMUnit(Tile *tile);
	/// Deploys the aliens, according to the alien deployment rules.
	void deployAliens(const AlienDeployment *deployment);
	/// Spawns civilians on a terror mission.
	void deployCivilians(bool markAsVIP, int nodeRank, int max, bool roundUp = false, const std::string &civilianType = "");
	/// Finds a spot near a friend to spawn at.
	bool placeUnitNearFriend(BattleUnit *unit);
	/// Load all Xcom weapons.
	void loadWeapons(const std::vector<BattleItem*> &itemList);
	/// Adds a craft (either a ufo or an xcom craft) somewhere on the map.
	bool addCraft(MapBlock *craftMap, MapScript *command, SDL_Rect &craftPos, RuleTerrain *terrain);
	/// Adds a line (generally a road) to the map.
	bool addLine(MapDirection lineType, const std::vector<SDL_Rect*> *rects, RuleTerrain *terrain, int verticalGroup, int horizontalGroup, int crossingGroup);
	/// Adds a single block at a given position.
	bool addBlock(int x, int y, MapBlock *block, RuleTerrain *terrain);
	/// Load the nodes from the associated map blocks.
	void loadNodes();
	/// Connects all the nodes together.
	void attachNodeLinks();
	/// Selects an unused position on the map of a given size.
	bool selectPosition(const std::vector<SDL_Rect *> *rects, int &X, int &Y, int sizeX, int sizeY);
	/// Generates a map from base modules.
	void generateBaseMap();
	/// Populates _verticalLevels vector according to a mapscript command and sorts them for use
	bool populateVerticalLevels(MapScript *command);
	/// Gets a terrain from a terrain name for a command or a vertical level
	RuleTerrain* pickTerrain(std::string terrainName);
	/// Loads the maps from the _verticalLevels vector
	void loadVerticalLevels(MapScript *command, bool repopulate = false, MapBlock *craftMap = 0);
	/// Clears a module from the map.
	void clearModule(int x, int y, int sizeX, int sizeY);
	/// Drills some tunnels between map blocks.
	void drillModules(TunnelData* data, const std::vector<SDL_Rect *> *rects, MapDirection dir, RuleTerrain* terrain);
	/// Clears all modules in a rect from a command.
	bool removeBlocks(MapScript *command);
	/// Sets the depth based on the terrain or the provided AlienDeployment rule.
	void setDepth(const AlienDeployment* ruleDeploy, bool nextStage);
	/// Sets the background music based on the terrain or the provided AlienDeployment rule.
	void setMusic(const AlienDeployment* ruleDeploy, bool nextStage);
public:
	/// Creates a new BattlescapeGenerator class
	BattlescapeGenerator(Game* game);
	/// Cleans up the BattlescapeGenerator.
	~BattlescapeGenerator();
	/// Sets the XCom craft.
	void setCraft(Craft *craft);
	/// Sets the ufo.
	void setUfo(Ufo* ufo);
	/// Sets the polygon texture.
	void setWorldTexture(Texture *missionTexture, Texture *globeTexture);
	/// Sets the polygon shade.
	void setWorldShade(int shade);
	/// Sets the alien race.
	void setAlienRace(const std::string &alienRace);
	/// Sets the alien item level.
	void setAlienItemlevel(int alienItemLevel);
	/// Sets the UFO damage percentage.
	void setUfoDamagePercentage(int ufoDamagePercentage);
	/// Sets the alien weapon deploy items.
	void setAlienCustomDeploy(const AlienDeployment *alienCustomDeploy = 0, const AlienDeployment* alienCustomBase = 0);
	/// Sets the XCom base.
	void setBase(Base *base);
	/// Sets the mission site.
	void setMissionSite(MissionSite* mission);
	/// Sets the alien base
	void setAlienBase(AlienBase* base);
	/// Sets the terrain.
	void setTerrain(RuleTerrain *terrain);
	/// Runs the generator.
	void run();
	/// Sets up the next stage (for Cydonia/TFTD missions).
	void nextStage();
	/// Generates an inventory battlescape.
	void runInventory(Craft *craft);
	/// Sets up the objectives for the map.
	void setupObjectives(const AlienDeployment *ruleDeploy);
	// Auto-equip a set of units
	static void autoEquip(std::vector<BattleUnit*> units, Mod *mod, std::vector<BattleItem*> *craftInv,
		RuleInventory *groundRuleInv, int worldShade, bool allowAutoLoadout, bool overrideEquipmentLayout);
};

}

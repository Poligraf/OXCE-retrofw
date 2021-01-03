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

#include "MapScript.h"
#include <yaml-cpp/yaml.h>
#include "../Engine/RNG.h"
#include "../Engine/Exception.h"
#include "../Engine/Logger.h"
#include "../Mod/RuleTerrain.h"


namespace OpenXcom
{

MapScript::MapScript() :
	_type(MSC_UNDEFINED), _canBeSkipped(true), _markAsReinforcementsBlock(false),
	_verticalGroup(MT_NSROAD), _horizontalGroup(MT_EWROAD), _crossingGroup(MT_CROSSING),
	_sizeX(1), _sizeY(1), _sizeZ(0),
	_executionChances(100), _executions(1), _cumulativeFrequency(0), _label(0),
	_direction(MD_NONE),
	_tunnelData(0), _randomTerrain(), _verticalLevels()
{
}

MapScript::~MapScript()
{
	for (std::vector<SDL_Rect*>::iterator i = _rects.begin(); i != _rects.end();++i)
	{
		delete *i;
	}
	delete _tunnelData;
}

/**
 * Loads a map script command from YAML.
 * @param node the YAML node from which to read.
 */
void MapScript::load(const YAML::Node& node)
{
	std::string command;
	if (const YAML::Node &map = node["type"])
	{
		command = map.as<std::string>("");
		if (command == "addBlock")
			_type = MSC_ADDBLOCK;
		else if (command == "addLine")
			_type = MSC_ADDLINE;
		else if (command == "addCraft")
		{
			_type = MSC_ADDCRAFT;
			_groups.push_back(1); // this is a default, and can be overridden
		}
		else if (command == "addUFO")
		{
			_type = MSC_ADDUFO;
			_groups.push_back(1); // this is a default, and can be overridden
		}
		else if (command == "digTunnel")
			_type = MSC_DIGTUNNEL;
		else if (command == "fillArea")
			_type = MSC_FILLAREA;
		else if (command == "checkBlock")
			_type = MSC_CHECKBLOCK;
		else if (command == "removeBlock")
			_type = MSC_REMOVE;
		else if (command == "resize")
		{
			_type = MSC_RESIZE;
			_sizeX = _sizeY = 0; // defaults: don't resize anything unless specified.
		}
		else
		{
			throw Exception("Unknown command: " + command);
		}
	}
	else
	{
		throw Exception("Missing command type.");
	}

	if (const YAML::Node &map = node["rects"])
	{
		for (YAML::const_iterator i = map.begin(); i != map.end(); ++i)
		{
			SDL_Rect *rect = new SDL_Rect();
			rect->x = (*i)[0].as<int>();
			rect->y = (*i)[1].as<int>();
			rect->w = (*i)[2].as<int>();
			rect->h = (*i)[3].as<int>();
			_rects.push_back(rect);
		}
	}
	if (const YAML::Node &map = node["tunnelData"])
	{
		_tunnelData = new TunnelData;
		_tunnelData->level = map["level"].as<int>(0);
		if (const YAML::Node &data = map["MCDReplacements"])
		{
			for (YAML::Node::const_iterator i = data.begin(); i != data.end(); ++i)
			{
				MCDReplacement replacement;
				std::string type = (*i)["type"].as<std::string>("");
				replacement.entry = (*i)["entry"].as<int>(-1);
				replacement.set = (*i)["set"].as<int>(-1);
				_tunnelData->replacements[type] = replacement;
			}
		}
	}
	if (const YAML::Node &map = node["conditionals"])
	{
		if (map.Type() == YAML::NodeType::Sequence)
		{
			_conditionals = map.as<std::vector<int> >(_conditionals);
		}
		else
		{
			_conditionals.push_back(map.as<int>(0));
		}
	}
	if (const YAML::Node &map = node["size"])
	{
		if (map.Type() == YAML::NodeType::Sequence)
		{
			int *sizes[3] = {&_sizeX, &_sizeY, &_sizeZ};
			int entry = 0;
			for (YAML::const_iterator i = map.begin(); i != map.end(); ++i)
			{
				*sizes[entry] = (*i).as<int>(1);
				entry++;
				if (entry == 3)
				{
					break;
				}
			}
		}
		else
		{
			_sizeX = map.as<int>(_sizeX);
			_sizeY = _sizeX;
		}
	}

	if (const YAML::Node &map = node["groups"])
	{
		_groups.clear();
		if (map.Type() == YAML::NodeType::Sequence)
		{
			for (YAML::const_iterator i = map.begin(); i != map.end(); ++i)
			{
				_groups.push_back((*i).as<int>(0));
			}
		}
		else
		{
			_groups.push_back(map.as<int>(0));
		}
	}
	size_t selectionSize = _groups.size();
	if (const YAML::Node &map = node["blocks"])
	{
		_groups.clear();
		if (map.Type() == YAML::NodeType::Sequence)
		{
			for (YAML::const_iterator i = map.begin(); i != map.end(); ++i)
			{
				_blocks.push_back((*i).as<int>(0));
			}
		}
		else
		{
			_blocks.push_back(map.as<int>(0));
		}
		selectionSize = _blocks.size();
	}

	_frequencies.resize(selectionSize, 1);
	_maxUses.resize(selectionSize, -1);

	if (const YAML::Node &map = node["freqs"])
	{
		if (map.Type() == YAML::NodeType::Sequence)
		{
			size_t entry = 0;
			for (YAML::const_iterator i = map.begin(); i != map.end(); ++i)
			{
				if (entry == selectionSize)
					break;
				_frequencies.at(entry) = (*i).as<int>(1);
				entry++;
			}
		}
		else
		{
			_frequencies.at(0) = map.as<int>(1);
		}
	}
	if (const YAML::Node &map = node["maxUses"])
	{
		if (map.Type() == YAML::NodeType::Sequence)
		{
			size_t entry = 0;
			for (YAML::const_iterator i = map.begin(); i != map.end(); ++i)
			{
				if (entry == selectionSize)
					break;
				_maxUses.at(entry) = (*i).as<int>(-1);
				entry++;
			}
		}
		else
		{
			_maxUses.at(0) = map.as<int>(-1);
		}
	}

	if (const YAML::Node &map = node["direction"])
	{
		std::string direction = map.as<std::string>("");
		if (!direction.empty())
		{
			char dir = toupper(direction[0]);
			switch (dir)
			{
			case 'V':
				_direction = MD_VERTICAL;
				break;
			case 'H':
				_direction = MD_HORIZONTAL;
				break;
			case 'B':
				_direction = MD_BOTH;
				break;
			default:
				throw Exception("direction must be [V]ertical, [H]orizontal, or [B]oth, what does " + direction + " mean?");
			}
		}
	}

	if (_direction == MD_NONE)
	{
		if (_type == MSC_DIGTUNNEL || _type == MSC_ADDLINE)
		{
			throw Exception("no direction defined for " + command + " command, must be [V]ertical, [H]orizontal, or [B]oth");
		}
	}


	_verticalGroup = node["verticalGroup"].as<int>(_verticalGroup);
	_horizontalGroup = node["horizontalGroup"].as<int>(_horizontalGroup);
	_crossingGroup = node["crossingGroup"].as<int>(_crossingGroup);
	_canBeSkipped = node["canBeSkipped"].as<bool>(_canBeSkipped);
	_markAsReinforcementsBlock = node["markAsReinforcementsBlock"].as<bool>(_markAsReinforcementsBlock);
	_executionChances = node["executionChances"].as<int>(_executionChances);
	_executions = node["executions"].as<int>(_executions);
	_ufoName = node["UFOName"].as<std::string>(_ufoName);
	_craftName = node["craftName"].as<std::string>(_craftName);
	if (node["terrain"])
	{
		_randomTerrain.clear();
		_randomTerrain.push_back(node["terrain"].as<std::string>());
	}
	_randomTerrain = node["randomTerrain"].as<std::vector<std::string> >(_randomTerrain);
	// take no chances, don't accept negative values here.
	_label = std::abs(node["label"].as<int>(_label));

	// Load any VerticalLevels into a map if we have them
	if (node["verticalLevels"])
	{
		_verticalLevels.clear();
		for (YAML::const_iterator i = node["verticalLevels"].begin(); i != node["verticalLevels"].end(); ++i)
		{
			if ((*i)["type"])
			{
				VerticalLevel level;
				level.load(*i);
				_verticalLevels.push_back(level);
			}
		}
	}
}

/**
 * Initializes all the various scratch values and such for the command.
 */
void MapScript::init()
{
	_cumulativeFrequency = 0;
	_blocksTemp.clear();
	_groupsTemp.clear();
	_frequenciesTemp.clear();
	_maxUsesTemp.clear();

	for (std::vector<int>::const_iterator i = _frequencies.begin(); i != _frequencies.end(); ++i)
	{
		_cumulativeFrequency += *i;
	}
	_blocksTemp = _blocks;
	_groupsTemp = _groups;
	_frequenciesTemp = _frequencies;
	_maxUsesTemp = _maxUses;
}

/**
 * Initializes scratch values for working in a vertical level
 */
void MapScript::initVerticalLevel(VerticalLevel level)
{
	_cumulativeFrequency = 0;
	_blocksTemp.clear();
	_groupsTemp.clear();
	_frequenciesTemp.clear();
	_maxUsesTemp.clear();

	_blocks = level.levelBlocks;
	_groups = level.levelGroups;
	_cumulativeFrequency = std::max(_blocks.size(), _groups.size());
	_frequenciesTemp.resize(_cumulativeFrequency, 1);
	_maxUsesTemp.resize(_cumulativeFrequency, -1);
	_blocksTemp = _blocks;
	_groupsTemp = _groups;
}

/**
 * Gets a random group number from the array, accounting for frequencies and max uses.
 * If no groups or blocks are defined, this command will return the default" group,
 * If all the max uses are used up, it will return "undefined".
 * @return Group number.
 */
int MapScript::getGroupNumber()
{
	if (!_groups.size())
	{
		return MT_DEFAULT;
	}
	if (_cumulativeFrequency > 0)
	{
		int pick = RNG::generate(0, _cumulativeFrequency-1);
		for (size_t i = 0; i != _groupsTemp.size(); ++i)
		{
			if (pick < _frequenciesTemp.at(i))
			{
				int retVal = _groupsTemp.at(i);

				if (_maxUsesTemp.at(i) > 0)
				{
					if (--_maxUsesTemp.at(i) == 0)
					{
						_groupsTemp.erase(_groupsTemp.begin() + i);
						_cumulativeFrequency -= _frequenciesTemp.at(i);
						_frequenciesTemp.erase(_frequenciesTemp.begin() + i);
						_maxUsesTemp.erase(_maxUsesTemp.begin() + i);
					}
				}
				return retVal;
			}
			pick -= _frequenciesTemp.at(i);
		}
	}
	return MT_UNDEFINED;
}

/**
 * Gets a random block number from the array, accounting for frequencies and max uses.
 * If no blocks are defined, it will use a group instead.
 * @return Block number.
 */
int MapScript::getBlockNumber()
{
	if (_cumulativeFrequency > 0)
	{
		int pick = RNG::generate(0, _cumulativeFrequency-1);
		for (size_t i = 0; i != _blocksTemp.size(); ++i)
		{
			if (pick < _frequenciesTemp.at(i))
			{
				int retVal = _blocksTemp.at(i);

				if (_maxUsesTemp.at(i) > 0)
				{
					if (--_maxUsesTemp.at(i) == 0)
					{
						_blocksTemp.erase(_blocksTemp.begin() + i);
						_cumulativeFrequency -= _frequenciesTemp.at(i);
						_frequenciesTemp.erase(_frequenciesTemp.begin() + i);
						_maxUsesTemp.erase(_maxUsesTemp.begin() + i);
					}
				}
				return retVal;
			}
			pick -= _frequenciesTemp.at(i);
		}
	}
	return MT_UNDEFINED;
}

/**
 * Gets a random map block from a given terrain, using either the groups or the blocks defined.
 * @param terrain the terrain to pick a block from.
 * @return Pointer to a randomly chosen map block, given the options available.
 */
MapBlock *MapScript::getNextBlock(RuleTerrain *terrain)
{
	if (_blocks.empty())
	{
		return terrain->getRandomMapBlock(_sizeX * 10, _sizeY * 10, getGroupNumber());
	}
	int result = getBlockNumber();
	if (result < (int)(terrain->getMapBlocks()->size()) && result != MT_UNDEFINED)
	{
		return terrain->getMapBlocks()->at((size_t)(result));
	}
	return 0;
}

/**
 * Gets the name of the UFO in the case of "setUFO"
 * @return the UFO name.
 */
std::string MapScript::getUFOName() const
{
	return _ufoName;
}

/**
* Gets the name of the craft in the case of "addCraft"
* @return the craft name.
*/
std::string MapScript::getCraftName()
{
	return _craftName;
}

/**
 * Gets the alternate terrain list for this command.
 * @return the vector of terrain names.
 */
const std::vector<std::string> &MapScript::getRandomAlternateTerrain() const
{
	return _randomTerrain;
}

/**
 * Gets the vertical levels defined in a map script command
 * @return the vector of VerticalLevels
 */
const std::vector<VerticalLevel> &MapScript::getVerticalLevels() const
{
	return _verticalLevels;
}

/**
 * For use only with base defense maps as a special case,
 * set _verticalLevels directly for a new MapScript
 * @param verticalLevels the vector of VerticalLevels
 * @param size the size of the facility whose VerticalLevels are being loaded
 */
void MapScript::setVerticalLevels(const std::vector<VerticalLevel> &verticalLevels, int size)
{
	_verticalLevels = verticalLevels;
	_sizeX = _sizeY = size;
}

}

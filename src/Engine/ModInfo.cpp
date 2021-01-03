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

#include "ModInfo.h"
#include "CrossPlatform.h"
#include "Exception.h"
#include <yaml-cpp/yaml.h>
#include <sstream>

namespace OpenXcom
{

ModInfo::ModInfo(const std::string &path) :
	 _path(path), _name(CrossPlatform::baseFilename(path)),
	_desc("No description."), _version("1.0"), _author("unknown author"),
	_id(_name), _master("xcom1"), _isMaster(false), _reservedSpace(1), _versionOk(true)
{
	// empty
}

void ModInfo::load(const YAML::Node& doc)
{
	_name     = doc["name"].as<std::string>(_name);
	_desc     = doc["description"].as<std::string>(_desc);
	_version  = doc["version"].as<std::string>(_version);
	_author   = doc["author"].as<std::string>(_author);
	_id       = doc["id"].as<std::string>(_id);
	_isMaster = doc["isMaster"].as<bool>(_isMaster);
	_reservedSpace = doc["reservedSpace"].as<int>(_reservedSpace);
	_requiredExtendedVersion = doc["requiredExtendedVersion"].as<std::string>(_requiredExtendedVersion);

	if (!_requiredExtendedVersion.empty())
	{
		_versionOk = !CrossPlatform::isHigherThanCurrentVersion(_requiredExtendedVersion);
	}

	if (_reservedSpace < 1)
	{
		_reservedSpace = 1;
	}
	else if (_reservedSpace > 100)
	{
		_reservedSpace = 100;
	}

	if (_isMaster)
	{
		// default a master's master to none.  masters can still have
		// masters, but they must be explicitly declared.
		_master = "";
		// only masters can load external resource dirs
		_externalResourceDirs = doc["loadResources"].as< std::vector<std::string> >(_externalResourceDirs);
	}
	_resourceConfigFile = doc["resourceConfig"].as<std::string>(_resourceConfigFile);

	_master = doc["master"].as<std::string>(_master);
	if (_master == "*")
	{
		_master = "";
	}
}

const std::string &ModInfo::getPath()                    const { return _path;                    }
const std::string &ModInfo::getName()                    const { return _name;                    }
const std::string &ModInfo::getDescription()             const { return _desc;                    }
const std::string &ModInfo::getVersion()                 const { return _version;                 }
const std::string &ModInfo::getAuthor()                  const { return _author;                  }
const std::string &ModInfo::getId()                      const { return _id;                      }
const std::string &ModInfo::getMaster()                  const { return _master;                  }
bool               ModInfo::isMaster()                   const { return _isMaster;                }
const std::string &ModInfo::getRequiredExtendedVersion() const { return _requiredExtendedVersion; }
const std::string &ModInfo::getResourceConfigFile()      const { return _resourceConfigFile;      }
int                ModInfo::getReservedSpace()           const { return _reservedSpace;           }

/**
 * Checks if a given mod can be activated.
 * It must either be:
 * - a Master mod
 * - a standalone mod (no master)
 * - depend on the current Master mod
 * @param curMaster Id of the active master mod.
 * @return True if it's activable, false otherwise.
*/
bool ModInfo::canActivate(const std::string &curMaster) const
{
	return (isMaster() || getMaster().empty() || getMaster() == curMaster);
}


const std::vector<std::string> &ModInfo::getExternalResourceDirs() const { return _externalResourceDirs; }

/**
 * Gets whether the current OXCE version is equal to (or higher than) the required OXCE version.
 * @return True if the version is OK.
*/
bool ModInfo::isVersionOk() const
{
	return _versionOk;
}

}

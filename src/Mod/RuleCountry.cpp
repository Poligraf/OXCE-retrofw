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
#include "RuleCountry.h"
#include "Mod.h"
#include "../Engine/RNG.h"
#include "../fmath.h"

namespace OpenXcom
{

/**
 * Creates a blank ruleset for a certain
 * type of country.
 * @param type String defining the type.
 */
RuleCountry::RuleCountry(const std::string &type) : _type(type), _fundingBase(0), _fundingCap(0), _labelLon(0.0), _labelLat(0.0), _labelColor(0), _zoomLevel(0)
{
}

/**
 *
 */
RuleCountry::~RuleCountry()
{
}

/**
 * Loads the country type from a YAML file.
 * @param node YAML node.
 */
void RuleCountry::load(const YAML::Node &node)
{
	if (const YAML::Node &parent = node["refNode"])
	{
		load(parent);
	}
	_type = node["type"].as<std::string>(_type);
	_signedPactEventName = node["signedPactEvent"].as<std::string>(_signedPactEventName);
	_rejoinedXcomEventName = node["rejoinedXcomEvent"].as<std::string>(_rejoinedXcomEventName);
	_fundingBase = node["fundingBase"].as<int>(_fundingBase);
	_fundingCap = node["fundingCap"].as<int>(_fundingCap);
	if (node["labelLon"])
		_labelLon = Deg2Rad(node["labelLon"].as<double>());
	if (node["labelLat"])
		_labelLat = Deg2Rad(node["labelLat"].as<double>());
	_labelColor = node["labelColor"].as<int>(_labelColor);
	_zoomLevel = node["zoomLevel"].as<int>(_zoomLevel);
	std::vector< std::vector<double> > areas;
	areas = node["areas"].as< std::vector< std::vector<double> > >(areas);
	for (size_t i = 0; i != areas.size(); ++i)
	{
		_lonMin.push_back(Deg2Rad(areas[i][0]));
		_lonMax.push_back(Deg2Rad(areas[i][1]));
		_latMin.push_back(Deg2Rad(areas[i][2]));
		_latMax.push_back(Deg2Rad(areas[i][3]));

		if (_latMin.back() > _latMax.back())
			std::swap(_latMin.back(), _latMax.back());
	}
}

/**
 * Cross link with other rules.
 */
void RuleCountry::afterLoad(const Mod* mod)
{
	mod->linkRule(_signedPactEvent, _signedPactEventName);
	mod->linkRule(_rejoinedXcomEvent, _rejoinedXcomEventName);
}

/**
 * Gets the language string that names
 * this country. Each country type
 * has a unique name.
 * @return The country's name.
 */
const std::string& RuleCountry::getType() const
{
	return _type;
}

/**
 * Generates the random starting funding for the country.
 * @return The monthly funding.
 */
int RuleCountry::generateFunding() const
{
	return RNG::generate(_fundingBase, _fundingBase*2) * 1000;
}

/**
 * Gets the country's funding cap.
 * Country funding can never exceed this.
 * @return The funding cap, in thousands.
 */
int RuleCountry::getFundingCap() const
{
	return _fundingCap;
}

/**
 * Gets the longitude of the country's label on the globe.
 * @return The longitude in radians.
 */
double RuleCountry::getLabelLongitude() const
{
	return _labelLon;
}

/**
 * Gets the latitude of the country's label on the globe.
 * @return The latitude in radians.
 */
double RuleCountry::getLabelLatitude() const
{
	return _labelLat;
}

/**
 * Checks if a point is inside this country.
 * @param lon Longitude in radians.
 * @param lat Latitude in radians.
 * @return True if it's inside, false if it's outside.
 */
bool RuleCountry::insideCountry(double lon, double lat) const
{
	for (size_t i = 0; i < _lonMin.size(); ++i)
	{
		bool inLon, inLat;

		if (_lonMin[i] <= _lonMax[i])
			inLon = (lon >= _lonMin[i] && lon < _lonMax[i]);
		else
			inLon = ((lon >= _lonMin[i] && lon < M_PI*2.0) || (lon >= 0 && lon < _lonMax[i]));

		inLat = (lat >= _latMin[i] && lat < _latMax[i]);

		if (inLon && inLat)
			return true;
	}
	return false;
}

/**
 * Gets the country's label color.
 * @return The color code.
 */
int RuleCountry::getLabelColor() const
{
	return _labelColor;
}

/**
 * Gets the minimum zoom level required to display the label.
 * Note: this works for extraGlobeLabels only, not for vanilla countries.
 * @return The zoom level.
 */
int RuleCountry::getZoomLevel() const
{
	return _zoomLevel;
}

}

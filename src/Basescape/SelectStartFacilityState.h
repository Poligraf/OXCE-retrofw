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
#include "BuildFacilitiesState.h"

namespace OpenXcom
{

class Globe;

/**
 * Window shown with all the facilities
 * available to build.
 */
class SelectStartFacilityState : public BuildFacilitiesState
{
private:
	Globe *_globe;
public:
	/// Creates the Build Facilities state.
	SelectStartFacilityState(Base *base, State *state, Globe *globe);
	/// Cleans up the Build Facilities state.
	~SelectStartFacilityState();
	/// Populates the build option list.
	virtual void populateBuildList() override;
	/// Handler for clicking the Reset button.
	void btnOkClick(Action *action);
	/// Handler for clicking the Facilities list.
	void lstFacilitiesClick(Action *action) override;
	/// Handler for when the facility is actually built.
	void facilityBuilt();
};

}

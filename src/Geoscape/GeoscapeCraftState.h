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
#include "../Engine/State.h"
#include "../Mod/RuleCraft.h"

namespace OpenXcom
{

class TextButton;
class Window;
class Text;
class Craft;
class Globe;
class Waypoint;

/**
 * Craft window that displays info about
 * a specific craft out on the Geoscape.
 */
class GeoscapeCraftState : public State
{
private:
	Craft *_craft;
	Globe *_globe;
	Waypoint *_waypoint;

	TextButton *_btnBase, *_btnTarget, *_btnPatrol, *_btnCancel;
	Window *_window;
	Text *_txtTitle, *_txtStatus, *_txtBase, *_txtSpeed, *_txtMaxSpeed, *_txtAltitude, *_txtFuel, *_txtDamage, *_txtShield, *_txtWeaponName[RuleCraft::WeaponMax], *_txtWeaponAmmo[RuleCraft::WeaponMax], *_txtRedirect, *_txtSoldier, *_txtHWP;

	int _weaponNum;
public:
	/// Creates the Geoscape Craft state.
	GeoscapeCraftState(Craft *craft, Globe *globe, Waypoint *waypoint);
	/// Cleans up the Geoscape Craft state.
	~GeoscapeCraftState();
	/// Handler for clicking the Return To Base button.
	void btnBaseClick(Action *action);
	/// Handler for clicking the Select New Target button.
	void btnTargetClick(Action *action);
	/// Handler for clicking the Patrol button.
	void btnPatrolClick(Action *action);
	/// Handler for clicking the Cancel button.
	void btnCancelClick(Action *action);
};

}

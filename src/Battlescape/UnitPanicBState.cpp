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

#include "UnitPanicBState.h"
#include "UnitTurnBState.h"
#include "ProjectileFlyBState.h"
#include "TileEngine.h"
#include "../Savegame/BattleUnit.h"
#include "../Savegame/SavedBattleGame.h"
#include "../Engine/RNG.h"
#include "BattlescapeGame.h"
#include "../Savegame/SavedBattleGame.h"

namespace OpenXcom
{

/**
 * Sets up an UnitPanicBState.
 * @param parent Pointer to the Battlescape.
 * @param unit Panicking unit.
 */
UnitPanicBState::UnitPanicBState(BattlescapeGame *parent, BattleUnit *unit) : BattleState(parent), _unit(unit), _shotsFired(0)
{
	_berserking = _unit->getStatus() == STATUS_BERSERK;
	unit->abortTurn(); //makes the unit go to status STANDING :p
}

/**
 * Deletes the UnitPanicBState.
 */
UnitPanicBState::~UnitPanicBState()
{
}

void UnitPanicBState::init()
{
}

/**
 * Runs state functionality every cycle.
 * Ends the panicking when done.
 */
void UnitPanicBState::think()
{
	if (_unit)
	{
		// berserking requires handling here, as the target selection isn't completely random
		// and needs updating between shots.
		if (!_unit->isOut() && _shotsFired < 10 && _berserking)
		{
			_shotsFired++;
			BattleAction ba;
			ba.actor = _unit;
			ba.weapon = _unit->getMainHandWeapon();
			{
				// make autoshots if possible.
				ba.type = BA_AUTOSHOT;
				ba.updateTU();
				bool canShoot = ba.haveTU() && _parent->getSave()->canUseWeapon(ba.weapon, ba.actor, _berserking, ba.type);

				if (!canShoot)
				{
					ba.type = BA_SNAPSHOT;
					ba.updateTU();
					canShoot = ba.haveTU() && _parent->getSave()->canUseWeapon(ba.weapon, ba.actor, _berserking, ba.type);
				}

				if (!canShoot)
				{
					ba.type = BA_AIMEDSHOT;
					ba.updateTU();
					canShoot = ba.haveTU() && _parent->getSave()->canUseWeapon(ba.weapon, ba.actor, _berserking, ba.type);
				}

				if (canShoot)
				{
					// if we see enemies, shoot at the closest living one.
					if (!_unit->getVisibleUnits()->empty())
					{
						int dist = 255;
						for (std::vector<BattleUnit*>::const_iterator i = _unit->getVisibleUnits()->begin(); i != _unit->getVisibleUnits()->end(); ++i)
						{
							int newDist = Position::distance2d(_unit->getPosition(), (*i)->getPosition());
							if (newDist < dist)
							{
								ba.target = (*i)->getPosition();
								dist = newDist;
							}
						}
					}
					else // otherwise shoot randomly
					{
						ba.target = Position(_unit->getPosition().x + RNG::generate(-6,6), _unit->getPosition().y + RNG::generate(-6,6), _unit->getPosition().z);
					}
					// include the cost for facing our target
					int turnCost = std::abs(_unit->getDirection() - _unit->directionTo(ba.target));
					if (turnCost > 4)
					{
						turnCost = 8-turnCost;
					}
					turnCost = turnCost * _unit->getTurnCost();

					_unit->spendTimeUnits(turnCost);
					_parent->statePushFront(new UnitTurnBState(_parent, ba, false));
					// even if we don't have enough TUs to turn AND shoot, we still want to turn.
					if (ba.haveTU())
					{
						_parent->statePushNext(new ProjectileFlyBState(_parent, ba));
					}
				}
			}
			return;
		}
		if (!_unit->isOut())
		{
			_unit->abortTurn(); // set the unit status to standing in case it wasn't otherwise changed from berserk/panicked
		}
		// reset the unit's time units when all panicking is done
		_unit->clearTimeUnits();
		_unit->moraleChange(+15);
	}
	_parent->popState();
	_parent->setupCursor();
}

/**
 * Panicking cannot be cancelled.
 */
void UnitPanicBState::cancel()
{
}

}

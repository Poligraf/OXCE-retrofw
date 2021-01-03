/*
 * Copyright 2010-2015 OpenXcom Developers.
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
#define _USE_MATH_DEFINES
#include <cmath>
#include "ItemSprite.h"
#include "../Engine/SurfaceSet.h"
#include "../Mod/RuleSoldier.h"
#include "../Mod/Unit.h"
#include "../Mod/RuleItem.h"
#include "../Mod/Armor.h"
#include "../Mod/RuleInventory.h"
#include "../Mod/Mod.h"
#include "../Savegame/BattleUnit.h"
#include "../Savegame/BattleItem.h"
#include "../Savegame/Soldier.h"
#include "../Battlescape/Position.h"

namespace OpenXcom
{

/**
 * Sets up a ItemSprite with the specified size and position.
 * @param width Width in pixels.
 * @param height Height in pixels.
 * @param x X position in pixels.
 * @param y Y position in pixels.
 */
ItemSprite::ItemSprite(Surface* dest, Mod* mod, int frame) :
	_itemSurface(mod->getSurfaceSet("FLOOROB.PCK")),
	_animationFrame(frame),
	_dest(dest)
{

}

/**
 * Deletes the ItemSprite.
 */
ItemSprite::~ItemSprite()
{

}

/**
 * Draws a item, using the drawing rules of the item or unit if it's corpse.
 * This function is called by Map, for each item on the screen.
 */
void ItemSprite::draw(BattleItem* item, int x, int y, int shade)
{
	Surface* sprite = item->getFloorSprite(_itemSurface, _animationFrame, shade);
	if (sprite)
	{
		ScriptWorkerBlit work;
		BattleItem::ScriptFill(&work, item, BODYPART_ITEM_FLOOR, _animationFrame, shade);
		work.executeBlit(sprite, _dest, x, y, shade);
	}
}

/**
 * Draws shadow of item.
 */
void ItemSprite::drawShadow(BattleItem* item, int x, int y)
{
	Surface* sprite = item->getFloorSprite(_itemSurface, _animationFrame, 16);
	if (sprite)
	{
		sprite->blitNShade(_dest, x, y, 16);
	}
}

} //namespace OpenXcom

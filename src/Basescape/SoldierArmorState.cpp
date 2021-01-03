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
#include "SoldierArmorState.h"
#include <sstream>
#include <algorithm>
#include "../Engine/Action.h"
#include "../Engine/Game.h"
#include "../Mod/Mod.h"
#include "../Engine/LocalizedText.h"
#include "../Engine/Options.h"
#include "../Interface/ArrowButton.h"
#include "../Interface/TextButton.h"
#include "../Interface/Window.h"
#include "../Interface/Text.h"
#include "../Interface/TextList.h"
#include "../Menu/ErrorMessageState.h"
#include "../Mod/Armor.h"
#include "../Mod/RuleInterface.h"
#include "../Savegame/SavedGame.h"
#include "../Savegame/Craft.h"
#include "../Savegame/Soldier.h"
#include "../Savegame/Base.h"
#include "../Savegame/ItemContainer.h"
#include "../Mod/RuleSoldier.h"
#include "../Ufopaedia/Ufopaedia.h"

namespace OpenXcom
{

struct compareArmorName
{
	typedef ArmorItem& first_argument_type;
	typedef ArmorItem& second_argument_type;
	typedef bool result_type;

	bool _reverse;

	compareArmorName(bool reverse) : _reverse(reverse) {}

	bool operator()(const ArmorItem &a, const ArmorItem &b) const
	{
		return Unicode::naturalCompare(a.name, b.name);
	}
};


/**
 * Initializes all the elements in the Soldier Armor window.
 * @param game Pointer to the core game.
 * @param base Pointer to the base to get info from.
 * @param soldier ID of the selected soldier.
 */
SoldierArmorState::SoldierArmorState(Base *base, size_t soldier, SoldierArmorOrigin origin) : _base(base), _soldier(soldier), _origin(origin)
{
	_screen = false;

	// Create objects
	_window = new Window(this, 192, 160, 64, 20, POPUP_BOTH);
	_btnCancel = new TextButton(140, 16, 90, 156);
	_txtTitle = new Text(182, 16, 69, 28);
	_txtType = new Text(90, 9, 80, 52);
	_txtQuantity = new Text(70, 9, 190, 52);
	_lstArmor = new TextList(160, 80, 73, 68);
	_sortName = new ArrowButton(ARROW_NONE, 11, 8, 80, 52);

	// Set palette
	if (_origin == SA_BATTLESCAPE)
	{
		setStandardPalette("PAL_BATTLESCAPE");
	}
	else
	{
		setInterface("soldierArmor");
	}

	add(_window, "window", "soldierArmor");
	add(_btnCancel, "button", "soldierArmor");
	add(_txtTitle, "text", "soldierArmor");
	add(_txtType, "text", "soldierArmor");
	add(_txtQuantity, "text", "soldierArmor");
	add(_lstArmor, "list", "soldierArmor");
	add(_sortName, "text", "soldierArmor");

	centerAllSurfaces();

	// Set up objects
	setWindowBackground(_window, "soldierArmor");

	_btnCancel->setText(tr("STR_CANCEL_UC"));
	_btnCancel->onMouseClick((ActionHandler)&SoldierArmorState::btnCancelClick);
	_btnCancel->onKeyboardPress((ActionHandler)&SoldierArmorState::btnCancelClick, Options::keyCancel);

	Soldier *s = _base->getSoldiers()->at(_soldier);
	_txtTitle->setAlign(ALIGN_CENTER);
	_txtTitle->setText(tr("STR_SELECT_ARMOR_FOR_SOLDIER").arg(s->getName()));

	_txtType->setText(tr("STR_TYPE"));

	_txtQuantity->setText(tr("STR_QUANTITY_UC"));

	_lstArmor->setColumns(2, 132, 21);
	_lstArmor->setSelectable(true);
	_lstArmor->setBackground(_window);
	_lstArmor->setMargin(8);

	_sortName->setX(_sortName->getX() + _txtType->getTextWidth() + 4);
	_sortName->onMouseClick((ActionHandler)&SoldierArmorState::sortNameClick);

	const auto &armors = _game->getMod()->getArmorsForSoldiers();
	for (auto* a : armors)
	{
		if (a->getRequiredResearch() && !_game->getSavedGame()->isResearched(a->getRequiredResearch()))
			continue;
		if (!a->getCanBeUsedBy(s->getRules()))
			continue;
		if (a->hasInfiniteSupply())
		{
			_armors.push_back(ArmorItem(a->getType(), tr(a->getType()), ""));
		}
		else if (_base->getStorageItems()->getItem(a->getStoreItem()) > 0)
		{
			std::ostringstream ss;
			if (_game->getSavedGame()->getMonthsPassed() > -1)
			{
				ss << _base->getStorageItems()->getItem(a->getStoreItem());
			}
			else
			{
				ss << "-";
			}
			_armors.push_back(ArmorItem(a->getType(), tr(a->getType()), ss.str()));
		}
	}

	_armorOrder = ARMOR_SORT_NONE;
	updateArrows();
	updateList();

	_lstArmor->onMouseClick((ActionHandler)&SoldierArmorState::lstArmorClick);
	_lstArmor->onMouseClick((ActionHandler)&SoldierArmorState::lstArmorClickMiddle, SDL_BUTTON_MIDDLE);

	// switch to battlescape theme if called from inventory
	if (_origin == SA_BATTLESCAPE)
	{
		applyBattlescapeTheme("soldierArmor");
	}
}

/**
 *
 */
SoldierArmorState::~SoldierArmorState()
{

}

/**
* Updates the sorting arrows based
* on the current setting.
*/
void SoldierArmorState::updateArrows()
{
	_sortName->setShape(ARROW_NONE);
	switch (_armorOrder)
	{
	case ARMOR_SORT_NAME_ASC:
		_sortName->setShape(ARROW_SMALL_UP);
		break;
	case ARMOR_SORT_NAME_DESC:
		_sortName->setShape(ARROW_SMALL_DOWN);
		break;
	default:
		break;
	}
}

/**
* Sorts the armor list.
* @param sort Order to sort the armors in.
*/
void SoldierArmorState::sortList()
{
	switch (_armorOrder)
	{
	case ARMOR_SORT_NAME_ASC:
		std::sort(_armors.begin(), _armors.end(), compareArmorName(false));
		break;
	case ARMOR_SORT_NAME_DESC:
		std::sort(_armors.rbegin(), _armors.rend(), compareArmorName(true));
		break;
	default:
		break;
	}
	updateList();
}

/**
* Updates the armor list with the current list
* of available armors.
*/
void SoldierArmorState::updateList()
{
	_lstArmor->clearList();
	for (std::vector<ArmorItem>::const_iterator j = _armors.begin(); j != _armors.end(); ++j)
	{
		_lstArmor->addRow(2, (*j).name.c_str(), (*j).quantity.c_str());
	}
}

/**
 * Returns to the previous screen.
 * @param action Pointer to an action.
 */
void SoldierArmorState::btnCancelClick(Action *)
{
	_game->popState();
}

/**
 * Equips the armor on the soldier and returns to the previous screen.
 * @param action Pointer to an action.
 */
void SoldierArmorState::lstArmorClick(Action *)
{
	Soldier *soldier = _base->getSoldiers()->at(_soldier);
	Armor *prev = soldier->getArmor();
	Armor *next = _game->getMod()->getArmor(_armors[_lstArmor->getSelectedRow()].type);
	Craft *craft = soldier->getCraft();
	if (craft != 0 && next->getSize() > prev->getSize())
	{
		if (craft->getNumVehicles() >= craft->getRules()->getVehicles() || craft->getSpaceAvailable() < 3)
		{
			_game->pushState(new ErrorMessageState(tr("STR_NOT_ENOUGH_CRAFT_SPACE"), _palette, _game->getMod()->getInterface("soldierInfo")->getElement("errorMessage")->color, "BACK01.SCR", _game->getMod()->getInterface("soldierInfo")->getElement("errorPalette")->color));
			return;
		}
	}
	if (_game->getSavedGame()->getMonthsPassed() != -1)
	{
		if (prev->getStoreItem())
		{
			_base->getStorageItems()->addItem(prev->getStoreItem());
		}
		if (next->getStoreItem())
		{
			_base->getStorageItems()->removeItem(next->getStoreItem());
		}
	}
	soldier->setArmor(next);
	_game->getSavedGame()->setLastSelectedArmor(next->getType());

	_game->popState();
}

/**
* Shows corresponding Ufopaedia article.
* @param action Pointer to an action.
*/
void SoldierArmorState::lstArmorClickMiddle(Action *action)
{
	auto armor = _game->getMod()->getArmor(_armors[_lstArmor->getSelectedRow()].type, true);
	std::string articleId = armor->getUfopediaType();
	Ufopaedia::openArticle(_game, articleId);
}

/**
* Sorts the armors by name.
* @param action Pointer to an action.
*/
void SoldierArmorState::sortNameClick(Action *)
{
	if (_armorOrder == ARMOR_SORT_NAME_ASC)
	{
		_armorOrder = ARMOR_SORT_NAME_DESC;
	}
	else
	{
		_armorOrder = ARMOR_SORT_NAME_ASC;
	}
	updateArrows();
	sortList();
}

}

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
#include <sstream>
#include "TrainingState.h"
#include "../Engine/Game.h"
#include "../Engine/Screen.h"
#include "../Engine/Action.h"
#include "../Engine/Language.h"
#include "../Engine/Palette.h"
#include "../Interface/TextButton.h"
#include "../Interface/Window.h"
#include "../Interface/Text.h"
#include "../Savegame/SavedGame.h"
#include "../Savegame/Base.h"
#include "GeoscapeState.h"
#include "AllocateTrainingState.h"
#include "../Engine/Options.h"
#include "../Mod/Mod.h"
#include "../Mod/RuleInterface.h"

namespace OpenXcom
{

/**
 * Initializes all the elements in the Psi Training screen.
 * @param game Pointer to the core game.
 */
TrainingState::TrainingState()
{
	// Create objects
	_window = new Window(this, 320, 200, 0, 0);
	_txtTitle = new Text(300, 17, 10, 16);
	_btnOk = new TextButton(160, 14, 80, 174);

	// Set palette
	setInterface("martialTraining");

	add(_window, "window", "martialTraining");
	add(_btnOk, "button2", "martialTraining");
	add(_txtTitle, "text", "martialTraining");

	// Set up objects
	setWindowBackground(_window, "martialTraining");

	_btnOk->setText(tr("STR_OK"));
	_btnOk->onMouseClick((ActionHandler)&TrainingState::btnOkClick);
	_btnOk->onKeyboardPress((ActionHandler)&TrainingState::btnOkClick, Options::keyCancel);

	_txtTitle->setBig();
	_txtTitle->setAlign(ALIGN_CENTER);
	_txtTitle->setText(tr("STR_PHYSICAL_TRAINING"));

	int buttons = 0;
	for (std::vector<Base*>::const_iterator b = _game->getSavedGame()->getBases()->begin(); b != _game->getSavedGame()->getBases()->end(); ++b)
	{
		if ((*b)->getAvailableTraining())
		{
			TextButton *btnBase = new TextButton(160, 14, 80, 40 + 16 * buttons);
			btnBase->setColor(Palette::blockOffset(15) + 6);
			btnBase->onMouseClick((ActionHandler)&TrainingState::btnBaseXClick);
			btnBase->setText((*b)->getName());
			add(btnBase, "button1", "martialTraining");
			_bases.push_back(*b);
			_btnBases.push_back(btnBase);
			++buttons;
			if (buttons >= 8)
			{
				break;
			}
		}
	}

	centerAllSurfaces();
}
/**
 *
 */
TrainingState::~TrainingState()
{

}

/**
 * Returns to the previous screen.
 * @param action Pointer to an action.
 */
void TrainingState::btnOkClick(Action *)
{
	_game->popState();
}

/**
 * Goes to the allocation screen for the corresponding base.
 * @param action Pointer to an action.
 */
void TrainingState::btnBaseXClick(Action *action)
{
	for (size_t i = 0; i < _btnBases.size(); ++i)
	{
		if (action->getSender() == _btnBases[i])
		{
			_game->pushState(new AllocateTrainingState(_bases.at(i)));
			break;
		}
	}
}

}

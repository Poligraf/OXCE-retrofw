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
#include "CraftErrorState.h"
#include "../Engine/Game.h"
#include "../Mod/Mod.h"
#include "../Engine/LocalizedText.h"
#include "../Interface/TextButton.h"
#include "../Interface/Window.h"
#include "../Interface/Text.h"
#include "GeoscapeState.h"
#include "../Engine/Options.h"

namespace OpenXcom
{

/**
 * Initializes all the elements in a Craft Error window.
 * @param game Pointer to the core game.
 * @param state Pointer to the Geoscape state.
 * @param msg Error message.
 */
CraftErrorState::CraftErrorState(GeoscapeState *state, const std::string &msg, bool enableHotkeys) : _state(state)
{
	_screen = false;

	// Create objects
	_window = new Window(this, 256, 160, 32, 20, POPUP_BOTH);
	_btnOk = new TextButton(100, 18, _state ? 48 : 110, 150);
	_btnOk5Secs = new TextButton(100, 18, 172, 150);
	_txtMessage = new Text(246, 96, 37, 42);

	// Set palette
	setInterface("craftError");

	add(_window, "window", "craftError");
	add(_btnOk, "button", "craftError");
	add(_btnOk5Secs, "button", "craftError");
	add(_txtMessage, "text1", "craftError");

	centerAllSurfaces();

	// Set up objects
	setWindowBackground(_window, "craftError");

	_btnOk->setText(tr("STR_OK"));
	_btnOk->onMouseClick((ActionHandler)&CraftErrorState::btnOkClick);
	if (enableHotkeys)
	{
		_btnOk->onKeyboardPress((ActionHandler)&CraftErrorState::btnOkClick, Options::keyCancel);
		if (!_state)
		{
			_btnOk->onKeyboardPress((ActionHandler)&CraftErrorState::btnOkClick, Options::keyOk);
		}
	}

	_btnOk5Secs->setText(tr("STR_OK_5_SECONDS"));
	_btnOk5Secs->onMouseClick((ActionHandler)&CraftErrorState::btnOk5SecsClick);
	if (enableHotkeys && _state)
	{
		_btnOk5Secs->onKeyboardPress((ActionHandler)&CraftErrorState::btnOk5SecsClick, Options::keyOk);
	}
	_btnOk5Secs->setVisible(_state != 0);

	_txtMessage->setAlign(ALIGN_CENTER);
	_txtMessage->setVerticalAlign(ALIGN_MIDDLE);
	_txtMessage->setBig();
	_txtMessage->setWordWrap(true);
	_txtMessage->setText(msg);
}

/**
 *
 */
CraftErrorState::~CraftErrorState()
{

}

/**
 * Closes the window.
 * @param action Pointer to an action.
 */
void CraftErrorState::btnOkClick(Action *)
{
	_game->popState();
}

/**
 * Closes the window.
 * @param action Pointer to an action.
 */
void CraftErrorState::btnOk5SecsClick(Action *)
{
	_state->timerReset();
	_game->popState();
}

}

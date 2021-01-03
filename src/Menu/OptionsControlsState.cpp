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
#include "OptionsControlsState.h"
#include <SDL.h>
#include "../Engine/Options.h"
#include "../Engine/LocalizedText.h"
#include "../Interface/Window.h"
#include "../Interface/TextList.h"
#include "../Engine/Action.h"

namespace OpenXcom
{

/**
 * Initializes all the elements in the Controls screen.
 * @param game Pointer to the core game.
 * @param origin Game section that originated this state.
 */
OptionsControlsState::OptionsControlsState(OptionsOrigin origin) : OptionsBaseState(origin), _selected(-1), _selKey(0)
{
	setCategory(_btnControls);

	// Create objects
	_lstControls = new TextList(200, 136, 94, 8);

	if (origin != OPT_BATTLESCAPE)
	{
		add(_lstControls, "optionLists", "controlsMenu");
	}
	else
	{
		add(_lstControls, "optionLists", "battlescape");
	}

	centerAllSurfaces();

	// Set up objects
	_lstControls->setColumns(2, 152, 48);
	_lstControls->setWordWrap(true);
	_lstControls->setSelectable(true);
	_lstControls->setBackground(_window);
	_lstControls->onMouseClick((ActionHandler)&OptionsControlsState::lstControlsClick, 0);
	_lstControls->onKeyboardPress((ActionHandler)&OptionsControlsState::lstControlsKeyPress);
	_lstControls->setFocus(true);
	_lstControls->setTooltip("STR_CONTROLS_DESC");
	_lstControls->onMouseIn((ActionHandler)&OptionsControlsState::txtTooltipIn);
	_lstControls->onMouseOut((ActionHandler)&OptionsControlsState::txtTooltipOut);

	_colorGroup = _lstControls->getSecondaryColor();
	_colorSel = _lstControls->getScrollbarColor();
	_colorNormal = _lstControls->getColor();

	const std::vector<OptionInfo> &options = Options::getOptionInfo();
	for (std::vector<OptionInfo>::const_iterator i = options.begin(); i != options.end(); ++i)
	{
		if (i->type() == OPTION_KEY && !i->description().empty())
		{
			if (i->category() == "STR_GENERAL")
			{
				_controlsGeneral.push_back(*i);
			}
			else if (i->category() == "STR_GEOSCAPE")
			{
				_controlsGeo.push_back(*i);
			}
			else if (i->category() == "STR_BATTLESCAPE")
			{
				_controlsBattle.push_back(*i);
			}
			else if (i->category() == "STR_OXCE")
			{
				_controlsOxce.push_back(*i);
			}
		}
	}
}

/**
 *
 */
OptionsControlsState::~OptionsControlsState()
{
}

/**
 * Fills the controls list based on category.
 */
void OptionsControlsState::init()
{
	OptionsBaseState::init();
	_lstControls->clearList();
	_lstControls->addRow(2, tr("STR_GENERAL").c_str(), "");
	_lstControls->setCellColor(0, 0, _colorGroup);
	addControls(_controlsGeneral);
	_lstControls->addRow(2, "", "");
	_lstControls->addRow(2, tr("STR_GEOSCAPE").c_str(), "");
	_lstControls->setCellColor(_controlsGeneral.size() + 2, 0, _colorGroup);
	addControls(_controlsGeo);
	_lstControls->addRow(2, "", "");
	_lstControls->addRow(2, tr("STR_BATTLESCAPE").c_str(), "");
	_lstControls->setCellColor(_controlsGeneral.size() + 2 + _controlsGeo.size() + 2, 0, _colorGroup);
	addControls(_controlsBattle);
	_lstControls->addRow(2, "", "");
	_lstControls->addRow(2, tr("STR_OXCE").c_str(), "");
	_lstControls->setCellColor(_controlsGeneral.size() + 2 + _controlsGeo.size() + 2 + _controlsBattle.size() + 2, 0, _colorGroup);
	addControls(_controlsOxce);
}

/**
 * Uppercases all the words in a string.
 * @param str Source string.
 * @return Destination string.
 */
std::string OptionsControlsState::ucWords(std::string str)
{
	if (!str.empty())
	{
		str[0] = toupper(str[0]);
	}
	for (size_t i = str.find_first_of(' '); i != std::string::npos; i = str.find_first_of(' ', i+1))
	{
		if (str.length() > i+1)
			str[i+1] = toupper(str[i+1]);
		else
			break;
	}
	return str;
}

/**
 * Adds a bunch of controls to the list.
 * @param keys List of controls.
 */
void OptionsControlsState::addControls(const std::vector<OptionInfo> &keys)
{
	for (std::vector<OptionInfo>::const_iterator i = keys.begin(); i != keys.end(); ++i)
	{
		std::string name = tr(i->description());
		SDLKey *key = i->asKey();
		std::string keyName = keyToButton(*key);
		if (*key == SDLK_UNKNOWN)
			keyName = "";
		_lstControls->addRow(2, name.c_str(), keyName.c_str());
	}
}

/**
 * Gets the currently selected control.
 * @param sel Selected row.
 * @return Pointer to option, NULL if none selected.
 */
OptionInfo *OptionsControlsState::getControl(size_t sel)
{
	if (sel > 0 &&
		sel <= _controlsGeneral.size())
	{
		return &_controlsGeneral[sel - 1];
	}
	else if (sel > _controlsGeneral.size() + 2 &&
			 sel <= _controlsGeneral.size() + 2 + _controlsGeo.size())
	{
		return &_controlsGeo[sel - 1 - _controlsGeneral.size() - 2];
	}
	else if (sel > _controlsGeneral.size() + 2 + _controlsGeo.size() + 2 &&
			 sel <= _controlsGeneral.size() + 2 + _controlsGeo.size() + 2 + _controlsBattle.size())
	{
		return &_controlsBattle[sel - 1 - _controlsGeneral.size() - 2 - _controlsGeo.size() - 2];
	}
	else if (sel > _controlsGeneral.size() + 2 + _controlsGeo.size() + 2 + _controlsBattle.size() + 2 &&
		sel <= _controlsGeneral.size() + 2 + _controlsGeo.size() + 2 + _controlsBattle.size() + 2 + _controlsOxce.size())
	{
		return &_controlsOxce[sel - 1 - _controlsGeneral.size() - 2 - _controlsGeo.size() - 2 - _controlsBattle.size() - 2];
	}
	else
	{
		return 0;
	}
}

/**
 * Select a control for changing.
 * @param action Pointer to an action.
 */
void OptionsControlsState::lstControlsClick(Action *action)
{
	if (action->getDetails()->button.button != SDL_BUTTON_LEFT && action->getDetails()->button.button != SDL_BUTTON_RIGHT)
	{
		return;
	}
	if (_selected != -1)
	{
		unsigned int selected = _selected;
		_lstControls->setCellColor(_selected, 0, _colorNormal);
		_lstControls->setCellColor(_selected, 1, _colorNormal);
		_selected = -1;
		_selKey = 0;
		if (selected == _lstControls->getSelectedRow())
			return;
	}
	_selected = _lstControls->getSelectedRow();
	_selKey = getControl(_selected);
	if (!_selKey)
	{
		_selected = -1;
		return;
	}

	if (action->getDetails()->button.button == SDL_BUTTON_LEFT)
	{
		_lstControls->setCellColor(_selected, 0, _colorSel);
		_lstControls->setCellColor(_selected, 1, _colorSel);
	}
	else if (action->getDetails()->button.button == SDL_BUTTON_RIGHT)
	{
		_lstControls->setCellText(_selected, 1, "");
		*_selKey->asKey() = SDLK_UNKNOWN;
		_selected = -1;
		_selKey = 0;
	}
}

/**
 * Change selected control.
 * @param action Pointer to an action.
 */
void OptionsControlsState::lstControlsKeyPress(Action *action)
{
	if (_selected != -1)
	{
		SDLKey key = action->getDetails()->key.keysym.sym;
		if (key != 0 &&
			key != SDLK_LSHIFT && key != SDLK_LALT && key != SDLK_LCTRL &&
			key != SDLK_RSHIFT && key != SDLK_RALT && key != SDLK_RCTRL)
		{
			*_selKey->asKey() = key;
			std::string name = keyToButton(key);
			_lstControls->setCellText(_selected, 1, name);
		}
		_lstControls->setCellColor(_selected, 0, _colorNormal);
		_lstControls->setCellColor(_selected, 1, _colorNormal);
		_selected = -1;
		_selKey = 0;
	}
}


std::string OptionsControlsState::keyToButton(SDLKey key){
	std::string keyName;
	switch (key){
		case SDLK_UP:
			keyName = "Up";
			break;
		case SDLK_DOWN:
			keyName = "Down";
			break;
		case SDLK_LEFT:
			keyName = "Left";
			break;
		case SDLK_RIGHT:
			keyName = "Right";
			break;
		case SDLK_LCTRL:
			keyName = "A";
			break;
		case SDLK_LALT:
			keyName = "B";
			break;
		case SDLK_LSHIFT:
			keyName = "Y";
			break;
		case SDLK_SPACE:
			keyName = "X";
			break;
		case SDLK_TAB:
			keyName = "L";
			break;
		case SDLK_BACKSPACE:
			keyName = "R";
			break;

		case SDLK_RETURN:
			keyName = "Start";
			break;
		case SDLK_ESCAPE:
			keyName = "Select";
			break;

		case SDLK_w:
		keyName = "Select + Up";
		break;

		case SDLK_s:
		keyName = "Select + Down";
		break;

		case SDLK_a:
		keyName = "Select + Left";
		break;

		case SDLK_d:
		keyName = "Select + Right";
		break;

		case SDLK_1:
		keyName = "Select + A";
		break;

		case SDLK_2:
		keyName = "Select + B";
		break;

		case SDLK_4:
		keyName = "Select + X";
		break;

		case SDLK_3:
		keyName = "Select + Y";
		break;

		case SDLK_PAGEUP:
		keyName = "Select + L";
		break;

		case SDLK_PAGEDOWN:
		keyName = "Select + R";
		break;

		case SDLK_HOME:
		keyName = "Select + Start";
		break;

		default:
			keyName = "";
	}
	return keyName;
}

}

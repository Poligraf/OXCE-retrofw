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

namespace OpenXcom
{

class Game;
class Window;
class TextButton;
class Text;
class Base;
class TextList;
class Soldier;

/**
 * Window which informs the player of finished soldier training.
 * Also allows to go to the AllocateTrainingState to assign more soldiers into training.
 */
class TrainingFinishedState : public State
{
private:
	Window *_window;
	Text *_txtTitle;
	TextList * _lstPossibilities;
	TextButton *_btnOpen, *_btnOk;
	Base *_base;
	bool _psi;
public:
	/// Creates the TrainingFinishedState state.
	TrainingFinishedState(Base *base, const std::vector<Soldier *> & list, bool psi);
	/// Handler for clicking the OK button.
	void btnOkClick(Action *action);
	/// Handler for clicking the Open button.
	void btnOpenClick(Action *action);
};

}

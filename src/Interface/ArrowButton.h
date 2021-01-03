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
#include "ImageButton.h"

namespace OpenXcom
{

enum ArrowShape { ARROW_NONE, ARROW_BIG_UP, ARROW_BIG_DOWN, ARROW_SMALL_UP, ARROW_SMALL_DOWN, ARROW_SMALL_LEFT, ARROW_SMALL_RIGHT };

class TextList;
class Timer;

/**
 * Button with an arrow on it. Can be used for
 * scrolling lists, spinners, etc. Contains various
 * arrow shapes.
 */
class ArrowButton : public ImageButton
{
private:
	ArrowShape _shape;
	TextList *_list;
	Timer *_timer;
protected:
	bool isButtonHandled(Uint8 button = 0) override;
public:
	/// Creates a new arrow button with the specified size and position.
	ArrowButton(ArrowShape shape, int width, int height, int x = 0, int y = 0);
	/// Cleans up the arrow button.
	~ArrowButton();
	/// Sets the arrow button's color.
	void setColor(Uint8 color) override;
	/// Sets the arrow button's shape.
	void setShape(ArrowShape shape);
	/// Sets the arrow button's list.
	void setTextList(TextList *list);
	/// Handles the timers.
	void think() override;
	/// Scrolls the list.
	void scroll();
	/// Draws the arrow button.
	void draw() override;
	/// Special handling for mouse presses.
	void mousePress(Action *action, State *state) override;
	/// Special handling for mouse releases.
	void mouseRelease(Action *action, State *state) override;
	/// Special handling for mouse clicks.
	void mouseClick(Action *action, State *state) override;
};

}

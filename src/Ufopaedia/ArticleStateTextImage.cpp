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

#include "../Mod/ArticleDefinition.h"
#include "ArticleStateTextImage.h"
#include "../Engine/Game.h"
#include "../Engine/Palette.h"
#include "../Engine/Surface.h"
#include "../Engine/LocalizedText.h"
#include "../Mod/Mod.h"
#include "../Interface/Text.h"
#include "../Interface/TextButton.h"

namespace OpenXcom
{

	ArticleStateTextImage::ArticleStateTextImage(ArticleDefinitionTextImage *defs, std::shared_ptr<ArticleCommonState> state) : ArticleState(defs->id, std::move(state))
	{
		// add screen elements
		_txtTitle = new Text(defs->text_width, 48, 5, 22);

		// Set palette
		if (defs->customPalette)
		{
			setCustomPalette(_game->getMod()->getSurface(defs->image_id)->getPalette(), Mod::UFOPAEDIA_CURSOR);
		}
		else
		{
			setStandardPalette("PAL_UFOPAEDIA");
		}

		ArticleState::initLayout();

		// add other elements
		add(_txtTitle);

		// Set up objects
		_game->getMod()->getSurface(defs->image_id)->blitNShade(_bg, 0, 0);
		_btnOk->setColor(Palette::blockOffset(5)+3);
		_btnPrev->setColor(Palette::blockOffset(5)+3);
		_btnNext->setColor(Palette::blockOffset(5)+3);

		_txtTitle->setColor(Palette::blockOffset(15)+4);
		_txtTitle->setBig();
		_txtTitle->setWordWrap(true);
		_txtTitle->setText(tr(defs->getTitleForPage(_state->current_page)));

		int text_height = _txtTitle->getTextHeight();

		if (defs->rect_text.width == 0)
		{
			_txtInfo = new Text(defs->text_width, 162, 5, 23 + text_height);
		}
		else
		{
			_txtInfo = new Text(defs->rect_text.width, defs->rect_text.height, defs->rect_text.x, defs->rect_text.y);
		}
		add(_txtInfo);

		_txtInfo->setColor(Palette::blockOffset(15)-1);
		_txtInfo->setSecondaryColor(Palette::blockOffset(15) + 4);
		_txtInfo->setWordWrap(true);
		_txtInfo->setText(tr(defs->getTextForPage(_state->current_page)));

		centerAllSurfaces();
	}

	ArticleStateTextImage::~ArticleStateTextImage()
	{}

}

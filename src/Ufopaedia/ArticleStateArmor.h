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
#include "ArticleState.h"

namespace OpenXcom
{
	class Game;
	class Surface;
	class Text;
	class TextList;
	class ArticleDefinitionArmor;

	/**
	 * ArticleStateArmor has a caption, preview image and a stats block.
	 * The image is found using the Armor class.
	 */

	class ArticleStateArmor : public ArticleState
	{
	public:
		ArticleStateArmor(ArticleDefinitionArmor *article_defs, std::shared_ptr<ArticleCommonState> state);
		virtual ~ArticleStateArmor();

	protected:
		void addStat(const std::string &label, int stat, bool plus = false);
		void addStat(const std::string &label, const std::string &stat);

		int _row;
		Surface *_image;
		Text *_txtTitle;
		TextList *_lstInfo;
		Text *_txtInfo;
		Uint8 _buttonColor, _textColor, _textColor2, _listColor1, _listColor2;
	};
}

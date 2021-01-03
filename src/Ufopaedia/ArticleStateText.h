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
	class Text;
	class ArticleDefinitionText;

	/**
	 * ArticleStateText has only a caption and a text.
	 */

	class ArticleStateText : public ArticleState
	{
	public:
		ArticleStateText(ArticleDefinitionText *article_defs, std::shared_ptr<ArticleCommonState> state);
		virtual ~ArticleStateText();

	protected:
		Text *_txtTitle;
		Text *_txtInfo;
	};
}
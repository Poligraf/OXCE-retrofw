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
	class Text;
	class ArticleDefinitionTFTD;

	/**
	 * Every TFTD article has a title, text block and a background image, with little to no variation.
	 */

	class ArticleStateTFTD : public ArticleState
	{
	public:
		ArticleStateTFTD(ArticleDefinitionTFTD *defs, std::shared_ptr<ArticleCommonState> state);
		virtual ~ArticleStateTFTD();

	protected:
		Text *_txtTitle;
		Text *_txtInfo;
		Uint8 _buttonColor, _textColor, _textColor2, _listColor1, _listColor2, _ammoColor, _arrowColor;
	};
}

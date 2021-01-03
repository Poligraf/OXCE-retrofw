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
 *e
 * You should have received a copy of the GNU General Public License
 * along with OpenXcom.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <map>
#include <vector>
#include <string>
#include "LocalizedText.h"
#include "FileMap.h"

namespace OpenXcom
{
enum TextDirection { DIRECTION_LTR, DIRECTION_RTL };
enum TextWrapping { WRAP_WORDS, WRAP_LETTERS };

class TextList;
class ExtraStrings;
class LanguagePlurality;

enum SoldierGender : char;

/**
 * Contains strings used throughout the game for localization.
 * Languages are just a set of strings identified by an ID string.
 */
class Language
{
private:
	std::string _id;
	std::map<std::string, LocalizedText> _strings;
	LanguagePlurality *_handler;
	TextDirection _direction;
	TextWrapping _wrap;

	static std::map<std::string, std::string> _names;
	static std::vector<std::string> _rtl, _cjk;

	/// Parses a text string loaded from an external file.
	std::string loadString(const std::string &s) const;
public:
	/// Creates a blank language.
	Language();
	/// Cleans up the language.
	~Language();
	/// Gets list of available languages.
	static void getList(std::vector<std::string> &ids, std::vector<std::string> &names);
	/// Loads the language from an external file.
	void loadFile(const FileMap::FileRecord *frec);
	/// Loads the language from a ruleset file.
	void loadRule(const std::map<std::string, ExtraStrings*> &extraStrings, const std::string &id);
	/// Gets the language's ID.
	std::string getId() const;
	/// Gets the language's name.
	std::string getName() const;
	/// Outputs the language to a HTML file.
	void toHtml(const std::string &filename) const;
	/// Get a localized text.
	LocalizedText getString(const std::string &id) const;
	/// Get a quantity-depended localized text.
	LocalizedText getString(const std::string &id, unsigned n) const;
	/// Get a gender-depended localized text.
	LocalizedText getString(const std::string &id, SoldierGender gender) const;
	/// Gets the direction of text in this language.
	TextDirection getTextDirection() const;
	/// Gets the wrapping of text in this language.
	TextWrapping getTextWrapping() const;
};

}

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
#include "Unicode.h"
#include <sstream>
#include <locale>
#include <stdexcept>
#include <algorithm>
#include "Logger.h"
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shlwapi.h>
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#endif

namespace OpenXcom
{
namespace Unicode
{

std::locale utf8;

/**
 * Store a UTF-8 locale to use when dealing with character conversions.
 * Windows doesn't have a UTF-8 locale so we just use its APIs directly.
 */
void getUtf8Locale()
{
	std::string loc;
#ifndef _WIN32
	// Find any UTF-8 locale
	FILE *fp = popen("locale -a", "r");
	if (fp != NULL)
	{
		char buf[50];
		while (fgets(buf, sizeof(buf), fp) != NULL)
		{
			if (strstr(buf, ".utf8") != NULL ||
				strstr(buf, ".UTF-8") != NULL)
			{
				// Trim newline
				size_t end = strlen(buf) - 1;
				if (buf[end] == '\n')
					buf[end] = '\0';

				loc = buf;
				break;
			}
		}
		pclose(fp);
	}
#endif
	// Try a UTF-8 locale (or default if none was found)
	try
	{
		Log(LOG_INFO) << "Attempted locale: " << loc.c_str();
		utf8 = std::locale(loc.c_str());
	}
	catch (const std::runtime_error &)
	{
		// Well we're stuck with the C locale, hope for the best
	}
	Log(LOG_INFO) << "Detected locale: " << utf8.name();
}

/**
 * Takes a Unicode 32-bit string and converts it
 * to a 8-bit string encoded in UTF-8.
 * Used for rendering text.
 * @note Adapted from https://stackoverflow.com/a/148766/2683561
 * @param src UTF-8 string.
 * @return Unicode string.
 */
UString convUtf8ToUtf32(const std::string &src)
{
	if (src.empty())
		return UString();
	UString out;
	out.reserve(src.size());
	UCode codepoint = 0;
	for (std::string::const_iterator i = src.begin(); i != src.end();)
	{
		unsigned char ch = static_cast<unsigned char>(*i);
		if (ch <= 0x7f)
			codepoint = ch;
		else if (ch <= 0xbf)
			codepoint = (codepoint << 6) | (ch & 0x3f);
		else if (ch <= 0xdf)
			codepoint = ch & 0x1f;
		else if (ch <= 0xef)
			codepoint = ch & 0x0f;
		else
			codepoint = ch & 0x07;
		++i;
		if (i == src.end() || ((*i & 0xc0) != 0x80 && codepoint <= 0x10ffff))
		{
			out.append(1, codepoint);
		}
	}
	return out;
}

/**
 * Takes a Unicode 32-bit string and converts it
 * to a 8-bit string encoded in UTF-8.
 * Used for rendering text.
 * @note Adapted from https://stackoverflow.com/a/148766/2683561
 * @param src Unicode string.
 * @return UTF-8 string.
 */
std::string convUtf32ToUtf8(const UString &src)
{
	if (src.empty())
		return std::string();
	std::string out;
	out.reserve(src.size());
	for (UString::const_iterator i = src.begin(); i != src.end(); ++i)
	{
		UCode codepoint = *i;
		if (codepoint <= 0x7f)
		{
			out.append(1, static_cast<char>(codepoint));
		}
		else if (codepoint <= 0x7ff)
		{
			out.append(1, static_cast<char>(0xc0 | ((codepoint >> 6) & 0x1f)));
			out.append(1, static_cast<char>(0x80 | (codepoint & 0x3f)));
		}
		else if (codepoint <= 0xffff)
		{
			out.append(1, static_cast<char>(0xe0 | ((codepoint >> 12) & 0x0f)));
			out.append(1, static_cast<char>(0x80 | ((codepoint >> 6) & 0x3f)));
			out.append(1, static_cast<char>(0x80 | (codepoint & 0x3f)));
		}
		else
		{
			out.append(1, static_cast<char>(0xf0 | ((codepoint >> 18) & 0x07)));
			out.append(1, static_cast<char>(0x80 | ((codepoint >> 12) & 0x3f)));
			out.append(1, static_cast<char>(0x80 | ((codepoint >> 6) & 0x3f)));
			out.append(1, static_cast<char>(0x80 | (codepoint & 0x3f)));
		}
	}
	return out;
}

/**
 * Takes a wide-character string and converts it to a
 * multibyte 8-bit string in a given encoding.
 * Used for Win32 APIs.
 * @param src Wide-character string.
 * @param cp Codepage of the destination string.
 * @return Multibyte string.
 */
std::string convWcToMb(const std::wstring &src, unsigned int cp)
{
	if (src.empty())
		return std::string();
#ifdef _WIN32
	int size = WideCharToMultiByte(cp, 0, &src[0], (int)src.size(), NULL, 0, NULL, NULL);
	std::string str(size, 0);
	WideCharToMultiByte(cp, 0, &src[0], (int)src.size(), &str[0], size, NULL, NULL);
	return str;
#elif defined(__CYGWIN__)
	(void)cp;
	assert(sizeof(wchar_t) == sizeof(Uint16));
	UString ustr(src.size(), 0);
	std::transform(src.begin(), src.end(), ustr.begin(),
		[](wchar_t c) -> UCode
		{
			//TODO: dropping surrogates, do proper implementation when someone will need that range
			if (c <= 0xD7FF || c >= 0xE000)
			{
				return c;
			}
			else
			{
				return '?';
			}
		}
	);
	return convUtf32ToUtf8(ustr);
#else
	(void)cp;
	assert(sizeof(wchar_t) == sizeof(UCode));
	const UString *ustr = reinterpret_cast<const UString*>(&src);
	return convUtf32ToUtf8(*ustr);
#endif
}

/**
 * Takes a multibyte 8-bit string in a given encoding
 * and converts it to a wide-character string.
 * Used for Win32 APIs.
 * @param src Multibyte string.
 * @param cp Codepage of the source string.
 * @return Wide-character string.
 */
std::wstring convMbToWc(const std::string &src, unsigned int cp)
{
	if (src.empty())
		return std::wstring();
#ifdef _WIN32
	int size = MultiByteToWideChar(cp, 0, &src[0], (int)src.size(), NULL, 0);
	std::wstring wstr(size, 0);
	MultiByteToWideChar(cp, 0, &src[0], (int)src.size(), &wstr[0], size);
	return wstr;
#elif defined(__CYGWIN__)
	(void)cp;
	assert(sizeof(wchar_t) == sizeof(Uint16));
	const UString ustr = convUtf8ToUtf32(src);

	std::wstring wstr(ustr.size(), 0);
	std::transform(ustr.begin(), ustr.end(), wstr.begin(),
		[](UCode c) -> wchar_t
		{
			//TODO: dropping surrogates, do proper implementation when someone will need that range
			if (c <= 0xD7FF || (c >= 0xE000 && c <= 0xFFFF))
			{
				return c;
			}
			else
			{
				return '?';
			}
		}
	);
	return wstr;
#else
	(void)cp;
	assert(sizeof(wchar_t) == sizeof(UCode));
	UString ustr = convUtf8ToUtf32(src);
	const std::wstring *wstr = reinterpret_cast<const std::wstring*>(&ustr);
	return *wstr;
#endif
}

/**
 * Checks if UTF-8 string is well-formed.
 * @param ss candidate string
 * @return true if valid
 * @note based on https://www.cl.cam.ac.uk/~mgk25/ucs/utf8_check.c
 */
bool isValidUTF8(const std::string& ss) {
	std::basic_string<unsigned char> s((unsigned char *)ss.c_str());
	s.append(4, 0); // we need to slap 4 bytes on the end of s or we can overrun it
	size_t i = 0;
	while ((i < ss.length()) && (s[i] != 0)) {
		if (s[i] < 0x80) {
			/* 0xxxxxxx */
			i++;
		} else if ((s[i+0] & 0xe0) == 0xc0) {
			/* 110XXXXx 10xxxxxx */
			if ((s[i+1] & 0xc0) != 0x80 || (s[i+0] & 0xfe) == 0xc0)	{	/* overlong? */
				return false;
			} else {
				i += 2;
			}
		} else if ((s[i] & 0xf0) == 0xe0) {
			/* 1110XXXX 10Xxxxxx 10xxxxxx */
			if ((s[i+1] & 0xc0) != 0x80 ||
				(s[i+2] & 0xc0) != 0x80 ||
				(s[i+0] == 0xe0 && (s[i+1] & 0xe0) == 0x80) ||		/* overlong? */
				(s[i+0] == 0xed && (s[i+1] & 0xe0) == 0xa0) ||		/* surrogate? */
				(s[i+0] == 0xef && s[i+1] == 0xbf &&
				(s[i+2] & 0xfe) == 0xbe)) {							/* U+FFFE or U+FFFF? */
					return false;
			} else {
				i += 3;
			}
		} else if ((s[0] & 0xf8) == 0xf0) {
			/* 11110XXX 10XXxxxx 10xxxxxx 10xxxxxx */
			if ((s[i+1] & 0xc0) != 0x80 ||
				(s[i+2] & 0xc0) != 0x80 ||
				(s[i+3] & 0xc0) != 0x80 ||
				(s[i+0] == 0xf0 && (s[i+1] & 0xf0) == 0x80) ||		/* overlong? */
				(s[i+0] == 0xf4 && s[i+1] > 0x8f) || s[i+0] > 0xf4) { 	/* > U+10FFFF? */
					return false;
			} else {
				i += 4;
			}
		} else {
			return false;
		}
	}
	return true;
}

/**
 * Compares two UTF-8 strings using natural human ordering.
 * @param a String A.
 * @param b String B.
 * @return String A comes before String B.
 */
bool naturalCompare(const std::string &a, const std::string &b)
{
#ifdef _WIN32
	typedef int (WINAPI *WinStrCmp)(PCWSTR, PCWSTR);
	WinStrCmp pWinStrCmp = (WinStrCmp)GetProcAddress(GetModuleHandleA("shlwapi.dll"), "StrCmpLogicalW");
	if (pWinStrCmp)
	{
		std::wstring wa = convMbToWc(a, CP_UTF8);
		std::wstring wb = convMbToWc(b, CP_UTF8);
		return (pWinStrCmp(wa.c_str(), wb.c_str()) < 0);
	}
	else
#endif
	{
		// fallback to lexical sort
		return caseCompare(a, b);
	}
}

/**
 * Compares two UTF-8 strings ignoring case.
 * @param a String A.
 * @param b String B.
 * @return String A comes before String B.
 */
bool caseCompare(const std::string &a, const std::string &b)
{
#ifdef _WIN32
	std::wstring wa = convMbToWc(a, CP_UTF8);
	std::wstring wb = convMbToWc(b, CP_UTF8);
	return (StrCmpIW(wa.c_str(), wb.c_str()) < 0);
#else
	return (std::use_facet< std::collate<char> >(utf8).compare(&a[0], &a[a.size()], &b[0], &b[b.size()]) < 0);
#endif
}

/**
 * Searches for a substring in another string ignoring case.
 * @param haystack String to search.
 * @param needle String to find.
 * @return True if the needle is in the haystack.
 */
bool caseFind(const std::string &haystack, const std::string &needle)
{
#ifdef _WIN32
	std::wstring wa = convMbToWc(haystack, CP_UTF8);
	std::wstring wb = convMbToWc(needle, CP_UTF8);
	return (StrStrIW(wa.c_str(), wb.c_str()) != NULL);
#else
	std::wstring wa = convMbToWc(haystack, 0);
	std::wstring wb = convMbToWc(needle, 0);
	std::use_facet< std::ctype<wchar_t> >(utf8).toupper(&wa[0], &wb[wb.size()]);
	std::use_facet< std::ctype<wchar_t> >(utf8).toupper(&wb[0], &wb[wb.size()]);
	return (wa.find(wb) != std::wstring::npos);
#endif
}

/**
 * Uppercases a UTF-8 string, modified in place.
 * Used for case-insensitive comparisons.
 * @param s Source string.
 */
void upperCase(std::string &s)
{
	if (s.empty())
		return;
#ifdef _WIN32
	std::wstring ws = convMbToWc(s, CP_UTF8);
	CharUpperW(&ws[0]);
	s = convWcToMb(ws, CP_UTF8);
#else
	std::wstring ws = convMbToWc(s, 0);
	std::use_facet< std::ctype<wchar_t> >(utf8).toupper(&ws[0], &ws[ws.size()]);
	s = convWcToMb(ws, 0);
#endif
}

/**
 * Lowercases a UTF-8 string, modified in place.
 * Used for case-insensitive comparisons.
 * @param s Source string.
 */
void lowerCase(std::string &s)
{
	if (s.empty())
		return;
#ifdef _WIN32
	std::wstring ws = convMbToWc(s, CP_UTF8);
	CharLowerW(&ws[0]);
	s = convWcToMb(ws, CP_UTF8);
#else
	std::wstring ws = convMbToWc(s, 0);
	std::use_facet< std::ctype<wchar_t> >(utf8).tolower(&ws[0], &ws[ws.size()]);
	s = convWcToMb(ws, 0);
#endif
}

/**
 * Replaces every instance of a substring.
 * @param str The string to modify.
 * @param find The substring to find.
 * @param replace The substring to replace it with.
 */
void replace(std::string &str, const std::string &find, const std::string &replace)
{
	for (size_t i = str.find(find); i != std::string::npos; i = str.find(find, i + replace.length()))
	{
		str.replace(i, find.length(), replace);
	}
}

/**
 * Takes an integer value and formats it as number with separators (spacing the thousands).
 * @param value The value.
 * @param currency Currency symbol.
 * @return The formatted string.
 */
std::string formatNumber(int64_t value, const std::string &currency)
{
	const std::string thousands_sep = "\xC2\xA0"; // TOK_NBSP

	bool negative = (value < 0);
	std::ostringstream ss;
	ss << (negative ? -value : value);
	std::string s = ss.str();
	size_t spacer = s.size() - 3;
	while (spacer > 0 && spacer < s.size())
	{
		s.insert(spacer, thousands_sep);
		spacer -= 3;
	}
	if (!currency.empty())
	{
		s.insert(0, currency);
	}
	if (negative)
	{
		s.insert(0, "-");
	}
	return s;
}

/**
 * Takes an integer value and formats it as currency,
 * spacing the thousands and adding a $ sign to the front.
 * @param funds The funding value.
 * @return The formatted string.
 */
std::string formatFunding(int64_t funds)
{
	return formatNumber(funds, "$");
}

/**
 * Takes an integer value and formats it as percentage,
 * adding a % sign.
 * @param value The percentage value.
 * @return The formatted string.
 */
std::string formatPercentage(int value)
{
	std::ostringstream ss;
	ss << value << "%";
	return ss.str();
}

}
}

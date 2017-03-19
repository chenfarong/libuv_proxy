﻿
#ifndef StrUtil_h__
#define StrUtil_h__

#include <sstream>
#include <algorithm>
#include <string>
#include <vector>
#include <time.h>
#include <stdarg.h>

typedef std::vector<std::string> Tokens;

typedef int64_t int64;
typedef int32_t int32;
typedef int16_t int16;
typedef int8_t int8;
typedef uint64_t uint64;
typedef uint32_t uint32;
typedef uint16_t uint16;
typedef uint8_t uint8;

enum TimeConstants
{
	MINUTE = 60,
	HOUR = MINUTE * 60,
	DAY = HOUR * 24,
	WEEK = DAY * 7,
	MONTH = DAY * 30,
	YEAR = MONTH * 12,
	IN_MILLISECONDS = 1000
};

Tokens StrSplit(const std::string& src, const std::string& sep);
uint32 GetUInt32ValueFromArray(Tokens const& data, uint16 index);
float GetFloatValueFromArray(Tokens const& data, uint16 index);

void stripLineInvisibleChars(std::string& src);

bool Utf8toWStr(const std::string& utf8str, std::wstring& wstr);
// in wsize==max size of buffer, out wsize==real string size
bool Utf8toWStr(char const* utf8str, size_t csize, wchar_t* wstr, size_t& wsize);
inline bool Utf8toWStr(const std::string& utf8str, wchar_t* wstr, size_t& wsize)
{
	return Utf8toWStr(utf8str.c_str(), utf8str.size(), wstr, wsize);
}

bool WStrToUtf8(std::wstring wstr, std::string& utf8str);
// size==real string size
bool WStrToUtf8(wchar_t* wstr, size_t size, std::string& utf8str);

size_t utf8length(std::string& utf8str);                    // set string to "" if invalid utf8 sequence
void utf8truncate(std::string& utf8str, size_t len);

inline bool isBasicLatinCharacter(wchar_t wchar)
{
	if (wchar >= L'a' && wchar <= L'z')                     // LATIN SMALL LETTER A - LATIN SMALL LETTER Z
		return true;
	if (wchar >= L'A' && wchar <= L'Z')                     // LATIN CAPITAL LETTER A - LATIN CAPITAL LETTER Z
		return true;
	return false;
}

inline bool isExtendedLatinCharacter(wchar_t wchar)
{
	if (isBasicLatinCharacter(wchar))
		return true;
	if (wchar >= 0x00C0 && wchar <= 0x00D6)                 // LATIN CAPITAL LETTER A WITH GRAVE - LATIN CAPITAL LETTER O WITH DIAERESIS
		return true;
	if (wchar >= 0x00D8 && wchar <= 0x00DF)                 // LATIN CAPITAL LETTER O WITH STROKE - LATIN CAPITAL LETTER THORN
		return true;
	if (wchar == 0x00DF)                                    // LATIN SMALL LETTER SHARP S
		return true;
	if (wchar >= 0x00E0 && wchar <= 0x00F6)                 // LATIN SMALL LETTER A WITH GRAVE - LATIN SMALL LETTER O WITH DIAERESIS
		return true;
	if (wchar >= 0x00F8 && wchar <= 0x00FE)                 // LATIN SMALL LETTER O WITH STROKE - LATIN SMALL LETTER THORN
		return true;
	if (wchar >= 0x0100 && wchar <= 0x012F)                 // LATIN CAPITAL LETTER A WITH MACRON - LATIN SMALL LETTER I WITH OGONEK
		return true;
	if (wchar == 0x1E9E)                                    // LATIN CAPITAL LETTER SHARP S
		return true;
	return false;
}

inline bool isCyrillicCharacter(wchar_t wchar)
{
	if (wchar >= 0x0410 && wchar <= 0x044F)                 // CYRILLIC CAPITAL LETTER A - CYRILLIC SMALL LETTER YA
		return true;
	if (wchar == 0x0401 || wchar == 0x0451)                 // CYRILLIC CAPITAL LETTER IO, CYRILLIC SMALL LETTER IO
		return true;
	return false;
}

inline bool isEastAsianCharacter(wchar_t wchar)
{
	if (wchar >= 0x1100 && wchar <= 0x11F9)                 // Hangul Jamo
		return true;
	if (wchar >= 0x3041 && wchar <= 0x30FF)                 // Hiragana + Katakana
		return true;
	if (wchar >= 0x3131 && wchar <= 0x318E)                 // Hangul Compatibility Jamo
		return true;
	if (wchar >= 0x31F0 && wchar <= 0x31FF)                 // Katakana Phonetic Ext.
		return true;
	if (wchar >= 0x3400 && wchar <= 0x4DB5)                 // CJK Ideographs Ext. A
		return true;
	if (wchar >= 0x4E00 && wchar <= 0x9FC3)                 // Unified CJK Ideographs
		return true;
	if (wchar >= 0xAC00 && wchar <= 0xD7A3)                 // Hangul Syllables
		return true;
	if (wchar >= 0xFF01 && wchar <= 0xFFEE)                 // Halfwidth forms
		return true;
	return false;
}

inline bool isWhiteSpace(char c)
{
	return ::isspace(int(c)) != 0;
}

inline bool isNumeric(wchar_t wchar)
{
	return (wchar >= L'0' && wchar <= L'9');
}

inline bool isNumeric(char c)
{
	return (c >= '0' && c <= '9');
}

inline bool isNumericOrSpace(wchar_t wchar)
{
	return isNumeric(wchar) || wchar == L' ';
}

inline bool isNumeric(char const* str)
{
	for (char const* c = str; *c; ++c)
		if (!isNumeric(*c))
			return false;

	return true;
}

inline bool isNumeric(std::string const& str)
{
	for (std::string::const_iterator itr = str.begin(); itr != str.end(); ++itr)
		if (!isNumeric(*itr))
			return false;

	return true;
}

inline bool isNumeric(std::wstring const& str)
{
	for (std::wstring::const_iterator itr = str.begin(); itr != str.end(); ++itr)
		if (!isNumeric(*itr))
			return false;

	return true;
}

inline bool isBasicLatinString(const std::wstring& wstr, bool numericOrSpace)
{
	for (size_t i = 0; i < wstr.size(); ++i)
		if (!isBasicLatinCharacter(wstr[i]) && (!numericOrSpace || !isNumericOrSpace(wstr[i])))
			return false;
	return true;
}

inline bool isExtendedLatinString(const std::wstring& wstr, bool numericOrSpace)
{
	for (size_t i = 0; i < wstr.size(); ++i)
		if (!isExtendedLatinCharacter(wstr[i]) && (!numericOrSpace || !isNumericOrSpace(wstr[i])))
			return false;
	return true;
}

inline bool isCyrillicString(const std::wstring& wstr, bool numericOrSpace)
{
	for (size_t i = 0; i < wstr.size(); ++i)
		if (!isCyrillicCharacter(wstr[i]) && (!numericOrSpace || !isNumericOrSpace(wstr[i])))
			return false;
	return true;
}

inline bool isEastAsianString(const std::wstring& wstr, bool numericOrSpace)
{
	for (size_t i = 0; i < wstr.size(); ++i)
		if (!isEastAsianCharacter(wstr[i]) && (!numericOrSpace || !isNumericOrSpace(wstr[i])))
			return false;
	return true;
}

inline void strToUpper(std::string& str)
{
	//	std::transform(str.begin(), str.end(), str.begin(), toupper);

	char s = 'A' - 'a';
	for (size_t i = 0; i < str.length(); i++) {
		char c = str[i];
		//::uppercase(c);
		if (c >= 'a' && c <= 'z') str[i] = c + s;
	}
	
}

inline void strToLower(std::string& str)
{
//	std::transform(str.begin(), str.end(), str.begin(), tolower);
	char s = 'a' - 'A';
	for (size_t i = 0; i < str.length(); i++) {
		char c = str[i];
		//::uppercase(c);
		if (c >= 'A' && c <= 'Z') str[i] = c + s;
	}

}

inline wchar_t wcharToUpper(wchar_t wchar)
{
	if (wchar >= L'a' && wchar <= L'z')                     // LATIN SMALL LETTER A - LATIN SMALL LETTER Z
		return wchar_t(uint16(wchar) - 0x0020);
	if (wchar == 0x00DF)                                    // LATIN SMALL LETTER SHARP S
		return wchar_t(0x1E9E);
	if (wchar >= 0x00E0 && wchar <= 0x00F6)                 // LATIN SMALL LETTER A WITH GRAVE - LATIN SMALL LETTER O WITH DIAERESIS
		return wchar_t(uint16(wchar) - 0x0020);
	if (wchar >= 0x00F8 && wchar <= 0x00FE)                 // LATIN SMALL LETTER O WITH STROKE - LATIN SMALL LETTER THORN
		return wchar_t(uint16(wchar) - 0x0020);
	if (wchar >= 0x0101 && wchar <= 0x012F)                 // LATIN SMALL LETTER A WITH MACRON - LATIN SMALL LETTER I WITH OGONEK (only %2=1)
	{
		if (wchar % 2 == 1)
			return wchar_t(uint16(wchar) - 0x0001);
	}
	if (wchar >= 0x0430 && wchar <= 0x044F)                 // CYRILLIC SMALL LETTER A - CYRILLIC SMALL LETTER YA
		return wchar_t(uint16(wchar) - 0x0020);
	if (wchar == 0x0451)                                    // CYRILLIC SMALL LETTER IO
		return wchar_t(0x0401);

	return wchar;
}

inline wchar_t wcharToUpperOnlyLatin(wchar_t wchar)
{
	return isBasicLatinCharacter(wchar) ? wcharToUpper(wchar) : wchar;
}

inline wchar_t wcharToLower(wchar_t wchar)
{
	if (wchar >= L'A' && wchar <= L'Z')                     // LATIN CAPITAL LETTER A - LATIN CAPITAL LETTER Z
		return wchar_t(uint16(wchar) + 0x0020);
	if (wchar >= 0x00C0 && wchar <= 0x00D6)                 // LATIN CAPITAL LETTER A WITH GRAVE - LATIN CAPITAL LETTER O WITH DIAERESIS
		return wchar_t(uint16(wchar) + 0x0020);
	if (wchar >= 0x00D8 && wchar <= 0x00DE)                 // LATIN CAPITAL LETTER O WITH STROKE - LATIN CAPITAL LETTER THORN
		return wchar_t(uint16(wchar) + 0x0020);
	if (wchar >= 0x0100 && wchar <= 0x012E)                 // LATIN CAPITAL LETTER A WITH MACRON - LATIN CAPITAL LETTER I WITH OGONEK (only %2=0)
	{
		if (wchar % 2 == 0)
			return wchar_t(uint16(wchar) + 0x0001);
	}
	if (wchar == 0x1E9E)                                    // LATIN CAPITAL LETTER SHARP S
		return wchar_t(0x00DF);
	if (wchar == 0x0401)                                    // CYRILLIC CAPITAL LETTER IO
		return wchar_t(0x0451);
	if (wchar >= 0x0410 && wchar <= 0x042F)                 // CYRILLIC CAPITAL LETTER A - CYRILLIC CAPITAL LETTER YA
		return wchar_t(uint16(wchar) + 0x0020);

	return wchar;
}

inline void wstrToUpper(std::wstring& str)
{
	std::transform(str.begin(), str.end(), str.begin(), wcharToUpper);
}

inline void wstrToLower(std::wstring& str)
{
	std::transform(str.begin(), str.end(), str.begin(), wcharToLower);
}

std::wstring GetMainPartOfName(std::wstring wname, uint32 declension);

//bool utf8ToConsole(const std::string& utf8str, std::string& conStr);
//bool consoleToUtf8(const std::string& conStr, std::string& utf8str);
bool Utf8FitTo(const std::string& str, std::wstring search);

//void utf8printf(FILE* out, const char* str, ...);
//void vutf8printf(FILE* out, const char* str, va_list* ap);


#endif // StrUtil_h__



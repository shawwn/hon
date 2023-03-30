// (C)2008 S2 Games
// k0_string.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k0_common.h"

#include "k0_string.h"
//=============================================================================

/*====================
  EscapeCharacters
  ====================*/
string	EscapeCharacters(const string &str, const string &sChars, const string &sEscape)
{
	string sReturn;

	size_t zPos(0);
	while (zPos != string::npos)
	{
		size_t zNext(str.find_first_of(sChars, zPos));
		if (zNext == string::npos)
			return sReturn + str.substr(zPos);

		sReturn += str.substr(zPos, zNext - zPos) + sEscape + str[zNext];
		zPos = zNext + 1;
	}

	return sReturn;
}

wstring	EscapeCharacters(const wstring &str, const wstring &sChars, const wstring &sEscape)
{
	wstring sReturn;

	size_t zPos(0);
	while (zPos != wstring::npos)
	{
		size_t zNext(str.find_first_of(sChars, zPos));
		if (zNext == wstring::npos)
			return sReturn + str.substr(zPos);

		sReturn += str.substr(zPos, zNext - zPos) + sEscape + str[zNext];
		zPos = zNext + 1;
	}

	return sReturn;
}


/*====================
  UnescapeCharacters
  ====================*/
string	UnescapeCharacters(const string &str, const string &sEscape)
{
	string sReturn;

	size_t zPos(0);
	while (zPos != string::npos)
	{
		size_t zNext(str.find(sEscape, zPos));
		if (zNext == string::npos)
			return sReturn + str.substr(zPos);

		sReturn += str.substr(zPos, zNext - zPos);
		zPos = zNext + 1;
	}

	return sReturn;
}

wstring	UnescapeCharacters(const wstring &str, const wstring &sEscape)
{
	wstring sReturn;

	size_t zPos(0);
	while (zPos != wstring::npos)
	{
		size_t zNext(str.find(sEscape, zPos));
		if (zNext == wstring::npos)
			return sReturn + str.substr(zPos);

		sReturn += str.substr(zPos, zNext - zPos);
		if (zNext + 1 < str.length())
			sReturn += str[zNext + 1];

		zPos = zNext + 2;
	}

	return sReturn;
}


/*====================
  LowerString
  ====================*/
string	LowerString(const string &sStr)
{
	string sRet(sStr);
	if (sRet.begin() == sRet.end())
		return sRet;
	transform(sRet.begin(), sRet.end(), sRet.begin(), tolower);
	return sRet;
}

wstring	LowerString(const wstring &sStr)
{
	wstring sRet(sStr);
	if (sRet.begin() == sRet.end())
		return sRet;
	transform(sRet.begin(), sRet.end(), sRet.begin(), towlower);
	return sRet;
}


/*====================
  UpperString
  ====================*/
tstring	UpperString(const tstring &str)
{
	tstring ret(str);
	if (ret.begin() == ret.end())
		return ret;
	transform(ret.begin(), ret.end(), ret.begin(), toupper);
	return ret;
}


/*====================
  UpperStringRef
 ====================*/
void	UpperStringRef(tstring &str)
{
	str = UpperString(str);
}


/*====================
  TokenizeString
 ====================*/
svector	TokenizeString(const string &sStr, char cSeparator, char cEscape)
{
	svector vReturn;

	size_t zLast(0);
	for (size_t zIndex(0); zIndex < sStr.length(); ++zIndex)
	{
		if (sStr[zIndex] == cSeparator && (zIndex == 0 || sStr[zIndex - 1] != cEscape))
		{
			vReturn.push_back(sStr.substr(zLast, zIndex - zLast));
			zLast = zIndex + 1;
		}
	}

	if (zLast < sStr.length())
		vReturn.push_back(sStr.substr(zLast));

	return vReturn;
}

wsvector	TokenizeString(const wstring &sStr, wchar_t cSeparator, wchar_t cEscape)
{
	wsvector	vReturn;

	size_t	zLast(0);
	for (size_t zIndex(0); zIndex < sStr.length(); ++zIndex)
	{
		if (sStr[zIndex] == cSeparator && (zIndex == 0 || sStr[zIndex - 1] != cEscape))
		{
			vReturn.push_back(sStr.substr(zLast, zIndex - zLast));
			zLast = zIndex + 1;
		}
	}
	if (zLast < sStr.length())
		vReturn.push_back(sStr.substr(zLast));

	return vReturn;
}


/*====================
  JoinString

  returns an string of all the strings in the vector joined by the joinBy string
 ====================*/
string	JoinString(const svector &vStrings, const string &sJoinBy)
{
	string ret;

	for (svector_cit it(vStrings.begin()); it != vStrings.end(); ++it)
		ret += *it + sJoinBy;
	return ret;
}

wstring	JoinString(const wsvector &vStrings, const wstring &sJoinBy)
{
	wstring ret;

	for (wsvector_cit it(vStrings.begin()); it != vStrings.end(); ++it)
		ret += *it + sJoinBy;
	return ret;
}


/*====================
  IsLineSeparator
 ====================*/
bool	IsLineSeparator(int c)
{
	static TCHAR s_cList[] = { '\x00', '\n', '\r' };

	for (uint n(0); n < sizeof(s_cList); ++n)
		if (c == s_cList[n])
			return true;
	return false;
}


/*====================
  IsTokenSeparator
 ====================*/
bool	IsTokenSeparator(int c)
{
	static TCHAR s_cList[] = { '\n', '\r', ' ', '\t' };

	for (uint n(0); n < sizeof(s_cList); ++n)
		if (c == s_cList[n])
			return true;
	return false;
}


/*====================
  IsEOL
  ====================*/
bool	IsEOL(int c)
{
	static TCHAR s_cList[] = { '\n', '\r' };

	for (uint n(0); n < sizeof(s_cList); ++n)
		if (c == s_cList[n])
			return true;
	return false;
}


/*====================
  IsNotDigit
  ====================*/
bool	IsNotDigit(int c)
{
	return !isdigit(c);
}


/*====================
  IsLetter
  ====================*/
bool	IsLetter(int c)
{
	if (c >= _T('A') && c <= _T('Z'))
		return true;
	if (c >= _T('a') && c <= _T('z'))
		return true;

	return false;
}


/*====================
  GetTimeString
  ====================*/
string	GetTimeString(uint uiMilliseconds, int iSeperations, uint uiPrecision, uint uiFlags)
{
	// Clamp seperations to allowed values
	iSeperations = CLAMP(iSeperations, -1, 2);

	// Read flags
	bool bAlphaSeperator((uiFlags & TIME_STRING_ALPHA_SEPERATOR) != 0);
	bool bPadZero((uiFlags & TIME_STRING_PAD_ZEROS) != 0);

	// Round to seconds for zero precision
	if (uiPrecision == 0)
	{
		uint uiFraction(uiMilliseconds % MS_PER_SEC);
		uiMilliseconds -= uiFraction;

		// Countdowns round up
		if ((uiFlags & TIME_STRING_COUNTDOWN) && uiFraction > 0)
			uiMilliseconds += MS_PER_SEC;
	}

	string sReturn;

	// Hours
	bool bShowHours(iSeperations == 2 || (iSeperations == -1 && uiMilliseconds >= HrToMs(1u)));
	if (bShowHours)
	{
		uint uiHours(INT_FLOOR(MsToHr(uiMilliseconds)));
		uiMilliseconds %= MS_PER_HR;

		sReturn += XtoS(uiHours);
		sReturn += bAlphaSeperator ? "h " : ":";
		
		if (!bAlphaSeperator)
			bPadZero = true;
	}

	// Minutes
	bool bShowMins(bShowHours || iSeperations >= 1 || (iSeperations == -1 && uiMilliseconds >= MinToMs(1u)));
	if (bShowMins)
	{
		uint uiMins(INT_FLOOR(MsToMin(uiMilliseconds)));
		uiMilliseconds %= MS_PER_MIN;

		sReturn += XtoS(uiMins, bPadZero ? FMT_PADZERO : 0, bPadZero ? 2 : 0);
		sReturn += bAlphaSeperator ? "m " : ":";
		
		if (!bAlphaSeperator)
			bPadZero = true;
	}

	// Seconds
	float fSec(MsToSec(uiMilliseconds));
	float fPow(pow(10.0f, float(uiPrecision)));

	// Chop off remaining decimals past precision point
	fSec = (fSec * fPow) / fPow;

	if (bPadZero)
		sReturn += XtoS(fSec, FMT_PADZERO, 2 + uiPrecision + (uiPrecision > 0 ? 1 : 0), uiPrecision);
	else
		sReturn += XtoS(fSec, 0, 0, uiPrecision);

	if (bAlphaSeperator)
		sReturn += "s";

	return sReturn;
}

wstring	GetTimeStringW(uint uiMilliseconds, int iSeperations, uint uiPrecision, uint uiFlags)
{
	// Clamp seperations to allowed values
	iSeperations = CLAMP(iSeperations, -1, 2);

	// Read flags
	bool bAlphaSeperator((uiFlags & TIME_STRING_ALPHA_SEPERATOR) != 0);
	bool bPadZero((uiFlags & TIME_STRING_PAD_ZEROS) != 0);

	// Round to seconds for zero precision
	if (uiPrecision == 0)
	{
		uint uiFraction(uiMilliseconds % MS_PER_SEC);
		uiMilliseconds -= uiFraction;

		// Countdowns round up
		if ((uiFlags & TIME_STRING_COUNTDOWN) && uiFraction > 0)
			uiMilliseconds += MS_PER_SEC;
	}

	wstring sReturn;

	// Hours
	bool bShowHours(iSeperations == 2 || (iSeperations == -1 && uiMilliseconds >= HrToMs(1u)));
	if (bShowHours)
	{
		uint uiHours(INT_FLOOR(MsToHr(uiMilliseconds)));
		uiMilliseconds %= MS_PER_HR;

		sReturn += XtoW(uiHours);
		sReturn += bAlphaSeperator ? L"h " : L":";
		
		if (!bAlphaSeperator)
			bPadZero = true;
	}

	// Minutes
	bool bShowMins(bShowHours || iSeperations >= 1 || (iSeperations == -1 && uiMilliseconds >= MinToMs(1u)));
	if (bShowMins)
	{
		uint uiMins(INT_FLOOR(MsToMin(uiMilliseconds)));
		uiMilliseconds %= MS_PER_MIN;

		sReturn += XtoW(uiMins, bPadZero ? FMT_PADZERO : 0, bPadZero ? 2 : 0);
		sReturn += bAlphaSeperator ? L"m " : L":";
		
		if (!bAlphaSeperator)
			bPadZero = true;
	}

	// Seconds
	float fSec(MsToSec(uiMilliseconds));
	float fPow(pow(10.0f, float(uiPrecision)));

	// Chop off remaining decimals past precision point
	fSec = (fSec * fPow) / fPow;

	if (bPadZero)
		sReturn += XtoW(fSec, FMT_PADZERO, 2 + uiPrecision + (uiPrecision > 0 ? 1 : 0), uiPrecision);
	else
		sReturn += XtoW(fSec, 0, 0, uiPrecision);

	if (bAlphaSeperator)
		sReturn += L"s";

	return sReturn;
}


/*====================
  IsIPAddress

  determines whether a given string is a valid ip address in the form (255.255.255.255)
  ====================*/
bool	IsIPAddress(const string &sIP)
{
	if (sIP.empty())
		return false;

	// Locate the seperators
	size_t dot1 = sIP.find_first_of('.');
	size_t dot2 = sIP.find_first_of('.', dot1 + 1);
	size_t dot3 = sIP.find_first_of('.', dot2 + 1);
	size_t dot4 = sIP.find_first_of('.', dot3 + 1); // sanity check for extra dots

	// If the three seperators don't exist,
	// or more than three exist,
	// we know this isn't a valid ip address
	if (dot1 == string::npos ||
		dot2 == string::npos ||
		dot3 == string::npos ||
		dot4 != string::npos)
		return false;

	// Each part of the ip must contain only digits
	string s1(sIP.substr(0, dot1));
	string s2(sIP.substr(dot1 + 1, dot2 - dot1 - 1));
	string s3(sIP.substr(dot2 + 1, dot3 - dot2 - 1));
	string s4(sIP.substr(dot3 + 1));

	if (count_if(s1.begin(), s1.end(), IsNotDigit) > 0 ||
		count_if(s2.begin(), s2.end(), IsNotDigit) > 0 ||
		count_if(s3.begin(), s3.end(), IsNotDigit) > 0 ||
		count_if(s4.begin(), s4.end(), IsNotDigit) > 0)
		return false;

	// Each component must be in the range 0 - 255
	int n1 = AtoI(s1);
	int n2 = AtoI(s2);
	int n3 = AtoI(s3);
	int n4 = AtoI(s4);

	if (n1 < 0 || n1 > 255 ||
		n2 < 0 || n2 > 255 ||
		n3 < 0 || n3 > 255 ||
		n4 < 0 || n4 > 255)
		return false;

	return true;
}

bool	IsIPAddress(const wstring &sIP)
{
	if (sIP.empty())
		return false;

	// Locate the seperators
	size_t dot1 = sIP.find_first_of(L'.');
	size_t dot2 = sIP.find_first_of(L'.', dot1 + 1);
	size_t dot3 = sIP.find_first_of(L'.', dot2 + 1);
	size_t dot4 = sIP.find_first_of(L'.', dot3 + 1); // sanity check for extra dots

	// If the three seperators don't exist,
	// or more than three exist,
	// we know this isn't a valid ip address
	if (dot1 == wstring::npos ||
		dot2 == wstring::npos ||
		dot3 == wstring::npos ||
		dot4 != wstring::npos)
		return false;

	// Each part of the ip must contain only digits
	wstring s1(sIP.substr(0, dot1));
	wstring s2(sIP.substr(dot1 + 1, dot2 - dot1 - 1));
	wstring s3(sIP.substr(dot2 + 1, dot3 - dot2 - 1));
	wstring s4(sIP.substr(dot3 + 1));

	if (count_if(s1.begin(), s1.end(), IsNotDigit) > 0 ||
		count_if(s2.begin(), s2.end(), IsNotDigit) > 0 ||
		count_if(s3.begin(), s3.end(), IsNotDigit) > 0 ||
		count_if(s4.begin(), s4.end(), IsNotDigit) > 0)
		return false;

	// Each component must be in the range 0 - 255
	int n1 = AtoI(s1);
	int n2 = AtoI(s2);
	int n3 = AtoI(s3);
	int n4 = AtoI(s4);

	if (n1 < 0 || n1 > 255 ||
		n2 < 0 || n2 > 255 ||
		n3 < 0 || n3 > 255 ||
		n4 < 0 || n4 > 255)
		return false;

	return true;
}


/*====================
  CompareNoCase
  ====================*/
int		CompareNoCase(const string &s1, const string &s2)
{
	string::const_iterator p1 = s1.begin();
	string::const_iterator p2 = s2.begin();

	while (p1 != s1.end() && p2 != s2.end())
	{
		if (tolower(*p1) < tolower(*p2))
			return -1;
		else if (tolower(*p1) > tolower(*p2))
			return 1;

		++p1;
		++p2;
	}

	return p1 == s1.end() ? p2 == s2.end() ? 0 : -1 : 1;
}

int		CompareNoCase(const wstring &s1, const wstring &s2)
{
	wstring::const_iterator p1 = s1.begin();
	wstring::const_iterator p2 = s2.begin();

	while (p1 != s1.end() && p2 != s2.end())
	{
		if (tolower(*p1) < tolower(*p2))
			return -1;
		else if (tolower(*p1) > tolower(*p2))
			return 1;

		++p1;
		++p2;
	}

	return p1 == s1.end() ? p2 == s2.end() ? 0 : -1 : 1;
}


/*====================
  EqualsWildcard
  ====================*/
bool	EqualsWildcard(const tstring &sWild, const tstring &sComp)
{
	tstring::size_type zStart(0);
	tstring::size_type zEnd(tstring::npos);
	tstring::size_type zPos(0);

	tstring::size_type zLengthWild(sWild.length());
	tstring::size_type zLengthComp(sComp.length());

	// Loop until we hit the end of the string
	while (zStart < zLengthWild && zPos < zLengthComp)
	{
		// Find the first wildcard character
		zEnd = sWild.find_first_of(_T("?*"), zStart);

		// If the text from the current position to the next wildcard character
		// (or the end of the string, whichever is first) doesn't match, return false
		if (sWild.substr(zStart, zEnd - zStart) != sComp.substr(zPos, zEnd - zPos))
			return false;

		// If we hit the end of the string and the text does match, break out
		if (zEnd == tstring::npos)
			break;

		// If it's a single character wildcard
		if (sWild[zEnd] == _T('?'))
		{
			// Skip the current character in both strings, anything will match
			zPos += (zEnd - zStart) + 1;
			zStart = zEnd + 1;
		}
		else // Otherwise, if it's a * (any length) wildcard
		{
			// Set the new positions to directly after where the wildcard occurs
			zPos += (zEnd - zStart);
			zStart = zEnd + 1;

			// If the wildcard ends the string, then anything past here matches - return true
			if (zStart >= zLengthWild)
				return true;

			// Find the next wildcard
			zEnd = sWild.find_first_of(_T("?*"), zStart);

			// Find the string from the current position to the next wildcard in the
			// string we're comparing
			tstring::size_type zFind;
			zFind = sComp.find(sWild.substr(zStart, zEnd - zStart), zPos);

			// While we successfully matched it... (check all matches)
			while (zFind != tstring::npos)
			{
				// Recursively check the remaining text
				if (EqualsWildcard(sWild.substr(zStart), sComp.substr(zFind)))
					return true;

				// Otherwise, look for the next match
				zFind = sComp.find(sWild.substr(zStart, zEnd - zStart), zFind + 1);
			}
		}
	}

	// If we've hit the end of both strings at this point (or the same amount
	// of length is remaining), they're a match, return true. Otherwise, return false.
	return (zLengthWild - zStart == zLengthComp - zPos);
}


/*====================
  CompareNum
  ====================*/
int		CompareNum(const tstring &s1, const tstring &s2, size_t n)
{
	tstring::const_iterator p1 = s1.begin();
	tstring::const_iterator p2 = s2.begin();

	while (p1 != s1.end() && p2 != s2.end() && n != 0)
	{
		if (*p1 < *p2)
			return -1;
		else if (*p1 > *p2)
			return 1;

		++p1;
		++p2;
		--n;
	}

	if (n == 0)
		return 0;
	else
		return p1 == s1.end() ? p2 == s2.end() ? 0 : -1 : 1;
}


/*====================
  CompareNum
  ====================*/
int		CompareNum(const tstring &s1, const TCHAR *s2, size_t n)
{
	tstring::const_iterator p1 = s1.begin();
	const TCHAR *p2 = s2;

	while (p1 != s1.end() && *p2 != 0 && n != 0)
	{
		if (*p1 < *p2)
			return -1;
		else if (*p1 > *p2)
			return 1;

		++p1;
		++p2;
		--n;
	}

	if (n == 0)
		return 0;
	else
		return p1 == s1.end() ? p2 == NULL ? 0 : -1 : 1;
}


/*====================
  CompareNoCaseNum
  ====================*/
int		CompareNoCaseNum(const tstring &s1, const tstring &s2, size_t n)
{
	tstring::const_iterator p1 = s1.begin();
	tstring::const_iterator p2 = s2.begin();

	while (p1 != s1.end() && p2 != s2.end() && n != 0)
	{
		if (tolower(*p1) < tolower(*p2))
			return -1;
		else if (tolower(*p1) > tolower(*p2))
			return 1;

		++p1;
		++p2;
		--n;
	}

	if (n == 0)
		return 0;
	else
		return p1 == s1.end() ? p2 == s2.end() ? 0 : -1 : 1;
}


/*====================
  StripEOL
 ====================*/
char*	StripEOL(char *str)
{
	char *s = str;

	while (*s)
	{
		if (IsLineSeparator(*s))
		{
			*s = 0;
			return str;
		}
		++s;
	}
	return str;
}


/*====================
  GetNextWord
 ====================*/
char*	GetNextWord(const char *str)
{
	char *s = const_cast<char *>(str);

	while (!IsTokenSeparator(*s) && *s)
		++s;

	if (!*s)
		return s;

	while (IsTokenSeparator(*s) && *s)
		++s;

	if (!*s)
		return s;
	return s;
}

tstring	GetNextWord(const tstring &sStr, size_t &zStart)
{
	size_t zEnd(sStr.find_first_of(_T("\n\r\t "), zStart));
	tstring sWord(sStr.substr(zStart, zEnd));
	zStart = zEnd;

	return sWord;
}


/*====================
  SkipSpaces
 ====================*/
TCHAR*	SkipSpaces(TCHAR *str)
{
	TCHAR *s = str;

	while (*s) {
		switch (*s) {
			case _T(' '):
			case _T('\n'):
			case _T('\r'):
			case _T('\t'):
				++s;
				break;

			default:
				return s;
		}
	}

	return s;
}

size_t	SkipSpaces(const string &sStr, size_t zStart)
{
	size_t zResult(sStr.find_first_not_of("\n\r\t ", zStart));
	return zResult;
}

size_t	SkipSpaces(const wstring &sStr, size_t zStart)
{
	size_t zResult(sStr.find_first_not_of(L"\n\r\t ", zStart));
	return zResult;
}


/*====================
  Filename_GetExtension
 ====================*/
string	Filename_GetExtension(const string &sPath)
{
	size_t zEnd(sPath.find_last_of('.'));

	if (zEnd == string::npos)
		return SNULL;
	else
		return sPath.substr(zEnd + 1);
}

wstring	Filename_GetExtension(const wstring &sPath)
{
	size_t zEnd(sPath.find_last_of(L'.'));

	if (zEnd == wstring::npos)
		return WSNULL;
	else
		return sPath.substr(zEnd + 1);
}


/*====================
  Filename_GetName
  ====================*/
string	Filename_GetName(const string &sPath)
{
	if (sPath.empty())
		return SNULL;

	size_t zStart(sPath.find_last_of("/"));
	size_t zEnd(sPath.find_last_of("."));

	if (zStart == string::npos)
	{
		if (sPath.length() > 0 && (sPath[0] == '~' || sPath[0] == '@' || sPath[0] == ':'))
			zStart = 1;
		else
			zStart = 0;
	}
	else
	{
		zStart += 1;
	}

	if (zStart >= zEnd)
		return SNULL;

	if (zEnd == string::npos)
		return sPath.substr(zStart, string::npos);
	else
		return sPath.substr(zStart, zEnd - zStart);
}

wstring	Filename_GetName(const wstring &sPath)
{
	if (sPath.empty())
		return WSNULL;

	size_t zStart(sPath.find_last_of(L"/"));
	size_t zEnd(sPath.find_last_of(L"."));

	if (zStart == wstring::npos)
	{
		if (sPath.length() > 0 && (sPath[0] == L'~' || sPath[0] == L'@' || sPath[0] == L':'))
			zStart = 1;
		else
			zStart = 0;
	}
	else
	{
		zStart += 1;
	}

	if (zStart >= zEnd)
		return WSNULL;

	if (zEnd == wstring::npos)
		return sPath.substr(zStart, wstring::npos);
	else
		return sPath.substr(zStart, zEnd - zStart);
}


/*====================
  Filename_GetPath
 ====================*/
string	Filename_GetPath(const string &sPath)
{
	size_t zEnd(sPath.find_last_of("/"));

	if (zEnd == string::npos)
	{
		if (sPath.length() > 0 && (sPath[0] == '~' || sPath[0] == '@' || sPath[0] == ':'))
			return sPath.substr(0, 1);
		
		return SNULL;
	}

	return sPath.substr(0, zEnd + 1);
}

wstring	Filename_GetPath(const wstring &sPath)
{
	size_t zEnd(sPath.find_last_of(L"/"));

	if (zEnd == wstring::npos)
	{
		if (sPath.length() > 0 && (sPath[0] == L'~' || sPath[0] == L'@' || sPath[0] == L':'))
			return sPath.substr(0, 1);
		
		return WSNULL;
	}

	return sPath.substr(0, zEnd + 1);
}


/*====================
  Filename_StripPath
  ====================*/
string		Filename_StripPath(const string &sPath)
{
	if (sPath.empty())
		return SNULL;

	size_t zStart(sPath.find_last_of("/"));

	if (zStart == string::npos)
	{
		if (sPath.length() > 0 && (sPath[0] == '~' || sPath[0] == '@' || sPath[0] == ':'))
			return sPath.substr(1, sPath.length() - 1);

		return sPath;
	}

	return sPath.substr(zStart + 1, string::npos);
}

wstring		Filename_StripPath(const wstring &sPath)
{
	if (sPath.empty())
		return WSNULL;

	size_t zStart(sPath.find_last_of(L"/"));

	if (zStart == wstring::npos)
	{
		if (sPath.length() > 0 && (sPath[0] == L'~' || sPath[0] == L'@' || sPath[0] == L':'))
			return sPath.substr(1, sPath.length() - 1);

		return sPath;
	}

	return sPath.substr(zStart + 1, wstring::npos);
}


/*====================
  Filename_SanitizePath
  ====================*/
string	Filename_SanitizePath(const string &sPath)
{
	size_t pos(0);
	string sReturn(sPath);

	if (sPath.empty() || sPath[0] == '$' || sPath[0] == '!')
		return sPath;

	// Keep drive letter lower case
	if (sPath.size() > 1 && sPath[1] == ':')
		sReturn[0] = tolower(sPath[0]);

	// Convert all \ to /
	while ((pos = sReturn.find('\\', pos)) != string::npos)
		sReturn.replace(pos, 1, "/");

	// Remove any instances of consecutive //// (except for protocol markers)
	pos = 0;
	while (sReturn.length() - pos >= 2 && (pos = sReturn.find("//", pos)) != string::npos)
	{
		if (pos > 0 && sReturn[pos - 1] == ':')
		{
			pos += 1;
			continue;
		}

		size_t end = sReturn.find_first_not_of("/", pos);
		if (end > sReturn.length())
			sReturn = sReturn.substr(0, pos + 1);
		else
			sReturn = sReturn.substr(0, pos + 1) + sReturn.substr(end);
	}

	// Clean out all the ".."
	// Mostly, this just shortens the path to a standard format so that it can
	// be safely compared, but it will also discard any /.. that try to go above
	// the root game directory
	pos = 0;
	while (sReturn.length() - pos >= 3 && (pos = sReturn.find("/..")) != string::npos)
	{
		// if the directory tries to go above the root, stop it there
		if (sReturn.find("/") >= pos)
		{
			size_t next = sReturn.find("/", pos + 1);
			if (next > sReturn.length())
				sReturn = "/";
			else
				sReturn = sReturn.substr(next);

			continue;
		}

		size_t end = sReturn.find("/", pos + 1);
		if (end > sReturn.length())
			sReturn = sReturn.substr(0, sReturn.find_last_of("/", pos - 1) + 1);
		else
			sReturn = sReturn.substr(0, sReturn.find_last_of("/", pos - 1) + 1) + sReturn.substr(end + 1);
	}

	return sReturn;
}

wstring	Filename_SanitizePath(const wstring &sPath)
{
	size_t pos(0);
	wstring sReturn(sPath);

	if (sPath.empty() || sPath[0] == L'$' || sPath[0] == L'!')
		return sPath;

	// Keep drive letter lower case
	if (sPath.size() > 1 && sPath[1] == L':')
		sReturn[0] = towlower(sPath[0]);

	// Convert all \ to /
	while ((pos = sReturn.find(L'\\', pos)) != wstring::npos)
		sReturn.replace(pos, 1, L"/");

	// Remove any instances of consecutive //// (except for protocol markers)
	pos = 0;
	while (sReturn.length() - pos >= 2 && (pos = sReturn.find(L"//", pos)) != wstring::npos)
	{
		if (pos > 0 && sReturn[pos - 1] == L':')
		{
			pos += 1;
			continue;
		}

		size_t end = sReturn.find_first_not_of(L"/", pos);
		if (end > sReturn.length())
			sReturn = sReturn.substr(0, pos + 1);
		else
			sReturn = sReturn.substr(0, pos + 1) + sReturn.substr(end);
	}

	// Clean out all the ".."
	// Mostly, this just shortens the path to a standard format so that it can
	// be safely compared, but it will also discard any /.. that try to go above
	// the root game directory
	pos = 0;
	while (sReturn.length() - pos >= 3 && (pos = sReturn.find(L"/..")) != wstring::npos)
	{
		// if the directory tries to go above the root, stop it there
		if (sReturn.find(L"/") >= pos)
		{
			size_t next = sReturn.find(L"/", pos + 1);
			if (next > sReturn.length())
				sReturn = L"/";
			else
				sReturn = sReturn.substr(next);

			continue;
		}

		size_t end = sReturn.find(L"/", pos + 1);
		if (end > sReturn.length())
			sReturn = sReturn.substr(0, sReturn.find_last_of(L"/", pos - 1) + 1);
		else
			sReturn = sReturn.substr(0, sReturn.find_last_of(L"/", pos - 1) + 1) + sReturn.substr(end + 1);
	}

	return sReturn;
}


/*====================
  StrChrBackwards
 ====================*/
char*	StrChrBackwards(const char *str, int c)
{
	int i = int(strlen(str)) - 1;
	for (; i >= 0; --i)
	{
		if (str[i] == c)
			return (char*)&str[i];
	}
	if (i < 0)
		i = 0;
	return const_cast < char *>(&str[i]);
}


/*====================
  FirstToken
 ====================*/
char*	FirstToken(const char *str)
{
	static char buf[NUM_TOK_STRINGS][128];
	static uint marker = 0;
	uint idx = marker;
	char *s = const_cast < char *>(str);
	int pos = 0;

	while(*s) {
		if (IsTokenSeparator(*s))
			break;

		buf[idx][pos] = *s;
		++pos;
		if (pos >= 128)
			return "";
		++s;
	}
	buf[idx][pos] = 0;
	marker = (marker + 1) % NUM_TOK_STRINGS;
	return buf[idx];
}


/*====================
  SplitArgs
 ====================*/
int	SplitArgs(TCHAR *in, TCHAR **argv, int maxargs)
{
	TCHAR *s = in;
	int argc = 0;

	s = SkipSpaces(s);

	while (*s)
	{
		argv[argc] = s;
		++argc;

		if (argc >= maxargs)
			return argc;

		++s;

		while (!IsTokenSeparator(*s) && *s)
			++s;

		if (*s)
		{
			*s = 0;		//null terminate each arg
			s = SkipSpaces(s + 1);
		}
	}
	return argc;
}

int		SplitArgs(const tstring &sString, tsvector &vArgList)
{
	size_t	zPos = sString.find_first_not_of(_T(" \n\r\t"));

	while (zPos != tstring::npos)
	{
		size_t zNextPos = sString.find_first_of(_T(" \n\r\t"), zPos + 1);

		if (zNextPos != tstring::npos)
			vArgList.push_back(sString.substr(zPos, zNextPos - zPos));
		else if (zPos + 1 < sString.length())
			vArgList.push_back(sString.substr(zPos));

		zPos = sString.find_first_not_of(_T(" \n\r\t"), zNextPos);
	}

	return int(vArgList.size());
}


/*====================
  ConcatinateArgs
 ====================*/
string	ConcatinateArgs(const svector &vArgList, const string &sSeperator)
{
	string sResult;

	for (svector_cit it(vArgList.begin()); it != vArgList.end(); ++it)
	{
		sResult += (*it);

		if (it + 1 != vArgList.end())
			sResult += sSeperator;
	}

	return sResult;
}

wstring	ConcatinateArgs(const wsvector &vArgList, const wstring &sSeperator)
{
	wstring sResult;

	for (wsvector_cit it(vArgList.begin()); it != vArgList.end(); ++it)
	{
		sResult += (*it);

		if (it + 1 != vArgList.end())
			sResult += sSeperator;
	}

	return sResult;
}

tstring	ConcatinateArgs(tsvector_cit begin, tsvector_cit end, const tstring &sSeperator)
{
	tstring sResult;

	for (tsvector_cit it(begin); it != end; ++it)
	{
		sResult += (*it);

		if (it + 1 != end)
			sResult += sSeperator;
	}

	return sResult;
}


/*====================
  StripDuplicateSpaces
  ====================*/
string	StripDuplicateSpaces(const string &sStr)
{
	size_t zPos(0);
	size_t zEndPos(0);
	string sReturn(sStr);

	while (zPos != string::npos)
	{
		zPos = sReturn.find_first_of(" ", zPos);

		if (zPos != string::npos)
			zPos++;

		if (zPos < sReturn.length())
		{
			zEndPos = SkipSpaces(sReturn, zPos);

			if (zEndPos == zPos)
				continue;

			if (zEndPos != string::npos)
				sReturn.erase(zPos, zEndPos);
			else
				sReturn.erase(zPos, sReturn.length() - zPos);
		}
	}

	return sReturn;
}

wstring	StripDuplicateSpaces(const wstring &sStr)
{
	size_t zPos(0);
	size_t zEndPos(0);
	wstring sReturn(sStr);

	while (zPos != wstring::npos)
	{
		zPos = sReturn.find_first_of(L" ", zPos);

		if (zPos != wstring::npos)
			zPos++;

		if (zPos < sReturn.length())
		{
			zEndPos = SkipSpaces(sReturn, zPos);

			if (zEndPos == zPos)
				continue;

			if (zEndPos != wstring::npos)
				sReturn.erase(zPos, zEndPos);
			else
				sReturn.erase(zPos, sReturn.length() - zPos);
		}
	}

	return sReturn;
}


/*====================
  StripExtraneousSpacesRef
  ====================*/
string&		StripExtraneousSpacesRef(string &s)
{
	StripStartingSpacesRef(s);
	StripTrailingSpaces(s);

	size_t zStart(0);
	size_t zEnd(0);

	for (;;)
	{
		zStart = s.find_first_of(" ", zStart);
		if (zStart == string::npos)
			break;

		++zStart;
		zEnd = s.find_first_not_of(" ", zStart + 1);
		if (zEnd == string::npos)
			s.erase(zStart);
		else
			s.erase(zStart, zEnd - zStart);
	}

	return s;
}

wstring&	StripExtraneousSpacesRef(wstring &s)
{
	StripStartingSpacesRef(s);
	StripTrailingSpacesRef(s);

	size_t zStart(0);
	size_t zEnd(0);

	for (;;)
	{
		zStart = s.find_first_of(L" ", zStart);
		if (zStart == wstring::npos)
			break;

		++zStart;
		zEnd = s.find_first_not_of(L" ", zStart);
		if (zEnd == wstring::npos)
			s.erase(zStart);
		else
			s.erase(zStart, zEnd - zStart);
	}

	return s;
}


/*====================
  SingleToWide

  Returns a wide char version of 'in', without any special conversion.
  'len' is the maximum length of the 'out' buffer
  ====================*/
wchar_t*	SingleToWide(wchar_t *out, const char *in, size_t len)
{
	for (size_t n = 0; n < len; ++n)
	{
		out[n] = in[n];
		if (in[n] == '\x00')
			break;
	}

	return out;
}

wstring		SingleToWide(const string &sIn)
{
	wstring sOut;
	for (size_t z(0); z < sIn.length(); ++z)
		sOut += wchar_t(sIn[z]);

	return sOut;
}


/*====================
  WideToSingle

  Returns a single char version of 'in', truncating the wide chars in 'out'.
  'len' is the maximum length of the 'out' buffer
  ====================*/
char*	WideToSingle(char *out, const wchar_t *in, size_t len)
{
	for (size_t n = 0; n < len; ++n)
	{
		if (in[n] == L'\x00')
			break;
		out[n] = char(in[n]);
	}

//	Console.Perf << _T("WideToSingle()") << newl;
	return out;
}

string		WideToSingle(const wstring &sIn)
{
	string sOut;
	for (size_t z(0); z < sIn.length(); ++z)
		sOut += char(sIn[z]);

//	Console.Perf << _T("WideToSingle() : ") << newl;
	return sOut;
}


/*====================
  UTF8to16
  ====================*/
wchar_t*	UTF8to16(wchar_t *out, const char *in, size_t len)
{
	unsigned long *decoded = new unsigned long[len];
	int count = 0;
	const char *at = in;

	// decode the UTF-8 into an array
	while (*at != '\x00')
	{
		// ignore anything in the ASCII equivelant range
		if (((*at) & 0x80) == 0)
		{
			decoded[count++] = *at;
			++at;
			continue;
		}

		// determine how many bytes are in this character
		uint mask(0x80);
		int numbytes = 0;
		while (*at & mask)
		{
			++numbytes;
			mask >>= 1;
		}

		// assign the remaining bits of the first byte
		mask -= 1;
		unsigned long c;
		c = (*at & mask) << ((numbytes - 1) * 6);
		++at;

		// assemble the remaining bytes
		for (int b = 2; b <= numbytes; ++b)
		{
			if ((*at == '\x00') || (*at & 0xc0) != 0x80)
			{
				cerr << _T("UTF8to16(): Invalid UTF-8 string!") << newl;
				out[0] = L'\x0000';
				delete[] decoded;
				return out;
			}

			c |= (*at & 0x3f) << ((numbytes - b) * 6);
			++at;
		}

		decoded[count++] = c;
	}

	// create the utf-16 string
	wchar_t *wat = out;
	for (int o = 0; o < count; ++o)
	{
		// 16 bit characters remain unchanged
		if (decoded[o] < 0xffff)
		{
			*wat = wchar_t(decoded[o]);
			++wat;
			continue;
		}

		// construct a surrogate pair
		wchar_t c0 = 0xd800, c1 = 0xdc00;
		c0 |= ((decoded[o] - 0x10000) >> 10) & 0x03ff;
		c1 |= (decoded[o] - 0x10000) & 0x03ff;
		*wat = c0;
		++wat;
		*wat = c1;
		++wat;
	}

	delete[] decoded;
	return out;
}


/*====================
  UTF16to8
  ====================*/
char*	UTF16to8(char *out, const wchar_t *in, size_t len)
{
	unsigned long *decoded = new unsigned long[len];
	size_t count = 0;
	const wchar_t *wat = in;

	// Decode the UTF-16 into an array
	while (*wat != L'\x0000')
	{
		// Watch out for overflow (preliminary check)
		if (count >= len - 1)
		{
			cerr << _T("UTF16to8(): Truncating input string") << newl;
			break;
		}

		// This is part of a surrogate pair
		if (*wat >= 0xd800 && *wat <= 0xdfff)
		{
			// TODO: Handle endianess?
			// Save the first 10 bits
			unsigned long c = (*wat & 0x03ff) << 10;

			// Advance and verify next byte
			++wat;
			if (((*wat) & 0xfc00) != 0xdc00)
			{
				cerr << _T("UTF16to8(): Invalid UTF-16 string") << newl;
				out[0] = '\x00';
				delete[] decoded;
				return out;
			}

			// Get the second set of 10 bits
			c |= (*wat & 0x03ff);
			decoded[count++] = c;
			continue;
		}

		// No decoding necessary
		decoded[count++] = *wat;
		++wat;
	}

	// Create the UTF-8 string
	char *at = out;
	size_t b = 0;
	for (size_t o = 0; o < count; ++o)
	{
		// Single byte
		if ((decoded[o] & 0xffffff80) == 0)
		{
			// Check for overflow
			if (b + 1 >= len - 1)
			{
				cerr << _T("UTF16to8(): Truncating output") << newl;
				*at = '\x00';
				delete[] decoded;
				return out;
			}

			*at = char(decoded[o] & 0x0000007f);
			++at;
			++b;
			continue;
		}

		// Two bytes
		if ((decoded[o] & 0xfffff800) == 0)
		{
			// Check for overflow
			if (b + 2 >= len - 1)
			{
				cerr << _T("UTF16to8(): Truncating output") << newl;
				*at = '\x00';
				delete[] decoded;
				return out;
			}

			*at = 0xc0 | char((decoded[o] >> 6) & 0x1f);
			++at;
			*at = 0x80 | char(decoded[o] & 0x3f);
			++at;
			b += 2;
			continue;
		}

		// Three bytes
		if ((decoded[o] & 0xffff0000) == 0)
		{
			// Check for overflow
			if (b + 3 >= len - 1)
			{
				cerr << _T("UTF16to8(): Truncating output") << newl;
				*at = '\x00';
				delete[] decoded;
				return out;
			}

			*at = 0xe0 | char((decoded[o] >> 12) & 0x1f);
			++at;
			*at = 0x80 | char((decoded[o] >> 6) & 0x3f);
			++at;
			*at = 0x80 | char(decoded[o] & 0x3f);
			++at;
			b += 3;
			continue;
		}

		// Four bytes

		// Check for overflow
		if (b + 4 >= len - 1)
		{
			cerr << _T("UTF16to8(): Truncating output") << newl;
			*at = '\x00';
			delete[] decoded;
			return out;
		}

		*at = 0xf0 | char((decoded[o] >> 18) & 0x1f);
		++at;
		*at = 0x80 | char((decoded[o] >> 12) & 0x3f);
		++at;
		*at = 0x80 | char((decoded[o] >> 6) & 0x3f);
		++at;
		*at = 0x80 | char(decoded[o] & 0x3f);
		++at;
		b += 4;
	}

	*at = '\x00';
	delete[] decoded;
	return out;
}

string	UTF16to8(const wstring &in)
{
	unsigned long *decoded = new unsigned long[in.length()];
	size_t count = 0;
	const wchar_t *wat = in.c_str();
	string out;

	// Decode the UTF-16 into an array
	while (*wat != L'\x0000')
	{
		// Watch out for overflow (preliminary check)
		if (count >= in.length())
		{
			cerr << _T("UTF16to8(): Truncating input string") << newl;
			break;
		}

		// This is part of a surrogate pair
		if (*wat >= 0xd800 && *wat <= 0xdfff)
		{
			// TODO: Handle endianess?
			// Save the first 10 bits
			unsigned long c = (*wat & 0x03ff) << 10;

			// Advance and verify next byte
			++wat;
			if (((*wat) & 0xfc00) != 0xdc00)
			{
				cerr << _T("UTF16to8(): Invalid UTF-16 string") << newl;
				out.clear();
				delete[] decoded;
				return out;
			}

			// Get the second set of 10 bits
			c |= (*wat & 0x03ff);
			decoded[count++] = c;
			continue;
		}

		// No decoding necessary
		decoded[count++] = *wat;
		++wat;
	}

	// Create the UTF-8 string
	for (size_t o(0); o < count; ++o)
	{
		// Single byte
		if ((decoded[o] & 0xffffff80) == 0)
		{
			out += char(decoded[o] & 0x0000007f);
			continue;
		}

		// Two bytes
		if ((decoded[o] & 0xfffff800) == 0)
		{
			out += 0xc0 | char((decoded[o] >> 6) & 0x1f);
			out += 0x80 | char(decoded[o] & 0x3f);
			continue;
		}

		// Three bytes
		if ((decoded[o] & 0xffff0000) == 0)
		{
			out += 0xe0 | char((decoded[o] >> 12) & 0x0f);
			out += 0x80 | char((decoded[o] >> 6) & 0x3f);
			out += 0x80 | char(decoded[o] & 0x3f);
			continue;
		}

		// Four bytes

		// Check for overflow
		out += 0xf0 | char((decoded[o] >> 18) & 0x07);
		out += 0x80 | char((decoded[o] >> 12) & 0x3f);
		out += 0x80 | char((decoded[o] >> 6) & 0x3f);
		out += 0x80 | char(decoded[o] & 0x3f);
	}

	delete[] decoded;
	return out;
}


/*====================
  UTF8ToString
  ====================*/
string	UTF8ToString(const string &sIn)
{
	string sReturn;
	size_t zSize(sIn.length());

	for (size_t zPosition(0); zPosition < zSize; )
	{
		uint uiResult(sIn[zPosition]);
		if ((uiResult & 0x80) == 0)
		{
			// Single byte
			sReturn += char(uiResult & 0xff);
			++zPosition;
			continue;
		}

		// Determine how many bytes are in this character
		uint uiMask(0x80);
		uint uiNumBytes(0);
		while (zPosition < zSize && (sIn[zPosition] & uiMask))
		{
			++uiNumBytes;
			uiMask >>= 1;
		}

		// Assign the remaining bits of the first byte
		uiMask -= 1;
		uiResult = (uiResult & uiMask) << ((uiNumBytes - 1) * 6);
		++zPosition;

		// Assemble the remaining bytes
		for (uint ui(2); ui <= uiNumBytes; ++ui)
		{
			uint uiNext(sIn[zPosition]);
			if (uiNext == 0 || (uiNext & 0xc0) != 0x80)
			{
				uiResult = 0;
				break;
			}

			uiResult |= (uiNext & 0x3f) << ((uiNumBytes - ui) * 6);
			++zPosition;
		}

		if (uiResult == 0)
			continue;

		if (uiResult > 0xff)
			uiResult = '?';

		sReturn += char(uiResult & 0xff);
	}

	return sReturn;
}


/*====================
  UTF8ToWString
  ====================*/
wstring	UTF8ToWString(const string &sIn)
{
	wstring sReturn;
	size_t zSize(sIn.length());

	for (size_t zPosition(0); zPosition < zSize; )
	{
		uint uiResult(sIn[zPosition]);
		if ((uiResult & 0x80) == 0)
		{
			// Single byte
			sReturn += wchar_t(uiResult);
			++zPosition;
			continue;
		}

		// Determine how many bytes are in this character
		uint uiMask(0x80);
		uint uiNumBytes(0);
		while (zPosition < zSize && (sIn[zPosition] & uiMask))
		{
			++uiNumBytes;
			uiMask >>= 1;
		}

		// Assign the remaining bits of the first byte
		uiMask -= 1;
		uiResult = (uiResult & uiMask) << ((uiNumBytes - 1) * 6);
		++zPosition;

		// Assemble the remaining bytes
		for (uint ui(2); ui <= uiNumBytes; ++ui)
		{
			uint uiNext(sIn[zPosition]);
			if (uiNext == 0 || (uiNext & 0xc0) != 0x80)
			{
				uiResult = L'?';
				break;
			}

			uiResult |= (uiNext & 0x3f) << ((uiNumBytes - ui) * 6);
			++zPosition;
		}

#ifdef _WIN32
		if (uiResult > 0xffff)
			uiResult = L'?';
#endif

		sReturn += wchar_t(uiResult);
	}

	return sReturn;
}

string	WStringToUTF8(const wstring &in)
{
#ifdef _WIN32
	unsigned long *decoded(new unsigned long[in.length()]);
	size_t count = 0;
	const wchar_t *wat = in.c_str();
#else
	size_t count = in.length();
	const wstring &decoded(in);
#endif
	string out;

#ifdef _WIN32
	// Decode the UTF-16 into an array
	while (*wat != L'\x0000')
	{
		// Watch out for overflow (preliminary check)
		if (count >= in.length())
		{
			cerr << _T("WStringToUTF8(): Truncating input string") << newl;
			break;
		}

		// This is part of a surrogate pair
		if (*wat >= 0xd800 && *wat <= 0xdfff)
		{
			// TODO: Handle endianess?
			// Save the first 10 bits
			unsigned long c = (*wat & 0x03ff) << 10;

			// Advance and verify next byte
			++wat;
			if (((*wat) & 0xfc00) != 0xdc00)
			{
				cerr << _T("WStringToUTF8(): Invalid UTF-16 string") << newl;
				out.clear();
				delete[] decoded;
				return out;
			}

			// Get the second set of 10 bits
			c |= (*wat & 0x03ff);
			decoded[count++] = c;
			continue;
		}

		// No decoding necessary
		decoded[count++] = *wat;
		++wat;
	}
#endif

	// Create the UTF-8 string
	for (size_t o(0); o < count; ++o)
	{
		// Single byte
		if ((decoded[o] & 0xffffff80) == 0)
		{
			out += char(decoded[o] & 0x0000007f);
			continue;
		}

		// Two bytes
		if ((decoded[o] & 0xfffff800) == 0)
		{
			out += 0xc0 | char((decoded[o] >> 6) & 0x1f);
			out += 0x80 | char(decoded[o] & 0x3f);
			continue;
		}

		// Three bytes
		if ((decoded[o] & 0xffff0000) == 0)
		{
			out += 0xe0 | char((decoded[o] >> 12) & 0x0f);
			out += 0x80 | char((decoded[o] >> 6) & 0x3f);
			out += 0x80 | char(decoded[o] & 0x3f);
			continue;
		}

		// Four bytes

		// Check for overflow
		out += 0xf0 | char((decoded[o] >> 18) & 0x07);
		out += 0x80 | char((decoded[o] >> 12) & 0x3f);
		out += 0x80 | char((decoded[o] >> 6) & 0x3f);
		out += 0x80 | char(decoded[o] & 0x3f);
	}

#ifdef _WIN32
	delete[] decoded;
#endif
	return out;
}


/*====================
  StringToUTF8
  ====================*/
string	StringToUTF8(const string &sIn)
{
	string sOut;
	size_t zLength(sIn.length());

	for (size_t z(0); z < zLength; ++z)
	{
		// Single byte
		if ((sIn[z] & 0x80) == 0)
		{
			sOut += sIn[z];
			continue;
		}

		uint uiResult(sIn[z]);

		// Determine how many bytes are in this character
		uint uiMask(0x80);
		uint uiNumBytes(0);
		while (uiResult & uiMask)
		{
			++uiNumBytes;
			uiMask >>= 1;
		}

		if (zLength + uiNumBytes >= zLength)
		{
			sOut += '?';
			continue;
		}

		// Assign the remaining bits of the first byte
		uiMask -= 1;
		uiResult = (uiResult & uiMask) << ((uiNumBytes - 1) * 6);
		++z;

		// Assemble the remaining bytes
		for (uint ui(2); ui <= uiNumBytes; ++ui)
		{
			uint uiNext(sIn[z]);
			if ((uiNext & 0xc0) != 0x80)
			{
				sOut += '?';
				break;
			}

			uiResult |= (uiNext & 0x3f) << ((uiNumBytes - ui) * 6);
			++z;
		}
	}

	return sOut;
}

#ifdef linux
// the behaviour of these when an error is encountered might need to be modified

/*====================
  WCSToMBS
  string version of wcstombs
  ====================*/
string WCSToMBS(const wstring &sIn)
{
	string sOut;
	size_t zLength(sIn.length());
	char s[MB_CUR_MAX];
	int ret;
	
	for (size_t z(0); z < zLength; ++z)
	{
		if ((ret = wctomb(s, sIn[z])) > 0)
			for (int i(0); i < ret; ++i)
				sOut += s[i];
		else if (ret == 0) // hit NUL
			break;
	}
	
	return sOut;
}

string WCSToMBS(const wchar_t *sIn)
{
	string sOut;
	size_t zLength(wcslen(sIn));
	char s[MB_CUR_MAX];
	int ret;
	
	for (size_t z(0); z < zLength; ++z)
	{
		if ((ret = wctomb(s, sIn[z])) > 0)
			for (int i(0); i < ret; ++i)
				sOut += s[i];
		else if (ret == 0) // hit NUL
			break;
	}
	
	return sOut;
}

/*====================
  MBSToWCS
  string version of mbstowcs
  ====================*/
wstring MBSToWCS(const string &sIn)
{
	wstring sOut;
	size_t zLength(sIn.length());
	wchar_t wc;
	int ret;
	
	for (size_t z(0); z < zLength; z += ret)
	{
		if ((ret = mbtowc(&wc, &(sIn.c_str()[z]), zLength - z)) > 0)
			sOut += wc;
		else if (ret == 0) // hit NUL
			break;
		else // error! try next char for start of a multi-byte sequence
			ret = 1;
	}
	
	return sOut;
}

wstring MBSToWCS(const char *sIn)
{
	wstring sOut;
	size_t zLength(strlen(sIn));
	wchar_t wc;
	int ret;
	
	for (size_t z(0); z < zLength; z += ret)
	{
		if ((ret = mbtowc(&wc, &sIn[z], zLength - z)) > 0)
			sOut += wc;
		else if (ret == 0) // hit NUL
			break;
		else // error! try next char for start of a multi-byte sequence
			ret = 1;
	}
	
	return sOut;
}
#endif


/*====================
  StripNewline
  ====================*/
void	StripNewline(tstring &sStr)
{
	for (size_t zOffset(0); ;)
	{
		size_t zNextN(sStr.find('\n', zOffset));
		size_t zNextR(sStr.find('\r', zOffset));
		zOffset = MIN(zNextN, zNextR);
		if (zOffset == tstring::npos)
			break;

		sStr.erase(zOffset, 1);
	}
}


/*====================
  NormalizeLineBreaks
  ====================*/
void	NormalizeLineBreaks(string &sStr, const string &sBreak)
{
	for (size_t zOffset(0); ; zOffset += sBreak.length())
	{
		size_t zNextN(sStr.find('\n', zOffset));
		size_t zNextR(sStr.find('\r', zOffset));
		zOffset = MIN(zNextN, zNextR);
		if (zOffset == string::npos)
			break;

		// Check for a windows line break
		if (sStr[zOffset] == '\r' &&
			zOffset + 1 < sStr.length() &&
			sStr[zOffset + 1] == '\n')
		{
			sStr.replace(zOffset, 2, sBreak);
			continue;
		}

		sStr.replace(zOffset, 1, sBreak);
	}
}

void	NormalizeLineBreaks(wstring &sStr, const wstring &sBreak)
{
	for (size_t zOffset(0); ; zOffset += sBreak.length())
	{
		size_t zNextN(sStr.find(L'\n', zOffset));
		size_t zNextR(sStr.find(L'\r', zOffset));
		zOffset = MIN(zNextN, zNextR);
		if (zOffset == wstring::npos)
			break;

		// Check for a windows line break
		if (sStr[zOffset] == L'\r' &&
			zOffset + 1 < sStr.length() &&
			sStr[zOffset + 1] == L'\n')
		{
			sStr.replace(zOffset, 2, sBreak);
			continue;
		}

		sStr.replace(zOffset, 1, sBreak);
	}
}


/*====================
  IsPercentage
  ====================*/
bool	IsPercentage(const tstring &sStr)
{
	if (sStr.empty())
		return false;

	if (sStr[sStr.length() - 1] != _T('%'))
		return false;

	bool bFoundDecimal(false);
	for (size_t z(0); z < sStr.length() - 1; ++z)
	{
		if (z == 0 && sStr[z] == _T('-'))
			continue;

		if (sStr[z] == _T('.'))
		{
			if (bFoundDecimal)
				return false;
			bFoundDecimal = true;
			continue;
		}

		if (sStr[z] < _T('0') || sStr[z] > _T('9'))
			return false;
	}

	return true;
}


/*====================
  GetPortFromAddress
  ====================*/
int		GetPortFromAddress(const tstring &sAddr)
{
	// Skip "protocol://" if the string starts with that
	size_t zMarker(sAddr.find(_T("//")));
	if (zMarker == tstring::npos)
		zMarker = 0;

	// Grab the port number from after the last ':'
	zMarker = sAddr.find_last_of(_T(":"), zMarker);
	if (zMarker == tstring::npos)
		return 0;

	return AtoI(sAddr.substr(zMarker + 1));
}


/*====================
  ExtractPortFromAddress
  ====================*/
int		ExtractPortFromAddress(tstring &sAddr)
{
	// Skip "protocol://" if the string starts with that
	size_t zMarker(sAddr.find(_T("//")));
	if (zMarker == tstring::npos)
		zMarker = 0;

	// Grab the port number from after the last ':'
	zMarker = sAddr.find(_T(":"), zMarker);
	if (zMarker == tstring::npos)
		return -1;

	int iPort(AtoI(sAddr.substr(zMarker + 1)));
	sAddr = sAddr.substr(0, zMarker);
	return iPort;
}


/*====================
  GetByteString
  ====================*/
string	GetByteString(unsigned int z)
{
	if (z < 1024)
		return XtoS(z) + " b ";
	else if (z < 1024 * 1024)
		return XtoS(z / float(1024), 0, 0, 2) + " kb";
	else if (z < 1024 * 1024 * 1024)
		return XtoS(z / float(1024 * 1024), 0, 0, 2) + " mb";
	else
		return XtoS(z / float(1024 * 1024 * 1024), 0, 0, 2) + " gb";
}

string	GetByteString(unsigned long z)
{
	if (z < 1024)
		return XtoS(z) + " b ";
	else if (z < 1024 * 1024)
		return XtoS(z / float(1024), 0, 0, 2) + " kb";
	else if (z < 1024 * 1024 * 1024)
		return XtoS(z / float(1024 * 1024), 0, 0, 2) + " mb";
	else
		return XtoS(z / float(1024 * 1024 * 1024), 0, 0, 2) + " gb";
}

string	GetByteString(ULONGLONG z)
{
	if (z < 1024)
		return XtoS(z) + " b ";
	else if (z < 1024 * 1024)
		return XtoS(z / float(1024), 0, 0, 2) + " kb";
	else if (z < 1024 * 1024 * 1024)
		return XtoS(z / float(1024 * 1024), 0, 0, 2) + " mb";
	else
		return XtoS(z / float(1024 * 1024 * 1024), 0, 0, 2) + " gb";
}

wstring	GetByteStringW(unsigned int z)
{
	if (z < 1024)
		return XtoW(z) + L" b ";
	else if (z < 1024 * 1024)
		return XtoW(z / float(1024), 0, 0, 2) + L" kb";
	else if (z < 1024 * 1024 * 1024)
		return XtoW(z / float(1024 * 1024), 0, 0, 2) + L" mb";
	else
		return XtoW(z / float(1024 * 1024 * 1024), 0, 0, 2) + L" gb";
}

wstring	GetByteStringW(unsigned long z)
{
	if (z < 1024)
		return XtoW(z) + L" b ";
	else if (z < 1024 * 1024)
		return XtoW(z / float(1024), 0, 0, 2) + L" kb";
	else if (z < 1024 * 1024 * 1024)
		return XtoW(z / float(1024 * 1024), 0, 0, 2) + L" mb";
	else
		return XtoW(z / float(1024 * 1024 * 1024), 0, 0, 2) + L" gb";
}

wstring	GetByteStringW(ULONGLONG z)
{
	if (z < 1024)
		return XtoW(z) + L" b ";
	else if (z < 1024 * 1024)
		return XtoW(z / float(1024), 0, 0, 2) + L" kb";
	else if (z < 1024 * 1024 * 1024)
		return XtoW(z / float(1024 * 1024), 0, 0, 2) + L" mb";
	else
		return XtoW(z / float(1024 * 1024 * 1024), 0, 0, 2) + L" gb";
}


/*====================
  Filename_AppendSuffix
  ====================*/
string		Filename_AppendSuffix(const string &sPath, const string &sSuffix)
{
	string sWorking(Filename_StripExtension(sPath));
	if (sWorking.empty())
		return SNULL;

	if (sPath.find('.') == string::npos)
		sWorking += sSuffix;
	else
		sWorking += sSuffix + "." + Filename_GetExtension(sPath);

	return sWorking;
}

wstring		Filename_AppendSuffix(const wstring &sPath, const wstring &sSuffix)
{
	wstring sWorking(Filename_StripExtension(sPath));
	if (sWorking.empty())
		return WSNULL;

	if (sPath.find(L'.') == wstring::npos)
		sWorking += sSuffix;
	else
		sWorking += sSuffix + L"." + Filename_GetExtension(sPath);

	return sWorking;
}


/*====================
  GetColorFromString
  ====================*/
CVec4f	GetColorFromString(const tstring &sIn)
{
	// Quick checks
	if (sIn.empty())
		return CVec4f(0.0f, 0.0f, 0.0f, 1.0f);

	// Check for names
	if (sIn[0] != _T('#'))
	{
		tstring color( LowerString(sIn) );
		if (color == _T("aqua") || color == _T("cyan"))
			return CYAN;
		else if (color == _T("gray"))
			return GRAY;
		else if (color == _T("navy"))
			return NAVY;
		else if (color == _T("silver"))
			return SILVER;
		else if (color == _T("black"))
			return BLACK;
		else if (color == _T("green"))
			return GREEN;
		else if (color == _T("olive"))
			return OLIVE;
		else if (color == _T("teal"))
			return TEAL;
		else if (color == _T("blue"))
			return BLUE;
		else if (color == _T("lime"))
			return LIME;
		else if (color == _T("purple"))
			return PURPLE;
		else if (color == _T("white"))
			return WHITE;
		else if (color == _T("fuchsia") || color == _T("magenta"))
			return MAGENTA;
		else if (color == _T("maroon"))
			return MAROON;
		else if (color == _T("red"))
			return RED;
		else if (color == _T("yellow"))
			return YELLOW;
		else if (color == _T("orange"))
			return ORANGE;
		else if (color == _T("invisible"))
			return CLEAR;
		//else
		//	cerr << _T("Invalid color string: ") << sIn << newl;

		return AtoV4(color);
		//return CVec4f(0.0f, 0.0f, 0.0f, 1.0f);
	}

	// Make sure this is a valid hex value
	if (sIn.length() < 7)
	{
		cerr << _T("Invalid color string: ") << sIn << endl;
		return CVec4f(0.0f, 0.0f, 0.0f, 1.0f);
	}

	// Set alpha value
	int iRGBA[4] = { 0, 0, 0, 255 };
	if (sIn.length() == 9)
		iRGBA[3] = HexAtoI(sIn.substr(7, 2));
	else if (sIn.length() != 7)
		cerr << _T("Invalid color string: ") << sIn << endl;

	// Read color
	for (int i(0); i < 3; ++i)
		iRGBA[i] = HexAtoI(sIn.substr((i * 2) + 1, 2));

	return CVec4f(iRGBA[0] / 255.0f, iRGBA[1] / 255.0f, iRGBA[2] / 255.0f, iRGBA[3] / 255.0f);
}


/*====================
  AddEscapeChars
  ====================*/
tstring	AddEscapeChars(const tstring &sIn)
{
	tstring sRet;
	tstring::const_iterator it(sIn.begin());

	while (it != sIn.end())
	{
		switch (*it)
		{
			case _T('"'):
			case _T('\\'):
			case _T('$'):
				sRet += _T('\\');
				sRet += *it;
				break;
			default:
				sRet += *it;
				break;
		}

		++it;
	}

	return sRet;
}


/*====================
  Format

  C-style formatted strings
  ====================*/
tstring	Format(const TCHAR *sz, ...)
{
	static TCHAR szBuffer[8192];
	
	va_list argptr;

	va_start(argptr, sz);

#ifdef USE_SECURE_CRT
	if (_vsntprintf_s(szBuffer, 8192, 8191, sz, argptr) == -1)
#else
	if (_vsntprintf(szBuffer, 8192, sz, argptr) >= 8192)
#endif
		szBuffer[8191] = 0;

	va_end(argptr);

	return tstring(szBuffer);
}


/*====================
  IsValidURLChar
  ====================*/
bool IsValidURLChar(char c)
{
	//Check if the specified character needs to be URLEncoded
	if (c >= 'A' && c <= 'Z')
		return true;
	if (c >= 'a' && c <= 'z')
		return true;
	if (c >= '0' && c <= '9')
		return true;
	if (c == '_' || c == '.' || c == '-')
		return true;

	return false;
}

bool IsValidURLChar(wchar_t c)
{
	//Check if the specified character needs to be URLEncoded
	if (c >= L'A' && c <= L'Z')
		return true;
	if (c >= L'a' && c <= L'z')
		return true;
	if (c >= L'0' && c <= L'9')
		return true;
	if (c == L'_' || c == L'.' || c == L'-')
		return true;

	return false;
}


/*====================
  URLEncode
  ====================*/
string	URLEncode(const string &sMessage, bool bAllowSlashes, bool bEncodeSpaces)
{
	string sNewMessage;
	uint uStringPos(0);

	while (uStringPos < sMessage.length())
	{
		if (!IsValidURLChar(sMessage[uStringPos]))
		{
			if ((sMessage[uStringPos] == '/' || sMessage[uStringPos] == '\\') && bAllowSlashes)
				sNewMessage = sNewMessage + sMessage[uStringPos];
			else if (sMessage[uStringPos] == ' ' && !bEncodeSpaces)
				sNewMessage = sNewMessage + "+";
			else
				sNewMessage = sNewMessage + "%" + (BYTE_HEX_STR(sMessage[uStringPos]).substr(2,2));

			uStringPos++;
		}
		else
			sNewMessage = sNewMessage + sMessage[uStringPos++];
	}

	return sNewMessage;
}

string URLEncode(const wstring &sMessage, bool bAllowSlashes, bool bEncodeSpaces)
{
	string sNewMessage;
	uint uStringPos(0);

	while (uStringPos < sMessage.length())
	{
		if (!IsValidURLChar(sMessage[uStringPos]))
		{
			if ((sMessage[uStringPos] == L'/' || sMessage[uStringPos] == L'\\') && bAllowSlashes)
				sNewMessage = sNewMessage + char(sMessage[uStringPos] & 0xff);
			else if (sMessage[uStringPos] == L' ' && !bEncodeSpaces)
				sNewMessage = sNewMessage + "+";
			else
				sNewMessage = sNewMessage + "%" + (BYTE_HEX_STR(sMessage[uStringPos]).substr(2,2));

			++uStringPos;
		}
		else
			sNewMessage = sNewMessage + char(sMessage[uStringPos++] & 0xff);
	}

	return sNewMessage;
}


/*====================
  URLDecode
  ====================*/
tstring URLDecode(const tstring &sMessage)
{
	tstring sNewMessage;
	uint uStringPos(0);

	while (uStringPos < sMessage.length())
	{
		if (sMessage[uStringPos] == _T('%'))
		{
			sNewMessage = sNewMessage + TCHAR(HexAtoI(sMessage.substr(uStringPos + 1, 2)));
			uStringPos += 3;
		}
		else if (sMessage[uStringPos] == _T('+'))
		{
			uStringPos++;
			sNewMessage = sNewMessage + _T(" ");
		}
		else
			sNewMessage = sNewMessage + sMessage[uStringPos++];
	}

	return sNewMessage;
}

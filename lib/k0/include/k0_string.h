// (C)2008 S2 Games
// k0_string.h
//
//=============================================================================
#ifndef __K0_STRING_H__
#define __K0_STRING_H__

//=============================================================================
// Headers
//=============================================================================
#include <string>
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
namespace std
{
	typedef basic_string<char, char_traits<char>, allocator<char> > string;
	typedef basic_string<wchar_t, char_traits<wchar_t>, allocator<wchar_t> > wstring;
	typedef basic_string<TCHAR, char_traits<TCHAR>, allocator<TCHAR> > tstring;
	typedef basic_fstream<TCHAR, char_traits<TCHAR> > tfstream;
}

using std::string;
using std::wstring;
using std::tstring;

typedef vector<tstring>				tsvector;
typedef tsvector::iterator			tsvector_it;
typedef tsvector::const_iterator	tsvector_cit;
typedef tsvector::reverse_iterator	tsvector_rit;

typedef vector<string>				svector;
typedef svector::iterator			svector_it;
typedef svector::const_iterator		svector_cit;
typedef svector::reverse_iterator	svector_rit;

typedef vector<wstring>				wsvector;
typedef wsvector::iterator			wsvector_it;
typedef wsvector::const_iterator	wsvector_cit;
typedef wsvector::reverse_iterator	wsvector_rit;

typedef list<tstring>				slist;
typedef slist::iterator				slist_it;
typedef slist::const_iterator		slist_cit;
typedef slist::reverse_iterator		slist_rit;

typedef set<tstring>				sset;
typedef sset::iterator				sset_it;
typedef sset::const_iterator		sset_cit;
typedef sset::reverse_iterator		sset_rit;

typedef deque<tstring>				tsdeque;
typedef tsdeque::iterator			tsdeque_it;
typedef tsdeque::const_iterator		tsdeque_cit;
typedef tsdeque::reverse_iterator	tsdeque_rit;

typedef deque<string>				sdeque;
typedef sdeque::iterator			sdeque_it;
typedef sdeque::const_iterator		sdeque_cit;
typedef sdeque::reverse_iterator	sdeque_rit;

typedef deque<wstring>				wsdeque;
typedef wsdeque::iterator			wsdeque_it;
typedef wsdeque::const_iterator		wsdeque_cit;
typedef wsdeque::reverse_iterator	wsdeque_rit;

typedef map<tstring, tstring>		smaps;
typedef smaps::iterator				smaps_it;
typedef smaps::const_iterator		smaps_cit;
typedef smaps::reverse_iterator		smaps_rit;

typedef map<int, tstring>			imaps;
typedef imaps::iterator				imaps_it;
typedef imaps::const_iterator		imaps_cit;
typedef imaps::reverse_iterator		imaps_rit;

typedef map<short, tstring>			nmaps;
typedef nmaps::iterator				nmaps_it;
typedef nmaps::const_iterator		nmaps_cit;
typedef nmaps::reverse_iterator		nmaps_rit;

typedef map<uint, tstring>			uimaps;
typedef uimaps::iterator			uimaps_it;
typedef uimaps::const_iterator		uimaps_cit;
typedef uimaps::reverse_iterator	uimaps_rit;

typedef map<ushort, tstring>		unmaps;
typedef unmaps::iterator			unmaps_it;
typedef unmaps::const_iterator		unmaps_cit;
typedef unmaps::reverse_iterator	unmaps_rit;

typedef map<tstring, tstring>		PropertyMap;

#ifdef _UNICODE

#define TStringToString(in)			WideToSingle(in)
#define StringToTString(in)			SingleToWide(in)
#define WStringToTString(in)		(in)
#define TStringToWString(in)		(in)

#define	TStringCompare(s, sz)		(s).compare(0, (s).length(), sz, (sizeof(sz) / sizeof(TCHAR)) - 1)
#define	StringCompare(s, sz)		(s).compare(0, (s).length(), sz, sizeof(sz) - 1)
#define	WStringCompare(s, sz)		(s).compare(0, (s).length(), sz, (sizeof(sz) / sizeof(wchar_t)) - 1)

#define StrToTString(out, in)		{ size_t z(strlen(in) + 1); TCHAR *pBuffer(new TCHAR[z]); SingleToWide(pBuffer, (in), z); (out) = pBuffer; delete[] pBuffer; }

#else //_UNICODE

#define TStringToString(in)			(in)
#define StringToTString(in)			(in)
#define WStringToTString(in)		WideToSingle(in)
#define TStringToWString(in)		SingleToWide(in)

#define StrToTString(out, in)		{ if ((in) != NULL) (out) = (in); }

#endif

#define _TS(s) tstring(_T(s))

//=============================================================================
// Constants
//=============================================================================
const tstring	TSNULL(_T(""));
const string	SNULL("");
const wstring	WSNULL(L"");
const tsvector	VSNULL;

const tstring	tnewl(_T("\n"));
const string	newl("\n");
const wstring	wnewl(L"\n");

const TCHAR TSPACE(_T(' '));
const char SPACE(' ');
const wchar_t WSPACE(L' ');

#ifdef _WIN32
const string LINEBREAK("\x0d\x0a");
const wstring WLINEBREAK(L"\x0d\x0a");
#endif
#if defined(linux) || defined(__APPLE__) || defined(__FreeBSD__)
#define LINEBREAK string("\x0a")
#define WLINEBREAK wstring(L"\x0a")
#endif

const size_t LINEBREAK_SIZE(sizeof(LINEBREAK));
const size_t WLINEBREAK_SIZE(sizeof(WLINEBREAK));

const int NUM_TOK_STRINGS(16);

const uint TIME_STRING_ALPHA_SEPERATOR	(BIT(0));
const uint TIME_STRING_COUNTDOWN		(BIT(1));
const uint TIME_STRING_PAD_ZEROS		(BIT(2));
//=============================================================================

//=============================================================================
// Functions
//=============================================================================
svector			TokenizeString(const string &sStr, char cSeparator, char cEscape = '\x00');
wsvector		TokenizeString(const wstring &sStr, wchar_t cSeparator, wchar_t cEscape = L'\x00');

string			JoinString(const svector &vStrings, const string &sJoinBy);
wstring			JoinString(const wsvector &vStrings, const wstring &sJoinBy);

inline string	ReplaceSeparator(const string &str, char cOld, const string &sNew)		{ return JoinString(TokenizeString(str, cOld), sNew); }
inline wstring	ReplaceSeparator(const wstring &str, wchar_t cOld, const wstring &sNew)	{ return JoinString(TokenizeString(str, cOld), sNew); }

string			EscapeCharacters(const string &str, const string &sChars, const string &sEscape = "\\");
wstring			EscapeCharacters(const wstring &str, const wstring &sChars, const wstring &sEscape = L"\\");

string			UnescapeCharacters(const string &str, const string &sEscape = "\\");
wstring			UnescapeCharacters(const wstring &str, const wstring &sEscape = L"\\");

string			LowerString(const string &str);
wstring			LowerString(const wstring &str);
inline string&	LowerStringRef(string &str)			{ transform(str.begin(), str.end(), str.begin(), tolower); return str; }
inline wstring&	LowerStringRef(wstring &str)		{ transform(str.begin(), str.end(), str.begin(), towlower); return str; }

tstring			UpperString(const tstring &str);
void			UpperStringRef(tstring &str);

bool			IsLineSeparator(int c);
bool			IsEOL(int c);
bool			IsTokenSeparator(int c);
inline bool		IsAllWhiteSpace(const string &sStr)		{ return sStr.find_first_not_of("\n\r\t ") == string::npos; }
inline bool		IsAllWhiteSpace(const wstring &sStr)	{ return sStr.find_first_not_of(L"\n\r\t ") == wstring::npos; }

string			Filename_GetExtension(const string &sPath);
wstring			Filename_GetExtension(const wstring &sPath);
string			Filename_GetName(const string &sPath);
wstring			Filename_GetName(const wstring &sPath);
string			Filename_GetPath(const string &sPath);
wstring			Filename_GetPath(const wstring &sPath);
inline string	Filename_StripExtension(const string &sPath)	{ return sPath.substr(0, sPath.find_last_of(".")); }
inline wstring	Filename_StripExtension(const wstring &sPath)	{ return sPath.substr(0, sPath.find_last_of(L".")); }
string			Filename_StripPath(const string &sPath);
wstring			Filename_StripPath(const wstring &sPath);
string			Filename_AppendSuffix(const string &sPath, const string &sSuffix);
wstring			Filename_AppendSuffix(const wstring &sPath, const wstring &sSuffix);
string			Filename_SanitizePath(const string &sPath);
wstring			Filename_SanitizePath(const wstring &sPath);

char*		GetNextWord(const char *str);
tstring		GetNextWord(const tstring &sStr, size_t &zStart);
tstring		GetNextSymbol(const tstring &sStr, size_t &zStart);
char*		StripEOL(char *str);
char*		StrChrBackwards (const char *str, int c);
TCHAR*		SkipSpaces(TCHAR *str);
size_t		SkipSpaces(const string &sStr, size_t zStart = 0);
size_t		SkipSpaces(const wstring &sStr, size_t zStart = 0);
char*		FirstToken(const char *str);
int			SplitArgs(TCHAR *s, TCHAR **argv, int maxargs);
int			SplitArgs(const tstring &sString, tsvector &vArgList);
string		ConcatinateArgs(const svector &vArgList, const string &sSeperator = " ");
wstring		ConcatinateArgs(const wsvector &vArgList, const wstring &sSeperator = L" ");
tstring		ConcatinateArgs(tsvector_cit begin, tsvector_cit end, const tstring &sSeperator = _T(" "));

string		StripDuplicateSpaces(const string &sStr);
wstring		StripDuplicateSpaces(const wstring &sStr);

string&			StripExtraneousSpacesRef(string &s);
wstring&		StripExtraneousSpacesRef(wstring &s);
inline string	StripExtraneousSpaces(const string &s)		{ string sReturn(s); return StripExtraneousSpacesRef(sReturn); }
inline wstring	StripExtraneousSpaces(const wstring &s)		{ wstring sReturn(s); return StripExtraneousSpacesRef(sReturn); }

inline string	StripStartingSpaces(const string &s)	{ size_t z(s.find_first_not_of("\n\r\t ")); if (z == string::npos) return SNULL; return s.substr(z); }
inline wstring	StripStartingSpaces(const wstring &s)	{ size_t z(s.find_first_not_of(L"\n\r\t ")); if (z == wstring::npos) return WSNULL; return s.substr(z); }
inline string&	StripStartingSpacesRef(string &s)		{ size_t z(s.find_first_not_of("\n\r\t ")); if (z != string::npos) s.erase(0, z); return s; }
inline wstring&	StripStartingSpacesRef(wstring &s)		{ size_t z(s.find_first_not_of(L"\n\r\t ")); if (z != wstring::npos) s.erase(0, z); return s; }

inline string	StripTrailingSpaces(const string &s)	{ size_t z(s.find_last_not_of("\n\r\t ")); if (z == string::npos) return s; return s.substr(0, z + 1); }
inline wstring	StripTrailingSpaces(const wstring &s)	{ size_t z(s.find_last_not_of(L"\n\r\t ")); if (z == wstring::npos) return s; return s.substr(0, z + 1); }
inline string&	StripTrailingSpacesRef(string &s)		{ size_t z(s.find_last_not_of("\n\r\t ")); if (z != string::npos) s.erase(z + 1); return s; }
inline wstring&	StripTrailingSpacesRef(wstring &s)		{ size_t z(s.find_last_not_of(L"\n\r\t ")); if (z != wstring::npos) s.erase(z + 1); return s; }

int			CompareNoCase(const string &s1, const string &s2);
int			CompareNoCase(const wstring &s1, const wstring &s2);
int			CompareNum(const tstring &s1, const tstring &s2, size_t n);
int			CompareNum(const tstring &s1, const TCHAR *s2, size_t n);
int			CompareNoCaseNum(const tstring &s1, const tstring &s2, size_t n);

bool		EqualsWildcard(const tstring &sWild, const tstring &sComp);

wstring		SingleToWide(const string &sIn);
wchar_t*	SingleToWide(wchar_t *out, const char *in, size_t len);
string		WideToSingle(const wstring &sIn);
char*		WideToSingle(char *out, const wchar_t *in, size_t len);

wchar_t*	UTF8to16(wchar_t *out, const char *in, size_t len);
char*		UTF16to8(char *out, const wchar_t *in, size_t len);
string		UTF16to8(const wstring &in);

string		UTF8ToString(const string &sIn);
wstring		UTF8ToWString(const string &sIn);
string		WStringToUTF8(const wstring &sIn);
string		StringToUTF8(const string &sIn);

#ifdef linux
string		WCSToMBS(const wstring &sIn);
string		WCSToMBS(const wchar_t *sIn);
wstring		MBSToWCS(const string &sIn);
wstring		MBSToWCS(const char *sIn);
#endif

void		StripNewline(tstring &sStr);
void		NormalizeLineBreaks(string &sStr, const string &sBreak = LINEBREAK);
void		NormalizeLineBreaks(wstring &sStr, const wstring &sBreak = WLINEBREAK);
inline string	NormalizeLineBreaks(const string &sStr, const string &sBreak = LINEBREAK)		{ string sResult(sStr); NormalizeLineBreaks(sResult, sBreak); return sResult; }
inline wstring	NormalizeLineBreaks(const wstring &sStr, const wstring &sBreak = WLINEBREAK)	{ wstring sResult(sStr); NormalizeLineBreaks(sResult, sBreak); return sResult; }
bool		IsPercentage(const tstring &sStr);

bool		IsIPAddress(const string &sIP);
bool		IsIPAddress(const wstring &sIP);
int			GetPortFromAddress(const tstring &sAddr);
int			ExtractPortFromAddress(tstring &sAddr);

string		GetByteString(unsigned int z);
string		GetByteString(unsigned long z);
string		GetByteString(ULONGLONG z);

wstring		GetByteStringW(unsigned int z);
wstring		GetByteStringW(unsigned long z);
wstring		GetByteStringW(ULONGLONG z);

CVec4f		GetColorFromString(const tstring &sIn);

tstring		AddEscapeChars(const tstring &sIn);
tstring		Format(const TCHAR *sz, ...);

bool		IsValidURLChar(char c);
bool		IsValidURLChar(wchar_t c);
string		URLEncode(const string &sMessage, bool bAllowSlashes = false, bool bEncodeSpaces = false);
string		URLEncode(const wstring &sMessage, bool bAllowSlashes = false, bool bEncodeSpaces = false);
tstring		URLDecode(const tstring &sMessage);

bool		IsNotDigit(int c);
bool		IsLetter(int c);

string		GetTimeString(uint uiMilliseconds, int iSeperations = -1, uint uiPrecision = 0, uint uiFlags = 0);
wstring		GetTimeStringW(uint uiMilliseconds, int iSeperations = -1, uint uiPrecision = 0, uint uiFlags = 0);

#define	TStringCompare(s, sz) (s).compare(0, (s).length(), sz, (sizeof(sz) / sizeof(TCHAR)) - 1)
#define	StringCompare(s, sz) (s).compare(0, (s).length(), sz, sizeof(sz) - 1)
#define	WStringCompare(s, sz) (s).compare(0, (s).length(), sz, (sizeof(sz) / sizeof(wchar_t)) - 1)
//=============================================================================

//=============================================================================
// Template functions
//=============================================================================

/*====================
  QuoteStr
  ====================*/
template<class T>
inline tstring	QuoteStr(T x)
{
	return _T("\"") + XtoA(x) + _T("\"");
}

template<>
inline tstring	QuoteStr(const TCHAR *x)
{
	return _T("\"") + tstring(x) + _T("\"");
}

template<>
inline tstring	QuoteStr(TCHAR *x)
{
	return _T("\"") + tstring(x) + _T("\"");
}


/*====================
  ParenStr
  ====================*/
template<class T>
inline tstring	ParenStr(T x)
{
	return _T("(") + XtoA(x) + _T(")");
}


/*====================
  SingleQuoteStr
  ====================*/
template<class T>
inline tstring	SingleQuoteStr(T x)
{
	return _T("'") + XtoA(x) + _T("'");
}


/*====================
  StringReplace
  ====================*/
inline tstring	StringReplace(tstring sString, const tstring sSearch, const tstring sReplace, const int iReplaceFlag = 0)
{
	tstring::size_type pos = tstring::npos;
	
	if (iReplaceFlag == 0)
	{		
		// replace all occurences of the search string
		pos = sString.find(sSearch);

		while (pos != tstring::npos) 
		{
			sString.replace(pos, sSearch.size(), sReplace);
			pos = sString.find(sSearch, pos + sSearch.size());
		}
	   
		return sString;
	}
	else if (iReplaceFlag == 1)
	{
		// replace only the first occurence
		if((pos = sString.find(sSearch)) != tstring::npos)
			return sString.replace(pos, sSearch.length(), sReplace);
		else
			return sString;
	}
	else
		return TSNULL;		
}
//=============================================================================

#endif //__K0_STRING_H__



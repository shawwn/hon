// (C)2008 S2 Games
// xtoa.h
//
//=============================================================================
#ifndef __XTOA_H__
#define __XTOA_H__

//=============================================================================
// Definitions
//=============================================================================
enum EXtoAFormatFlags
{
	FMT_NONE =		0x0000,
	FMT_SIGN =		0x0001,
	FMT_PADZERO =	0x0002,
	FMT_ALIGNLEFT =	0x0004,
	FMT_PADSIGN =	0x0008,
	FMT_NOPREFIX =	0x0010,
	FMT_DELIMIT =	0x0020,
	FMT_LOWERCASE = 0x0040
};

const int XTOA_DEFAULT_FLOAT_PRECISION(4);

#define BYTE_HEX_STR(y)		XtoA((y), FMT_PADZERO, 4, 16)
#define SHORT_HEX_STR(n)	XtoA((n), FMT_PADZERO, 6, 16)
#define INT_HEX_STR(i)		XtoA((i), FMT_PADZERO, 10, 16)
#ifdef __x86_64__
#define INTPTR_HEX_STR(i)	XtoA((i), FMT_PADZERO, 18, 16)
#else
#define INTPTR_HEX_STR(i)	XtoA((i), FMT_PADZERO, 10, 16)
#endif

#ifndef _WIN32
#define _wtoi(x) wcstol(x, NULL, 10)
#endif

#if defined(_WIN32)
inline double WTOF(const wstring &s) { return _wtof(s.c_str()); }
#elif defined(__APPLE__)
inline double WTOF(const wstring &s) { return atof(TStringToNative(s).c_str()); }
#elif defined(linux)
inline double WTOF(const wstring &s) { return wcstod(s.c_str(), NULL); }
#endif
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
// Integers
string		XtoS(short s, int flags = 0, int width = 0, int base = 10);
wstring		XtoW(short s, int flags = 0, int width = 0, int base = 10);
string		XtoS(int i, int flags = 0, int width = 0, int base = 10);
wstring		XtoW(int i, int flags = 0, int width = 0, int base = 10);
string		XtoS(long l, int flags = 0, int width = 0, int base = 10);
wstring		XtoW(long l, int flags = 0, int width = 0, int base = 10);
string		XtoS(LONGLONG ll, int flags = 0, int width = 0, int base = 10);
wstring		XtoW(LONGLONG ll, int flags = 0, int width = 0, int base = 10);
string		XtoS(unsigned short us, int flags = 0, int width = 0, int base = 10);
wstring		XtoW(unsigned short us, int flags = 0, int width = 0, int base = 10);
string		XtoS(unsigned int ui, int flags = 0, int width = 0, int base = 10);
wstring		XtoW(unsigned int ui, int flags = 0, int width = 0, int base = 10);
string		XtoS(unsigned long ul, int flags = 0, int width = 0, int base = 10);
wstring		XtoW(unsigned long ul, int flags = 0, int width = 0, int base = 10);
string		XtoS(ULONGLONG ull, int flags = 0, int width = 0, int base = 10);
wstring		XtoW(ULONGLONG ull, int flags = 0, int width = 0, int base = 10);

// Floating point
string		XtoS(float f, int flags = 0, int width = 0, int precision = XTOA_DEFAULT_FLOAT_PRECISION);
wstring		XtoW(float f, int flags = 0, int width = 0, int precision = XTOA_DEFAULT_FLOAT_PRECISION);
string		XtoS(double d, int flags = 0, int width = 0, int precision = XTOA_DEFAULT_FLOAT_PRECISION);
wstring		XtoW(double d, int flags = 0, int width = 0, int precision = XTOA_DEFAULT_FLOAT_PRECISION);

// Vectors
string		XtoS(const CVec2f &vec, int flags = 0, int width = 0, int precision = XTOA_DEFAULT_FLOAT_PRECISION);
wstring		XtoW(const CVec2f &vec, int flags = 0, int width = 0, int precision = XTOA_DEFAULT_FLOAT_PRECISION);
string		XtoS(const CVec3f &vec, int flags = 0, int width = 0, int precision = XTOA_DEFAULT_FLOAT_PRECISION);
wstring		XtoW(const CVec3f &vec, int flags = 0, int width = 0, int precision = XTOA_DEFAULT_FLOAT_PRECISION);
string		XtoS(const CVec3<double> &vec, int flags = 0, int width = 0, int precision = XTOA_DEFAULT_FLOAT_PRECISION);
wstring		XtoW(const CVec3<double> &vec, int flags = 0, int width = 0, int precision = XTOA_DEFAULT_FLOAT_PRECISION);
string		XtoS(const CVec4f &vec, int flags = 0, int width = 0, int precision = XTOA_DEFAULT_FLOAT_PRECISION);
wstring		XtoW(const CVec4f &vec, int flags = 0, int width = 0, int precision = XTOA_DEFAULT_FLOAT_PRECISION);

// Boolean
string		XtoS(bool b, bool bNum = false);
wstring		XtoW(bool b, bool bNum = false);

// Pointers
string		XtoS(const void *p, bool bLower = false);
wstring		XtoW(const void *p, bool bLower = false);

// Text
string					XtoS(const string &s, int flags, size_t width = 0);
wstring					XtoW(const wstring &s, int flags, size_t width = 0);
inline const string&	XtoS(const string &s)	{ return s; }
inline const wstring&	XtoW(const wstring &s)	{ return s; }
inline string			XtoS(char c)			{ return string(c, 1); }
inline wstring			XtoW(wchar_t c)			{ return wstring(c, 1); }
inline string			XtoS(const wstring &s)	{ return WideToSingle(s); }
inline wstring			XtoW(const string &s)	{ return SingleToWide(s); }

// AtoX
inline int&		AtoX(const string &s, int &i)			{ i = atoi(s.c_str()); return i; }
inline int&		AtoX(const wstring &s, int &i)			{ i = _wtoi(s.c_str()); return i; }
inline uint&	AtoX(const string &s, uint &ui)			{ ui = atoi(s.c_str()); return ui; }
inline uint&	AtoX(const wstring &s, uint &ui)		{ ui = _wtoi(s.c_str()); return ui; }
inline float&	AtoX(const string &s, float &f)			{ f = float(atof(s.c_str())); return f; }
inline float&	AtoX(const wstring &s, float &f)		{ f = float(WTOF(s.c_str())); return f; }
inline double&	AtoX(const string &s, double &d)		{ d = atof(s.c_str()); return d; }
inline double&	AtoX(const wstring &s, double &d)		{ d = WTOF(s.c_str()); return d; }
inline string&	AtoX(const string &s, string &str)		{ str = s; return str; }
inline wstring&	AtoX(const wstring &s, wstring &str)	{ str = s; return str; }
inline bool&	AtoX(const string &s, bool &b)			{ if (CompareNoCase(s, "true") == 0) b = true; else b = (atoi(s.c_str()) != 0); return b; }
inline bool&	AtoX(const wstring &s, bool &b)			{ if (CompareNoCase(s, L"true") == 0) b = true; else b = (_wtoi(s.c_str()) != 0); return b; }
CVec2f&		AtoX(const tstring &s, CVec2f &vec);
CVec3f&		AtoX(const tstring &s, CVec3f &vec);
CVec4f&		AtoX(const tstring &s, CVec4f &vec);

// Non-ref versions
inline int		AtoI(const string &s)		{ return atoi(s.c_str()); }
inline int		AtoI(const wstring &s)		{ return _wtoi(s.c_str()); }
inline short	AtoN(const string &s)		{ return short(AtoI(s) & USHRT_MAX); }
inline short	AtoN(const wstring &s)		{ return short(AtoI(s) & USHRT_MAX); }
inline float	AtoF(const string &s)		{ return float(atof(s.c_str())); }
inline float	AtoF(const wstring &s)		{ return float(WTOF(s.c_str())); }
inline double	AtoD(const string &s)		{ return atof(s.c_str()); }
inline double	AtoD(const wstring &s)		{ return WTOF(s.c_str()); }
bool			AtoB(const string &s);
bool			AtoB(const wstring &s);
CVec2f			AtoV2(const tstring &s);
CVec3f			AtoV3(const tstring &s);
CVec4f			AtoV4(const tstring &s);

// Percentages
int			PtoI(const tstring &s);
float		PtoF(const tstring &s);
float		P2toF(const tstring &s);

// Hex
int			HexAtoI(const tstring &s);
//=============================================================================

#endif	//__XTOA_H__

// (C)2008 S2 Games
// k0_unicode.h
//
// This file sets up all the unicode definitions necessary for all modules of the game
//=============================================================================
#ifndef __K0_UNICODE_H__
#define __K0_UNICODE_H__

//=============================================================================
// Definitions
//=============================================================================
//#define UNICODE

#ifdef UNICODE
#ifndef _UNICODE
#define _UNICODE
#endif
#endif


#ifdef _UNICODE

#ifndef UNICODE
#define UNICODE
#endif

#ifndef __GNUC__
#pragma message("*** Unicode is enabled ***")
#endif

#define WideToTCHAR(out, size, in, len)		WCSNCPY_S(out, size, in, len)
#define TCHARToWide(out, size, in, len)		WCSNCPY_S(out, size, in, len)
#define SingleToTCHAR(out, size, in, len)	SingleToWide(out, in, len)
#define TCHARToSingle(out, size, in, len)	WideToSingle(out, in, len)

#else //_UNICODE

#ifndef __GNUC__
#pragma message("*** Unicode is disabled ***")
#endif

#define WideToTCHAR(out, size, in, len)		WideToSingle(out, in, len)
#define TCHARToWide(out, size, in, len)		SingleToWide(out, in, len)
#define SingleToTCHAR(out, size, in, len)	STRNCPY_S(out, size, in, len)
#define TCHARToSingle(out, size, in, len)	STRNCPY_S(out, size, in, len)

#endif //_UNICODE

#ifdef UNICODE
#define XtoA			XtoW
#define FormatTime		FormatTimeW
#define UTF8ToTString	UTF8ToWString
#define TStringToUTF8	WStringToUTF8
#else
#define XtoA			XtoS
#define FormatTime		FormatTimeS
#define UTF8ToTString	UTF8ToString
#define TStringToUTF8	StringToUTF8
#endif

#ifdef _WIN32
#include <tchar.h>
#else
#include "tchar_linux.h"
#endif
#include <string>
#include <fstream>
//=============================================================================

#endif //__K0_UNICODE_H__

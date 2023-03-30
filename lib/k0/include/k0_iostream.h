// (C)2010 S2 Games
// k0_iostream.h
//
//=============================================================================
#ifndef __K0_IOSTREAM_H__
#define __K0_IOSTREAM_H__

//=============================================================================
// Headers
//=============================================================================
#include <iostream>
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
#ifdef _UNICODE
#define tcout wcout
#define tcerr wcerr
#else
#define tcout cout
#define tcerr cerr
#endif

using std::tcout;
using std::tcerr;
using std::endl;
//=============================================================================

#endif //__K0_STRING_H__



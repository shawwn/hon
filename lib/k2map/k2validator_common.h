// k2validator_common.h
//
// Precompiled header file for K0 testing
//=============================================================================
#ifndef __K0VALIDATOR_COMMON_H__
#define __K0VALIDATOR_COMMON_H__

//=============================================================================
// Compile Defines
//=============================================================================
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

//#define UNICODE

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER						// Allow use of features specific to Windows XP or later.
#define WINVER				0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT				// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT		0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

#ifndef _WIN32_WINDOWS				// Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS		0x0410	// Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE					// Allow use of features specific to IE 6.0 or later.
#define _WIN32_IE			0x0600	// Change this to the appropriate value to target other versions of IE.
#endif

#endif // WIN32

#define VERSION				_T("0.42.69")

#define K2VALIDATOR_TEST	1		// For testing purposes.  This should always be commented out when
									//	committing the code.
//=============================================================================

//=============================================================================
// Unicode
//
// This comes before everything else, because it affects a lot of the other
// libraries initialization
//=============================================================================
#include "k0_unicode.h"
//=============================================================================

//=============================================================================
// Windows headers
//=============================================================================
#ifdef _WIN32
#include <windows.h>
#endif
//=============================================================================

//=============================================================================
// CRT headers
//=============================================================================
#ifdef _WIN32
#include <tchar.h>
#include <commdlg.h>
#else
#include <tchar_linux.h>
#endif
#include <float.h>
#include <errno.h>
#include <cassert>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>

//=============================================================================

//=============================================================================
// STL headers
//=============================================================================
#include <vector>
using std::vector;
//=============================================================================

//=============================================================================
// K0 headers
//=============================================================================
#include <k0_types.h>
#include <k0_stl.h>
#include <k0_endian.h>
#include <k0_math.h>
#include <k0_string.h>
#include <xtoa.h>
//=============================================================================

//=============================================================================
// Project headers
//=============================================================================
//#include "k2validator.h"
//=============================================================================

#endif //__K0TEMPLATE_COMMON_H__

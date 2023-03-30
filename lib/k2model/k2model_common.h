// k2model_common.h
//
// Precompiled header file for K0 testing
//=============================================================================
#ifndef __K2MODEL_COMMON_H__
#define __K2MODEL_COMMON_H__

//=============================================================================
// Compile Defines
//=============================================================================
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER				// Allow use of features specific to Windows XP or later.
#define WINVER 0x0501		// Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

#ifndef _WIN32_WINDOWS		// Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE			// Allow use of features specific to IE 6.0 or later.
#define _WIN32_IE 0x0600	// Change this to the appropriate value to target other versions of IE.
#endif

#define VERSION _T("0.1.0")
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
#include <windows.h>
//=============================================================================

//=============================================================================
// CRT headers
//=============================================================================
#include <tchar.h>
#include <float.h>
#include <errno.h>
#include <cassert>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <commdlg.h>
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
//#include "k2model.h"
//=============================================================================

#endif //__K2MODEL_COMMON_H__
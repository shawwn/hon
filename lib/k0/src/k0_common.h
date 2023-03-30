// (C)2008 S2 Games
// k0_common.h
//
// Precompiled header file for the k0 module
//=============================================================================
#ifndef __K0_COMMON_H__
#define __K0_COMMON_H__

//=============================================================================
// Compile Defines
//=============================================================================
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define _SECURE_SCL 1
#define _SECURE_SCL_THROWS 1
//=============================================================================

//=============================================================================
// Unicode
//
// This comes before everything else, because it affects a lot of the other
// libraries initialization
//=============================================================================
#include "k0_secure_crt.h"
#include "k0_unicode.h"
//=============================================================================

//=============================================================================
// Standard headers
//=============================================================================
#ifdef _WIN32
#include <tchar.h>
#endif
#include <float.h>
#include <errno.h>
#include <cassert>
#include <stdlib.h>
#ifndef __FreeBSD__
#include <malloc.h>
#endif
#include <memory.h>
#include <stdarg.h>
//=============================================================================

//=============================================================================
// K0 headers
//=============================================================================
#include "k0_types.h"
#include "k0_stl.h"
#include "k0_endian.h"
#include "k0_math.h"
#include "k0_string.h"
#include "k0_singleton.h"
#include "xtoa.h"
//=============================================================================

#endif //__K0_COMMON_H__

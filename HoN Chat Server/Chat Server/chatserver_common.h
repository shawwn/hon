// (C)2008 S2 Games
// chatserver_common.h
//
// Precompiled header
//=============================================================================
#ifndef __CHATSERVER_COMMON_H__
#define __CHATSERVER_COMMON_H__

//=============================================================================
// Compile Defines
//=============================================================================
#define UNICODE

//#define _SECURE_SCL 1
//#define _SECURE_SCL_THROWS 1

//#define NO_PROFILE
//#define NO_STD_CONSOLE
#define USE_SECURE_CRT
#define DEBUG_MEM					// Use custom memory manager to track allocations
//#define CURL_CUSTOM_MEM_MANAGER		// Enable custom memory manager in curl
//#define VALIDATE_MEM				// Aggressively scans the heaps each alloc/dealloc for any errors
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
#include <cassert>
//=============================================================================

//=============================================================================
// K0 headers
//=============================================================================
#include "k0_types.h"
#include "k0_stl.h"
#include "k0_math.h"
#include "k0_string.h"
#include "k0_singleton.h"
#include "k0_endian.h"
#include "xtoa.h"
#include "c_exception.h"
#include "c_filehandle.h"
#include "c_buffer.h"
#include "md5.h"
//=============================================================================

//=============================================================================
// Shared headers
//=============================================================================
#include "chatserver_types.h"
#include "chatserver_protocol.h"

#include "c_memmanager.h"
#include "c_profiler.h"
#include "c_core.h"
#include "c_profilesample.h"
#include "c_command.h"
#include "c_cvar.h"
//=============================================================================

#endif //__CHATSERVER_COMMON_H__

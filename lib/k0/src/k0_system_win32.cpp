// (C)2010 S2 Games
// k0_system_win32.cpp
//
//=============================================================================
#include "k0_common.h"
#ifdef _WIN32
#include "k0_system.h"

//=============================================================================
// Headers
//=============================================================================
# define _WIN32_DCOM
# define _WIN32_WINNT 0x0500
# define _WIN32_WINDOWS 0x0410
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
#include <direct.h>

// These aren't required yet, uncomment them as needed.
#if 0
#include <shlobj.h>
#include <mmsystem.h>
#include <winsock2.h>
#include <dbghelp.h>
#include <Psapi.h>
#include <comdef.h>
#include <Wbemidl.h>
#include <Iphlpapi.h>
#include <mmsystem.h>
#endif
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
SINGLETON_INIT(CK0System);
CK0System& K0System(*CK0System::GetInstance());
//=============================================================================

/*====================
  CK0System::CK0System
  ====================*/
CK0System::CK0System()
{
}


/*====================
  CK0System::ChDir
  ====================*/
void	CK0System::ChDir(const char* pDirUTF8)
{
	_chdir(pDirUTF8);
}
#endif

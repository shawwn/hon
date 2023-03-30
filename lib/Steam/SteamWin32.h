
/************ (C) Copyright 2002 Valve, L.L.C. All rights reserved. ***********
**
** The copyright to the contents herein is the property of Valve, L.L.C.
** The contents may be used and/or copied only with the written permission of
** Valve, L.L.C., or in accordance with the terms and conditions stipulated in
** the agreement/contract under which the contents have been supplied.
**
*******************************************************************************
**
** Contents:
**
**		This file provides a PARTIAL wrapper for the Steam Client Interface
**      provided by Steam.h.  It provides a Win32-like interface
**      with functions such as CreateFile(), ReadFile(), FindNextFile(), etc. 
**
**		It is intended to help minimize difficulty in porting existing
**      applications to Steam. For new applications, it is recommended you
**      use the Steam.h interface directly.
**
** Target restrictions:
**
** Tool restrictions:
**
******************************************************************************/

#ifndef INCLUDED_STEAMWIN32API_H
#define INCLUDED_STEAMWIN32API_H

#if defined(_MSC_VER) && (_MSC_VER > 1000)
#pragma once
#endif

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum					// Filter elements returned by SteamFind{First,Next}
{
	STEAMWin32FindLocalOnly,			// limit search to local filesystem
	STEAMWin32FindRemoteOnly,			// limit search to remote repository
	STEAMWin32FindAll					// do not limit search (duplicates allowed)
} STEAMWin32FindFilter;

typedef enum
{
	eSteamWin32InterfaceHandleTypeDontUse
} STEAM_HANDLE;

extern const STEAM_HANDLE STEAM_INVALID_HANDLE_VALUE;

STEAM_HANDLE 
WINAPI 
STEAM_CreateFile(
	LPCTSTR lpFileName, 
	DWORD dwDesiredAccess, 
	DWORD dwShareMode,							// must be 0
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,	// must be NULL
	DWORD dwCreationDisposition, 
	DWORD dwFlagsAndAttributes,					// must be 0
	HANDLE hTemplateFile						// must be NULL
	);

BOOL
WINAPI
STEAM_CloseHandle(
	STEAM_HANDLE hObject   // handle to object
	);

BOOL
WINAPI
STEAM_ReadFile(
	STEAM_HANDLE hFile,                // handle to file
	LPVOID lpBuffer,             // data buffer
	DWORD nNumberOfBytesToRead,  // number of bytes to read
	LPDWORD lpNumberOfBytesRead, // number of bytes read
	LPOVERLAPPED lpOverlapped    // overlapped buffer, must be NULL
	);

BOOL 
WINAPI
STEAM_WriteFile(
	STEAM_HANDLE hFile,                    // handle to file
	LPCVOID lpBuffer,                // data buffer
	DWORD nNumberOfBytesToWrite,     // number of bytes to write
	LPDWORD lpNumberOfBytesWritten,  // number of bytes written
	LPOVERLAPPED lpOverlapped        // overlapped buffer, must be NULL
	);

BOOL
WINAPI
STEAM_FlushFileBuffers(
	STEAM_HANDLE hFIle
	);

DWORD 
WINAPI
STEAM_SetFilePointer(
  STEAM_HANDLE hFile,                // handle to file
  LONG lDistanceToMove,        // bytes to move pointer
  PLONG lpDistanceToMoveHigh,  // bytes to move pointer
  DWORD dwMoveMethod           // starting point
);

STEAM_HANDLE
WINAPI
STEAM_FindFirstFile(
	LPCTSTR pszMatchName, 
	STEAMWin32FindFilter filter, 
	WIN32_FIND_DATA *findInfo
	);

BOOL
WINAPI
STEAM_FindNextFile(
	STEAM_HANDLE dir, 
	WIN32_FIND_DATA *findInfo
	);

BOOL
WINAPI
STEAM_FindClose(
	STEAM_HANDLE dir
	);

DWORD 
WINAPI
STEAM_GetFileAttributes(
	LPCTSTR lpFileName   // name of file or directory
	);

BOOL 
WINAPI
STEAM_GetFileAttributesEx(
  LPCTSTR lpFileName,                   // file or directory name
  GET_FILEEX_INFO_LEVELS fInfoLevelId,  // attribute 
  LPVOID lpFileInformation              // attribute information 
);

//BOOL 
//WINAPI
//STEAM_GetFileInformationByHandle(
//	STEAM_HANDLE hFile,                                  // handle to file 
//	LPBY_HANDLE_FILE_INFORMATION lpFileInformation // buffer
//	);

DWORD
WINAPI
STEAM_GetFileSize(
	STEAM_HANDLE hFile,           // handle to file
	LPDWORD lpFileSizeHigh  // high-order word of file size
	);

//DWORD 
//WINAPI
//STEAM_GetFileType(
//	STEAM_HANDLE hFile   // handle to file
//	);			   // Steam-controlled -> FILE_TYPE_REMOTE

#ifdef __cplusplus
}
#endif

#endif /* #ifndef INCLUDED_STEAMLIBAPI_H */


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
** Target restrictions:
**
** Tool restrictions:
**
******************************************************************************/

// Any precompiled headers (precompiled must come first).
//#include "Stable.h"

// Conditional-compilation control for the current project.

// Definitions used by all of our projects and components.
//#ifndef INCLUDED_COMMONENVIRONMENT_H
//	#include "Projects/Common/Inc/CommonEnvironment.h"
//#endif
//#ifndef INCLUDED_COMMONASSERTS_H
//	#include "Projects/Common/Asserts/Inc/CommonAsserts.h"
//#endif

// Definitions shared throughout this project/component.

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <tchar.h>
#include <string.h>

#include "SteamWin32.h"
#ifndef INCLUDED_STEAM_H
    #include "Steam.h"
#endif

#include <assert.h>

const STEAM_HANDLE STEAM_INVALID_HANDLE_VALUE = (STEAM_HANDLE)(STEAM_INVALID_HANDLE);

static const char * TCharToChar( char *buf, LPCTSTR source )
{
	char *pszName;
	size_t nRet;
	assert(buf);
	assert(source);
	pszName = buf;//lint -esym(613,pszName)
#ifdef _UNICODE
	nRet = wcstombs(buf, source, MAX_PATH);//lint !e668
	assert(nRet == _tcslen(source));
#else
	strcpy(buf, source);//lint !e668
	nRet = 0;
#endif
	while ( *pszName ) 
	{
		if ( *pszName == _T('/') )
			*pszName = _T('\\');
		pszName++;
	}

	return buf;
}//lint !e550

static 	void TimetToFileTime( long t, LPFILETIME pft )
{
	LONGLONG ll = Int32x32To64(t, 10000000) + 116444736000000000;//lint !e1924
	assert(pft);//lint !e717
	pft->dwLowDateTime = (DWORD)(ll & 0xFFFFFFFF);//lint !e613
	pft->dwHighDateTime = (DWORD)(ll >>32);//lint !e613
}



static DWORD ConvertSteamErrorToWin32ErrorCode(const TSteamError *e)
{
	DWORD uCode = 0;

	assert(e);

	switch(e->eSteamError)//lint !e613
	{
	case eSteamErrorNone:
		uCode = ERROR_SUCCESS;
		break;
	case eSteamErrorUnknown:
		uCode = ERROR_INVALID_FUNCTION;
		break;
	case eSteamErrorLibraryNotInitialized:
		uCode = ERROR_NOT_READY;
		break;
	case eSteamErrorBadHandle:
		uCode = ERROR_INVALID_HANDLE;
		break;
	case eSteamErrorHandlesExhausted:
		uCode = ERROR_TOO_MANY_OPEN_FILES;
		break;
	case eSteamErrorNotFound:
		uCode = ERROR_FILE_NOT_FOUND;
		break;
	case eSteamErrorBadArg:
		uCode = ERROR_INVALID_PARAMETER;
		break;
	case eSteamErrorCacheRead:
	case eSteamErrorCacheCorrupted:
	case eSteamErrorCacheWrite:
	case eSteamErrorCacheSession:
	case eSteamErrorCacheInternal:
	case eSteamErrorCacheBadApp:
	case eSteamErrorCacheVersion:
	case eSteamErrorCacheBadFingerPrint:
	default:
		uCode = ERROR_GEN_FAILURE;
		break;
	}

	return uCode;
}

///////////////////////////////////////////////////////////////////////////

STEAM_HANDLE 
WINAPI 
STEAM_CreateFile(
	LPCTSTR lpFileName, 
	DWORD dwDesiredAccess, 
	DWORD dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,	// must be NULL
	DWORD dwCreationDisposition, 
	DWORD dwFlagsAndAttributes, 
	HANDLE hTemplateFile
	)
{
	TSteamError e;
	STEAM_HANDLE hRet = STEAM_INVALID_HANDLE_VALUE;
	SteamHandle_t hSteamHandle;
	char buf[_MAX_PATH];
	char cszOpenFlags[8];

	assert(lpFileName);
	if (!lpFileName)//lint !e774 always
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return hRet;
	}

	assert(dwShareMode == 0);
	assert(lpSecurityAttributes == NULL);
	assert(dwFlagsAndAttributes == 0);
	assert(hTemplateFile == NULL);

	if (dwShareMode || lpSecurityAttributes || dwFlagsAndAttributes || hTemplateFile)//lint !e774
	{
		SetLastError(ERROR_NOT_SUPPORTED);
		return hRet;
	}

	cszOpenFlags[0] = '\0';

	// up-front checks
	if (dwCreationDisposition == CREATE_NEW)
	{
		// fail if it's already there
		if (0xffffffff != STEAM_GetFileAttributes(lpFileName))
		{
			SetLastError(ERROR_FILE_EXISTS);
			return hRet;
		}
	}
	else if (dwCreationDisposition == CREATE_ALWAYS)
	{
		// delete it if it exists
		DeleteFile(lpFileName);//lint !e534
		// fail if it's still there (in Steam)
		if (0xffffffff != STEAM_GetFileAttributes(lpFileName))
		{
			SetLastError(ERROR_ACCESS_DENIED);
			return hRet;
		}
	}

	if ((dwDesiredAccess & (GENERIC_READ | GENERIC_WRITE)) == (GENERIC_READ | GENERIC_WRITE))
	{
		switch( dwCreationDisposition )
		{
		case OPEN_EXISTING:
			strcat(cszOpenFlags, "r+");
			break;
		case CREATE_NEW:
		case CREATE_ALWAYS:
		case TRUNCATE_EXISTING:
			strcat(cszOpenFlags, "w+");
			break;
		case OPEN_ALWAYS:
			strcat(cszOpenFlags, "a+");
			break;
		default:
			assert(0);
			return hRet;//lint !e527
		}
	}
	else if (dwDesiredAccess & GENERIC_WRITE)
	{
		switch (dwCreationDisposition)
		{
		case OPEN_EXISTING:
			if (0xFFFFFFFF == STEAM_GetFileAttributes(lpFileName))
			{
				// catch this here since "w" will creat if doesn't exist
				SetLastError(ERROR_FILE_NOT_FOUND);
				return hRet;
			}
			// no break
		case CREATE_NEW:
		case CREATE_ALWAYS:
		case TRUNCATE_EXISTING:
			strcat(cszOpenFlags, "w");
			break;
		case OPEN_ALWAYS:
			strcat(cszOpenFlags, "a");
			break;
		default:
			assert(0);
			return hRet;//lint !e527
		};
	}
	else
	{
		assert(dwCreationDisposition == OPEN_EXISTING);
		strcat(cszOpenFlags, "r");
	}

	hSteamHandle = SteamOpenFile(TCharToChar(buf, lpFileName), cszOpenFlags, &e);

	if (hSteamHandle == STEAM_INVALID_HANDLE)
	{
		SetLastError(ConvertSteamErrorToWin32ErrorCode(&e));
		return hRet;
	}

	hRet = (STEAM_HANDLE)hSteamHandle;
	return hRet;
}

BOOL
WINAPI
STEAM_CloseHandle(
	STEAM_HANDLE hFile   // handle to object
	)
{
	TSteamError e;
	BOOL bRet = FALSE;
	SteamHandle_t hSteamHandle = (SteamHandle_t)(hFile);

	if (SteamCloseFile(hSteamHandle, &e) < 0)
	{
		SetLastError(ConvertSteamErrorToWin32ErrorCode(&e));
	}
	else
	{
		bRet = TRUE;
	}

	return bRet;
}

BOOL
WINAPI
STEAM_ReadFile(
	STEAM_HANDLE hFile,                // handle to file
	LPVOID lpBuffer,             // data buffer
	DWORD nNumberOfBytesToRead,  // number of bytes to read
	LPDWORD lpNumberOfBytesRead, // number of bytes read
	LPOVERLAPPED lpOverlapped    // overlapped buffer, must be NULL
	)
{
	TSteamError e;
	BOOL bRet = FALSE;
	SteamHandle_t hSteamHandle = (SteamHandle_t)(hFile);

	assert(lpBuffer);
	assert(lpNumberOfBytesRead);
	if (!lpBuffer || !lpNumberOfBytesRead)//lint !e774 always
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return bRet;
	}

	assert(lpOverlapped == NULL);
	if (lpOverlapped)//lint !e774
	{
		SetLastError(ERROR_NOT_SUPPORTED);
		return bRet;
	}

	*lpNumberOfBytesRead = 0; // to match MSDN docs

	*lpNumberOfBytesRead = SteamReadFile(lpBuffer, 1, nNumberOfBytesToRead, hSteamHandle, &e);

	if (*lpNumberOfBytesRead == nNumberOfBytesToRead)
	{
		bRet = TRUE;
	}
	else
	{
		// set last error
		SetLastError(ConvertSteamErrorToWin32ErrorCode(&e));
	}

	return bRet;
}

BOOL 
WINAPI
STEAM_WriteFile(
	STEAM_HANDLE hFile,                    // handle to file
	LPCVOID lpBuffer,                // data buffer
	DWORD nNumberOfBytesToWrite,     // number of bytes to write
	LPDWORD lpNumberOfBytesWritten,  // number of bytes written
	LPOVERLAPPED lpOverlapped        // overlapped buffer, must be NULL
	)
{
	TSteamError e;
	BOOL bRet = FALSE;
	SteamHandle_t hSteamHandle = (SteamHandle_t)(hFile);

	assert(lpBuffer);
	assert(lpNumberOfBytesWritten);
	if (!lpBuffer || !lpNumberOfBytesWritten)//lint !e774 always
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return bRet;
	}

	assert(lpOverlapped == NULL);
	if (lpOverlapped)//lint !e774
	{
		SetLastError(ERROR_NOT_SUPPORTED);
		return bRet;
	}

	*lpNumberOfBytesWritten = 0; // to match MSDN docs

	*lpNumberOfBytesWritten = SteamWriteFile(lpBuffer, 1, nNumberOfBytesToWrite, hSteamHandle, &e);

	if (*lpNumberOfBytesWritten == nNumberOfBytesToWrite)
	{
		bRet = TRUE;
	}
	else
	{
		SetLastError(ConvertSteamErrorToWin32ErrorCode(&e));
	}

	return bRet;
}

BOOL
WINAPI
STEAM_FlushFileBuffers(
	STEAM_HANDLE hFile
	)
{
	TSteamError e;
	BOOL bRet = TRUE;
	SteamHandle_t hSteamHandle = (SteamHandle_t)(hFile);
	
	if (SteamFlushFile(hSteamHandle, &e))
	{
		bRet = FALSE;
		SetLastError(ConvertSteamErrorToWin32ErrorCode(&e));
	}

	return bRet;
}

DWORD 
WINAPI
STEAM_SetFilePointer(
	STEAM_HANDLE hFile,                // handle to file
	LONG lDistanceToMove,        // bytes to move pointer
	PLONG lpDistanceToMoveHigh,  // bytes to move pointer
	DWORD dwMoveMethod           // starting point
	)
{
	TSteamError e;
	DWORD dwRet = 0xFFFFFFFF;
	SteamHandle_t hSteamHandle = (SteamHandle_t)(hFile);
	ESteamSeekMethod eMethod;
	long lRet;

	assert(lpDistanceToMoveHigh == NULL);
	if (lpDistanceToMoveHigh)//lint !e774
	{
		SetLastError(ERROR_NOT_SUPPORTED);
		return dwRet;
	}

	switch(dwMoveMethod)
	{
	case FILE_BEGIN:
		eMethod = eSteamSeekMethodSet;
		break;
	case FILE_END:
		eMethod = eSteamSeekMethodEnd;
		break;
	case FILE_CURRENT:
		eMethod = eSteamSeekMethodCur;
		break;
	default:
		assert(0);
		return dwRet;//lint !e527
	}

	if (lDistanceToMove)
	{
		if (SteamSeekFile(hSteamHandle, lDistanceToMove, eMethod, &e) < 0)
		{
			SetLastError(ConvertSteamErrorToWin32ErrorCode(&e));
			return dwRet;
		}
	}

	lRet = SteamTellFile(hSteamHandle, &e);
	if ( e.eSteamError != eSteamErrorNone )
	{
		SetLastError(ConvertSteamErrorToWin32ErrorCode(&e));
		return dwRet;
	}

	dwRet = (DWORD)lRet;

	return dwRet;
}

DWORD 
WINAPI
STEAM_GetFileAttributes(
	LPCTSTR lpFileName   // name of file or directory
	)
{
	TSteamError e;
	TSteamElemInfo steamFindInfo;
	DWORD dwRet = 0xFFFFFFFF;
	char buf[_MAX_PATH];

	assert(lpFileName);
	if (!lpFileName)//lint !e774 always
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return dwRet;
	}

	if ( SteamStat(TCharToChar(buf, lpFileName), &steamFindInfo, &e) >= 0 )
	{
		if (steamFindInfo.bIsLocal)
		{
			dwRet = GetFileAttributes(lpFileName);
		}
		else
		{
			dwRet = FILE_ATTRIBUTE_OFFLINE;  // HACK to indicate Steam-ized

			if (steamFindInfo.bIsDir)
				dwRet |= FILE_ATTRIBUTE_DIRECTORY;
		}
	}
	else
	{
		SetLastError(ConvertSteamErrorToWin32ErrorCode(&e));
	}

	return dwRet;
}

BOOL 
WINAPI
STEAM_GetFileAttributesEx(
	LPCTSTR lpFileName,                   // file or directory name
	GET_FILEEX_INFO_LEVELS fInfoLevelId,  // attribute 
	LPVOID lpFileInformation              // attribute information 
	)
{
	TSteamError e;
	TSteamElemInfo steamFindInfo;
	BOOL bRet = FALSE;
	char buf[_MAX_PATH];

	assert(lpFileName);
	assert(lpFileInformation);
	assert (fInfoLevelId == GetFileExInfoStandard);

	if (!lpFileName || !lpFileInformation
		|| (fInfoLevelId != GetFileExInfoStandard))//lint !e774 always
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return bRet;
	}

	if ( SteamStat(TCharToChar(buf, lpFileName), &steamFindInfo, &e) >= 0 )
	{
		if (steamFindInfo.bIsLocal)
		{
			bRet = GetFileAttributesEx(lpFileName, fInfoLevelId, lpFileInformation);
		}
		else
		{
			WIN32_FILE_ATTRIBUTE_DATA *pInfo = (WIN32_FILE_ATTRIBUTE_DATA*)lpFileInformation;

			pInfo->dwFileAttributes = 0;
			pInfo->dwFileAttributes |= FILE_ATTRIBUTE_OFFLINE;  // HACK to indicate Steam-ized

			if (steamFindInfo.bIsDir)
				pInfo->dwFileAttributes |= FILE_ATTRIBUTE_DIRECTORY;

			TimetToFileTime(steamFindInfo.lCreationTime, &pInfo->ftCreationTime);
			TimetToFileTime(steamFindInfo.lLastAccessTime, &pInfo->ftLastAccessTime);
			TimetToFileTime(steamFindInfo.lLastModificationTime, &pInfo->ftLastWriteTime);

			pInfo->nFileSizeLow = steamFindInfo.uSizeOrCount;
			pInfo->nFileSizeHigh = 0;
		}
	}
	else
	{
		SetLastError(ConvertSteamErrorToWin32ErrorCode(&e));
	}

	return bRet;
}

/*
BOOL 
WINAPI
STEAM_GetFileInformationByHandle(
	STEAM_HANDLE hFile,                                  // handle to file 
	LPBY_HANDLE_FILE_INFORMATION lpFileInformation // buffer
	)
{
	TSteamError e;
	BOOL bRet = FALSE;
	SteamHandle_t hSteamHandle = (SteamHandle_t)(hFile);

	assert(lpFileInformation);
	if (!lpFileInformation)//lint !e774 always
		return bRet;

	// crap, no way of doing this now. Unsupported.

	return bRet;
}
*/

DWORD
WINAPI
STEAM_GetFileSize(
	STEAM_HANDLE hFile,           // handle to file
	LPDWORD lpFileSizeHigh  // high-order word of file size
	)
{
	TSteamError e;
	SteamHandle_t hSteamHandle = (SteamHandle_t)(hFile);
	long orig_pos, end_pos;
	
	// Figure out where in the file we currently are...
	orig_pos = SteamTellFile(hSteamHandle, &e);
	if ( e.eSteamError != eSteamErrorNone )
	{
		SetLastError(ConvertSteamErrorToWin32ErrorCode(&e));
		return INVALID_FILE_SIZE;
	}

	// Jump to the end...
	if ( SteamSeekFile(hSteamHandle, 0L, eSteamSeekMethodEnd, &e) != 0 )
	{
		SetLastError(ConvertSteamErrorToWin32ErrorCode(&e));
		return INVALID_FILE_SIZE;
	}

	// get location
	end_pos = SteamTellFile(hSteamHandle, &e);
	if ( e.eSteamError != eSteamErrorNone )
	{
		SetLastError(ConvertSteamErrorToWin32ErrorCode(&e));
		return INVALID_FILE_SIZE;
	}

	// Go back
	if ( SteamSeekFile(hSteamHandle, orig_pos, eSteamSeekMethodSet, &e) != 0)
	{
		SetLastError(ConvertSteamErrorToWin32ErrorCode(&e));
		return INVALID_FILE_SIZE;
	}

	if(lpFileSizeHigh)
		*lpFileSizeHigh = 0;

	return (DWORD)end_pos;
}

/*
DWORD 
WINAPI
STEAM_GetFileType(
	STEAM_HANDLE hFile   // handle to file
	)			   // Steam-controlled -> FILE_TYPE_REMOTE
{
	TSteamError e;
	DWORD dwRet = 0;
	SteamHandle_t hSteamHandle = (SteamHandle_t)(hFile);

	return dwRet;
}
*/

/**************/
//STEAM_FindFirstFile
/**************/

STEAM_HANDLE
WINAPI
STEAM_FindFirstFile(
	LPCTSTR pszMatchName, 
	STEAMWin32FindFilter filter, 
	WIN32_FIND_DATA *findInfo
	)
{
	TSteamError e;
	TSteamElemInfo steamFindInfo;
	STEAM_HANDLE hResult = STEAM_INVALID_HANDLE_VALUE;
	SteamHandle_t steamResult;
	ESteamFindFilter eFilter;
	char buf[_MAX_PATH];

	assert(pszMatchName);
	assert(findInfo);
	if (!pszMatchName || !findInfo)//lint !e774 always
		return STEAM_INVALID_HANDLE_VALUE;

	switch (filter)
	{
	case STEAMWin32FindLocalOnly:
		eFilter = eSteamFindLocalOnly;
		break;
	case STEAMWin32FindRemoteOnly:
		eFilter = eSteamFindRemoteOnly;
		break;
	case STEAMWin32FindAll:
		eFilter = eSteamFindAll;
		break;
	default:
		assert(0);
		return STEAM_INVALID_HANDLE_VALUE;//lint !e527 unreachable
	}

	steamResult = SteamFindFirst(TCharToChar(buf, pszMatchName), eFilter, &steamFindInfo, &e);

	if ( steamResult == STEAM_INVALID_HANDLE )
	{
		hResult = STEAM_INVALID_HANDLE_VALUE;
	}
	else
	{
		hResult = (STEAM_HANDLE)(steamResult);

#ifdef _UNICODE
		mbstowcs(findInfo->cFileName, steamFindInfo.cszName, MAX_PATH);//lint !e534
#else
		strcpy(findInfo->cFileName, steamFindInfo.cszName);
#endif

		// this isnt' the right call. don't know what to do.
		//GetShortPathName(findInfo->cFileName, findInfo->cAlternateFileName, sizeof(findInfo->cAlternateFileName));//lint !e534
		findInfo->cAlternateFileName[0] = _T('\0');
		
		TimetToFileTime(steamFindInfo.lLastModificationTime, &findInfo->ftLastWriteTime);
		TimetToFileTime(steamFindInfo.lCreationTime, &findInfo->ftCreationTime);
		TimetToFileTime(steamFindInfo.lLastAccessTime, &findInfo->ftLastAccessTime);
		findInfo->nFileSizeLow = steamFindInfo.uSizeOrCount;
		findInfo->nFileSizeHigh = 0;

		findInfo->dwFileAttributes = 0;
		// Determine if the found object is a directory...
		if ( steamFindInfo.bIsDir )
			findInfo->dwFileAttributes |= FILE_ATTRIBUTE_DIRECTORY;

		// Determine if the found object was local or remote.
		// ***NOTE*** we are hijacking the FILE_ATTRIBUTE_OFFLINE bit and using it in a different
		//            (but similar) manner than the WIN32 documentation indicates ***NOTE***
		if ( !steamFindInfo.bIsLocal )
			findInfo->dwFileAttributes |= FILE_ATTRIBUTE_OFFLINE;
	}
	
	return hResult;
}

/**************/
//STEAM_FindNextFile
/**************/

BOOL
WINAPI
STEAM_FindNextFile(
	STEAM_HANDLE dir, 
	WIN32_FIND_DATA *findInfo
	)
{
	TSteamError e;
	TSteamElemInfo steamFindInfo;
	SteamHandle_t hDir = (SteamHandle_t)(dir);
	int result;

	assert(findInfo);
	if (!findInfo)//lint !e774 always
		return FALSE;

	// STEAM return success per _findnext()...flip for FindNextFile()
	result = (SteamFindNext(hDir, &steamFindInfo, &e) == 0);

	if ( result )
	{
		dir = (STEAM_HANDLE)(hDir);

#ifdef _UNICODE
		mbstowcs(findInfo->cFileName, steamFindInfo.cszName, MAX_PATH);//lint !e534
#else
		strcpy(findInfo->cFileName, steamFindInfo.cszName);
#endif

		TimetToFileTime(steamFindInfo.lLastModificationTime, &findInfo->ftLastWriteTime);
		TimetToFileTime(steamFindInfo.lCreationTime, &findInfo->ftCreationTime);
		TimetToFileTime(steamFindInfo.lLastAccessTime, &findInfo->ftLastAccessTime);
		findInfo->nFileSizeLow = steamFindInfo.uSizeOrCount;
		findInfo->nFileSizeHigh = 0;

		if ( steamFindInfo.bIsDir )
			findInfo->dwFileAttributes |= FILE_ATTRIBUTE_DIRECTORY;
		else
			findInfo->dwFileAttributes &= ~FILE_ATTRIBUTE_DIRECTORY;

		// Determine if the found object was local or remote.
		// ***NOTE*** we are hijacking the FILE_ATTRIBUTE_OFFLINE bit and using it in a different
		//            (but similar) manner than the WIN32 documentation indicates ***NOTE***
		if ( steamFindInfo.bIsLocal )
			findInfo->dwFileAttributes &= ~FILE_ATTRIBUTE_OFFLINE;
		else
			findInfo->dwFileAttributes |= FILE_ATTRIBUTE_OFFLINE;
	}
	
	return result;
}

/**************/
//STEAM_FindClose
/**************/

BOOL
WINAPI
STEAM_FindClose(
	STEAM_HANDLE dir
	)
{
	TSteamError e;
	SteamHandle_t hDir = (SteamHandle_t)(dir);
	int result;
	
	// STEAM return success per _findclose()...flip for FindClose()
	result = (SteamFindClose(hDir, &e) == 0); 
	return result;
}









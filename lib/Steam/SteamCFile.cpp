
//*********** (C) Copyright 2002 Valve, L.L.C. All rights reserved. ***********
//
// The copyright to the contents herein is the property of Valve, L.L.C.
// The contents may be used and/or copied only with the written permission of
// Valve, L.L.C., or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
//*****************************************************************************
//
// Contents:
//
//		This file provides the implementation details for the Steam CFile class.
//      This class is intended as a wrapper for the Steam API provided in
//		Steam.h.  This class provides an interface like that of
//		CFile, but where the underlying file may be Steam-controlled.
//      The class is NOT polymorphic with CFile, and not all of the API
//		features of CFile are currently implemented.
//
//		This file also provides the implementation for the Steam CFileFind class.
//      This class is intended as a PARTIAL wrapper for the Steam API provided in
//		Steam.h.  This class provides an interface like that of
//		CFileFind, but where the underlying files and directories may be Steam-controlled.
//      The class is NOT polymorphic with CFileFind, but all of the API
//		features of CFile are implemented.
//
//		Using the EFindType, you can specify whether to find only Steam files,
//		only "local" files (might as well use CFileFind in that case, however),
//		or both.
//
// Target restrictions:
//
// Tool restrictions:
//
// Things to do:
//
//		
//
//*****************************************************************************


//*****************************************************************************
//
// Include files required to build and use this module.
//
//*****************************************************************************

// Precompiled header (must come first - includes project common headers)
#include "Stable.h"

// Definitions shared throughout this project/component.

#include <direct.h>

// This module's own header.
#ifndef INCLUDED_STEAMCFILE_H
    #include "SteamCFile.h"
#endif


#ifndef INCLUDED_STEAM_H
    #include "Steam.h"
#endif

// Other module headers required by this module.


//*****************************************************************************
//
// Any 'using' directives or declarations (namespaces) required by this module.
//
//*****************************************************************************

//*****************************************************************************
//
// File global macros.
//
//*****************************************************************************

//*****************************************************************************
//
// File global scalar type and enumerated type definitions.
// - wrap these definitions in an unnamed namespace since static deprecated.
//
//*****************************************************************************

#ifdef _DEBUG
#define __MY_ASSERT_VALID AssertValid()
#else
#define __MY_ASSERT_VALID 
#endif





//*****************************************************************************
//
// File global class, structure, and complex type definitions.
// - wrap these definitions in an unnamed namespace since static deprecated.
//
//*****************************************************************************


namespace
{
	struct TPathInfo
	{
		TCHAR m_FilePathBuf[_MAX_PATH];
		TCHAR *m_sOriginalPath;
		TCHAR *m_sFileName;
		TCHAR *m_sFileExtension;

		TPathInfo()
			: m_sOriginalPath(m_FilePathBuf), m_sFileName(m_FilePathBuf), m_sFileExtension(m_FilePathBuf)
		{
			memset(m_FilePathBuf, 0, _MAX_PATH);
		}
		
		void Init(LPCTSTR cszFilePath)
		{
			ASSERT(cszFilePath);
			
			if (!cszFilePath || (_tcslen(cszFilePath)==0))	//lint !e774 Boolean within 'left side of || within if' always evaluates to False (due to ASSERT)
			{
				memset(m_FilePathBuf, 0, _MAX_PATH);
			}
			else
			{
				TCHAR drive[_MAX_DRIVE];
				TCHAR dir[_MAX_DIR];
				TCHAR fname[_MAX_FNAME];
				TCHAR ext[_MAX_EXT];
				
				_tsplitpath(cszFilePath, drive, dir, fname, ext);

				if (drive[0] == 0)
				{
					TCHAR cwd[_MAX_PATH];
					TCHAR *pRet = _tgetcwd(cwd, _MAX_PATH);
					ASSERT(pRet);
					//lint -esym(529,pRet)

					// not a full path
					if (dir[0] == _TCHAR('\\') || dir[0] == _TCHAR('/'))
					{
						// root of current drive, get current drive
						_tcsncpy(m_FilePathBuf, cwd, 2);//lint !e534
						_tcscat(m_FilePathBuf, cszFilePath);//lint !e534
						m_sOriginalPath = &m_FilePathBuf[2];
						m_sFileName = m_sOriginalPath + _tcslen(dir);
						m_sFileExtension = m_sFileName + _tcslen(fname);//lint !e662 OOB ptr
					}
					else
					{
						// starts where we are now, get cwd
						_tcscpy(m_FilePathBuf, cwd);//lint !e534
						if (_tcslen(cwd) > 3)
							_tcscat(m_FilePathBuf, _T("\\"));//lint !e534
						m_sOriginalPath = &m_FilePathBuf[_tcslen(m_FilePathBuf)];
						_tcscat(m_FilePathBuf, cszFilePath);//lint !e534
						m_sFileName = m_sOriginalPath + _tcslen(dir);//lint !e662 OOB ptr
						m_sFileExtension = m_sFileName + _tcslen(fname);//lint !e662 OOB ptr
					}
				}
				else
				{
					// full path
					_tcscpy(m_FilePathBuf, cszFilePath);//lint !e534
					m_sOriginalPath = &m_FilePathBuf[0];
					m_sFileName = m_sOriginalPath + (2+_tcslen(dir));
					m_sFileExtension = m_sFileName + _tcslen(fname);//lint !e662 OOB ptr
				}
			}
		}
	};
}

struct CSteamFile::CImpl
{
	SteamHandle_t m_FileHandle;
	UINT m_nOpenFlags;
	TPathInfo m_PathInfo;

	CImpl()
		: m_FileHandle(STEAM_INVALID_HANDLE),
		  m_nOpenFlags(0),
		  m_PathInfo()
	{
	}
};

struct CSteamFileFind::CImpl
{
	SteamHandle_t m_DirHandle;
	TSteamElemInfo m_CurElemInfo;
	TSteamElemInfo m_NextElemInfo;
	CString m_sRoot;
	CString m_sFilter;
	TSteamError m_LastError;

	CImpl()
		: m_DirHandle(STEAM_INVALID_HANDLE),
		  m_CurElemInfo(),
		  m_NextElemInfo(),
		  m_sRoot(),
		  m_sFilter(),
		  m_LastError()
	{
		Init();
	}

	void Init()
	{
		m_DirHandle = STEAM_INVALID_HANDLE;
		memset(&m_CurElemInfo, 0, sizeof(TSteamElemInfo));
		memset(&m_NextElemInfo, 0, sizeof(TSteamElemInfo));
		memset(&m_LastError, 0, sizeof(TSteamError));
	}
};


//*****************************************************************************
//
// Exported variable and data definitions
// - declared as extern in header file, and defined without storage class here.
//
//*****************************************************************************


//*****************************************************************************
//
// File global variable and data definitions
// - wrap these definitions in an unnamed namespace since static deprecated.
//
//*****************************************************************************





//*****************************************************************************
//
// File global stand-alone function prototypes
// - wrap these definitions in an unnamed namespace since static deprecated.
//
//*****************************************************************************

namespace
{
	void TimetToFileTime( long t, LPFILETIME pft )
	{
		ASSERT(pft);//lint !e717
		LONGLONG ll = Int32x32To64(t, 10000000) + 116444736000000000;//lint !e1924
		pft->dwLowDateTime = static_cast<DWORD>(ll & 0xFFFFFFFF);//lint !e613
		pft->dwHighDateTime = static_cast<DWORD>(ll >>32);//lint !e613
	}

	int ConvertSteamErrorToMFCException(ESteamError e )
	{
		int m_cause;

		switch(e)
		{
		case eSteamErrorNone:
			m_cause = CFileException::none;
			break;
		case eSteamErrorUnknown:
			m_cause = CFileException::generic;
			break;
		case eSteamErrorLibraryNotInitialized:
			m_cause = CFileException::accessDenied; //FIX ME?
			break;
		case eSteamErrorBadHandle:
			m_cause = CFileException::invalidFile;
			break;
		case eSteamErrorHandlesExhausted:
			m_cause = CFileException::tooManyOpenFiles;
			break;
		case eSteamErrorNotFound:
			m_cause = CFileException::fileNotFound;
			break;
		case eSteamErrorRead:
			m_cause = CFileException::generic; //FIX ME?
			break;
		case eSteamErrorEOF:
			m_cause = CFileException::endOfFile;
			break;
		case eSteamErrorSeek:
			m_cause = CFileException::badSeek;
			break;
		case eSteamErrorCannotWriteNonUserConfigFile:
			m_cause = CFileException::accessDenied;
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
			m_cause = CFileException::generic;
			break;
		}

		return m_cause;
	}

	DWORD ConvertSteamErrorToWin32ErrorCode(const TSteamError &e)
	{
		DWORD uCode = 0;

		switch(e.eSteamError)
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
}


//*****************************************************************************
//
// Global stand-alone function definitions
// - declared as extern when prototyped in the header file, and defined
//	 without a storage class here.
//
// Also, any file global sub-routines of these global functions
// - declared as static here, and in their prototypes, above.
//
//*****************************************************************************


//*****************************************************************************
//
// Class definitions:
//
//*****************************************************************************

IMPLEMENT_DYNAMIC(CSteamFile, CObject)//lint !e1924 c-style

CSteamFile::~CSteamFile()
{
	delete m_pImpl;
}

CSteamFile::CSteamFile()
: CObject(),
  m_pImpl(new CImpl)
{
}

CSteamFile::CSteamFile(LPCTSTR lpszFileName, UINT nOpenFlags)
: CObject(),
  m_pImpl(new CImpl)
{
	CFileException e;
	if (!Open(lpszFileName, nOpenFlags, &e))//lint !e1506 virtual call
	{
		AfxThrowFileException(e.m_cause, e.m_lOsError, lpszFileName);
	}
}

// Attributes

DWORD CSteamFile::GetPosition() const
{
	__MY_ASSERT_VALID;
	ASSERT(m_pImpl->m_FileHandle != STEAM_INVALID_HANDLE);

	TSteamError e;
	long lRet = SteamTellFile(m_pImpl->m_FileHandle, &e);

	if (e.eSteamError != eSteamErrorNone)
	{
		AfxThrowFileException(ConvertSteamErrorToMFCException(e.eSteamError), -1, m_pImpl->m_PathInfo.m_FilePathBuf);
	}

	return static_cast<DWORD>(lRet);
}

CString CSteamFile::GetFileName() const
{
	__MY_ASSERT_VALID;
	ASSERT(m_pImpl->m_FileHandle != STEAM_INVALID_HANDLE);

	return m_pImpl->m_PathInfo.m_sFileName;
}

CString CSteamFile::GetFileTitle() const
{
	__MY_ASSERT_VALID;
	ASSERT(m_pImpl->m_FileHandle != STEAM_INVALID_HANDLE);

	return CString(m_pImpl->m_PathInfo.m_sFileName, 
		m_pImpl->m_PathInfo.m_sFileExtension -m_pImpl->m_PathInfo.m_sFileName);
}

CString CSteamFile::GetFilePath() const
{
	__MY_ASSERT_VALID;
	ASSERT(m_pImpl->m_FileHandle != STEAM_INVALID_HANDLE);

	return m_pImpl->m_PathInfo.m_FilePathBuf;
}

// Operations
BOOL CSteamFile::Open(LPCTSTR lpszFileName, UINT nOpenFlags,
	CFileException* pError)
{
	__MY_ASSERT_VALID;

	ASSERT(lpszFileName);//lint !e717
	TSteamError e;

	// must close if already opened
	if (m_pImpl->m_FileHandle != STEAM_INVALID_HANDLE)
	{
		// fill out error struct
		return FALSE;
	}

	char cszOpenFlags[8];
	cszOpenFlags[0] = '\0';

	if (nOpenFlags & CFile::modeReadWrite)
	{
		if (nOpenFlags & CFile::modeCreate)
		{
			if (nOpenFlags & CFile::modeNoTruncate)
			{
				// a+
				strcat(cszOpenFlags, "a+");
			}
			else
			{
				// w+
				strcat(cszOpenFlags, "w+");
			}
		}
		else
		{
			// r+
			strcat(cszOpenFlags, "r+");
		}
	}
	else if (nOpenFlags & CFile::modeWrite)
	{
		if (nOpenFlags & CFile::modeCreate)
		{
			if (nOpenFlags & CFile::modeNoTruncate)
			{
				// a
				strcat(cszOpenFlags, "a");
			}
			else
			{
				// w
				strcat(cszOpenFlags, "w");
			}
		}
		else
		{
			// r+
			strcat(cszOpenFlags, "r+");
		}
	}
	else
	{
		// r
		strcat(cszOpenFlags, "r");
	}

	if (nOpenFlags & CFile::typeText)
	{
		strcat(cszOpenFlags, "t");
	}
	else
	{
		strcat(cszOpenFlags, "b");
	}

	const char *pFilePath;
#ifdef UNICODE
	char buf[MAX_PATH];
	size_t nRet = wcstombs(buf, lpszFileName, MAX_PATH);//lint !e668
	ASSERT(nRet > 0);//lint !e717
	if (nRet != wcslen(lpszFileName))
	{
		if (pError)
		{
			pError->m_cause = CFileException::invalidFile;
			pError->m_lOsError = -1;
			pError->m_strFileName = lpszFileName;
		}
		return FALSE;
	}
	pFilePath = buf;
#else
	pFilePath = lpszFileName;
#endif
	m_pImpl->m_FileHandle = SteamOpenFile(pFilePath, cszOpenFlags, &e);

	if (e.eSteamError != eSteamErrorNone)
	{
		if (pError)
		{
			pError->m_cause = ConvertSteamErrorToMFCException(e.eSteamError);
			pError->m_lOsError = -1;
			pError->m_strFileName = lpszFileName;
		}
		return FALSE;
	}
	else
	{
		m_pImpl->m_PathInfo.Init(lpszFileName);
		m_pImpl->m_nOpenFlags = nOpenFlags;
		return TRUE;
	}

}

BOOL CSteamFile::GetStatus(CFileStatus& rStatus) const
{
	__MY_ASSERT_VALID;
	ASSERT(m_pImpl->m_FileHandle != STEAM_INVALID_HANDLE);

	return CSteamFile::GetStatus(m_pImpl->m_PathInfo.m_sOriginalPath, rStatus);
}

BOOL PASCAL CSteamFile::GetStatus(LPCTSTR lpszFileName,
			CFileStatus& rStatus)
{
	ASSERT(lpszFileName);//lint !e717
	TSteamError e;
	BOOL bRet = FALSE;

	TSteamElemInfo tInfo;

	const char *pFilePath;

#ifdef UNICODE

	char buf[MAX_PATH];

	size_t nRet = wcstombs(buf, lpszFileName, MAX_PATH);//lint !e668
	ASSERT(nRet > 0);

	if (nRet != wcslen(lpszFileName))
	{
		return FALSE;
	}

	pFilePath = buf;

#else

	pFilePath = lpszFileName;

#endif

	int iRet = SteamStat(pFilePath, &tInfo, &e);

	if (iRet >= 0)
	{
		if (tInfo.bIsLocal)
		{	
			bRet = CFile::GetStatus(lpszFileName, rStatus);
			ASSERT(bRet);
		}
		else 
		{
			bRet = TRUE;
			rStatus.m_ctime = CTime(static_cast<time_t>(tInfo.lCreationTime));			//lint !e747 significant prototype coercion (arg. no. 1) long to long long
			rStatus.m_mtime = CTime(static_cast<time_t>(tInfo.lLastModificationTime));	//lint !e747 significant prototype coercion (arg. no. 1) long to long long
			rStatus.m_atime = CTime(static_cast<time_t>(tInfo.lLastAccessTime));		//lint !e747 significant prototype coercion (arg. no. 1) long to long long
			rStatus.m_size = static_cast<LONG>(tInfo.uSizeOrCount);						//lint !e732 loss of sign
			rStatus.m_attribute = 0; // FIX ME

			//this could be a bit redundant, but oh well
			TPathInfo pathInfo;
			pathInfo.Init(lpszFileName);
			_tcscpy(rStatus.m_szFullName, pathInfo.m_FilePathBuf);//lint !e534
		}
	}

	return bRet;
}

DWORD CSteamFile::SeekToEnd()
{
	__MY_ASSERT_VALID;
	ASSERT(m_pImpl->m_FileHandle != STEAM_INVALID_HANDLE);

	TSteamError e;

	if (SteamSeekFile(m_pImpl->m_FileHandle, 0, eSteamSeekMethodEnd, &e) < 0)
	{
		AfxThrowFileException(ConvertSteamErrorToMFCException(e.eSteamError), -1, m_pImpl->m_PathInfo.m_FilePathBuf);
	}

	long lRet = SteamSizeFile(m_pImpl->m_FileHandle, &e);

	if (e.eSteamError != eSteamErrorNone)
	{
		AfxThrowFileException(ConvertSteamErrorToMFCException(e.eSteamError), -1, m_pImpl->m_PathInfo.m_FilePathBuf);
	}

	return static_cast<DWORD>(lRet);
}

void CSteamFile::SeekToBegin()
{
	__MY_ASSERT_VALID;
	ASSERT(m_pImpl->m_FileHandle != STEAM_INVALID_HANDLE);

	TSteamError e;

	if (SteamSeekFile(m_pImpl->m_FileHandle, 0, eSteamSeekMethodSet, &e) < 0)
	{
		AfxThrowFileException(ConvertSteamErrorToMFCException(e.eSteamError), -1, m_pImpl->m_PathInfo.m_FilePathBuf);
	}
}

// Overridables
CSteamFile* CSteamFile::Duplicate() const
{
	__MY_ASSERT_VALID;
	ASSERT(m_pImpl->m_FileHandle != STEAM_INVALID_HANDLE);

	return new CSteamFile(m_pImpl->m_PathInfo.m_sOriginalPath, m_pImpl->m_nOpenFlags);
}

LONG CSteamFile::Seek(LONG lOff, UINT nFrom)
{
	__MY_ASSERT_VALID;
	ASSERT(m_pImpl->m_FileHandle != STEAM_INVALID_HANDLE);

	TSteamError e;

	ESteamSeekMethod eMethod = eSteamSeekMethodCur;//make Lint happy
	if (nFrom == CFile::begin)
		eMethod = eSteamSeekMethodSet;
	else if (nFrom == CFile::end)
		eMethod = eSteamSeekMethodEnd;
	else if (nFrom == CFile::current)
		eMethod = eSteamSeekMethodCur;
	else
	{
		AfxThrowFileException(CFileException::badSeek, -1, m_pImpl->m_PathInfo.m_FilePathBuf);
	}

	if (SteamSeekFile(m_pImpl->m_FileHandle, lOff, eMethod, &e) < 0)
	{
		AfxThrowFileException(ConvertSteamErrorToMFCException(e.eSteamError), -1, m_pImpl->m_PathInfo.m_FilePathBuf);
	}

	long lRet = SteamTellFile(m_pImpl->m_FileHandle, &e);

	if (e.eSteamError != eSteamErrorNone)
	{
		AfxThrowFileException(ConvertSteamErrorToMFCException(e.eSteamError), -1, m_pImpl->m_PathInfo.m_FilePathBuf);
	}

	return static_cast<LONG>(lRet);

}

DWORD CSteamFile::GetLength() const
{
	__MY_ASSERT_VALID;
	ASSERT(m_pImpl->m_FileHandle != STEAM_INVALID_HANDLE);

	TSteamError e;

	long lRet = SteamSizeFile(m_pImpl->m_FileHandle, &e);

	if (e.eSteamError != eSteamErrorNone)
	{
		AfxThrowFileException(ConvertSteamErrorToMFCException(e.eSteamError), -1, m_pImpl->m_PathInfo.m_FilePathBuf);
	}

	return static_cast<DWORD>(lRet);
}


UINT CSteamFile::Read(void* lpBuf, UINT nCount)
{
	__MY_ASSERT_VALID;
	ASSERT(m_pImpl->m_FileHandle != STEAM_INVALID_HANDLE);

	TSteamError e;
	unsigned int uCountRead = 0;

	uCountRead = SteamReadFile(lpBuf, 1, nCount, m_pImpl->m_FileHandle, &e);

	if (e.eSteamError != eSteamErrorNone && e.eSteamError != eSteamErrorEOF)
	{
		AfxThrowFileException(ConvertSteamErrorToMFCException(e.eSteamError), -1, m_pImpl->m_PathInfo.m_FilePathBuf);
	}

	return uCountRead;
}

void CSteamFile::Write(const void* lpBuf, UINT nCount)
{
	__MY_ASSERT_VALID;
	ASSERT(m_pImpl->m_FileHandle != STEAM_INVALID_HANDLE);

	TSteamError e;
	unsigned int uCountWritten = 0;

	uCountWritten = SteamWriteFile(lpBuf, 1, nCount, m_pImpl->m_FileHandle, &e);

	if (e.eSteamError != eSteamErrorNone)
	{
		AfxThrowFileException(ConvertSteamErrorToMFCException(e.eSteamError), -1, m_pImpl->m_PathInfo.m_FilePathBuf);
	}

	if (uCountWritten != nCount)
	{
		// throw?
	}
}

void CSteamFile::Abort()
{
	__MY_ASSERT_VALID;

	if (m_pImpl->m_FileHandle != STEAM_INVALID_HANDLE)
	{
		TRY
		{
			Close();
		}
		CATCH_ALL( e )//lint !e1752 !e1924 !e717 catch not ref, c-style, do-while
		{
		}
		END_CATCH_ALL
	}
}

void CSteamFile::Flush()
{
	__MY_ASSERT_VALID;
	ASSERT(m_pImpl->m_FileHandle != STEAM_INVALID_HANDLE);

	TSteamError e;

	if (SteamFlushFile(m_pImpl->m_FileHandle, &e))
	{
		AfxThrowFileException(ConvertSteamErrorToMFCException(e.eSteamError), -1, m_pImpl->m_PathInfo.m_FilePathBuf);
	}
}

void CSteamFile::Close()
{
	__MY_ASSERT_VALID;
	ASSERT(m_pImpl->m_FileHandle != STEAM_INVALID_HANDLE);

	TSteamError e;

	if (m_pImpl->m_FileHandle == STEAM_INVALID_HANDLE)
	{
		AfxThrowFileException(CFileException::invalidFile, -1, _T(""));
	}

	if (SteamCloseFile(m_pImpl->m_FileHandle, &e) < 0)
	{
		AfxThrowFileException(ConvertSteamErrorToMFCException(e.eSteamError), -1, m_pImpl->m_PathInfo.m_FilePathBuf);
	}

	m_pImpl->m_FileHandle = STEAM_INVALID_HANDLE;
	m_pImpl->m_PathInfo.Init(_T(""));
	m_pImpl->m_nOpenFlags = 0;
}

#ifdef _DEBUG

void CSteamFile::Dump(CDumpContext &dc) const
{
	dc << "CSteamFile -- ";
	dc << "File: " << m_pImpl->m_PathInfo.m_FilePathBuf;
	dc << ", Steam Handle: " << m_pImpl->m_FileHandle;
	//dc << ", Is Local? " << (m_pImpl->m_FileInfo.bIsLocal ? "yes" : "no");
	dc << "\n";
}

void CSteamFile::AssertValid() const 
{
	if (m_pImpl->m_FileHandle != STEAM_INVALID_HANDLE)
	{
		ASSERT(*m_pImpl->m_PathInfo.m_FilePathBuf != 0);
	}
	else
	{
		ASSERT(*m_pImpl->m_PathInfo.m_FilePathBuf == 0);
	}
}

#endif

//*****************************************************************************
//
// Class definitions:
//
//*****************************************************************************

IMPLEMENT_DYNAMIC(CSteamFileFind,CObject)//lint !e1924 c-style

CSteamFileFind::CSteamFileFind()
: CObject(),
  m_pImpl(new CImpl)
{
}

CSteamFileFind::~CSteamFileFind()
{
	delete m_pImpl;
	m_pImpl = 0;
}

// Attributes

DWORD CSteamFileFind::GetLength() const
{
	__MY_ASSERT_VALID;

	return static_cast<DWORD>(m_pImpl->m_CurElemInfo.uSizeOrCount);
}

__int64 CSteamFileFind::GetLength64() const
{
	__MY_ASSERT_VALID;

	return static_cast<__int64>(m_pImpl->m_CurElemInfo.uSizeOrCount);

}

CString CSteamFileFind::GetFileName() const
{
	__MY_ASSERT_VALID;

	return CString( (LPCSTR)(m_pImpl->m_CurElemInfo.cszName ) ); //lint !e1924 C-sytle cast
}

CString CSteamFileFind::GetFilePath() const
{
	__MY_ASSERT_VALID;

	return m_pImpl->m_sRoot + CString( (LPCSTR)(m_pImpl->m_CurElemInfo.cszName) ); //lint !e1924 C-sytle cast
}

CString CSteamFileFind::GetFileTitle() const
{
	__MY_ASSERT_VALID;

	char buf[_MAX_FNAME];
	_splitpath(m_pImpl->m_CurElemInfo.cszName, NULL, NULL, buf, NULL);
	return CString(buf);
}

CString CSteamFileFind::GetFileURL() const
{
	__MY_ASSERT_VALID;

	return CString(CString(_T("file://")) + GetFilePath());
}

CString CSteamFileFind::GetRoot() const
{
	__MY_ASSERT_VALID;

	return m_pImpl->m_sRoot.Left(m_pImpl->m_sRoot.GetLength()-1);
}

BOOL CSteamFileFind::GetLastWriteTime(FILETIME* pTimeStamp) const
{
	__MY_ASSERT_VALID;
	if (*m_pImpl->m_CurElemInfo.cszName == 0)
	{
		return FALSE;
	}

	TimetToFileTime(m_pImpl->m_CurElemInfo.lLastModificationTime, pTimeStamp);

	return TRUE;
}

BOOL CSteamFileFind::GetLastAccessTime(FILETIME* pTimeStamp) const
{
	__MY_ASSERT_VALID;
	if (*m_pImpl->m_CurElemInfo.cszName == 0)
	{
		return FALSE;
	}

	TimetToFileTime(m_pImpl->m_CurElemInfo.lLastAccessTime, pTimeStamp);

	return TRUE;
}

BOOL CSteamFileFind::GetCreationTime(FILETIME* pTimeStamp) const
{
	__MY_ASSERT_VALID;
	if (*m_pImpl->m_CurElemInfo.cszName == 0)
	{
		return FALSE;
	}

	TimetToFileTime(m_pImpl->m_CurElemInfo.lCreationTime, pTimeStamp);

	return TRUE;
}

BOOL CSteamFileFind::GetLastWriteTime(CTime& refTime) const
{
	__MY_ASSERT_VALID;
	if (*m_pImpl->m_CurElemInfo.cszName == 0)
	{
		return FALSE;
	}

	refTime = CTime(static_cast<time_t>(m_pImpl->m_CurElemInfo.lLastModificationTime));	//lint !e747 significant prototype coercion (arg. no. 1) long to long long

	return TRUE;
}

BOOL CSteamFileFind::GetLastAccessTime(CTime& refTime) const
{
	__MY_ASSERT_VALID;
	if (*m_pImpl->m_CurElemInfo.cszName == 0)
	{
		return FALSE;
	}

	refTime = CTime(static_cast<time_t>(m_pImpl->m_CurElemInfo.lLastAccessTime));	//lint !e747 significant prototype coercion (arg. no. 1) long to long long

	return TRUE;
}

BOOL CSteamFileFind::GetCreationTime(CTime& refTime) const
{
	__MY_ASSERT_VALID;
	if (*m_pImpl->m_CurElemInfo.cszName == 0)
	{
		return FALSE;
	}

	refTime = CTime(static_cast<time_t>(m_pImpl->m_CurElemInfo.lCreationTime));	//lint !e747 significant prototype coercion (arg. no. 1) long to long long

	return TRUE;
}

BOOL CSteamFileFind::MatchesMask(DWORD dwMask) const
{
	__MY_ASSERT_VALID;
	if (*m_pImpl->m_CurElemInfo.cszName == 0)
	{
		return FALSE;
	}

	if (m_pImpl->m_CurElemInfo.bIsLocal)
	{
		DWORD dwAttribs = GetFileAttributes(GetFilePath());
		ASSERT(dwAttribs != 0xFFFFFFFF);
		return (0 != (dwMask & dwAttribs));
	}
	else
	{
		if (dwMask == FILE_ATTRIBUTE_NORMAL)
		{
			return (!m_pImpl->m_CurElemInfo.bIsDir && m_pImpl->m_CurElemInfo.bIsLocal);
		}
		else 
		{
			if( dwMask & FILE_ATTRIBUTE_DIRECTORY 
				&& m_pImpl->m_CurElemInfo.bIsDir )
			{
				return TRUE;
			}

			/*if (dwMask & FILE_ATTRIBUTE_READONLY
				&& m_pImpl->m_CurElemInfo.bIsLocal )
			{
				return TRUE;
			}*/
		}
	}

	return FALSE;
}

BOOL CSteamFileFind::IsDots() const
{
	__MY_ASSERT_VALID;
	if (*m_pImpl->m_CurElemInfo.cszName == 0)
	{
		return FALSE;
	}

	bool bRet = !strcmp(m_pImpl->m_CurElemInfo.cszName,".")
		|| !strcmp(m_pImpl->m_CurElemInfo.cszName,"..");

	return bRet;
}

// these aren't virtual because they all use MatchesMask(), which is
BOOL CSteamFileFind::IsReadOnly() const
{
	__MY_ASSERT_VALID;
	if (*m_pImpl->m_CurElemInfo.cszName == 0)
	{
		return FALSE;
	}
	
	return MatchesMask(FILE_ATTRIBUTE_READONLY);
}

BOOL CSteamFileFind::IsDirectory() const
{
	__MY_ASSERT_VALID;
	if (*m_pImpl->m_CurElemInfo.cszName == 0)
	{
		return FALSE;
	}
	
	return MatchesMask(FILE_ATTRIBUTE_DIRECTORY);
}

BOOL CSteamFileFind::IsCompressed() const
{
	__MY_ASSERT_VALID;
	if (*m_pImpl->m_CurElemInfo.cszName == 0)
	{
		return FALSE;
	}
	
	return MatchesMask(FILE_ATTRIBUTE_COMPRESSED);
}

BOOL CSteamFileFind::IsSystem() const
{
	__MY_ASSERT_VALID;
	if (*m_pImpl->m_CurElemInfo.cszName == 0)
	{
		return FALSE;
	}
	
	return MatchesMask(FILE_ATTRIBUTE_SYSTEM);
}

BOOL CSteamFileFind::IsHidden() const
{
	__MY_ASSERT_VALID;
	if (*m_pImpl->m_CurElemInfo.cszName == 0)
	{
		return FALSE;
	}
	
	return MatchesMask(FILE_ATTRIBUTE_HIDDEN);
}

BOOL CSteamFileFind::IsTemporary() const
{
	__MY_ASSERT_VALID;
	if (*m_pImpl->m_CurElemInfo.cszName == 0)
	{
		return FALSE;
	}
	
	return MatchesMask(FILE_ATTRIBUTE_TEMPORARY);
}

BOOL CSteamFileFind::IsNormal() const
{
	__MY_ASSERT_VALID;
	if (*m_pImpl->m_CurElemInfo.cszName == 0)
	{
		return FALSE;
	}
	
	return MatchesMask(FILE_ATTRIBUTE_NORMAL);
}

BOOL CSteamFileFind::IsArchived() const
{
	__MY_ASSERT_VALID;
	if (*m_pImpl->m_CurElemInfo.cszName == 0)
	{
		return FALSE;
	}
	
	return MatchesMask(FILE_ATTRIBUTE_ARCHIVE);
}

// Operations
void CSteamFileFind::Close()
{
	__MY_ASSERT_VALID;

	int nRet = -1;
	TSteamError e;
	if (m_pImpl->m_DirHandle != STEAM_INVALID_HANDLE)
	{
		nRet = SteamFindClose(m_pImpl->m_DirHandle, &e);
	}
	
	m_pImpl->Init(); //regardless

	if (nRet < 0)
	{
		// throw?
	}
}

BOOL CSteamFileFind::FindFile(LPCTSTR pstrName, EFindType eFindType)
{
	TSteamError e;

	m_pImpl->Init();

	TCHAR drive[_MAX_DRIVE];
	TCHAR dir[_MAX_DIR];
	TCHAR fname[_MAX_FNAME];
	TCHAR ext[_MAX_EXT];
	
	_tsplitpath(pstrName, drive, dir, fname, ext);
	m_pImpl->m_sFilter = fname;

	if (drive[0] == 0)
	{
		TCHAR cwd[_MAX_PATH];
		TCHAR *pRet = _tgetcwd(cwd, _MAX_PATH);
		ASSERT(pRet);

		// not a full path
		if (dir[0] == _TCHAR('\\') || dir[0] == _TCHAR('/'))
		{
			// root of current drive, get current drive
			m_pImpl->m_sRoot = CString(cwd, 2);
			m_pImpl->m_sRoot += dir;
		}
		else
		{
			// starts where we are now, get cwd
			m_pImpl->m_sRoot = cwd;
			m_pImpl->m_sRoot += "\\";
			m_pImpl->m_sRoot += dir;
		}
	}//lint !e529

	const char *pFilePath;
#ifdef UNICODE
	char buf[MAX_PATH];
	size_t nRet = wcstombs(buf, pstrName, MAX_PATH);//lint !e668
	ASSERT(nRet > 0);
	if (nRet != wcslen(pstrName))
	{
		return FALSE;
	}
	pFilePath = buf;
#else
	pFilePath = pstrName;
#endif

	ESteamFindFilter eFilter = (eFindType == eFindLocal ? (eSteamFindLocalOnly) 
		: (eFindType == eFindRemote ? eSteamFindRemoteOnly : eSteamFindAll));

	// catch invalid eFindType parameter
	ASSERT((eFilter != eSteamFindAll) || (eFindType == eFindBoth));

	m_pImpl->m_DirHandle = SteamFindFirst(pFilePath, eFilter, &m_pImpl->m_NextElemInfo, &e);

	if (e.eSteamError != eSteamErrorNone)
	{
		SetLastError(ConvertSteamErrorToWin32ErrorCode(e));
	}

	return (m_pImpl->m_DirHandle != STEAM_INVALID_HANDLE);
}//lint !e715 symbol dwUnused

BOOL CSteamFileFind::FindNextFile()
{
	ASSERT(m_pImpl->m_DirHandle != STEAM_INVALID_HANDLE);//lint !e717 do-while

	TSteamError e;

	m_pImpl->m_CurElemInfo = m_pImpl->m_NextElemInfo;

	int nRet = SteamFindNext(m_pImpl->m_DirHandle, &m_pImpl->m_NextElemInfo, &e);

	if (e.eSteamError != eSteamErrorNone)
	{
		SetLastError(ConvertSteamErrorToWin32ErrorCode(e));
	}

	return (nRet == 0);
}

#ifdef _DEBUG

void CSteamFileFind::Dump(CDumpContext &dc) const
{
	dc << "CSteamFileFind -- ";
	dc << "Root: " << m_pImpl->m_sRoot;
	dc << ", Filter: " << m_pImpl->m_sFilter;
	dc << ", Steam Handle: " << m_pImpl->m_DirHandle;
	dc << ", Current Element: " << m_pImpl->m_CurElemInfo.cszName;
	dc << ", Directory? " << (m_pImpl->m_CurElemInfo.bIsDir ? ("yes") : ("no"));
	dc << "\n";
}

void CSteamFileFind::AssertValid() const 
{
	if (m_pImpl->m_DirHandle == STEAM_INVALID_HANDLE)
	{
		ASSERT(*m_pImpl->m_CurElemInfo.cszName == 0
			&& *m_pImpl->m_NextElemInfo.cszName == 0);//lint !e717 do-while
	}
	else
	{
		ASSERT(*m_pImpl->m_CurElemInfo.cszName != 0
			&& *m_pImpl->m_NextElemInfo.cszName != 0);//lint !e717 do-while
	}
}

#endif


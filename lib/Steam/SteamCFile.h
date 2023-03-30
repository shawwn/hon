
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
**		This file provides the public interface to the Steam CFile class.
**      This class is intended as a PARTIAL wrapper for the Steam API provided in
**		Steam.h.  This class provides an interface like that of
**		CFile, but where the underlying file may be Steam-controlled.
**      The class is NOT polymorphic with CFile, and not all of the API
**		features of CFile are currently implemented.
**
**		This file also provides the public interface to the Steam CFileFind class.
**      This class is intended as a PARTIAL wrapper for the Steam API provided in
**		Steam.h.  This class provides an interface like that of
**		CFileFind, but where the underlying files and directories may be Steam-controlled.
**      The class is NOT polymorphic with CFileFind, but all of the API
**		features of CFile are implemented.
**
**		Using the EFindType, you can specify whether to find only Steam files,
**		only "local" files (might as well use CFileFind in that case, however),
**		or both.
**
**		It is intended to help minimize difficulty in porting existing
**      applications to Steam.
**
** Target restrictions:
**
** Tool restrictions:
**
**		Needs afx.h included before this file.
**
******************************************************************************/

#ifndef INCLUDED_STEAMCFILE_H
#define INCLUDED_STEAMCFILE_H

#if defined(_MSC_VER) && (_MSC_VER > 1000)
#pragma once
#endif

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxcview.h>
#include <afxdisp.h>        // MFC Automation classes

#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT


//////////////////////////////////////////////////////////////////////
//
// CSteamFile
//
//////////////////////////////////////////////////////////////////////

// based on CFile interface, but does not inherit
class CSteamFile : public CObject
{//lint !e1932 base not abstract
	DECLARE_DYNAMIC(CSteamFile)
public:
// Constructors
	CSteamFile();
	//CSteamFile(int hFile); not implemented
	CSteamFile(LPCTSTR lpszFileName, UINT nOpenFlags);

// Attributes
	//operator HFILE() const; not implemented

	virtual DWORD GetPosition() const;
	BOOL GetStatus(CFileStatus& rStatus) const;
	virtual CString GetFileName() const;
	virtual CString GetFileTitle() const;
	virtual CString GetFilePath() const;
	//virtual void SetFilePath(LPCTSTR lpszNewName);

// Operations
	virtual BOOL Open(LPCTSTR lpszFileName, UINT nOpenFlags,
		CFileException* pError = NULL);//lint !e1735 default param

	// not sure about these
	//static void PASCAL Rename(LPCTSTR lpszOldName,
	//			LPCTSTR lpszNewName);
	//static void PASCAL Remove(LPCTSTR lpszFileName);
	static BOOL PASCAL GetStatus(LPCTSTR lpszFileName,
				CFileStatus& rStatus);
	//static void PASCAL SetStatus(LPCTSTR lpszFileName,
	//			const CFileStatus& status);

	DWORD SeekToEnd();
	void SeekToBegin();

// Overridables
	virtual CSteamFile* Duplicate() const;

	virtual LONG Seek(LONG lOff, UINT nFrom);
	//virtual void SetLength(DWORD dwNewLen);
	virtual DWORD GetLength() const;

	virtual UINT Read(void* lpBuf, UINT nCount);
	virtual void Write(const void* lpBuf, UINT nCount);

	//virtual void LockRange(DWORD dwPos, DWORD dwCount);
	//virtual void UnlockRange(DWORD dwPos, DWORD dwCount);

	virtual void Abort();
	virtual void Flush();
	virtual void Close();

// Implementation
public:
	virtual ~CSteamFile();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	CSteamFile(const CSteamFile &);
	CSteamFile& operator=(const CSteamFile &);
	struct CImpl;
	CImpl *m_pImpl;
};

//////////////////////////////////////////////////////////////////////
//
// CSteamFileFind
//
//////////////////////////////////////////////////////////////////////

// based on CSteamFileFind interface
class CSteamFileFind : public CObject
{//lint !e1932 base not abstract
	DECLARE_DYNAMIC(CSteamFileFind)
public:
	CSteamFileFind();
	virtual ~CSteamFileFind();

// Attributes
public:

	enum EFindType { eFindLocal, eFindRemote, eFindBoth };

	DWORD GetLength() const;
	__int64 GetLength64() const;
	virtual CString GetFileName() const;
	virtual CString GetFilePath() const;
	virtual CString GetFileTitle() const;
	virtual CString GetFileURL() const;
	virtual CString GetRoot() const;

	virtual BOOL GetLastWriteTime(FILETIME* pTimeStamp) const;
	virtual BOOL GetLastAccessTime(FILETIME* pTimeStamp) const;
	virtual BOOL GetCreationTime(FILETIME* pTimeStamp) const;
	virtual BOOL GetLastWriteTime(CTime& refTime) const;
	virtual BOOL GetLastAccessTime(CTime& refTime) const;
	virtual BOOL GetCreationTime(CTime& refTime) const;

	virtual BOOL MatchesMask(DWORD dwMask) const;

	virtual BOOL IsDots() const;
	// these aren't virtual because they all use MatchesMask(), which is
	BOOL IsReadOnly() const;
	BOOL IsDirectory() const;
	BOOL IsCompressed() const;
	BOOL IsSystem() const;
	BOOL IsHidden() const;
	BOOL IsTemporary() const;
	BOOL IsNormal() const;
	BOOL IsArchived() const;

// Operations
	void Close();

	// DIFFERS from MFC
	virtual BOOL FindFile(LPCTSTR pstrName, EFindType eFindType);

	virtual BOOL FindNextFile();

// Implementation
protected:
	struct CImpl;
	CImpl *m_pImpl;

#ifdef _DEBUG
	void Dump(CDumpContext& dc) const;
	void AssertValid() const;
#endif

	// disallowed
	CSteamFileFind(const CSteamFileFind &);
	CSteamFileFind& operator=(const CSteamFileFind &);
};


#endif

// (C)2010 S2 Games
// k0_system.h
//
//=============================================================================
#ifndef __K0_SYSTEM_H__
#define __K0_SYSTEM_H__

//=============================================================================
// Headers
//=============================================================================
#include "k0_unicode.h"
#include "k0_singleton.h"

#include <stdlib.h>
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
extern class CK0System	&K0System;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
//=============================================================================

//=============================================================================
// CK0System
//=============================================================================
class CK0System
{
SINGLETON_DEF(CK0System)
private:
	tstring				m_sRootDir;

	//-------------------------------------------------------------------------
	// Methods to implement on each platform
	//-------------------------------------------------------------------------
	void				ChDir(const char* pDirUTF8);
	//-------------------------------------------------------------------------

public:
	~CK0System() {}

	const tstring&		GetRootDir() const				{ return m_sRootDir; }

#ifdef _WIN32
	inline void			Init(int argc, _TCHAR* argv[]);
#else
	inline void			Init(int argc, char* argv[]);
#endif

	inline void			Init(const tsvector& vArgv);

	inline void			Exit(int iErrorLevel);
};
//=============================================================================

/*====================
CK0System::Init
====================*/
inline
#ifdef _WIN32
void	CK0System::Init(int argc, _TCHAR* argv[])
#else
void	CK0System::Init(int argc, char* argv[])
#endif
{
	tsvector vArgv;
	for (int i(0); i < argc; ++i)
		vArgv.push_back(StringToTString(argv[i]));

	Init(vArgv);
}


/*====================
  CK0System::Init
  ====================*/
inline
void	CK0System::Init(const tsvector& vArgv)
{
	assert(vArgv.size() >= 1);
	if (vArgv.empty() || vArgv[0].empty())
		Exit(-1);

	tstring sStartDir(vArgv[0]);
	// strip off any quotes.
	sStartDir = StringReplace(sStartDir, _T("\""), _T(""));
	// replace backslashes with forward slashes.
	sStartDir = StringReplace(sStartDir, _T("\\"), _T("/"));
	// strip the exe name.
	sStartDir = sStartDir.substr(0, sStartDir.find_last_of(_T('/')));

	// store the exe dir.
	m_sRootDir = sStartDir;

	// switch the current working directory to the exe dir.
	ChDir(TStringToUTF8(m_sRootDir).c_str());
}


/*====================
  CK0System::Exit
  ====================*/
inline
void	CK0System::Exit(int iErrorLevel)
{
	exit(iErrorLevel);
}

#endif //__K0_SYSTEM_H__

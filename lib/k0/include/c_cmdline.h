// (C)2008 S2 Games
// c_cmdline.h
//
//=============================================================================
#ifndef __C_CMDLINE_H__
#define __C_CMDLINE_H__

//=============================================================================
// Headers
//=============================================================================
#include "k0_string.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
//=============================================================================

//=============================================================================
// CCmdLine
//=============================================================================
class CCmdLine
{
private:
	map<tstring, tstring>	m_mapParams;

public:
	~CCmdLine();
	CCmdLine();
	CCmdLine(const TCHAR *szCmdLine);
	CCmdLine(const tstring &sCmdLine);

	void			ProcessCommandLine(const TCHAR *szCmdLine);

	bool			HasParameter(const tstring &sName);
	const tstring&	GetParameter(const tstring &sName, const tstring &sDefaultValue = TSNULL);
	int				GetParameterInt(const tstring &sName, int iDefaultValue = 0);
};
//=============================================================================

#endif //__C_CMDLINE_H__

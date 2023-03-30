// (C)2008 S2 Games
// c_cmdline.cpp
//
// Command line processing
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k0_common.h"

#include "c_cmdline.h"

#include "k0_string.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
//=============================================================================

/*====================
  CCmdLine::~CCmdLine
  ====================*/
CCmdLine::~CCmdLine()
{
}


/*====================
  CCmdLine::CCmdLine
  ====================*/
CCmdLine::CCmdLine()
{
}

CCmdLine::CCmdLine(const TCHAR *szCmdLine)
{
	ProcessCommandLine(szCmdLine);
}

CCmdLine::CCmdLine(const tstring &sCmdLine)
{
	ProcessCommandLine(sCmdLine.c_str());
}


/*====================
  CCmdLine::ProcessCommandLine
  ====================*/
void	CCmdLine::ProcessCommandLine(const TCHAR *szCmdLine)
{
	tstring sCmdLine(szCmdLine);
	size_t zLength(sCmdLine.length());

	size_t zPos(0);

	while (zPos != zLength && zPos != tstring::npos)
	{
		if (sCmdLine[zPos] != _T('-'))
		{
			++zPos;
			continue;
		}
		else
		{
			size_t zNameEnd(sCmdLine.find_first_of(_T(" '"), zPos + 1));

			tstring sName;
			tstring sValue;

			if (zNameEnd != tstring::npos)
			{
				sName = sCmdLine.substr(zPos + 1, zNameEnd - zPos - 1);
				zPos = zNameEnd;

				if (sCmdLine[zNameEnd + 1] == _T('\''))
				{
					size_t zValueEnd(sCmdLine.find_first_of(_T("'"), zNameEnd + 2));

					if (zValueEnd != tstring::npos)
						sValue = sCmdLine.substr(zNameEnd + 2, zValueEnd - zNameEnd - 2);

					zPos = zValueEnd;
				}
				else
				{
					size_t zValueEnd(sCmdLine.find_first_of(_T(" "), zNameEnd + 1));

					if (zValueEnd != tstring::npos)
						sValue = sCmdLine.substr(zNameEnd + 1, zValueEnd - zNameEnd - 1);
					else
						sValue = sCmdLine.substr(zNameEnd + 1);

					zPos = zValueEnd;
				}
			}

			m_mapParams[sName] = sValue;
		}
	}
}


/*====================
  CCmdLine::HasParameter
  ====================*/
bool	CCmdLine::HasParameter(const tstring &sName)
{
	return m_mapParams.find(sName) != m_mapParams.end();
}


/*====================
  CCmdLine::GetParameter
  ====================*/
const tstring&	CCmdLine::GetParameter(const tstring &sName, const tstring &sDefaultValue)
{
	map<tstring, tstring>::const_iterator findit(m_mapParams.find(sName));

	if (findit == m_mapParams.end())
		return sDefaultValue;
	else
		return findit->second;
}


/*====================
  CCmdLine::GetParameterInt
  ====================*/
int		CCmdLine::GetParameterInt(const tstring &sName, int iDefaultValue)
{
	map<tstring, tstring>::const_iterator findit(m_mapParams.find(sName));

	if (findit == m_mapParams.end())
		return iDefaultValue;
	else
		return AtoI(findit->second);
}
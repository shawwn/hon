// (C)2008 S2 Games
// c_exception.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k0_common.h"

#include "c_exception.h"
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
//=============================================================================

/*====================
  CException::CException
  ====================*/
CException::CException(const string &sMessage, EExceptionType eType) :
m_eType(eType),
m_sMessage(SingleToWide(sMessage))
{
}

CException::CException(const wstring &sMessage, EExceptionType eType) :
m_eType(eType),
m_sMessage(sMessage)
{
}


/*====================
  CException::Process
  ====================*/
void	CException::Process(const wstring &sPrefix, bool bThrow)
{
	if (!bThrow)
	{
		switch (m_eType)
		{
		case E_MESSAGE:
			//wcout << sPrefix << m_sMessage << endl;
			break;

		default:
		case E_ERROR:
			//wcerr << sPrefix << m_sMessage << endl;
			break;
		}
		return;
	}

	m_sMessage = sPrefix + m_sMessage;
	throw *this;
}

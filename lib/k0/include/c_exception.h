// (C)2008 S2 Games
// c_exception.h
//
//=============================================================================
#ifndef __C_EXCEPTION_H__
#define __C_EXCEPTION_H__

//=============================================================================
// Headers
//=============================================================================
#include "k0_string.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
const bool THROW(true);
const bool NO_THROW(false);

enum EExceptionType
{
	E_MESSAGE,
	E_ERROR
};

#define EX_MESSAGE(msg)	throw CException((msg), E_MESSAGE)
#define EX_ERROR(msg)	throw CException((msg), E_ERROR)
//=============================================================================

//=============================================================================
// CException
//=============================================================================
class CException
{
private:
	EExceptionType	m_eType;
	wstring			m_sMessage;

	CException();

public:
	~CException()	{}
	CException(const string &sMessage, EExceptionType eType);
	CException(const wstring &sMessage, EExceptionType eType);

	void			Process(const string &sPrefix, bool bThrow = THROW)		{ Process(SingleToWide(sPrefix), bThrow); }
	void			Process(const wstring &sPrefix, bool bThrow = THROW);

	wstring			GetMsg() const					{ return m_sMessage; }
	EExceptionType	GetType() const					{ return m_eType; }
	void			SetType(EExceptionType eType)	{ m_eType = eType; }
};
//=============================================================================

#endif //__C_EXCEPTION_H__

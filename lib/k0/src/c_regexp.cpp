// (C)2010 S2 Games
// c_regexp.cpp
//
//=============================================================================
#include "k0_common.h"
#include "c_regexp.h"

//=============================================================================
// Headers
//=============================================================================
#include "../ext/creg/creg.h"
//=============================================================================


/*====================
  CRegexp::Init
    ====================*/
void	CRegexp::Init(const char* pExprUTF8)
{
	assert(m_pRegex == NULL);
	if (m_pRegex != NULL)
		return;

  m_pRegex = regcomp((char*)pExprUTF8);
}


/*====================
  CRegexp::~CRegexp
  ====================*/
CRegexp::~CRegexp()
{
}


/*====================
CRegexp::Match
====================*/
bool	CRegexp::Match(const char* pTestUTF8)
{
  int iResult(regexec(m_pRegex, (char*)pTestUTF8));
  
  if (!iResult)
	  return false;
  return true;
}


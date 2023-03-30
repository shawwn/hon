// (C)2010 S2 Games
// c_phpdata.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "chatserver_common.h"

#include "c_phpdata.h"
#include "c_console.h"
//=============================================================================

/*====================
  CPHPData::ReadInteger
  ====================*/
TCHAR*	CPHPData::ReadInteger(TCHAR *p)
{
	if (*p != _T(':') || !*(++p))
		return NULL;
	
	TCHAR *sValue = p;
	
	if (!(p = _tcschr(p, _T(';'))))
		return NULL;
	
	*p = 0;
	
	SetValue(int(_tstoi(sValue)));
	
	return p+1;
}


/*====================
  CPHPData::ReadFloat
  ====================*/
TCHAR*	CPHPData::ReadFloat(TCHAR *p)
{
	if (*p != _T(':') || !*(++p))
		return NULL;
	
	TCHAR *sValue = p;
	
	if (!(p = _tcschr(p, _T(';'))))
		return NULL;
	
	*p = 0;
	
	SetValue(float(_tstof(sValue)));
	
	return p+1;
}


/*====================
  CPHPData::ReadString
  ====================*/
TCHAR*	CPHPData::ReadString(TCHAR *p)
{
	int size;
	if (*p != _T(':') || !*(++p))
		return NULL;
	
	size = _tcstol(p, &p, 0);
	if (*p != _T(':') || *(++p) != _T('"') || !*(++p))
		return NULL;
	
	TCHAR *sValue = p;
	
	while (*p && p < sValue + size) ++p;
	
	if (*p != _T('"') || *(p+1) != _T(';'))
			return NULL;
	
	*p = 0;
	
	SetValue(sValue);
	
	return p+2;
}


/*====================
  CPHPData::ReadBool
  ====================*/
TCHAR*	CPHPData::ReadBool(TCHAR *p)
{
	if (*p != _T(':') || !*(++p))
		return NULL;
	
	TCHAR *sValue = p;
	
	if (!(p = _tcschr(p, _T(';'))))
		return NULL;
	
	*p = 0;
	
	SetValue(AtoB(sValue));
	
	return p+1;
}


/*====================
  CPHPData::ReadArray
  ====================*/
TCHAR*	CPHPData::ReadArray(TCHAR *p)
{
	int i, size;
	if (*p != _T(':') || !*(++p))
		return NULL;
	
	size = _tcstol(p, &p, 0);
	if (*p != _T(':') || *(++p) != _T('{') || !*(++p))
		return NULL;
	
	m_eType = PHP_ARRAY;
	m_vArray.reserve(size);
	
	for (i = 0; i < size; ++i)
	{
		CPHPData phpKey;
		if (!(p = phpKey.Deserialize(p)))
			break;
		if (phpKey.GetType() == PHP_ARRAY)
			break;
		
		CPHPData phpData;
		if (!(p = phpData.Deserialize(p)))
			break;
		
		phpData.m_sKey = phpKey.GetString();
		m_vArray.push_back(phpData);
	}
	
	if (!p || *p != _T('}'))
	{
		return NULL;
	}
	
	return p+1;
}


/*====================
  CPHPData::Deserialize
  ====================*/
TCHAR*	CPHPData::Deserialize(TCHAR *p)
{
	switch (*p)
	{
		case(_T('a')):
			p = ReadArray(p+1);
			break;
		
		case(_T('i')):
			p = ReadInteger(p+1);
			break;
			
		case(_T('d')):
			p = ReadFloat(p+1);
			break;
		
		case(_T('s')):
			p = ReadString(p+1);
			break;
		
		case(_T('b')):
			p = ReadBool(p+1);
			break;
		
		case(_T('N')):
			if (!*(++p) || *p != _T(';'))
				return NULL;
			++p;
			break;
		
		default:
			p = NULL;
			break;
	}
	
	return p;
}


/*====================
  CPHPData::CPHPData
  ====================*/
CPHPData::CPHPData() :
m_eType(PHP_NULL),
m_iValue(0),
m_fValue(0.0f),
m_bValue(false)
{
}

CPHPData::CPHPData(const tstring &sData) :
m_eType(PHP_NULL),
m_iValue(0),
m_fValue(0.0f),
m_bValue(false)
{
	TCHAR *sBuf = new TCHAR[sData.length()+1];
	//_tcsncpy(sBuf, sData.c_str(), sData.length());  // this throws warning in Win32 build - JT
	_TCSNCPY_S(sBuf, sData.length()+1, sData.c_str(), _TRUNCATE);	
	sBuf[sData.length()] = 0;
	
	if (!Deserialize(sBuf))
		m_eType = PHP_INVALID;
	
	delete[] sBuf;
}


/*====================
  CPHPData::Print
  ====================*/
void	CPHPData::Print(class CConsole *pConsole) const
{
	static int s_iIndent(0);
	
	switch (m_eType)
	{
		case PHP_NULL:		pConsole->Admin() << L"NULL"; break;
		case PHP_INTEGER:	pConsole->Admin() << L"INT: " << m_iValue; break;
		case PHP_STRING:	pConsole->Admin() << L"STRING: " << m_sValue; break;
		case PHP_FLOAT:		pConsole->Admin() << L"FLOAT: " << m_fValue; break;
		case PHP_BOOL:		pConsole->Admin() << L"BOOL: " << m_bValue; break;
		case PHP_ARRAY:
			pConsole->Admin() << L"ARRAY:\n";
			++s_iIndent;
			for (vector<CPHPData>::const_iterator cit(m_vArray.begin()); cit != m_vArray.end(); ++cit)
			{
				for (int i(0); i < s_iIndent * 2; ++i)
					pConsole->Admin() << SPACE;
				
				pConsole->Admin() << cit->m_sKey << L", ";
				cit->Print(pConsole);
			}
			--s_iIndent;
			return;
		case PHP_INVALID:
			return;
	}
	pConsole->Admin() << newl;
}

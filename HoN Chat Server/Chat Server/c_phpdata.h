// (C)2010 S2 Games
// c_phpdata.h
//
//=============================================================================
#ifndef __C_PHPDATA_H__
#define __C_PHPDATA_H__

//=============================================================================
// Definitions
//=============================================================================
enum EPHPDataType
{
	PHP_INVALID,
	PHP_NULL,
	PHP_ARRAY,
	PHP_INTEGER,
	PHP_STRING,
	PHP_FLOAT,
	PHP_BOOL
};
//=============================================================================

//=============================================================================
// CPHPData
//=============================================================================
class CPHPData
{
private:
	EPHPDataType		m_eType;
	int                 m_iValue;
	float               m_fValue;
	wstring				m_sValue;
	bool                m_bValue;
	wstring				m_sKey;
	vector<CPHPData>	m_vArray;
	
	wchar_t*	ReadArray(wchar_t *p);
	wchar_t*	ReadInteger(wchar_t *p);
	wchar_t*	ReadFloat(wchar_t *p);
	wchar_t*	ReadString(wchar_t *p);
	wchar_t*	ReadBool(wchar_t *p);
	wchar_t*	Deserialize(wchar_t *p);
	
	void	SetValue(int iValue)			{ m_eType = PHP_INTEGER; m_iValue = iValue; }
	void	SetValue(float fValue)			{ m_eType = PHP_FLOAT; m_fValue = fValue; }
	void	SetValue(const wchar_t *sValue)	{ m_eType = PHP_STRING; m_sValue = sValue; }
	void	SetValue(bool bValue)			{ m_eType = PHP_BOOL; m_bValue = bValue; }

public:
	~CPHPData() {}
	CPHPData();
	CPHPData(const wstring &sData);

	bool				IsValid() const						{ return m_eType != PHP_INVALID; }

	EPHPDataType		GetType() const						{ return m_eType; }
	EPHPDataType		GetType(const wstring &sKey) const
	{
		for (vector<CPHPData>::const_iterator cit(m_vArray.begin()); cit != m_vArray.end(); ++cit)
		{
			if (sKey == cit->m_sKey)
			{
				return cit->GetType();
			}
		}
		
		return PHP_NULL;
	}

	wstring	GetString(const wstring &sKey, const wstring &sDefault = TSNULL) const
	{
		const CPHPData *pNode(GetVar(sKey));
		if (pNode == NULL)
			return sDefault;
		return pNode->GetString();
	}

	wstring	GetString() const
	{
		switch (m_eType)
		{
		case PHP_INTEGER:	return XtoW(m_iValue);
		case PHP_FLOAT:		return XtoW(m_fValue);
		case PHP_BOOL:		return XtoW(m_bValue);
		case PHP_STRING:	return m_sValue;
		}

		return TSNULL;
	}

	int		GetInteger(const wstring &sKey, int iDefault = 0) const
	{
		const CPHPData *pNode(GetVar(sKey));
		if (pNode == NULL)
			return iDefault;
		return pNode->GetInteger();
	}

	int		GetInteger() const
	{
		switch (m_eType)
		{
		case PHP_INTEGER:	return m_iValue;
		case PHP_FLOAT:		return INT_ROUND(m_fValue);
		case PHP_BOOL:		return m_bValue ? 1 : 0;
		case PHP_STRING:	return AtoI(m_sValue);
		}

		return 0;
	}

	float	GetFloat(const wstring &sKey, float fDefault = 0.0f) const
	{
		const CPHPData *pNode(GetVar(sKey));
		if (pNode == NULL)
			return fDefault;
		return pNode->GetFloat();
	}

	float	GetFloat() const
	{
		switch (m_eType)
		{
		case PHP_INTEGER:	return float(m_iValue);
		case PHP_FLOAT:		return m_fValue;
		case PHP_BOOL:		return m_bValue ? 1.0f : 0.0f;
		case PHP_STRING:	return AtoF(m_sValue);
		}

		return 0.0f;
	}

	bool	GetBool(const wstring &sKey, bool bDefault = false) const
	{
		const CPHPData *pNode(GetVar(sKey));
		if (pNode == NULL)
			return bDefault;
		return pNode->GetBool();
	}

	bool	GetBool() const
	{
		switch (m_eType)
		{
		case PHP_INTEGER:	return m_iValue != 0;
		case PHP_FLOAT:		return m_fValue != 0.0f;
		case PHP_BOOL:		return m_bValue;
		case PHP_STRING:	return AtoB(m_sValue);
		}

		return false;
	}

	const CPHPData*		GetVar(const wstring &sKey) const
	{
		for (vector<CPHPData>::const_iterator cit(m_vArray.begin()); cit != m_vArray.end(); ++cit)
		{
			if (sKey == cit->m_sKey)
			{
				return &(*cit);
			}
		}
		
		return NULL;
	}

	bool				HasVar(const wstring &sKey) const	{ return GetVar(sKey) != NULL; }

	size_t				GetSize() const						{ if (m_eType == PHP_ARRAY) return m_vArray.size(); if (m_eType == PHP_NULL) return 0; return 1; }
	const CPHPData*		GetVar(size_t zOffset) const		{ if (m_eType != PHP_ARRAY || zOffset >= GetSize()) return NULL; return &(m_vArray[zOffset]); }
	const wstring&		GetKeyName(size_t zOffset) const	{ if (m_eType != PHP_ARRAY || zOffset >= GetSize()) return TSNULL; return m_vArray[zOffset].m_sKey; }
	
	void				Print(class CConsole *pConsole) const;
};
//=============================================================================

#endif //__C_PHPDATA_H__

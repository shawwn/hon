// (C)2008 S2 Games
// c_xmlnode.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k0_common.h"

#include "c_xmlnode.h"
//=============================================================================

/*====================
  CXMLNode::CXMLNode
  ====================*/
CXMLNode::CXMLNode()
{
}


/*====================
  CXMLNode::~CXMLNode
  ====================*/
CXMLNode::~CXMLNode()
{
}


/*====================
  CXMLNode::~CXMLNode
  ====================*/
void CXMLNode::Clear()
{
	m_sName.clear();
	m_mapProperties.clear();
	m_lChildren.clear();
}


/*====================
  CXMLNode::GetProperty
  ====================*/
const tstring&	CXMLNode::GetProperty(const tstring &sName, const tstring &sDefaultValue) const
{
	PropertyMap::const_iterator findit = m_mapProperties.find(sName);

	if (findit == m_mapProperties.end())
		return sDefaultValue;
	else
		return findit->second;
}


/*====================
  CXMLNode::GetPropertyInt
  ====================*/
int		CXMLNode::GetPropertyInt(const tstring &sName, int iDefaultValue) const
{
	PropertyMap::const_iterator findit = m_mapProperties.find(sName);

	if (findit == m_mapProperties.end())
		return iDefaultValue;
	else
		return AtoI(findit->second);
}


/*====================
  CXMLNode::GetPropertyFloat
  ====================*/
float	CXMLNode::GetPropertyFloat(const tstring &sName, float fDefaultValue) const
{
	PropertyMap::const_iterator findit = m_mapProperties.find(sName);

	if (findit == m_mapProperties.end())
		return fDefaultValue;
	else
		return AtoF(findit->second);
}


/*====================
  CXMLNode::GetPropertyBool
  ====================*/
bool	CXMLNode::GetPropertyBool(const tstring &sName, bool bDefaultValue) const
{
	PropertyMap::const_iterator findit = m_mapProperties.find(sName);

	if (findit == m_mapProperties.end())
		return bDefaultValue;
	else
		return AtoB(findit->second);
}


/*====================
  CXMLNode::GetPropertyV2
  ====================*/
CVec2f	CXMLNode::GetPropertyV2(const tstring &sName, const CVec2f &v2Default) const
{
	PropertyMap::const_iterator itFind(m_mapProperties.find(sName));
	if (itFind == m_mapProperties.end())
		return v2Default;
	else
		return AtoV2(itFind->second);
}


/*====================
  CXMLNode::GetPropertyV3
  ====================*/
CVec3f	CXMLNode::GetPropertyV3(const tstring &sName, const CVec3f &v3Default) const
{
	PropertyMap::const_iterator itFind(m_mapProperties.find(sName));
	if (itFind == m_mapProperties.end())
		return v3Default;
	else
		return AtoV3(itFind->second);
}


/*====================
  CXMLNode::GetPropertyV4
  ====================*/
CVec4f	CXMLNode::GetPropertyV4(const tstring &sName, const CVec4f &v4Default) const
{
	PropertyMap::const_iterator itFind(m_mapProperties.find(sName));
	if (itFind == m_mapProperties.end())
		return v4Default;
	else
		return AtoV4(itFind->second);
}


/*====================
  CXMLNode::SetProperty
  ====================*/
void	CXMLNode::SetProperty(const tstring &sName, const tstring &sValue)
{
	m_mapProperties[sName] = sValue;
}


/*====================
  CXMLNode::IsText
  ====================*/
bool	CXMLNode::IsText()
{
	PropertyMap::iterator find = m_mapProperties.find(_T("type"));

	if (find != m_mapProperties.end())
		if (find->second == _T("text"))
			return true;

	return false;
}


/*====================
  CXMLNodeWrite::CXMLNodeWrite
  ====================*/
CXMLNodeWrite::CXMLNodeWrite()
{
}


/*====================
  CXMLNodeWrite::~CXMLNodeWrite
  ====================*/
CXMLNodeWrite::~CXMLNodeWrite()
{
}


/*====================
  CXMLNodeWrite::GetProperty
  ====================*/
const tstring&	CXMLNodeWrite::GetProperty(const tstring &sName, const tstring &sDefaultValue) const
{
	PropertyMap::const_iterator findit = m_mapProperties.find(sName);

	if (findit == m_mapProperties.end())
		return sDefaultValue;
	else
		return findit->second;
}


/*====================
  CXMLNodeWrite::SetProperty
  ====================*/
void	CXMLNodeWrite::SetProperty(const tstring &sName, const tstring &sValue)
{
	PropertyMap::iterator findit(m_mapProperties.find(sName));

	if (findit == m_mapProperties.end())
	{
		m_lProperties.push_back(pair<tstring, tstring>(sName, sValue));
	}
	else
	{
		for (PropertyList::iterator it(m_lProperties.begin()); it != m_lProperties.end(); ++it)
		{
			if (it->first == sName)
			{
				it->second = sValue;
				break;
			}
		}
	}

	m_mapProperties[sName] = sValue;
}


/*====================
  CXMLNodeWrite::RemoveProperty
  ====================*/
void	CXMLNodeWrite::RemoveProperty(const tstring &sName)
{
	PropertyMap::iterator itFind(m_mapProperties.find(sName));

	if (itFind == m_mapProperties.end())
		return;

	for (PropertyList::iterator it(m_lProperties.begin()); it != m_lProperties.end(); ++it)
	{
		if (it->first == sName)
		{
			m_lProperties.erase(it);
			break;
		}
	}

	m_mapProperties.erase(itFind);
}


/*====================
  CXMLNodeWrite::IsText
  ====================*/
bool	CXMLNodeWrite::IsText()
{
	PropertyMap::iterator find = m_mapProperties.find(_T("type"));

	if (find != m_mapProperties.end())
		if (find->second == _T("text"))
			return true;

	return false;
}

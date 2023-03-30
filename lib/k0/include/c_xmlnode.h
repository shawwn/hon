// (C)2008 S2 Games
// c_xmlnode.h
//
//=============================================================================
#ifndef __C_XMLNODE_H__
#define __C_XMLNODE_H__

//=============================================================================
// Declarations
//=============================================================================
class CXMLNode;
class CXMLNodeWrite;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
typedef list<CXMLNode>					XMLNodeList;
typedef XMLNodeList::iterator			XMLNodeList_it;
typedef XMLNodeList::const_iterator		XMLNodeList_cit;

typedef list< pair<tstring, tstring> >	PropertyList;
typedef list<CXMLNodeWrite>				XMLNodeWriteList;
//=============================================================================

//=============================================================================
// CXMLNode
//=============================================================================
class CXMLNode
{
private:
	tstring				m_sName;
	PropertyMap			m_mapProperties;
	XMLNodeList			m_lChildren;
	tstring				m_sContents;

public:
	CXMLNode();
	~CXMLNode();

	void Clear();

	const tstring&			GetName() const					{ return m_sName; }
	void					SetName(const tstring &sName)	{ m_sName = sName; }
	void					SetName(const TCHAR *szName)	{ m_sName = szName; }

	inline bool				HasProperty(const tstring &sName) const;

	const tstring&			GetProperty(const tstring &sName, const tstring &sDefaultValue = TSNULL) const;
	int						GetPropertyInt(const tstring &sName, int iDefaultValue = 0) const;
	float					GetPropertyFloat(const tstring &sName, float fDefaultValue = 0.0f) const;
	bool					GetPropertyBool(const tstring &sName, bool bDefaultValue = false) const;
	CVec2f					GetPropertyV2(const tstring &sName, const CVec2f &v2Default = V2_ZERO) const;
	CVec3f					GetPropertyV3(const tstring &sName, const CVec3f &v3Default = V_ZERO) const;
	CVec4f					GetPropertyV4(const tstring &sName, const CVec4f &v4Default = V4_ZERO) const;
	
	void					SetProperty(const tstring &sName, const tstring &sValue);

	const tstring&			GetContents() const						{ return m_sContents; }
	void					SetContents(const tstring &sContents)	{ m_sContents = sContents; }

	inline bool				IsText();

	const XMLNodeList&		GetChildren() const				{ return m_lChildren; }
	XMLNodeList&			GetChildren()					{ return m_lChildren; }
	void					AddChild(CXMLNode &child)		{ m_lChildren.push_back(child); }
	CXMLNode&				PushChild()						{ m_lChildren.push_back(CXMLNode()); return m_lChildren.back(); }
	const PropertyMap&		GetPropertyMap() const			{ return m_mapProperties; }
};

/*====================
  CXMLNode::HasProperty
  ====================*/
inline
bool	CXMLNode::HasProperty(const tstring &sName) const
{
	return m_mapProperties.find(sName) != m_mapProperties.end();
}
//=============================================================================

//=============================================================================
// CXMLNodeWrite
//=============================================================================
class CXMLNodeWrite
{
private:
	tstring				m_sName;
	PropertyMap			m_mapProperties;
	PropertyList		m_lProperties;
	XMLNodeWriteList	m_lChildren;
	tstring				m_sContents;

public:
	CXMLNodeWrite();
	~CXMLNodeWrite();

	void Clear();

	const tstring&			GetName() const					{ return m_sName; }
	void					SetName(const tstring &sName)	{ m_sName = sName; }
	void					SetName(const TCHAR *szName)	{ m_sName = szName; }
	
	const tstring&			GetProperty(const tstring &sName, const tstring &sDefaultValue = TSNULL) const;
	void					SetProperty(const tstring &sName, const tstring &sValue);
	void					RemoveProperty(const tstring &sName);

	inline bool				HasProperty(const tstring &sName) const;

	const tstring&			GetContents() const						{ return m_sContents; }
	void					SetContents(const tstring &sContents)	{ m_sContents = sContents; }

	inline bool				IsText();

	const XMLNodeWriteList&	GetChildren() const				{ return m_lChildren; }
	XMLNodeWriteList&		GetChildren()					{ return m_lChildren; }
	void					AddChild(CXMLNodeWrite &child)	{ m_lChildren.push_back(child); }
	CXMLNodeWrite&			PushChild()						{ m_lChildren.push_back(CXMLNodeWrite()); return m_lChildren.back(); }
	const PropertyList&		GetPropertyList() const			{ return m_lProperties; }
};

/*====================
  CXMLNodeWrite::HasProperty
  ====================*/
inline
bool	CXMLNodeWrite::HasProperty(const tstring &sName) const
{
	return m_mapProperties.find(sName) != m_mapProperties.end();
}
//=============================================================================

#endif //__C_XMLNODE_H__

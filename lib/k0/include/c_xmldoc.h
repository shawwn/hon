// (C)2008 S2 Games
// c_xmldoc.h
//
//=============================================================================
#ifndef __C_XMLDOC_H__
#define __C_XMLDOC_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_buffer.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CXMLNode;
class CXMLNodeWrite;
class CFileHandle;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
enum EXMLEncoding
{
	XML_ENCODE_UTF8,
	XML_ENCODE_UTF16
};
//=============================================================================

//=============================================================================
// CXMLDoc
// Provides an interface for constructing an XML document and then retrieving
// it as a buffer
//=============================================================================
class CXMLDoc
{
private:
	void			*m_pDoc;
	void			*m_pRoot;

	EXMLEncoding	m_eEncoding;

	stack<void*>	m_nodeStack;

	CBufferDynamic	m_buffer;

	void		ConvertNodes(void *node, CXMLNodeWrite &c_node);

	static int	WriteBuffer(void *context, const char *buffer, int len);
	void		WriteNode(CFileHandle &hFile, const CXMLNodeWrite &node, int iIndent);
	void		WriteNodeLong(CFileHandle &hFile, const CXMLNodeWrite &node, int iIndent);

public:
	~CXMLDoc();
	CXMLDoc(EXMLEncoding eEncoding = XML_ENCODE_UTF8);

	void	Clear();

	bool	NewNode(const tstring &sName, const tstring &sContent = _T(""));
	bool	EndNode();

	IBuffer*	GetBuffer();

	void	WriteFile(const tstring &sPath, bool bLong);
	void	WriteFile(CFileHandle &hFile, bool bLong = false);
	void	WriteNodes(const tstring &sPath, CXMLNodeWrite &cRoot, bool bLong);

	bool	AddProperty(const tstring &sName, const tstring &sValue);
	template<class T> bool	AddProperty(const tstring &sName, T value)
	{
		return AddProperty(sName, XtoA(value));
	}

	bool	AddProperty(const tstring &sName, const TCHAR *szValue)
	{
		return AddProperty(sName, tstring(szValue));
	}

	bool	ReadBuffer(const char *pBuffer, int iSize);

	void	TranslateNodes(CXMLNodeWrite &cRoot);
};
//=============================================================================

#endif //__C_XMLDOC_H__

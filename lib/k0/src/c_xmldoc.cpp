// (C)2008 S2 Games
// c_xmldoc.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k0_common.h"

#include <libxml/tree.h>
#include <libxml/globals.h>
#include <libxml/parser.h>

#include "c_xmldoc.h"
#include "c_buffer.h"
#include "c_xmlnode.h"
#include "c_filehandle.h"
//=============================================================================

/*====================
  CXMLDoc::CXMLDoc
  ====================*/
CXMLDoc::CXMLDoc(EXMLEncoding eEncoding) :
m_pDoc(NULL),
m_pRoot(NULL),
m_eEncoding(eEncoding)
{
}


/*====================
  CXMLDoc::~CXMLDoc
  ====================*/
CXMLDoc::~CXMLDoc()
{
	if (m_pDoc)
		xmlFreeDoc(static_cast<xmlDocPtr>(m_pDoc));
}


/*====================
  CXMLDoc::Clear
  ====================*/
void	CXMLDoc::Clear()
{
	if (m_pDoc)
		xmlFreeDoc(static_cast<xmlDocPtr>(m_pDoc));
	m_pRoot = NULL;

	while (!m_nodeStack.empty())
		m_nodeStack.pop();
	m_nodeStack.push(NULL);
}


/*====================
  CXMLDoc::NewNode
  ====================*/
bool	CXMLDoc::NewNode(const tstring &sName, const tstring &sContent)
{
	if (!m_pDoc)
		m_pDoc = xmlNewDoc((xmlChar*)"1.0");

	if (!m_pRoot)
	{
		m_pRoot = xmlNewNode(NULL, (xmlChar*)sName.c_str());
		if (!m_pRoot)
			return false;
		xmlDocSetRootElement(static_cast<xmlDocPtr>(m_pDoc), static_cast<xmlNodePtr>(m_pRoot));
		m_nodeStack.push(m_pRoot);
	}
	else
	{
		xmlNode *newNode = xmlNewChild(static_cast<xmlNodePtr>(m_nodeStack.top()), NULL, (xmlChar*)sName.c_str(), (xmlChar*)sContent.c_str());
		if (!newNode)
			return false;
		m_nodeStack.push(newNode);
	}

	return true;
}


/*====================
  CXMLDoc::AddProperty
  ====================*/
bool	CXMLDoc::AddProperty(const tstring &sName, const tstring &sValue)
{
	xmlNewProp(static_cast<xmlNodePtr>(m_nodeStack.top()), (xmlChar*)sName.c_str(), (xmlChar*)sValue.c_str());
	return true;
}


/*====================
  CXMLDoc::EndNode
  ====================*/
bool	CXMLDoc::EndNode()
{
	if (m_nodeStack.top() == NULL)
		return false;

	m_nodeStack.pop();
	return true;
}


/*====================
  CXMLDoc::ConvertNodes
  ====================*/
void	CXMLDoc::ConvertNodes(void *node, CXMLNodeWrite &c_node)
{
	xmlNode *xmlnode(static_cast<xmlNode *>(node));

	if (xmlnode->type == XML_ELEMENT_NODE)
	{
		// Set element name
		tstring sName;
		StrToTString(sName, (const char*)xmlnode->name);
		c_node.SetName(sName);

		// Set element properties
		xmlAttrPtr prop = xmlnode->properties;

		while (prop != NULL)
		{
			xmlChar *ret(xmlNodeListGetString(xmlnode->doc, prop->children, 1));

			if (ret != NULL)
			{
				tstring sPropertyName;
				StrToTString(sPropertyName, (const char*)prop->name);
				tstring sValue;
				StrToTString(sValue, (const char*)ret);

				c_node.SetProperty(sPropertyName, sValue);

				xmlFree(ret);
			}
			prop = prop->next;
		}

		if (xmlNodeIsText(xmlnode->children))
		{
			xmlChar *szText(xmlNodeGetContent(xmlnode->children));
			tstring sContents;
			StrToTString(sContents, (const char*)szText);
			StripNewline(sContents);

			// Get contents of container if the contents aren't indentation characters
			if (!IsAllWhiteSpace(sContents))
				c_node.SetContents(sContents);
		}
	}

	for (xmlNode *cur_node(xmlnode->children); cur_node; cur_node = cur_node->next)
	{
		if (cur_node->type == XML_ELEMENT_NODE)
		{
			CXMLNodeWrite &new_node(c_node.PushChild());

			ConvertNodes(cur_node, new_node);
		}
	}

#if 0
	if (c_node.GetName() == _T("phase"))
	{
		c_node.SetProperty(_T("alphawrite"), _T("false"));
	}
	else if (c_node.GetName() == _T("sampler"))
	{
		c_node.RemoveProperty(_T("scale_u"));
		c_node.RemoveProperty(_T("scale_v"));
		c_node.RemoveProperty(_T("fps"));
		c_node.RemoveProperty(_T("border"));
	}
#endif
}


/*====================
  CXMLDoc::WriteBuffer
  ====================*/
int		CXMLDoc::WriteBuffer(void *context, const char *buffer, int len)
{
	IBuffer *buf = (IBuffer*)context;

	buf->Append(buffer, len);
	return len;
}


/*====================
  CXMLDoc::GetBuffer
  ====================*/
IBuffer*	CXMLDoc::GetBuffer()
{
	m_buffer.Clear();

	xmlOutputBufferPtr output = xmlOutputBufferCreateIO(CXMLDoc::WriteBuffer, NULL, &m_buffer, NULL);
	xmlSaveFormatFileTo(output, static_cast<xmlDocPtr>(m_pDoc), "UTF-8", 1);

	return &m_buffer;
}


/*====================
  CXMLDoc::WriteNode
  ====================*/
void	CXMLDoc::WriteNode(CFileHandle &hFile, const CXMLNodeWrite &node, int iIndent)
{
	const XMLNodeWriteList &lChildren(node.GetChildren());

	// Indent
	for (int i(0); i < iIndent; ++i) hFile << "\t";

	hFile << "<" << node.GetName();

	const PropertyList &lProperties(node.GetPropertyList());

	for (PropertyList::const_iterator it(lProperties.begin()); it != lProperties.end(); ++it)
		hFile << " " << it->first << "=" << QuoteStr(it->second);

	if (lChildren.empty())
	{
		hFile << " />" << newl;
	}
	else
	{
		hFile << ">" << newl;

		for (XMLNodeWriteList::const_iterator itChild(lChildren.begin()); itChild != lChildren.end(); ++itChild)
			WriteNode(hFile, *itChild, iIndent + 1);

		// Indent
		for (int i(0); i < iIndent; ++i) hFile << "\t";

		hFile << "</" << node.GetName() << ">" << newl;
	}

	hFile << node.GetContents();
}


/*====================
  CXMLDoc::WriteNodeLong
  ====================*/
void	CXMLDoc::WriteNodeLong(CFileHandle &hFile, const CXMLNodeWrite &node, int iIndent)
{
	const XMLNodeWriteList &lChildren(node.GetChildren());

	// Indent
	for (int i(0); i < iIndent; ++i) hFile << "\t";

	hFile << "<" << node.GetName();

	const PropertyList &lProperties(node.GetPropertyList());

	if (lProperties.size() > 0)
	{
		hFile << newl;

		for (PropertyList::const_iterator it(lProperties.begin()); it != lProperties.end(); ++it)
		{
			// Indent
			for (int i(0); i < iIndent; ++i) hFile << "\t";
			hFile << "\t";

			hFile << it->first << "=" << QuoteStr(it->second) << newl;
		}

		// Indent
		for (int i(0); i < iIndent; ++i) hFile << "\t";

		if (lChildren.empty())
		{
			hFile << "/>" << newl;
		}
		else
		{
			hFile << ">" << newl;

			for (XMLNodeWriteList::const_iterator itChild(lChildren.begin()); itChild != lChildren.end(); ++itChild)
				WriteNodeLong(hFile, *itChild, iIndent + 1);

			// Indent
			for (int i(0); i < iIndent; ++i) hFile << "\t";

			hFile << "</" << node.GetName() << ">" << newl;
		}
	}
	else
	{
		if (lChildren.empty())
		{
			hFile << " />" << newl;
		}
		else
		{
			hFile << ">" << newl;

			for (XMLNodeWriteList::const_iterator itChild(lChildren.begin()); itChild != lChildren.end(); ++itChild)
				WriteNodeLong(hFile, *itChild, iIndent + 1);

			// Indent
			for (int i(0); i < iIndent; ++i) hFile << "\t";

			hFile << "</" << node.GetName() << ">" << newl;
		}
	}

	hFile << node.GetContents();
}


/*====================
  CXMLDoc::WriteFile
  ====================*/
void	CXMLDoc::WriteFile(const tstring &sPath, bool bLong)
{
	int iMode(FILE_WRITE | FILE_TEXT);
	if (m_eEncoding == XML_ENCODE_UTF8)
		iMode |= FILE_UTF8;
	else if (m_eEncoding == XML_ENCODE_UTF16)
		iMode |= FILE_UTF16;

	CFileHandle	hFile(sPath, iMode);
	if (!hFile.IsOpen())
		return;

	if (m_eEncoding == XML_ENCODE_UTF8)
		hFile << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << newl;
	else if (m_eEncoding == XML_ENCODE_UTF16)
		hFile << "<?xml version=\"1.0\" encoding=\"UTF-16\"?>" << newl;
	
	CXMLNodeWrite nodes;
	ConvertNodes(xmlDocGetRootElement(static_cast<xmlDocPtr>(m_pDoc)), nodes);

	if (bLong)
		WriteNodeLong(hFile, nodes, 0);
	else
		WriteNode(hFile, nodes, 0);
}


/*====================
  CXMLDoc::WriteNodes
  ====================*/
void	CXMLDoc::WriteNodes(const tstring &sPath, CXMLNodeWrite &cRoot, bool bLong)
{
	int iMode(FILE_WRITE | FILE_TEXT);
	if (m_eEncoding == XML_ENCODE_UTF8)
		iMode |= FILE_UTF8;
	else if (m_eEncoding == XML_ENCODE_UTF16)
		iMode |= FILE_UTF16;

	CFileHandle	hFile(sPath, iMode);
	if (!hFile.IsOpen())
		return;

	if (m_eEncoding == XML_ENCODE_UTF8)
		hFile << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << newl;
	else if (m_eEncoding == XML_ENCODE_UTF16)
		hFile << "<?xml version=\"1.0\" encoding=\"UTF-16\"?>" << newl;

	if (bLong)
		WriteNodeLong(hFile, cRoot, 0);
	else
		WriteNode(hFile, cRoot, 0);
}


/*====================
  CXMLDoc::WriteFile
  ====================*/
void	CXMLDoc::WriteFile(CFileHandle &hFile, bool bLong)
{
	hFile << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << newl;

	CXMLNodeWrite nodes;
	ConvertNodes(xmlDocGetRootElement(static_cast<xmlDocPtr>(m_pDoc)), nodes);

	if (bLong)
		WriteNodeLong(hFile, nodes, 0);
	else
		WriteNode(hFile, nodes, 0);
}


/*====================
  CXMLDoc::ReadBuffer
  ====================*/
bool	CXMLDoc::ReadBuffer(const char *pBuffer, int iSize)
{
	try
	{
		if (m_pDoc)
		{
			xmlFreeDoc(static_cast<xmlDocPtr>(m_pDoc));
			m_pDoc = NULL;
		}

		if (pBuffer == NULL)
			EX_ERROR(_T("NULL buffer"));

		// Parse XML File
		m_pDoc = xmlParseMemory(pBuffer, iSize);

		if (m_pDoc == NULL)
		{
			xmlError *pError(xmlGetLastError());
			if (pError == NULL)
				EX_ERROR(_T("Unknown error in xmlParseMemory"));

			tstring sError;
			StrToTString(sError, pError->message);
			EX_ERROR(sError);
		}
		return true;
	}
	catch (CException &ex)
	{
		if (m_pDoc != NULL)
		{
			xmlFreeDoc(static_cast<xmlDocPtr>(m_pDoc));
			m_pDoc = NULL;
		}

		ex.Process(_T("CXMLDoc::ReadBuffer() - "), NO_THROW);
		return false;
	}
}


/*====================
  CXMLDoc::TranslateNodes
  ====================*/
void	CXMLDoc::TranslateNodes(CXMLNodeWrite &cRoot)
{
	ConvertNodes(xmlDocGetRootElement(static_cast<xmlDocPtr>(m_pDoc)), cRoot);
}


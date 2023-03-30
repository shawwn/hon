// c_outmdf.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2model_common.h"

#include "c_outmdf.h"

#include "c_xmldoc.h"
#include "c_filehandle.h"
//=============================================================================

/*====================
  COutMDF::COutMDF
  ====================*/
COutMDF::COutMDF() :
m_bIsValid(false)
{
	Validate();
}


/*====================
  COutMDF::WriteFile
  ====================*/
tstring	COutMDF::WriteFile(const tstring &sFilename)
{
	CXMLDoc	xmlMDF;

	xmlMDF.NewNode("model");
		xmlMDF.AddProperty("name", m_sName);
		xmlMDF.AddProperty("file", m_sModelFile);
		xmlMDF.AddProperty("type", "K2");
	xmlMDF.EndNode();

	CFileHandle	hFile(sFilename, FILE_WRITE | FILE_TEXT);
	xmlMDF.WriteFile(hFile);

	return "Wrote file " + Filename_StripPath(Filename_SanitizePath(sFilename)) + "\r\n";
}

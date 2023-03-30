// (C)2006 S2 Games
// c_outmdf.h
//
//=============================================================================
#ifndef __C_OUTMDF_H__
#define __C_OUTMDF_H__

//=============================================================================
// Declarations / Definitions
//=============================================================================
class INode;
class Mtl;
class CXMLDoc;
//=============================================================================

//=============================================================================
// COutMDF
//=============================================================================
class COutMDF
{
private:
	bool		m_bIsValid;
	tstring		m_sStatus;

	tstring		m_sName;
	tstring		m_sModelFile;

public:
	~COutMDF()						{}
	COutMDF();
	COutMDF(INode *pNode, TimeValue time);

	void	Validate()			{ m_bIsValid = true; }
	void	Invalidate()		{ m_bIsValid = false; }
	bool	IsValid() const		{ return m_bIsValid; }

	void	SetName(const tstring &sName)				{ m_sName = sName; }
	void	SetModelFile(const tstring &sModelFile)		{ m_sModelFile = sModelFile; }

	tstring	WriteFile(const tstring &sFilename);
};
//=============================================================================
#endif //__C_OUTMDF_H__

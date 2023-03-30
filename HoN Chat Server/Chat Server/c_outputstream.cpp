// (C)2008 S2 Games
// c_outputstream.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "chatserver_common.h"

#include "c_outputstream.h"
#include "c_system.h"
//=============================================================================

/*====================
  COutputStream::~COutputStream
  ====================*/
COutputStream::~COutputStream()
{
	m_hLogFile.Close();
}


/*====================
  COutputStream::COutputStream
  ====================*/
COutputStream::COutputStream(CSystem *pSystem, uint uiTargets, const wstring &sFileName) :
m_pSystem(pSystem),
m_uiTargets(uiTargets)
{
	assert(pSystem != NULL);

	SetLogFile(sFileName);
}


/*====================
  COutputStream::SetLogFile
  ====================*/
void	COutputStream::SetLogFile(const wstring &sFileName)
{
	CloseLogFile();

	if (sFileName.empty())
		return;
	
	m_hLogFile.Open(sFileName, FILE_APPEND | FILE_WRITE | FILE_TEXT | FILE_NOBUFFER);
	if (m_hLogFile.IsOpen())
		m_uiTargets |= STREAM_TARGET_FILE;
}


/*====================
  COutputStream::CloseLogFile
  ====================*/
void	COutputStream::CloseLogFile()
{
	m_hLogFile.Close();
	m_uiTargets &= ~STREAM_TARGET_FILE;
}


/*====================
  COutputStream::AddOutputHistory
  ====================*/
void	COutputStream::AddOutputHistory(const wstring &sLine)
{
	if (m_uiTargets & STREAM_TARGET_DEBUG)
		m_pSystem->PrintDebugMessage(sLine);
	if (m_uiTargets & STREAM_TARGET_CONSOLE)
		m_pSystem->WriteToConsole(sLine);
	if (m_uiTargets & STREAM_TARGET_FILE)
		m_hLogFile << sLine;
}

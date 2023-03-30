// (C)2008 S2 Games
// c_filehandle.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k0_common.h"

#include "c_filehandle.h"

#include "c_filedisk.h"
#include "c_exception.h"
//=============================================================================

/*====================
  CFileHandle::CFileHandle
  ====================*/
CFileHandle::CFileHandle() :
m_pFile(NULL)
{
}


/*====================
  CFileHandle::CFileHandle
  ====================*/
CFileHandle::CFileHandle(const string &sPath, int iMode) :
m_pFile(NULL)
{
	Open(sPath, iMode);
}

CFileHandle::CFileHandle(const wstring &sPath, int iMode) :
m_pFile(NULL)
{
	Open(sPath, iMode);
}


/*====================
  CFileHandle::Open

  Interprets sPath to determine what type of file to open, then allocates
  a CFileHandle* child class of the apropriate type and attempts to open it
  ====================*/
bool	CFileHandle::Open(const wstring &sFilePath, int iMode)
{
	if (IsOpen())
		return false;

	if (iMode & FILE_TEXT &&
		(iMode & FILE_TEXT_ENCODING_MASK) != FILE_ASCII &&
		(iMode & FILE_TEXT_ENCODING_MASK) != FILE_UTF8 &&
		(iMode & FILE_TEXT_ENCODING_MASK) != FILE_UTF16 &&
		(iMode & FILE_TEXT_ENCODING_MASK) != FILE_UTF32 &&
		(iMode & FILE_TEXT_ENCODING_MASK) != 0)
		return false;

	// Check for conflicting mode flags
	if (((iMode & FILE_READ) && (iMode & FILE_WRITE)) ||
		((iMode & FILE_TEXT) &&  (iMode & FILE_BINARY)) ||
		((iMode & FILE_APPEND) && (iMode & FILE_TRUNCATE)) ||
		((iMode & FILE_BUFFER) && (iMode & FILE_NOBUFFER)) ||
		((iMode & FILE_COMPRESS) && (iMode & FILE_NOCOMPRESS)) ||
		((iMode & FILE_BLOCK) && (iMode & FILE_NOBLOCK)))
		return false;

	m_pFile = GetFile(Filename_SanitizePath(sFilePath), iMode);
	return m_pFile != NULL;
}


/*====================
  CFileHandle::Close
  ====================*/
void	CFileHandle::Close()
{
	if (m_pFile != NULL)
		m_pFile->Close();
	
	delete m_pFile;
	m_pFile = NULL;
}


/*====================
  CFileHandle::WriteString
  ====================*/
bool	CFileHandle::WriteString(const string &sText)
{
	if (m_pFile)
		return m_pFile->WriteString(sText);
	else
		return false;
}

bool	CFileHandle::WriteString(const wstring &sText)
{
	if (m_pFile)
		return m_pFile->WriteString(sText);
	else
		return false;
}


/*====================
  CFileHandle::Read
  ====================*/
int	CFileHandle::Read(char *pBuffer, int iSize) const
{
	if (m_pFile)
		return m_pFile->Read(pBuffer, iSize);
	else
		return 0;
}


/*====================
  CFileHandle::Write
  ====================*/
size_t	CFileHandle::Write(const void *pBuffer, size_t size)
{
	if (m_pFile)
		return m_pFile->Write(pBuffer, size);
	else
		return 0;
}


/*====================
  CFileHandle::GetFile
  ====================*/
CFile*	CFileHandle::GetFile(const wstring &sFilename, int iMode)
{
	if (sFilename.empty())
		return NULL;

	CFile *pFile(new CFileDisk);
	if (!pFile->Open(sFilename, iMode))
	{
		delete pFile;
		return NULL;
	}

	return pFile;
}


/*====================
  CFileHandle::BuildBlockList
  ====================*/
bool	CFileHandle::BuildBlockList(const char *pHeader, uint uiHeaderSize, FileBlockList &vBlockList)
{
	vBlockList.clear();

	uint uiBufferSize;
	const char *pBuffer(GetBuffer(uiBufferSize));
	if (pBuffer == NULL)
	{
		cerr << _T("CFileHandle::BuildBlockList: Unable to read file") << endl;
		return false;
	}

	if (uiBufferSize < uiHeaderSize)
	{
		cerr << _T("CFileHandle::BuildBlockList: Truncated header") << endl;
		return false;
	}

	// Check Header
	for (uint ui(0); ui < uiHeaderSize; ++ui)
	{
		if (pBuffer[ui] != pHeader[ui])
		{
			cerr << _T("CFileHandle::BuildBlockList: Bad header") << endl;
			return false;
		}
	}

	pBuffer += uiHeaderSize;
	uiBufferSize -= uiHeaderSize;

	size_t zPos(0);
	while (zPos + 8 < uiBufferSize)
	{
		SFileBlock cBlock;
		memset(&cBlock, 0, sizeof(cBlock));

		cBlock.szName[0] = pBuffer[zPos++];
		cBlock.szName[1] = pBuffer[zPos++];
		cBlock.szName[2] = pBuffer[zPos++];
		cBlock.szName[3] = pBuffer[zPos++];
		cBlock.szName[4] = '\0';

		cBlock.uiLength = LittleInt(*(uint *)(&pBuffer[zPos]));
		if (zPos + cBlock.uiLength > uiBufferSize)
		{
			cerr << _T("CFileHandle::BuildBlockList: Truncated block") << endl;
			return false;
		}

		zPos += 4;
		cBlock.zPos = zPos;
		cBlock.pData = (byte *)&pBuffer[zPos];
		zPos += cBlock.uiLength;

		vBlockList.push_back(cBlock);
	}

	if (zPos != uiBufferSize)
	{
		cerr << _T("CFileHandle::BuildBlockList: Bad Filesize") << endl;
		return false;
	}

	return true;
}


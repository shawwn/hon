// (C)2008 S2 Games
// c_filedisk.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k0_common.h"

#include <sys/stat.h>

#include "c_filedisk.h"

#include "c_exception.h"
//=============================================================================

/*====================
  CFileDisk::CFileDisk
  ====================*/
CFileDisk::CFileDisk()
{
}


/*====================
  CFileDisk::Open
  ====================*/
bool	CFileDisk::Open(const wstring &sPath, int iMode)
{
	m_sPath = sPath;
	m_iMode = iMode;

	// Set the access mode
	ios_base::openmode iOpenFlags;

	if (m_iMode & FILE_READ)
	{
		m_iMode &= ~(FILE_APPEND | FILE_TRUNCATE);
		iOpenFlags = ios_base::in;
	}
	else if (m_iMode & FILE_WRITE)
	{
		iOpenFlags = ios_base::out;
	}
	else
	{
		return false;
	}

	if (m_iMode & FILE_APPEND)
	{
		iOpenFlags |= ios_base::app;
	}
	else if (m_iMode & FILE_WRITE)
	{
		m_iMode |= FILE_TRUNCATE;
		iOpenFlags |= ios_base::trunc;
	}

	// Always open the stream as binary so that the engine can
	// handle all cases of linebreaks properly
	iOpenFlags |= ios_base::binary;
	if (m_iMode & FILE_TEXT)
	{
		if ((m_iMode & FILE_WRITE) &&
			(m_iMode & FILE_TEXT_ENCODING_MASK) == 0)
			m_iMode |= FILE_DEFAULT_TEXT_ENCODING;

		if ((m_iMode & FILE_ASCII) ||
			(m_iMode & FILE_UTF8))
			m_iMode &= ~FILE_ENDIAN_MASK;
	}
	else
	{
		m_iMode |= FILE_BINARY;
		m_iMode &= ~(FILE_UTF8 | FILE_UTF16 | FILE_UTF32);
	}

	// open
#ifdef _WIN32
	m_File.open(m_sPath.c_str(), iOpenFlags);
#elif defined(linux)
	m_File.open(WCSToMBS(m_sPath).c_str(), iOpenFlags);
#endif

	// default behavior is to buffer
	if (!(m_iMode & FILE_NOBUFFER))
		m_iMode |= FILE_BUFFER;

	// default behavior is to block
	if (!(m_iMode & FILE_NOBLOCK))
		m_iMode |= FILE_BLOCK;

	// validate file
	if (!m_File.is_open())
		return false;

	// Unicode files get a BOM
	if ((m_iMode & FILE_TEXT) &&
		(m_iMode & FILE_WRITE) &&
		!(m_iMode & FILE_ASCII))
	{
		if (m_iMode & FILE_UTF8)
		{
			WriteByte('\xef');
			WriteByte('\xbb');
			WriteByte('\xbf');
		}
		else
		{
			if (m_iMode & FILE_BIG_ENDIAN)
			{
				if (m_iMode & FILE_UTF32)
				{
					WriteByte('\x00');
					WriteByte('\x00');
				}
				WriteByte('\xfe');
				WriteByte('\xff');
			}
			else
			{
				WriteByte('\xff');
				WriteByte('\xfe');
				if (m_iMode & FILE_UTF32)
				{
					WriteByte('\x00');
					WriteByte('\x00');
				}
			}
		}
	}

	// Get size
	if (m_iMode & FILE_READ)
	{
		m_File.seekg(0, std::ios::end);
		if (!m_File.fail())
			m_uiSize = m_File.tellg();

		m_File.seekg(0);
	}

	if ((m_iMode & FILE_READ))
	{
		if (m_iMode & FILE_BUFFER)
			GetBuffer(m_uiSize);

		m_bEOF = false;
	}

	return true;
}


/*====================
  CFileDisk::Close
  ====================*/
void	CFileDisk::Close()
{
	m_File.close();
	SAFE_DELETE_ARRAY(m_pBuffer);
}


/*====================
  CFileDisk::IsOpen
  ====================*/
bool	CFileDisk::IsOpen() const
{
	return m_File.is_open();
}


/*====================
  CFileDisk::ReadCharacter
  ====================*/
uint	CFileDisk::ReadCharacter()
{
	// ASCII
	if (m_iMode & FILE_ASCII)
		return ReadByte();

	// UTF-32
	if (m_iMode & FILE_UTF32)
		return ReadInt32(IsBigEndian());

	// UTF-16
	if (m_iMode & FILE_UTF16)
	{
		uint uiResult(ReadInt16(IsBigEndian()));
		if ((uiResult & 0xfc00) != 0xd800)
			return uiResult;

		uint uiSecond(ReadInt16(IsBigEndian()));
		if ((uiSecond & 0xfc00) != 0xdc00)
			return 0;

		return ((uiResult & 0x03ff) << 10) | (uiSecond & 0x03ff);
	}

	// UTF-8
	char cBuffer[4];
	cBuffer[0] = ReadByte();
	if ((cBuffer[0] & 0x80) == 0)
		return cBuffer[0];

	// Determine how many bytes are in this character
	uint uiMask(0x80);
	uint uiNumBytes(0);
	while (cBuffer[0] & uiMask)
	{
		++uiNumBytes;
		uiMask >>= 1;
	}

	if (uiNumBytes < 2 || uiNumBytes > 4)
		return 0;

	uint uiBytesRead(Read(&cBuffer[1], uiNumBytes - 1));
	if (uiBytesRead < uiNumBytes - 1)
		return 0;

	// Assign the remaining bits of the first byte
	uiMask -= 1;
	uint uiResult((cBuffer[0] & uiMask) << ((uiNumBytes - 1) * 6));

	// Assemble the remaining bytes
	for (uint ui(1); ui < uiNumBytes; ++ui)
	{
		if (cBuffer[ui] == 0 || (cBuffer[ui] & 0xc0) != 0x80)
			return 0;

		uiResult |= (cBuffer[ui] & 0x3f) << ((uiNumBytes - ui - 1) * 6);
	}

	return uiResult;
}


/*====================
  CFileDisk::ReadLine
  ====================*/
string	CFileDisk::ReadLine()
{
	if (m_iMode & FILE_WRITE)
		return SNULL;

	if (m_iMode & FILE_BINARY)
		return SNULL;

	if (m_bEOF)
		return SNULL;


	string sReturn;
	while (!m_bEOF)
	{
		char c(ReadCharacter() & 0xff);
		if (c == 0)
			continue;

		// Check for a line break
		if (c == '\r')
		{
			int iOldPos(Tell());
			char cNext(ReadByte());
			if (cNext != '\n')
				Seek(iOldPos);
			break;
		}

		if (c == '\n')
			break;

		sReturn += c;
	}

	return sReturn;
}


/*====================
  CFileDisk::ReadLineW
  ====================*/
wstring	CFileDisk::ReadLineW()
{
	if (m_iMode & FILE_WRITE)
		return WSNULL;

	if (m_iMode & FILE_BINARY)
		return WSNULL;

	if (m_bEOF)
		return WSNULL;


	wstring sReturn;
	while (!m_bEOF)
	{
		wchar_t c(ReadCharacter() & 0xffff);
		if (c == 0)
			continue;

		// Check for a line break
		if (c == L'\r')
		{
			int iOldPos(Tell());
			wchar_t cNext(ReadCharacter() & 0xffff);
			if (cNext != L'\n')
				Seek(iOldPos);
			break;
		}

		if (c == L'\n')
			break;

		sReturn += c;
	}

	return sReturn;
}


/*====================
  CFileDisk::WriteString
  ====================*/
bool	CFileDisk::WriteString(const string &sText)
{
	if (sText.empty())
		return true;

	if (m_iMode & FILE_READ)
	{
		wcerr << L"Cannot write to READ file: " << m_sPath << endl;
		return false;
	}

	string sTmp(NormalizeLineBreaks(sText));

	// Check to see if the string requires convesion
	if ((m_iMode & FILE_ASCII) || (m_iMode & FILE_UTF8) || (m_iMode & FILE_BINARY))
	{
		m_File.write(sTmp.data(), std::streamsize(sTmp.length() * sizeof(char)));
	}
	else
	{
		// Determine write character size
		size_t zCharSize(2);
		if (m_iMode & FILE_UTF32)
			zCharSize = 4;

		// Determine offsets
		size_t a(0), b(1), c(2), d(3);
		if (m_iMode & FILE_BIG_ENDIAN)
		{
			if (m_iMode & FILE_UTF32)
			{
				a = 3; b = 2; c = 1; d = 0;
			}
			else
			{
				a = 1; b = 0;
			}
		}

		// Fill the temporary buffer
		size_t zBufferSize(sTmp.length() * zCharSize);
		char *pBuffer(new char[zBufferSize]);
		for (size_t z(0); z < sText.length(); ++z)
		{
			pBuffer[(z * zCharSize) + a] = sTmp[z];
			pBuffer[(z * zCharSize) + b] = 0;
			if (m_iMode & FILE_UTF32)
			{
				pBuffer[(z * zCharSize) + c] = 0;
				pBuffer[(z * zCharSize) + d] = 0;
			}
		}

		m_File.write(pBuffer, std::streamsize(zBufferSize));
		delete[] pBuffer;
	}

	if (m_iMode & FILE_NOBUFFER)
		m_File.flush();

	return true;
}

bool	CFileDisk::WriteString(const wstring &sText)
{
	if (sText.empty())
		return true;

	if (m_iMode & FILE_READ)
	{
		wcerr << L"Cannot write to READ file: " << m_sPath << endl;
		return false;
	}

	wstring sTmp(NormalizeLineBreaks(sText));

	if (m_iMode & FILE_BINARY)
	{
		m_File.write(reinterpret_cast<const char*>(sTmp.data()), std::streamsize(sTmp.length() * sizeof(wchar_t)));
	}
	else
	{
		// Determine write character size
		size_t zCharSize(1);
		if (m_iMode & FILE_UTF16)
			zCharSize = 2;
		else if (m_iMode & FILE_UTF32)
			zCharSize = 4;

		// Determine offsets
		size_t a(0), b(1), c(2), d(3);
		if (m_iMode & FILE_BIG_ENDIAN)
		{
			if (m_iMode & FILE_UTF32)
			{
				a = 1; b = 0;
			}
			else
			{
				a = 3; b = 2; c = 1; d = 0;
			}
		}

		// Fill the temporary buffer
		size_t zBufferSize(sTmp.length() * zCharSize);
		char *pBuffer(new char[zBufferSize]);
		for (size_t z(0); z < sTmp.length(); ++z)
		{
			pBuffer[(z * zCharSize) + a] = (sTmp[z] & 0xff);
			if (zCharSize > 1)
				pBuffer[(z * zCharSize) + b] = ((sTmp[z] >> 8) & 0xff);
			if (zCharSize > 2)
			{
				pBuffer[(z * zCharSize) + c] = 0;
				pBuffer[(z * zCharSize) + d] = 0;
			}
		}

		m_File.write(pBuffer, std::streamsize(zBufferSize));
		delete[] pBuffer;
	}

	if (m_iMode & FILE_NOBUFFER)
		m_File.flush();

	return true;
}


/*====================
  CFileDisk::Read
  ====================*/
uint	CFileDisk::Read(char* pBuffer, uint uiBufferSize) const
{
	if (m_iMode & FILE_WRITE)
	{
		wcerr << L"Cannot read from WRITE file " << m_sPath << endl;
		return 0;
	}

	// Check for a buffer
	if (m_pBuffer)
	{
		// read from the buffer, and advance the position
		uint uiSize(uiBufferSize);

		if (m_uiSize - m_uiPos <= uiBufferSize)
		{
			uiSize = m_uiSize - m_uiPos;
			m_bEOF = true;
		}

		memcpy(pBuffer, m_pBuffer + m_uiPos, uiSize);
		m_uiPos += uiSize;
		return uiSize;
	}
	else
	{
		// read directly from the disk
		m_File.read(pBuffer, uiBufferSize);
		uint uiCount(m_File.gcount());
		if (uiCount < uiBufferSize)
			m_bEOF = true;
		return uiCount;
	}
}


/*====================
  CFileDisk::Write
  ====================*/
size_t	CFileDisk::Write(const void* pBuffer, size_t zBufferSize)
{
	if (!IsOpen())
		return 0;

	if (m_iMode & FILE_READ)
	{
		wcerr << L"Cannot write to READ file " << m_sPath << endl;
		return 0;
	}

	m_File.write(reinterpret_cast<const char*>(pBuffer), std::streamsize(zBufferSize));

	// flush the file if it was opened unbuffered
	if (m_iMode & FILE_NOBUFFER)
		m_File.flush();

	return zBufferSize;
}


/*====================
  CFileDisk::GetBuffer
  ====================*/
const char	*CFileDisk::GetBuffer(uint &uiSize)
{
	try
	{
		if (!IsOpen())
			EX_ERROR(_T("Trying to retrieve buffer without opening file"));

		// Check if the buffer has already been filled
		if (m_pBuffer != NULL)
		{
			uiSize = m_uiSize;
			return m_pBuffer;
		}
		
		m_pBuffer = new char[m_uiSize];
		if (m_pBuffer == NULL)
			EX_ERROR(L"Failed to allocate buffer for file: " + m_sPath);

		// Allocate and fill the buffer, but preserve any existing read position
		m_uiPos = m_File.tellg();

		if (m_uiPos != 0)
			m_File.seekg(0);

		m_File.read(m_pBuffer, std::streamsize(m_uiSize));
		m_File.seekg(m_uiPos);

		// Check for a unicode BOM
		if (m_iMode & FILE_TEXT &&
			(m_iMode & FILE_TEXT_ENCODING_MASK) == 0)
		{
			if (m_uiSize >= 4)
			{
				if (m_pBuffer[0] == char(0xff) &&
					m_pBuffer[1] == char(0xfe) &&
					m_pBuffer[2] == char(0x00) &&
					m_pBuffer[3] == char(0x00))
				{
					m_iMode |= (FILE_UTF32 | FILE_LITTLE_ENDIAN);
					m_uiPos = 4;
				}
				if (m_pBuffer[0] == char(0x00) &&
					m_pBuffer[1] == char(0x00) &&
					m_pBuffer[2] == char(0xfe) &&
					m_pBuffer[3] == char(0xff))
				{
					m_iMode |= (FILE_UTF32 | FILE_BIG_ENDIAN);
					m_uiPos = 4;
				}
			}
			if (m_uiSize >= 3)
			{
				if (m_pBuffer[0] == char(0xef) &&
					m_pBuffer[1] == char(0xbb) &&
					m_pBuffer[2] == char(0xbf))
				{
					m_iMode |= FILE_UTF8;
					m_uiPos = 3;
				}
			}
			if (m_uiSize >= 2)
			{
				if (m_pBuffer[0] == char(0xff) &&
					m_pBuffer[1] == char(0xfe))
				{
					m_iMode |= (FILE_UTF16 | FILE_LITTLE_ENDIAN);
					m_uiPos = 2;
				}
				if (m_pBuffer[0] == char(0xfe) &&
					m_pBuffer[1] == char(0xff))
				{
					m_iMode |= (FILE_UTF16 | FILE_BIG_ENDIAN);
					m_uiPos = 2;
				}
			}
			if ((m_iMode & FILE_TEXT_ENCODING_MASK) == 0)
				m_iMode |= FILE_ASCII;
		}

		// Return values
		uiSize = m_uiSize;
		return m_pBuffer;
	}
	catch (CException &ex)
	{
		ex.Process(_T("CFileDisk::GetBuffer() - "), NO_THROW);
		return NULL;
	}
}


/*====================
  CFileDisk::WriteByte
  ====================*/
bool	CFileDisk::WriteByte(char c)
{
	if (!Write(&c, 1))
	{
		wcerr << L"WriteByte() - Write failed" << m_sPath << endl;
		return false;
	}

	return true;
}


/*====================
  CFileDisk::WriteInt16
  ====================*/
bool	CFileDisk::WriteInt16(short n, bool bUseBigEndian)
{
	char	buffer[2];
	if (bUseBigEndian)
	{
		buffer[1] = n & 0xff;
		buffer[0] = (n >> 8) & 0xff;
	}
	else
	{
		buffer[0] = n & 0xff;
		buffer[1] = (n >> 8) & 0xff;
	}

	if (!Write(buffer, 2))
	{
		wcerr << L"WriteInt16() - Write failed" << m_sPath << endl;
		return false;
	}

	return true;
}


/*====================
  CFileDisk::WriteInt32
  ====================*/
bool	CFileDisk::WriteInt32(int i, bool bUseBigEndian)
{
	char buffer[4];
	if (bUseBigEndian)
	{
		buffer[3] = i & 0xff;
		buffer[2] = (i >> 8) & 0xff;
		buffer[1] = (i >> 16) & 0xff;
		buffer[0] = (i >> 24) & 0xff;
	}
	else
	{
		buffer[0] = i & 0xff;
		buffer[1] = (i >> 8) & 0xff;
		buffer[2] = (i >> 16) & 0xff;
		buffer[3] = (i >> 24) & 0xff;
	}


	if (!Write(buffer, 4))
	{
		wcerr << L"WriteInt32() - Write failed" << m_sPath << endl;
		return false;
	}

	return true;
}


/*====================
  CFileDisk::WriteInt64
  ====================*/
bool	CFileDisk::WriteInt64(LONGLONG ll, bool bUseBigEndian)
{
	char buffer[8];
	if (bUseBigEndian)
	{
		buffer[7] = char(ll & 0xff);
		buffer[6] = char((ll >> 8) & 0xff);
		buffer[5] = char((ll >> 16) & 0xff);
		buffer[4] = char((ll >> 24) & 0xff);
		buffer[3] = char((ll >> 32) & 0xff);
		buffer[2] = char((ll >> 40) & 0xff);
		buffer[1] = char((ll >> 48) & 0xff);
		buffer[0] = char((ll >> 56) & 0xff);
	}
	else
	{
		buffer[0] = char(ll & 0xff);
		buffer[1] = char((ll >> 8) & 0xff);
		buffer[2] = char((ll >> 16) & 0xff);
		buffer[3] = char((ll >> 24) & 0xff);
		buffer[4] = char((ll >> 32) & 0xff);
		buffer[5] = char((ll >> 40) & 0xff);
		buffer[6] = char((ll >> 48) & 0xff);
		buffer[7] = char((ll >> 56) & 0xff);
	}

	if (!Write(buffer, 8))
	{
		wcerr << L"WriteInt64() - Write failed" << m_sPath << endl;
		return false;
	}

	return true;
}


/*====================
  CFileDisk::Tell
  ====================*/
uint	CFileDisk::Tell() const
{
	if (m_pBuffer == NULL)
		return m_File.tellg();
	else
		return m_uiPos;
}


/*====================
  CFileDisk::Seek
  ====================*/
bool	CFileDisk::Seek(uint uiOffset, ESeekOrigin eOrigin)
{
	bool bResult;

	if (m_pBuffer != NULL)
	{
		bResult = CFile::Seek(uiOffset, eOrigin);

		if (m_uiPos == GetBufferSize())
			m_bEOF = true;
		else
			m_bEOF = false;

		return bResult;
	}

	switch (eOrigin)
	{
	case SEEK_ORIGIN_CURRENT:
		m_File.seekg(uiOffset, ios_base::cur);

		if (uiOffset + m_uiPos < GetBufferSize())
			m_bEOF = false;
		else
			m_bEOF = true;

		break;

	case SEEK_ORIGIN_END:
		m_File.seekg(uiOffset, ios_base::end);

		if (uiOffset > 0)
			m_bEOF = false;
		else
			m_bEOF = true;

		break;

	case SEEK_ORIGIN_START:
		m_File.seekg(uiOffset, ios_base::beg);

		if (uiOffset < GetBufferSize())
			m_bEOF = false;
		else
			m_bEOF = true;

		break;
	}

	bResult = (!m_File.fail());
	m_File.clear();
	return bResult;
}


/*====================
  CFileDisk::Flush
  ====================*/
void	CFileDisk::Flush()
{
	m_File.flush();
}

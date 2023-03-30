// (C)2008 S2 Games
// c_buffer.cpp
//
// A general purpose buffer class and friends
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k0_common.h"

#include "c_buffer.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
//=============================================================================

/*====================
  IBuffer::~IBuffer
  ====================*/
IBuffer::~IBuffer()
{
	SAFE_DELETE_ARRAY(m_pBuffer);
}


/*====================
  IBuffer::IBuffer
  ====================*/
IBuffer::IBuffer() :
m_pBuffer(NULL),
m_uiSize(0),
m_uiEnd(0),
m_uiRead(0),
m_iFaults(0)
{
}

IBuffer::IBuffer(const IBuffer &buffer) :
m_pBuffer(NULL),
m_uiSize(buffer.m_uiSize),
m_uiEnd(buffer.m_uiEnd),
m_uiRead(buffer.m_uiRead),
m_iFaults(buffer.m_iFaults)
{
	if (m_pBuffer)
		delete[] m_pBuffer;

	if (buffer.m_pBuffer)
	{
		m_pBuffer = new char[buffer.m_uiSize];
		if (m_pBuffer == NULL)
			m_iFaults |= BUFFER_FAULT_COPY | BUFFER_FAULT_ALLOCATE;
		memcpy(m_pBuffer, buffer.m_pBuffer, buffer.m_uiSize);
	}
}


/*====================
  IBuffer::Init
  ====================*/
void	IBuffer::Init(uint uiSize)
{
	if (uiSize > 0)
	{
		m_pBuffer = new char[uiSize];
		
		if (!m_pBuffer)
			m_iFaults |= BUFFER_FAULT_INITIALIZE | BUFFER_FAULT_ALLOCATE;
		else
			m_uiSize = uiSize;
	}
	else
	{
		m_pBuffer = NULL;
		m_uiSize = 0;
	}
	
	m_uiRead = 0;
}


/*====================
  IBuffer::Resize
  ====================*/
bool	IBuffer::Resize(uint uiSize)
{
	if (uiSize == m_uiSize)
		return true;

	if (uiSize > 0)
	{
		char *pTemp = new char[uiSize];
		if (!pTemp)
		{
			m_iFaults |= BUFFER_FAULT_ALLOCATE;
			return false;
		}

		// copy the old contents, if there is any
		uint uiCopyLen(0);
		if (m_uiEnd > 0)
		{
			uiCopyLen = (uiSize < m_uiEnd) ? uiSize : m_uiEnd;
			memcpy(pTemp, m_pBuffer, uiCopyLen);
		}

		m_uiEnd = uiCopyLen;

		if (m_pBuffer)
			delete[] m_pBuffer;

		m_pBuffer = pTemp;
		m_uiSize = uiSize;
	}
	else
	{
		m_uiEnd = 0;

		if (m_pBuffer)
			delete[] m_pBuffer;

		m_pBuffer = NULL;
		m_uiSize = 0;
	}
	
	return true;
}


/*====================
  IBuffer::Reserve
  ====================*/
bool	IBuffer::Reserve(uint uiSize)
{
	if (uiSize <= m_uiSize)
		return true;

	char *pTemp = new char[uiSize];
	if (!pTemp)
	{
		m_iFaults |= BUFFER_FAULT_ALLOCATE;
		return false;
	}

	// copy the old contents, if there is any
	uint uiCopyLen(0);
	if (m_uiEnd > 0)
	{
		uiCopyLen = (uiSize < m_uiEnd) ? uiSize : m_uiEnd;
		memcpy(pTemp, m_pBuffer, uiCopyLen);
	}
	if (m_pBuffer)
		delete[] m_pBuffer;

	m_pBuffer = pTemp;
	m_uiSize = uiSize;
	m_uiEnd = uiCopyLen;
	return true;
}



/*====================
  IBuffer::FindNext
  ====================*/
uint	IBuffer::FindNext(char c) const
{
	for (uint uiSeek(m_uiRead); uiSeek < m_uiEnd; ++uiSeek)
	{
		if (m_pBuffer[uiSeek] == c)
			return uiSeek;
	}

	return INVALID_INDEX;
}

uint	IBuffer::FindNext(wchar_t c) const
{
	for (uint uiSeek(m_uiRead); uiSeek < (m_uiEnd - 1); uiSeek += 2)
	{
		if (*(wchar_t*)(&m_pBuffer[uiSeek]) == c)
			return uiSeek;
	}

	return INVALID_INDEX;
}


/*====================
  IBuffer::ReadString
  ====================*/
string	IBuffer::ReadString() const
{
	char *sz(NULL);

	try
	{
		uint uiPos(FindNext('\x00'));

		if (uiPos == INVALID_INDEX)
			EX_ERROR(_T("Attempted to read unterminated string"));

		uint uiRead(uiPos - GetReadPos() + 1);
		sz = new char[uiRead];
		if (sz == NULL)
			EX_ERROR(_T("Failed to allocate temporary string buffer"));

		Read(sz, uiRead);
		if (GetFaults() & BUFFER_FAULT_UNDERRUN)
			EX_ERROR(_T("Attempted to read past end of buffer"));

		string sReturn(sz);
		delete[] sz;
		return sReturn;
	}
	catch (CException &ex)
	{
		if (sz != NULL)
			delete[] sz;

		ex.Process(_T("IBuffer::ReadString() - "), NO_THROW);
		return "";
	}
}


/*====================
  IBuffer::ReadWString
  ====================*/
wstring	IBuffer::ReadWString() const
{
	wchar_t *sz(NULL);

	try
	{
		uint uiPos(FindNext(L'\x0000'));
		if (uiPos == INVALID_INDEX)
			EX_ERROR(_T("Attempted to read unterminated string"));

		uint uiRead((uiPos - GetReadPos() + sizeof(wchar_t)));
		sz = new wchar_t[uiRead];
		if (sz == NULL)
			EX_ERROR(_T("Failed to allocate temporary string buffer"));

		Read(sz, uiRead);
		if (GetFaults() & BUFFER_FAULT_UNDERRUN)
			EX_ERROR(_T("Attempted to read past end of packet"));

		wstring sReturn(sz);
		delete[] sz;
		return sReturn;
	}
	catch (CException &ex)
	{
		if (sz != NULL)
			delete[] sz;

		ex.Process(_T("IBuffer::ReadString() - "), NO_THROW);
		return L"";
	}
}


/*====================
  CBufferStatic::Write

  Copy the input to the start of the buffer, overwriting
  ====================*/
bool	CBufferStatic::Write(const void *pBuffer, uint uiSize)
{
	bool ret(true);

	if (pBuffer == NULL)
		return false;

	uint uiCopyLen(uiSize);
	if (uiSize > m_uiSize)
	{
		uiCopyLen = m_uiSize;
		m_iFaults |= BUFFER_FAULT_OVERRUN;
		ret = false;
	}

	memcpy(m_pBuffer, pBuffer, uiCopyLen);
	m_uiEnd = uiCopyLen;
	m_uiRead = 0;
	return ret;
}


/*====================
  CBufferStatic::Append

  Copy the input to the end of the buffer 
  ====================*/
bool	CBufferStatic::Append(const void *pBuffer, uint uiSize)
{
	bool ret(true);

	uint uiCopyLen(uiSize);
	if (uiSize > (m_uiSize - m_uiEnd))
	{
		uiCopyLen = m_uiSize - m_uiEnd;
		m_iFaults |= BUFFER_FAULT_OVERRUN;
		ret = false;
	}

	memcpy(&m_pBuffer[m_uiEnd], pBuffer, uiCopyLen);
	m_uiEnd += uiCopyLen;
	return ret;
}


/*====================
  CBufferStatic::Lock

  return a writable region of the buffer from the current write position
  ====================*/
char*	CBufferStatic::Lock(uint uiSize)
{
	if (uiSize > (m_uiSize - m_uiEnd))
	{
		uiSize = m_uiSize - m_uiEnd;
		m_iFaults |= BUFFER_FAULT_OVERRUN;
	}

	if (uiSize == 0)
		return NULL;

	char *pBuffer(&m_pBuffer[m_uiEnd]);
	m_uiEnd += uiSize;
	
	return pBuffer;
}


/*====================
  CBufferStatic::Advance
  ====================*/
bool	CBufferStatic::Advance(uint uiSize) const
{
	if (uiSize == 0)
		return true;

	bool ret(true);

	if (m_uiRead + uiSize > m_uiEnd)
	{
		m_iFaults |= BUFFER_FAULT_OVERRUN;
		cerr << _T("IBuffer::Advance - Overrun") << endl;
		ret = false;
	}
	else
	{
		m_uiRead += uiSize;
	}

	return ret;
}


/*====================
  CBufferStatic::Seek
  ====================*/
bool	CBufferStatic::Seek(uint uiPos)
{
	if (uiPos <= m_uiEnd)
	{
		m_uiRead = uiPos;
		return true;
	}

	return false;
}


/*====================
  CBufferStatic::Read
  ====================*/
bool	CBufferStatic::Read(void *pBuffer, uint uiSize) const
{
	if (m_uiRead + uiSize > m_uiEnd)
	{
		m_iFaults |= BUFFER_FAULT_UNDERRUN;
		return false;
	}

	memcpy(pBuffer, &m_pBuffer[m_uiRead], uiSize);
	m_uiRead += uiSize;
	return true;
}


/*====================
  CBufferDynamic::Write
  ====================*/
bool	CBufferDynamic::Write(const void *pBuffer, uint uiSize)
{
	try
	{
		if (uiSize == 0)
		{
			m_uiRead = 0;
			m_uiEnd = 0;
			return true;
		}

		bool ret(true);

		if (pBuffer == NULL)
			EX_ERROR(_T("Invalid source"));

		uint uiCopyLen(uiSize);
		if (uiSize > m_uiSize)
		{
			uint uiNewSize(MAX(m_uiSize, 1u));
			while (uiNewSize < uiSize)
				uiNewSize <<= 1;
			if (!Resize(uiNewSize))
			{
				m_iFaults |= BUFFER_FAULT_OVERRUN;
				uiCopyLen = m_uiSize;
				ret = false;
			}
		}

		memcpy(m_pBuffer, pBuffer, uiCopyLen);
		m_uiEnd = uiCopyLen;
		m_uiRead = 0;
		return ret;
	}
	catch (CException &ex)
	{
		ex.Process(_T("CBufferDynamic::Write() - "));
		return false;
	}
}


/*====================
  CBufferDynamic::Append
  ====================*/
bool	CBufferDynamic::Append(const void *pBuffer, uint uiSize)
{
	bool	ret(true);

	if (uiSize == 0)
		return true;

	uint uiCopyLen(uiSize);
	if (uiSize > (m_uiSize - m_uiEnd))
	{
		uint uiNewSize(MAX(m_uiSize, 1u));
		while (uiNewSize < (m_uiEnd + uiSize))
			uiNewSize <<= 1;
		if (!Resize(uiNewSize))
		{
			m_iFaults |= BUFFER_FAULT_OVERRUN;
			ret = false;
			uiCopyLen = m_uiSize - m_uiEnd;
		}
	}

	memcpy(&m_pBuffer[m_uiEnd], pBuffer, uiCopyLen);
	m_uiEnd += uiCopyLen;
	return ret;
}


/*====================
  CBufferDynamic::Seek
  ====================*/
bool	CBufferDynamic::Seek(uint uiPos)
{
	if (uiPos < m_uiEnd)
	{
		m_uiRead = uiPos;
		return true;
	}

	return false;
}


/*====================
  CBufferDynamic::Read
  ====================*/
bool	CBufferDynamic::Read(void *pBuffer, uint uiSize) const
{
	if (m_uiRead + uiSize > m_uiEnd)
	{
		m_iFaults |= BUFFER_FAULT_UNDERRUN;
		return false;
	}

	memcpy(pBuffer, &m_pBuffer[m_uiRead], uiSize);
	m_uiRead += uiSize;
	return true;
}


/*====================
  CBufferDynamic::Overwrite
  ====================*/
bool	CBufferDynamic::Overwrite(const void *pBuffer, uint uiSize)
{
	if (uiSize == 0)
		return true;

	bool ret(true);

	if (pBuffer == NULL)
	{
		cerr << _T("CBufferDynamic::Overwrite - Invalid source") << endl;
		ret = false;
	}
	else
	{
		uint uiCopyLen(uiSize);
		if (m_uiRead + uiSize > m_uiEnd)
		{
			m_iFaults |= BUFFER_FAULT_OVERRUN;
			cerr << _T("CBufferDynamic::Overwrite - Overrun") << endl;
			ret = false;
		}
		else
		{
			memcpy(&m_pBuffer[m_uiRead], pBuffer, uiCopyLen);
			m_uiRead += uiSize;
		}
	}

	return ret;
}


/*====================
  CBufferDynamic::Overwrite
  ====================*/
bool	CBufferDynamic::Overwrite(tstring &sNewString)
{
	if (m_uiRead >= m_uiEnd)
		return false;

	uint readPosition(m_uiRead);

	// Advance through the buffer, past the current string
	while(ReadByte() && m_uiRead <= m_uiEnd)
		/* do nothing */;

	char *pTemp(NULL);
	uint uiRemainingBuffer(m_uiEnd - m_uiRead);
	if (uiRemainingBuffer)
	{
		pTemp = new char[m_uiEnd - m_uiRead];
		memcpy(pTemp, &m_pBuffer[m_uiRead], uiRemainingBuffer);
	}

	m_uiEnd = readPosition;
	Append(sNewString.c_str(), sizeof(TCHAR) * INT_SIZE(sNewString.size()));
	byte terminate(0);
	Append(&terminate, sizeof(byte));
	// Store the position of the string terminator
	readPosition = m_uiEnd;
	if (pTemp)
		Append(pTemp, uiRemainingBuffer);

	// Set the read position
	m_uiRead = readPosition;

	return true;
}


/*====================
  CBufferDynamic::Lock

  return a writable region of the buffer from the current write position
  ====================*/
char*	CBufferDynamic::Lock(uint uiSize)
{
	if (uiSize > (m_uiSize - m_uiEnd))
	{
		uiSize = m_uiSize - m_uiEnd;
		m_iFaults |= BUFFER_FAULT_OVERRUN;
	}

	if (uiSize == 0)
		return NULL;

	char *pBuffer(&m_pBuffer[m_uiEnd]);
	m_uiEnd += uiSize;
	
	return pBuffer;
}


/*====================
  CBufferDynamic::Advance
  ====================*/
bool	CBufferDynamic::Advance(uint uiSize) const
{
	if (uiSize == 0)
		return true;

	bool ret(true);

	if (m_uiRead + uiSize > m_uiEnd)
	{
		m_iFaults |= BUFFER_FAULT_OVERRUN;
		cerr << _T("IBuffer::Advance - Overrun") << endl;
		ret = false;
	}
	else
	{
		m_uiRead += uiSize;
	}

	return ret;
}


/*====================
  CBufferCircular::CBufferCircular
  ====================*/
CBufferCircular::CBufferCircular(const CBufferCircular &buffer) :
IBuffer(buffer),
m_uiLength(buffer.m_uiLength)
{
}


/*====================
  CBufferCircular::Write
  ====================*/
bool	CBufferCircular::Write(const void *pBuffer, uint uiSize)
{
	bool ret(true);

	uint uiCopyLen(uiSize);
	uint uiReadStart(0);

	// Data is larger than the size of the buffer, so we
	// just write the maximum possible portion of the data,
	// aligned to the end rather than the start
	if (uiSize > m_uiSize)
	{
		uiReadStart = uiSize - m_uiSize;
		uiCopyLen = m_uiSize;
		m_iFaults |= BUFFER_FAULT_OVERRUN;
		ret = false;
	}

	memcpy(m_pBuffer, &((const char*)pBuffer)[uiReadStart], uiCopyLen);
	m_uiLength = uiCopyLen;
	m_uiEnd = uiCopyLen;
	m_uiRead = 0;
	return ret;
}


/*====================
  CBufferCircular::Append
  ====================*/
bool	CBufferCircular::Append(const void *pBuffer, uint uiSize)
{
	// If the data is longer than the buffer, treating
	// it as a 'write' will handle the case properly
	if (uiSize > m_uiSize)
		return Write(pBuffer, uiSize);

	uint uiReadStart(0);
	uint uiTailsize(m_uiSize - m_uiEnd);

	// check for hitting the physical end of the buffer,
	// which will require two seperate writes
	if (uiSize > uiTailsize)
	{
		// Write as much of the data as possible, then set up
		// for a regular copy at the start of the buffer
		memcpy(&m_pBuffer[m_uiEnd], pBuffer, uiTailsize);
		uiReadStart = uiTailsize;
		m_uiEnd = 0;
	}

	memcpy(&m_pBuffer[m_uiEnd], &(((char*)pBuffer)[uiReadStart]), uiSize - uiReadStart);
	m_uiEnd += uiSize - uiReadStart;
	m_uiLength = MIN(m_uiSize, m_uiLength + uiSize);
	return true;
}


/*====================
  CBufferCircular::Seek
  ====================*/
bool	CBufferCircular::Seek(uint uiPos)
{
	if (uiPos < m_uiSize)
	{
		m_uiRead = uiPos;
		return true;
	}

	return false;
}


/*====================
  CBufferCircular::Rewind
  ====================*/
void	CBufferCircular::Rewind()
{
	m_uiRead = m_uiEnd + 1;
	if (m_uiRead >= m_uiSize)
		m_uiRead -= m_uiSize;
}


/*====================
  CBufferCircular::Read
  ====================*/
bool	CBufferCircular::Read(void *pBuffer, uint uiSize) const
{
	if (uiSize > m_uiSize)
	{
		m_iFaults |= BUFFER_FAULT_UNDERRUN;
		return false;
	}

	if (m_uiRead + uiSize > m_uiSize)
	{
		memcpy(pBuffer, &m_pBuffer[m_uiRead], m_uiSize - m_uiRead);
		m_uiRead = 0;
		uiSize -= (m_uiSize - m_uiRead);
	}

	memcpy(pBuffer, &m_pBuffer[m_uiRead], uiSize);
	m_uiRead += uiSize;
	return true;
}


/*====================
  CBufferCircular::FindNext
  ====================*/
uint	CBufferCircular::FindNext(char c) const
{
	for (uint uiSeek(m_uiRead); uiSeek != m_uiEnd; ++uiSeek)
	{
		if (uiSeek == m_uiSize)
			uiSeek = 0;

		if (m_pBuffer[uiSeek] == c)
			return uiSeek;
	}

	return INVALID_INDEX;
}

uint	CBufferCircular::FindNext(wchar_t c) const
{
	for (uint uiSeek(m_uiRead); uiSeek != m_uiEnd && uiSeek != m_uiEnd - 1; uiSeek += 2)
	{
		if (uiSeek == m_uiSize - 1)
		{
			wchar_t t((m_pBuffer[uiSeek] << 8) | m_pBuffer[0]);
			if (t == c)
				return uiSeek;
			uiSeek = 1;
		}
		else if (uiSeek == m_uiSize)
		{
			uiSeek = 0;
		}

		if (*((wchar_t*)&m_pBuffer[uiSeek]) == c)
			return uiSeek;
	}

	return INVALID_INDEX;
}

// (C)2008 S2 Games
// c_buffer.h
//
// All multi-byte values are stored in host endian order
//=============================================================================
#ifndef __C_BUFFER_H__
#define __C_BUFFER_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_exception.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
const int BUFFER_FAULT_INITIALIZE(1);	// Failure in Init()
const int BUFFER_FAULT_COPY(2);			// Failure in copy ctor
const int BUFFER_FAULT_ALLOCATE(4);		// Failed to allocate memory
const int BUFFER_FAULT_OVERRUN(8);		// A write to a static buffer was truncated
const int BUFFER_FAULT_UNDERRUN(16);	// A read went beyond the length of the buffer

const size_t DEFAULT_CBUFFER_SIZE = 64;
//=============================================================================


//=============================================================================
// IBuffer
// Base class
//=============================================================================
class IBuffer
{
protected:
	char*			m_pBuffer;
	uint			m_uiSize;
	uint			m_uiEnd;
	mutable uint	m_uiRead;
	mutable int		m_iFaults;

	void	Init(uint uiSize);

public:
	IBuffer();
	IBuffer(const IBuffer &buffer);
	virtual ~IBuffer();

	virtual void				Clear(char c);
	virtual void				Clear();

	size_t						GetSize() const				{ return m_uiSize; }
	virtual uint				GetLength() const 			{ return m_uiEnd; }
	bool						IsEmpty() const				{ return GetLength() == 0; }
	virtual uint				GetReadPos() const			{ return m_uiRead; }
	uint						GetUnreadLength() const		{ return GetLength() - GetReadPos(); }
	int							GetFaults() const			{ return m_iFaults; }
	void						ClearFaults()				{ m_iFaults = 0; }
	void						SetLength(uint uiLength)	{ m_uiEnd = uiLength; }

	bool						Resize(uint uiSize);
	bool						Reserve(uint uiSize);

	inline IBuffer&				operator=(const TCHAR *sz);
	inline IBuffer&				operator=(const IBuffer &B);

	virtual	inline char&		operator[](uint uiIndex);
	virtual	inline char			operator[](uint uiIndex) const;

	virtual bool				Seek(uint uiPos) = 0;
	virtual void				Rewind() const				{ m_uiRead = 0; }
	virtual uint				FindNext(char c) const;
	virtual uint				FindNext(wchar_t c) const;

	virtual inline char*		Duplicate(uint uiOffset, uint uiLength);
	virtual char*				Duplicate(uint uiOffset)	{ return Duplicate(uiOffset, m_uiEnd - uiOffset); }
	virtual char*				Duplicate()					{ return Duplicate(0, m_uiEnd); }

	virtual	inline const char*	Get(uint uiOffset) const;
	virtual const char*			Get() const					{ return Get(0); }

	virtual bool				Append(const void *pBuffer, uint uiSize) = 0;
	virtual bool				Write(const void *pBuffer, uint uiSize) = 0;
	virtual bool				Read(void *pBuffer, uint uiSize) const = 0;

	virtual char*				Lock(uint uiSize)			{ return NULL; }
	virtual inline bool			Advance(uint uiSize) const	{ return false; }

	byte						ReadByte() const			{ byte y; Read(&y, sizeof(byte)); return y; }
	short						ReadShort() const			{ short n; Read(&n, sizeof(short)); return n; }
	int							ReadInt() const				{ int i; Read(&i, sizeof(int)); return i; }
	LONGLONG					ReadInt64() const			{ LONGLONG ll; Read(&ll, sizeof(LONGLONG)); return ll; }
	float						ReadFloat() const			{ float f; Read(&f, sizeof(float)); return f; }
	string						ReadString() const;
	wstring						ReadWString() const;

	void						WriteByte(byte y)			{ Append(&y, sizeof(y)); }
	void						WriteShort(short n)			{ Append(&n, sizeof(n)); }
	void						WriteInt(int i)				{ Append(&i, sizeof(i)); }
	void						WriteInt64(LONGLONG ll)		{ Append(&ll, sizeof(ll)); }
	void						WriteFloat(float f)			{ Append(&f, sizeof(f)); }
	
	inline bool					ReadBit(uint &uiBitPos, byte &yLastRead) const;
	inline void					WriteBit(bool bValue, uint &uiBitPos, byte &yCurrentWrite);

	inline int					CompareBuffer(const IBuffer &cBuffer, uint uiSize) const;
	inline int					CompareBuffer4(const IBuffer &cBuffer) const;
};

/*====================
  IBuffer::Clear
  ====================*/
inline void	IBuffer::Clear()
{
	m_uiRead = m_uiEnd = 0;
	ClearFaults();
}


/*====================
  IBuffer::Clear
  ====================*/
inline void	IBuffer::Clear(char c)
{
	memset(m_pBuffer, c, m_uiSize);
	m_uiRead = m_uiEnd = 0;
	ClearFaults();
}


/*====================
  operator<<
  ====================*/
inline IBuffer&	operator<<(IBuffer &buffer, byte y)
{
	buffer.Append((char*)&y, sizeof(byte));
	return buffer;
}

inline IBuffer&	operator<<(IBuffer &buffer, char c)
{
	buffer.Append((char*)&c, sizeof(char));
	return buffer;
}

inline IBuffer&	operator<<(IBuffer &buffer, short n)
{
	buffer.Append((char*)&n, sizeof(short));
	return buffer;
}

inline IBuffer&	operator<<(IBuffer &buffer, ushort un)
{
	buffer.Append((char*)&un, sizeof(ushort));
	return buffer;
}

inline IBuffer&	operator<<(IBuffer &buffer, int i)
{
	buffer.Append((char*)&i, sizeof(int));
	return buffer;
}

inline IBuffer&	operator<<(IBuffer &buffer, uint ui)
{
	buffer.Append((char*)&ui, sizeof(uint));
	return buffer;
}

inline IBuffer&	operator<<(IBuffer &buffer, LONGLONG ll)
{
	buffer.Append((char*)&ll, sizeof(LONGLONG));
	return buffer;
}

inline IBuffer&	operator<<(IBuffer &buffer, ULONGLONG ull)
{
	buffer.Append((char*)&ull, sizeof(ULONGLONG));
	return buffer;
}

inline IBuffer&	operator<<(IBuffer &buffer, float f)
{
	buffer.Append((char*)&f, sizeof(float));
	return buffer;
}

inline IBuffer&	operator<<(IBuffer &buffer, double d)
{
	buffer.Append((char*)&d, sizeof(double));
	return buffer;
}

inline IBuffer&	operator<<(IBuffer &buffer, const char *sz)
{
	buffer.Append(sz, uint(strlen(sz)) * sizeof(char));
	return buffer;
}

inline IBuffer&	operator<<(IBuffer &buffer, const wchar_t *sz)
{
	buffer.Append(sz, uint(wcslen(sz)) * sizeof(wchar_t));
	return buffer;
}

inline IBuffer&	operator<<(IBuffer &buffer, const string &s)
{
	buffer.Append(s.c_str(), uint(s.length()) * sizeof(char));
	return buffer;
}

inline IBuffer&	operator<<(IBuffer &buffer, const wstring &s)
{
	buffer.Append(s.c_str(), uint(s.length()) * sizeof(wchar_t));
	return buffer;
}

inline IBuffer& operator<<(IBuffer &buffer, const CVec2f &v2)
{
	buffer.Append((char*)&v2, sizeof(CVec2f));
	return buffer;
}

inline IBuffer& operator<<(IBuffer &buffer, const CVec3f &v3)
{
	buffer.Append((char*)&v3, sizeof(CVec3f));
	return buffer;
}

inline IBuffer&	operator<<(IBuffer &bufferA, const IBuffer &bufferB)
{
	if (bufferB.GetLength() == 0)
		return bufferA;

	bufferA.Append(bufferB.Get(), bufferB.GetLength());
	return bufferA;
}


/*====================
  operator>>
  ====================*/
template <class _T>
inline const IBuffer&	operator>>(const IBuffer &buffer, _T &x)
{
	buffer.Read(&x, sizeof(_T));
	return buffer;
}


/*====================
  IBuffer::operator=
  ====================*/
inline IBuffer& IBuffer::operator=(const TCHAR *sz)
{
	Write(sz, uint(_tcslen(sz) * sizeof(TCHAR)));
	return *this;
}

inline IBuffer& IBuffer::operator=(const IBuffer &B)
{
	Write(B.Get(), B.GetLength());
	return *this;
}


/*====================
  IBuffer::operator[]
  ====================*/
inline
char&	IBuffer::operator[](uint uiIndex)
{
	if (uiIndex >= m_uiEnd)
		EX_ERROR(_T("IBuffer::operator[] - invalid index"));
	return m_pBuffer[uiIndex];
}


/*====================
  IBuffer::operator[]
  ====================*/
inline
char	IBuffer::operator[](uint uiIndex) const
{
	if (uiIndex >= m_uiEnd)
		EX_ERROR(_T("IBuffer::operator[] - invalid index"));
	return m_pBuffer[uiIndex];
}


/*====================
  IBuffer::Duplicate
  ====================*/
inline
char*	IBuffer::Duplicate(uint uiOffset, uint uiLength)
{
	if (uiLength <= 0)
		return NULL;

	if (uiOffset + uiLength > m_uiEnd)
		uiLength = m_uiEnd - uiOffset;

	char *pRet = new char[uiLength];
	memcpy(pRet, &m_pBuffer[uiOffset], uiLength);
	return pRet;
}


/*====================
  IBuffer::Get
  ====================*/
inline const char	*IBuffer::Get(uint uiOffset) const
{
	if (uiOffset >= m_uiEnd)
		return NULL;

	return &m_pBuffer[uiOffset];
}


/*====================
  IBuffer::ReadBit
  ====================*/
inline bool	IBuffer::ReadBit(uint &uiBitPos, byte &yLastRead) const
{
	if ((uiBitPos & 7) == 0)
		yLastRead = ReadByte();

	bool bValue((yLastRead & (1 << (uiBitPos & 7))) != 0);
	
	++uiBitPos;
	return bValue;
}


/*====================
  IBuffer::WriteBit
  ====================*/
inline void	IBuffer::WriteBit(bool bValue, uint &uiBitPos, byte &yCurrentWrite)
{
	if (bValue)
		yCurrentWrite |= (1 << (uiBitPos & 7));

	++uiBitPos;

	if ((uiBitPos & 7) == 0)
	{
		Append((char*)&yCurrentWrite, sizeof(yCurrentWrite));
		yCurrentWrite = 0;
	}
}


/*====================
  IBuffer::CompareBuffer
  ====================*/
inline int	IBuffer::CompareBuffer(const IBuffer &cBuffer, uint uiSize) const
{
	return memcmp(&m_pBuffer[m_uiRead], &cBuffer.m_pBuffer[cBuffer.m_uiRead], uiSize);
}


/*====================
  IBuffer::CompareBuffer4
  ====================*/
inline int	IBuffer::CompareBuffer4(const IBuffer &cBuffer) const
{
	char *pBuffer0(&m_pBuffer[m_uiRead]);
	char *pBuffer1(&cBuffer.m_pBuffer[cBuffer.m_uiRead]);

	if (*pBuffer0 != *pBuffer1) return 1; ++pBuffer0; ++pBuffer1;
	if (*pBuffer0 != *pBuffer1) return 1; ++pBuffer0; ++pBuffer1;
	if (*pBuffer0 != *pBuffer1) return 1; ++pBuffer0; ++pBuffer1;
	if (*pBuffer0 != *pBuffer1) return 1;

	return 0;
}
//=============================================================================


//=============================================================================
// CBufferDynamic
// Buffer that will allocate more memmory as needed
//=============================================================================
class CBufferDynamic : public IBuffer
{
private:
public:
	CBufferDynamic()			{ Init(DEFAULT_CBUFFER_SIZE); }
	CBufferDynamic(uint uiSize) { Init(uiSize); }

	bool	Append(const void *pBuffer, uint uiSize);
	bool	Write(const void *pBuffer, uint uiSize);
	bool	Read(void *pBuffer, uint uiSize) const;
	bool	Seek(uint uiPos);
	bool	Overwrite(const void *pBuffer, uint uiSize);
	bool	Overwrite(tstring &sNewString);
	char*	Lock(uint uiSize);
	bool	Advance(uint uiSize) const;

	inline bool	PopFront(uint uiSize);
};
//=============================================================================


/*====================
  CBufferDynamic::PopFront
  ====================*/
inline bool	CBufferDynamic::PopFront(uint uiSize)
{
	bool ret(true);

	if (uiSize == 0)
		return true;

	uint uiMoveLen(uiSize);
	if (uiSize > MIN(m_uiSize, m_uiEnd))
	{
		assert(false);
		uiMoveLen = MIN(m_uiSize, m_uiEnd);
		m_iFaults |= BUFFER_FAULT_OVERRUN;
		ret = false;
	}

	memmove(m_pBuffer, m_pBuffer + uiMoveLen, m_uiSize - uiMoveLen);

	assert(uiMoveLen <= m_uiEnd);
	m_uiEnd -= MIN(m_uiEnd, uiMoveLen);

	m_uiRead -= MIN(m_uiRead, uiMoveLen);
	return ret;
}


//=============================================================================
// CBufferStatic
// Buffer of a fixed size that only reallocates when requested
//=============================================================================
class CBufferStatic : public IBuffer
{
private:

public:
	CBufferStatic()				{ Init(DEFAULT_CBUFFER_SIZE); }
	CBufferStatic(uint uiSize)	{ Init(uiSize); }

	bool	Append(const void *pBuffer, uint uiSize);
	bool	Write(const void *pBuffer, uint uiSize);
	bool	Read(void *pBuffer, uint uiSize) const;
	bool	Seek(uint uiPos);
	char*	Lock(uint uiSize);
	bool	Advance(uint uiSize) const;
};
//=============================================================================


//=============================================================================
// CBufferCircular
// Buffer of a fixed size that loops
//=============================================================================
class CBufferCircular : public IBuffer
{
private:
	uint	m_uiLength;

	inline void	AlignBuffer();

public:
	~CBufferCircular()									{}
	CBufferCircular()									{ m_uiLength = 0; Init(DEFAULT_CBUFFER_SIZE); }
	CBufferCircular(const CBufferCircular &buffer);
	CBufferCircular(uint uiSize)						{ m_uiLength = 0; Init(uiSize); }

	void		Clear(char c)							{ IBuffer::Clear(c); m_uiLength = 0; }
	void		Clear()									{ Clear(0); }

	uint		GetLength() const						{ return m_uiLength; }

	const char*	Get(uint uiOffset)						{ AlignBuffer(); return IBuffer::Get(uiOffset); }
	const char*	Get()									{ return Get(0); }

	char*		Duplicate(uint uiOffset, uint uiLength)	{ AlignBuffer(); return IBuffer::Duplicate(uiOffset, uiLength); }
	char*		Duplicate(uint uiOffset)				{ return Duplicate(uiOffset, m_uiLength); }
	char*		Duplicate()								{ return Duplicate(0, m_uiLength); }

	inline char	operator[](uint uiIndex) const;

	bool		Append(const void *pBuffer, uint uiSize);
	bool		Write(const void *pBuffer, uint uiSize);
	bool		Read(void *pBuffer, uint uiSize) const;
	bool		Seek(uint uiPos);
	uint		FindNext(char c) const;
	uint		FindNext(wchar_t c) const;
	void		Rewind();
};

/*====================
  CBufferCircular::AlignBuffer
  ====================*/
inline void	CBufferCircular::AlignBuffer()
{
	if (m_uiLength < m_uiSize)
		return;

	char *pTmp = new char[m_uiEnd];
	memcpy(pTmp, m_pBuffer, m_uiEnd);

	memmove(m_pBuffer, &m_pBuffer[m_uiEnd], m_uiSize - m_uiEnd);
	memcpy(&m_pBuffer[m_uiSize - m_uiEnd], pTmp, m_uiEnd);
	m_uiEnd = m_uiSize;
}


/*====================
  CBufferCircular::operator[]
  ====================*/
inline
char	CBufferCircular::operator[](uint uiIndex) const
{
	if (uiIndex >= m_uiLength)
		EX_ERROR(_T("CBufferCircular::operator[] - Invalid index"));

	if (m_uiLength == m_uiSize)
		uiIndex = abs(int((m_uiEnd + uiIndex) % m_uiSize));
	return m_pBuffer[uiIndex];
}


//=============================================================================
// CBufferFixed
// Buffer of a fixed size that cannot reallocate
//=============================================================================
template <uint BUFFER_SIZE>
class CBufferFixed : public IBuffer
{
private:
	char m_pFixedBuffer[BUFFER_SIZE];

public:
	CBufferFixed()				{ Init(0); m_pBuffer = m_pFixedBuffer; m_uiSize = BUFFER_SIZE; }
	CBufferFixed(const CBufferFixed &buffer);
	~CBufferFixed()				{ m_pBuffer = NULL; }

	bool	Append(const void *pBuffer, uint uiSize);
	bool	Write(const void *pBuffer, uint uiSize);
	bool	Read(void *pBuffer, uint uiSize) const;
	bool	Seek(uint uiPos);
	bool	AppendNulls(uint uiNewMaxIndex);
	char*	Lock(uint uiSize);
	bool	Advance(uint uiSize) const;
};
//=============================================================================


/*====================
  CBufferFixed::CBufferFixed
  ====================*/
template <uint BUFFER_SIZE>
inline
CBufferFixed<BUFFER_SIZE>::CBufferFixed(const CBufferFixed<BUFFER_SIZE> &buffer)
{
	Init(0); m_pBuffer = m_pFixedBuffer; m_uiSize = BUFFER_SIZE;
	memcpy(m_pBuffer, buffer.m_pBuffer, MIN(buffer.m_uiSize, m_uiSize));
	m_uiEnd = buffer.m_uiEnd;
}


/*====================
  CBufferFixed::Write

  Copy the input to the start of the buffer, overwriting
  ====================*/
template <uint BUFFER_SIZE>
inline
bool	CBufferFixed<BUFFER_SIZE>::Write(const void *pBuffer, uint uiSize)
{
	bool ret(true);

	if (uiSize == 0)
	{
		m_uiEnd = 0;
		m_uiRead = 0;
		return true;
	}

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
  CBufferFixed::Append

  Copy the input to the end of the buffer 
  ====================*/
template <uint BUFFER_SIZE>
inline
bool	CBufferFixed<BUFFER_SIZE>::Append(const void *pBuffer, uint uiSize)
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
  CBufferFixed::Seek
  ====================*/
template <uint BUFFER_SIZE>
inline
bool CBufferFixed<BUFFER_SIZE>::AppendNulls(uint uiNewMaxIndex)
{
	if (uiNewMaxIndex < m_uiSize)
	{
		// Allocate buffer space for [0 through NewMaxIndex]
		uint uiCopyLen(uiNewMaxIndex - m_uiEnd + 1); 

		// Zero the buffer section
		memset(&m_pBuffer[m_uiEnd], 0, uiCopyLen);
		m_uiEnd += uiCopyLen;
		return true;
	}
	else
	{
		return false;
	}
}


/*====================
  CBufferFixed::Seek
  ====================*/
template <uint BUFFER_SIZE>
inline
bool	CBufferFixed<BUFFER_SIZE>::Seek(uint uiPos)
{
	if (uiPos <= m_uiEnd)
	{
		m_uiRead = uiPos;
		return true;
	}

	return false;
}


/*====================
  CBufferFixed::Read
  ====================*/
template <uint BUFFER_SIZE>
inline
bool	CBufferFixed<BUFFER_SIZE>::Read(void *pBuffer, uint uiSize) const
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
  CBufferFixed::Lock

  return a writable region of the buffer from the current write position
  ====================*/
template <uint BUFFER_SIZE>
inline
char*	CBufferFixed<BUFFER_SIZE>::Lock(uint uiSize)
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
  CBufferFixed::Advance
  ====================*/
template <uint BUFFER_SIZE>
inline
bool	CBufferFixed<BUFFER_SIZE>::Advance(uint uiSize) const
{
	if (uiSize == 0)
		return true;

	bool ret(true);

	if (m_uiRead + uiSize > m_uiEnd)
	{
		m_iFaults |= BUFFER_FAULT_OVERRUN;
		ret = false;
	}
	else
	{
		m_uiRead += uiSize;
	}

	return ret;
}

//=============================================================================
#endif

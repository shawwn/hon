// (C)2005 S2 Games
// c_packet.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "chatserver_common.h"

#include "c_packet.h"
#include "c_buffer.h"
#include "c_console.h"
//=============================================================================

/*====================
  CPacket::CPacket
  ====================*/
CPacket::CPacket() :
m_bOverflowed(false)
{
}

CPacket::CPacket(const CPacket &pkt) :
m_Buffer(pkt.m_Buffer),
m_bOverflowed(pkt.m_bOverflowed)
{
}

CPacket::CPacket(const char *pBuffer, uint uiLength) :
m_bOverflowed(false)
{
	Write(pBuffer, uiLength);
}


/*====================
  CPacket::WriteInt64
  ====================*/
bool	CPacket::WriteInt64(LONGLONG ll)
{
	assert(sizeof(LONGLONG) == 8);

	m_Buffer << LittleInt64(ll);
	if (m_Buffer.GetFaults() & BUFFER_FAULT_OVERRUN)
	{
		m_bOverflowed = true;
		return false;
	}

	return true;
}


/*====================
  CPacket::WriteInt
  ====================*/
bool	CPacket::WriteInt(int i)
{
	assert(sizeof(int) == 4);

	try
	{
		m_Buffer << LittleInt(i);
		if (m_Buffer.GetFaults() & BUFFER_FAULT_OVERRUN)
			EX_ERROR(L"Exceeded MAX_PACKET_SIZE while writing int " + XtoA(i));
		return true;
	}
	catch (CException &ex)
	{
		ex.Process(L"CPacket::WriteInt() - ", NO_THROW);
		m_bOverflowed = true;
		return false;
	}
}


/*====================
  CPacket::WriteFloat
  ====================*/
bool	CPacket::WriteFloat(float f)
{
	assert(sizeof(float) == 4);

	try
	{
		m_Buffer << LittleFloat(f);
		if (m_Buffer.GetFaults() & BUFFER_FAULT_OVERRUN)
			EX_ERROR(_T("Exceeded MAX_PACKET_SIZE while writing float ") + XtoA(f));
		return true;
	}
	catch (CException &ex)
	{
		ex.Process(_T("CPacket::WriteFloat() - "), NO_THROW);
		m_bOverflowed = true;
		return false;
	}
}


/*====================
  CPacket::WriteShort
  ====================*/
bool	CPacket::WriteShort(short n)
{
	assert(sizeof(short) == 2);

	try
	{
		m_Buffer << LittleShort(n);
		if (m_Buffer.GetFaults() & BUFFER_FAULT_OVERRUN)
			EX_ERROR(_T("Exceeded MAX_PACKET_SIZE while writing short ") + XtoA(n));
		return true;
	}
	catch (CException &ex)
	{
		ex.Process(_T("CPacket::WriteShort() - "), NO_THROW);
		m_bOverflowed = true;
		return false;
	}
}


/*====================
  CPacket::WriteByte
  ====================*/
bool	CPacket::WriteByte(byte y)
{
	assert(sizeof(byte) == 1);

	try
	{
		m_Buffer << y;
		if (m_Buffer.GetFaults() & BUFFER_FAULT_OVERRUN)
			EX_ERROR(_T("Exceeded MAX_PACKET_SIZE while writing byte ") + XtoA(int(y)));
		return true;
	}
	catch (CException &ex)
	{
		ex.Process(_T("CPacket::WriteByte() - "), NO_THROW);
		m_bOverflowed = true;
		return false;
	}
}


/*====================
  CPacket::WriteString
  ====================*/
bool	CPacket::WriteString(const string &s)
{
	m_Buffer << StringToUTF8(s) << byte(0);
	if (m_Buffer.GetFaults() & BUFFER_FAULT_OVERRUN)
	{
		m_bOverflowed = true;
		return false;
	}

	return true;
}

bool	CPacket::WriteString(const wstring &s)
{
	m_Buffer << WStringToUTF8(s) << byte(0);
	if (m_Buffer.GetFaults() & BUFFER_FAULT_OVERRUN)
	{
		m_bOverflowed = true;
		return false;
	}

	return true;
}


/*====================
  CPacket::WriteUTF8String
  ====================*/
bool	CPacket::WriteUTF8String(const string &s)
{
	m_Buffer << s << byte(0);
	if (m_Buffer.GetFaults() & BUFFER_FAULT_OVERRUN)
	{
		m_bOverflowed = true;
		return false;
	}

	return true;
}


/*====================
  CPacket::Write
  ====================*/
bool	CPacket::Write(const char *data, uint uiSize)
{
	try
	{
		m_Buffer.Append(data, uiSize);
		if (m_Buffer.GetFaults() & BUFFER_FAULT_OVERRUN)
			EX_ERROR(_T("Exceeded MAX_PACKET_SIZE while writing data: \"") + XtoA(data) + _T("\", length: ") + XtoA(uiSize));
		return true;
	}
	catch (CException &ex)
	{
		ex.Process(_T("CPacket::Write() - "), NO_THROW);
		m_bOverflowed = true;
		return false;
	}
}


/*====================
  CPacket::operator<<
  ====================*/
CPacket&	CPacket::operator<<(const IBuffer &buffer)
{
	if (!Write(buffer.Get(), buffer.GetLength()))
		EX_ERROR(_T("CPacket::operator<<() - Packet overflowed"));
	return *this;
}


/*====================
  CPacket::ReadInt
  ====================*/
int		CPacket::ReadInt(int iFailed)
{
	try
	{
		int i;
		m_Buffer >> i;
		if (m_Buffer.GetFaults() & BUFFER_FAULT_UNDERRUN)
			EX_ERROR(_T("Attempted to read past end of packet"));
		return LittleInt(i);
	}
	catch (CException &ex)
	{
		ex.Process(_T("CPacket::ReadInt() - "), NO_THROW);
		return iFailed;
	}
}


/*====================
  CPacket::ReadInt64
  ====================*/
LONGLONG	CPacket::ReadInt64(LONGLONG llFailed)
{
	LONGLONG ll;
	m_Buffer >> ll;
	if (m_Buffer.GetFaults() & BUFFER_FAULT_UNDERRUN)
		return llFailed;

	return LittleInt64(ll);
}


/*====================
  CPacket::ReadShort
  ====================*/
short	CPacket::ReadShort(short nFailed)
{
	short n(m_Buffer.ReadShort());
	if (m_Buffer.GetFaults() & BUFFER_FAULT_UNDERRUN)
		return nFailed;

	return n;
}


/*====================
  CPacket::ReadByte
  ====================*/
byte	CPacket::ReadByte(byte yFailed)
{
	byte y(m_Buffer.ReadByte());
	if (m_Buffer.GetFaults() & BUFFER_FAULT_UNDERRUN)
		return yFailed;

	return y;
}


/*====================
  CPacket::ReadUTF8String
  ====================*/
string	CPacket::ReadUTF8String(const string &sFailed)
{
	uint uiEnd(m_Buffer.FindNext('\x00'));
	if (uiEnd == INVALID_INDEX)
		return sFailed;

	uint uiPos(m_Buffer.GetReadPos());
	m_Buffer.Seek(uiEnd + 1);
	return m_Buffer.Get(uiPos);
}


/*====================
  CPacket::ReadString
  ====================*/
string	CPacket::ReadString(const string &sFailed)
{
	uint uiEnd(m_Buffer.FindNext('\x00'));
	if (uiEnd == INVALID_INDEX)
		return sFailed;

	uint uiPos(m_Buffer.GetReadPos());
	m_Buffer.Seek(uiEnd + 1);
	return UTF8ToString(m_Buffer.Get(uiPos));
}


/*====================
  CPacket::ReadWString
  ====================*/
wstring	CPacket::ReadWString(const wstring &sFailed)
{
	uint uiEnd(m_Buffer.FindNext('\x00'));
	if (uiEnd == INVALID_INDEX)
		return sFailed;

	uint uiPos(m_Buffer.GetReadPos());
	m_Buffer.Seek(uiEnd + 1);
	return UTF8ToWString(m_Buffer.Get(uiPos));
}


/*====================
  CPacket::Read
  ====================*/
size_t	CPacket::Read(char *out, uint uiSize)
{
	try
	{
		m_Buffer.Read(out, uiSize);
		if (m_Buffer.GetFaults() & BUFFER_FAULT_UNDERRUN)
			EX_ERROR(_T("Attempted to read past end of packet"));
		return uiSize;
	}
	catch (CException &ex)
	{
		ex.Process(_T("CPacket::Read() - "), NO_THROW);
		return 0;
	}
}


/*====================
  CPacket::ReadFloat
  ====================*/
float	CPacket::ReadFloat(float fFailed)
{
	int iValue(ReadInt(static_cast<int>(fFailed)));
	return FLOAT_CAST(iValue);
}


/*====================
  CPacket::operator=
  ====================*/
CPacket	 &CPacket::operator=(const CPacket &from)
{
	m_Buffer = from.m_Buffer;
	m_bOverflowed = from.m_bOverflowed;
	return *this;
}

// (C)2005 S2 Games
// c_packet.h
//
//=============================================================================
#ifndef __C_PACKET_H__
#define __C_PACKET_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_buffer.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
const size_t MAX_PACKET_SIZE(8192);
//=============================================================================

//=============================================================================
// CPacket
//=============================================================================
class CPacket
{
private:
	CBufferFixed<MAX_PACKET_SIZE>	m_Buffer;
	bool							m_bOverflowed;

public:
	~CPacket()	{}
	CPacket();
	CPacket(const CPacket &pkt);
	CPacket(const char *pBuffer, uint uiLength);

	CPacket &operator=(const CPacket &c);

	inline void			Clear()										{ m_Buffer.Clear(); m_bOverflowed = false; }
	inline bool			IsEmpty() const								{ return m_Buffer.GetLength() == 0; }
	inline uint			GetLength() const							{ return m_Buffer.GetLength(); }
	inline uint			GetUnreadLength() const						{ return m_Buffer.GetUnreadLength(); }
	inline uint			GetRemainingSpace() const					{ return INT_SIZE(m_Buffer.GetSize()) - m_Buffer.GetLength(); }
	inline bool			DoneReading() const							{ return (m_Buffer.GetUnreadLength() == 0 || GetReadPos() > GetLength()); }
	inline char*		LockBuffer(uint uiSize = MAX_PACKET_SIZE)	{ return m_Buffer.Lock(uiSize); }
	inline void			SetLength(uint uiLength)					{ return m_Buffer.SetLength(uiLength); }
	inline bool			HasFaults() const							{ return (m_Buffer.GetFaults() != 0); }
	inline uint			GetReadPos() const							{ return m_Buffer.GetReadPos(); }
	inline void			Advance(uint uiCount)						{ m_Buffer.Advance(uiCount); }

	inline const char*	GetBuffer() const							{ return m_Buffer.Get(m_Buffer.GetReadPos()); }
	inline const char*	GetBuffer(uint uiOffset) const				{ return m_Buffer.Get(uiOffset); }

	inline byte			operator[](int i) const						{ return m_Buffer[i]; }

	bool	Write(const char *data, uint uiSize);
	bool	WriteInt64(LONGLONG ll);
	bool	WriteInt(int i);
	bool	WriteShort(short n);
	bool	WriteByte(byte y);
	bool	WriteString(const string &s);
	bool	WriteString(const wstring &s);
	bool	WriteUTF8String(const string &s);
	bool	WriteFloat(float f);

#define PKT_WRITE(t, x)	if (!Write##t(x)) EX_ERROR(L"CPacket overflowed"); return *this;
	inline CPacket&	operator<<(LONGLONG ll)			{ PKT_WRITE(Int64, ll) }
	inline CPacket&	operator<<(int i)				{ PKT_WRITE(Int, i) }
	inline CPacket&	operator<<(uint ui)				{ PKT_WRITE(Int, ui) }
	inline CPacket&	operator<<(short n)				{ PKT_WRITE(Short, n) }
	inline CPacket&	operator<<(ushort un)			{ PKT_WRITE(Short, un) }
	inline CPacket&	operator<<(byte y)				{ PKT_WRITE(Byte, y) }
	inline CPacket&	operator<<(const char *sz)		{ PKT_WRITE(String, sz) }
	inline CPacket&	operator<<(const string &s)		{ PKT_WRITE(String, s) }
	inline CPacket&	operator<<(const wstring &s)	{ PKT_WRITE(String, s) }
	inline CPacket&	operator<<(float f)				{ PKT_WRITE(Float, f) }
#undef PK_WRITE
	CPacket&	operator<<(const IBuffer &buffer);

	size_t		Read(char *out, uint uiSize);
	int			ReadInt(int iFailed = 0);
	LONGLONG	ReadInt64(LONGLONG llFailed = 0);
	short		ReadShort(short nFailed = 0);
	byte		ReadByte(byte yFailed = 0);
	string		ReadUTF8String(const string &sFailed = SNULL);
	string		ReadString(const string &sFailed = SNULL);
	wstring		ReadWString(const wstring &sFailed = WSNULL);
	float		ReadFloat(float fFailed = 0.0f);

	inline CPacket&	operator>>(int &i)				{ i = ReadInt(); return *this; }
	inline CPacket&	operator>>(uint &ui)			{ ui = ReadInt(); return *this; }
	inline CPacket&	operator>>(short &n)			{ n = ReadShort(); return *this; }
	inline CPacket&	operator>>(ushort &un)			{ un = ReadShort(); return *this; }
	inline CPacket&	operator>>(byte &y)				{ y = ReadByte(); return *this; }
	inline CPacket&	operator>>(string &s)			{ s = ReadString(); return *this; }
	inline CPacket&	operator>>(wstring &s)			{ s = ReadWString(); return *this; }
	inline CPacket&	operator>>(float &f)			{ f = ReadFloat(); return *this; }
};
//=============================================================================

#endif	//__C_PACKET_H__

// (C)2008 S2 Games
// c_outputstream.h
//
//=============================================================================
#ifndef __C_OUTPUTSTREAM_H__
#define __C_OUTPUTSTREAM_H__

//=============================================================================
// Definitions
//=============================================================================
const uint STREAM_TARGET_NONE		(0);
const uint STREAM_TARGET_CONSOLE	(BIT(0));
const uint STREAM_TARGET_DEBUG		(BIT(1));
const uint STREAM_TARGET_FILE		(BIT(2));
//=============================================================================

//=============================================================================
// COutputStream
//=============================================================================
class COutputStream
{
private:
	class CSystem*	m_pSystem;
	uint			m_uiTargets;
	CFileHandle		m_hLogFile;

	COutputStream();

public:
	~COutputStream();
	COutputStream(CSystem *pSystem, uint uiTargets, const wstring &sFileName = WSNULL);

	void			SetLogFile(const wstring &sFileName);
	void			CloseLogFile();

	inline void		AddOutputHistory(const string &sLine)	{ AddOutputHistory(SingleToWide(sLine)); }
	void			AddOutputHistory(const wstring &sLine);

	COutputStream&	operator<<(const char *sz)			{ AddOutputHistory(sz); return *this; }
	COutputStream&	operator<<(const wchar_t *sz)		{ AddOutputHistory(sz); return *this; }
	COutputStream&	operator<<(const string &s)			{ AddOutputHistory(s); return *this; }
	COutputStream&	operator<<(const wstring &s)		{ AddOutputHistory(s); return *this; }
	COutputStream&	operator<<(const void *p)			{ *this << XtoW(p); return *this; }
	COutputStream&	operator<<(bool b)					{ *this << XtoW(b); return *this; }
	COutputStream&	operator<<(int i)					{ *this << XtoW(i); return *this; }
	COutputStream&	operator<<(long l)					{ *this << XtoW(l); return *this; }
	COutputStream&	operator<<(unsigned int ui)			{ *this << XtoW(ui); return *this; }
	COutputStream&	operator<<(unsigned long ul)		{ *this << XtoW(ul); return *this; }
	COutputStream&	operator<<(float f)					{ *this << XtoW(f); return *this; }
	COutputStream&	operator<<(double d)				{ *this << XtoW(d); return *this; }
	COutputStream&	operator<<(LONGLONG ll)				{ *this << XtoW(ll); return *this; }
	COutputStream&	operator<<(ULONGLONG ull)			{ *this << XtoW(ull); return *this; }
	COutputStream&	operator<<(const CVec2f &v2)		{ *this << XtoW(v2); return *this; }
	COutputStream&	operator<<(const CVec3f &v3)		{ *this << XtoW(v3); return *this; }
	COutputStream&	operator<<(const CVec4f &v4)		{ *this << XtoW(v4); return *this; }
};
//=============================================================================

#endif // __C_OUTPUTSTREAM_H__
// (C)2008 S2 Games
// c_filehandle.h
//
//=============================================================================
#ifndef __C_FILEHANDLE_H__
#define __C_FILEHANDLE_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_file.h"
#include "xtoa.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
enum EFileType
{
	FILE_INVALID,
	FILE_DISK,		// A regular file on the hard drive
	FILE_ARCHIVE,	// A file located inside an s2z archive
	FILE_HTTP,		// A file that is being retrieved from a remote url

	NUM_FILE_TYPES
};

struct SFileBlock
{
	char	szName[5];
	size_t	zPos;		// Position in buffer
	uint	uiLength;
	byte	*pData;		// Data pointer
};

typedef vector<SFileBlock> FileBlockList;
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class IBuffer;
class CFile;
//=============================================================================

//=============================================================================
// CFileHandle
// Provides a generic interface for accessing files of any type
//=============================================================================
class CFileHandle
{
	friend class CFile;
	friend class CFileDisk;

private:
	CFile*	m_pFile;

	CFile*	GetFile(const wstring &sFilename, int iMode);

public:
	~CFileHandle()	{ Close(); }
	CFileHandle();
	CFileHandle(const string &sPath, int iMode);
	CFileHandle(const wstring &sPath, int iMode);

	inline bool				Open(const string &sFilePath, int iMode)		{ return Open(SingleToWide(sFilePath), iMode); }
	bool					Open(const wstring &sFilePath, int iMode);
	void					Close();
	inline bool				IsOpen() const									{ return m_pFile ? m_pFile->IsOpen() : false; }
	inline bool				IsEOF() const									{ return m_pFile ? m_pFile->IsEOF() : true; }

	inline string			ReadLine()										{ return m_pFile ? m_pFile->ReadLine() : SNULL; }
	inline wstring			ReadLineW()										{ return m_pFile ? m_pFile->ReadLineW() : WSNULL; }
	inline bool				WriteLine(const string &sText)					{ return WriteString(sText + LINEBREAK); }
	inline bool				WriteLine(const wstring &sText)					{ return WriteString(sText + WLINEBREAK); }

	int						Read(char *pBuffer, int iSize) const;
	int						Read(IBuffer &cBuffer, int iSize) const;
	size_t					Write(const void *pBuffer, size_t size);

	inline const char*		GetBuffer(uint &uiSize)							{ return m_pFile ? m_pFile->GetBuffer(uiSize) : NULL; }

	inline const wstring&	GetPath()										{ return m_pFile ? m_pFile->GetPath() : WSNULL ; }

	inline byte				ReadByte()										{ return m_pFile ? m_pFile->ReadByte() : 0; }
	inline short			ReadInt16(bool bBigEndian = false)				{ return m_pFile ? m_pFile->ReadInt16(bBigEndian) : 0; }
	inline int				ReadInt32(bool bBigEndian = false)				{ return m_pFile ? m_pFile->ReadInt32(bBigEndian) : 0; }
	inline LONGLONG			ReadInt64(bool bBigEndian = false)				{ return m_pFile ? m_pFile->ReadInt64(bBigEndian) : 0; }
	inline float			ReadFloat(bool bBigEndian = false)				{ return m_pFile ? m_pFile->ReadFloat(bBigEndian) : 0.0f; }
	inline string			ReadString()									{ return m_pFile ? m_pFile->ReadString() : SNULL; }
	inline wstring			ReadWString()									{ return m_pFile ? m_pFile->ReadWString() : WSNULL; }

	inline size_t			Tell()														{ return m_pFile ? m_pFile->Tell() : 0; }
	inline bool				Seek(int iOffset, ESeekOrigin eOrigin = SEEK_ORIGIN_START)	{ return m_pFile ? m_pFile->Seek(iOffset, eOrigin) : false; }
	inline size_t			GetLength()													{ return m_pFile ? m_pFile->GetLength() : 0; }

	// TODO: Make all writing functions differentiate between text and binary mode
	inline bool				WriteByte(char c)									{ return m_pFile ? m_pFile->WriteByte(c) : false; }
	inline bool				WriteInt16(short n, bool bBigEndian = false)		{ return m_pFile ? m_pFile->WriteInt16(n, bBigEndian) : false; }
	inline bool				WriteInt32(int i, bool bBigEndian = false)			{ return m_pFile ? m_pFile->WriteInt32(i, bBigEndian) : false; }
	inline bool				WriteInt64(LONGLONG ll, bool bBigEndian = false)	{ return m_pFile ? m_pFile->WriteInt64(ll, bBigEndian) : false; }
	inline bool				WriteFloat(float f, bool bBigEndian = false)		{ return WriteInt32(*reinterpret_cast<int*>(&f), bBigEndian); }
	inline bool				WriteByte(byte c)									{ return WriteByte(char(c)); }
	inline bool				WriteInt16(ushort un, bool bBigEndian = false)		{ return WriteInt16(short(un), bBigEndian); }
	inline bool				WriteInt32(uint ui, bool bBigEndian = false)		{ return WriteInt32(int(ui), bBigEndian); }
	inline bool				WriteInt64(ULONGLONG ull, bool bBigEndian = false)	{ return WriteInt64(LONGLONG(ull), bBigEndian); }
	bool					WriteString(const string &s);
	bool					WriteString(const wstring &s);

	CFileHandle&			operator<<(const char *sz)						{ WriteString(sz); return *this; }
	CFileHandle&			operator<<(const wchar_t *sz)					{ WriteString(sz); return *this; }
	CFileHandle&			operator<<(const string &s)						{ WriteString(s); return *this; }
	CFileHandle&			operator<<(const wstring &s)					{ WriteString(s); return *this; }
	CFileHandle&			operator<<(char c)								{ Write(&c, sizeof(char)); return *this; }
	CFileHandle&			operator<<(wchar_t c)							{ Write(&c, sizeof(wchar_t)); return *this; }
	CFileHandle&			operator<<(int i)								{ *this << XtoA(i); return *this; }
	CFileHandle&			operator<<(long l)								{ *this << XtoA(l); return *this; }
	CFileHandle&			operator<<(uint ui)								{ *this << XtoA(ui); return *this; }
	CFileHandle&			operator<<(unsigned long ul)					{ *this << XtoA(ul); return *this; }
	CFileHandle&			operator<<(LONGLONG ll)							{ *this << XtoA(ll); return *this; }
	CFileHandle&			operator<<(float f)								{ *this << XtoA(f); return *this; }
	CFileHandle&			operator<<(double d)							{ *this << XtoA(d); return *this; }

	inline void				Flush()											{ if (m_pFile != NULL) m_pFile->Flush(); }

	bool					CheckHeader(const char *pHeader, uint uiHeaderSize);
	bool					BuildBlockList(const char *pHeader, uint uiHeaderSize, FileBlockList &vBlockList);
};
//=============================================================================

#endif //__C_FILEHANDLE_H__

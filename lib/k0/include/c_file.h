// (C)2008 S2 Games
// c_file.h
//
//=============================================================================
#ifndef __C_FILE_H__
#define __C_FILE_H__

//=============================================================================
// Definitions
//=============================================================================
const int FILE_READ				(BIT(0));
const int FILE_WRITE			(BIT(1));
const int FILE_TRUNCATE			(BIT(2));
const int FILE_APPEND			(BIT(3));
const int FILE_BUFFER			(BIT(4));
const int FILE_NOBUFFER			(BIT(5));
const int FILE_BINARY			(BIT(6));
const int FILE_TEXT				(BIT(7));
const int FILE_CREATE			(BIT(8));
const int FILE_BLOCK			(BIT(9));
const int FILE_NOBLOCK			(BIT(10));
const int FILE_ASCII			(BIT(11));
const int FILE_UTF8				(BIT(12));
const int FILE_UTF16			(BIT(13));
const int FILE_UTF32			(BIT(14));
const int FILE_LITTLE_ENDIAN	(BIT(15));
const int FILE_BIG_ENDIAN		(BIT(16));
const int FILE_HTTP_WRITETOFILE	(BIT(17));
const int FILE_HTTP_RESUME		(BIT(18));
const int FILE_NOUSERDIR		(BIT(19));
const int FILE_NOARCHIVES		(BIT(20));
const int FILE_TOPMODONLY		(BIT(21));
const int FILE_HTTP_UPLOAD		(BIT(22));
const int FILE_HTTP_GETSIZE		(BIT(23));
const int FILE_HTTP_POST		(BIT(24));
const int FILE_NOWORLDARCHIVE	(BIT(25));
const int FILE_COMPRESS			(BIT(26));
const int FILE_NOCOMPRESS		(BIT(27));
const int FILE_SILENT			(BIT(28));	// Silent files don't complain if they don't open

const int FILE_TEXT_ENCODING_MASK	(FILE_ASCII | FILE_UTF8 | FILE_UTF16 | FILE_UTF32);
const int FILE_ENDIAN_MASK			(FILE_LITTLE_ENDIAN | FILE_BIG_ENDIAN);

enum ESeekOrigin
{
	SEEK_ORIGIN_START,
	SEEK_ORIGIN_END,
	SEEK_ORIGIN_CURRENT
};

#ifdef UNICODE
const int FILE_DEFAULT_TEXT_ENCODING	(FILE_UTF16 | FILE_LITTLE_ENDIAN);
#else
const int FILE_DEFAULT_TEXT_ENCODING	(FILE_ASCII);
#endif

#if defined(linux) || defined(__APPLE__) || defined(__FreeBSD__)
#define _S_IFDIR S_IFDIR
#define _stat stat
#endif
//=============================================================================

//=============================================================================
// CFile
// Base class for different file types
//=============================================================================
class CFile
{
private:

protected:
	char*					m_pBuffer;
	int						m_iMode;
	uint					m_uiSize;
	mutable uint			m_uiPos;				// for the const read functions
	mutable bool			m_bEOF;
	wstring					m_sPath;
	wstring					m_sGamePath;

public:
	virtual ~CFile()	{}
	CFile();

	virtual bool		Open(const wstring &sPath, int iMode) = 0;
	virtual void		Close() = 0;
	virtual	bool		IsOpen() const = 0;
	inline bool			IsEOF()	const							{ return m_bEOF; };
	inline bool			IsBigEndian() const						{ return (m_iMode & FILE_BIG_ENDIAN) == FILE_BIG_ENDIAN; }

	inline const wstring&	GetPath() const						{ return m_sPath; }

	virtual string		ReadLine() = 0;
	virtual wstring		ReadLineW() = 0;
	virtual char		Peek(uint uiPos);
	virtual uint		Read(char *pBuffer, uint uiBufferSize) const = 0;
	virtual size_t		Write(const void *pBuffer, size_t iBufferSize) = 0;

	virtual const char*	GetBuffer(uint &uiSize) = 0;

	byte				ReadByte();
	short				ReadInt16(bool bUseBigEndian = false);
	int					ReadInt32(bool bUseBigEndian = false);
	LONGLONG			ReadInt64(bool bUseBigEndian = false);
	float				ReadFloat(bool bUseBigEndian = false);
	string				ReadString();
	wstring				ReadWString();

	uint				GetBufferSize()							{ return m_uiSize; }
	uint				GetUnreadLength()						{ return m_uiSize - m_uiPos; }

	virtual uint		Tell() const							{ return m_uiPos; }
	virtual bool		Seek(uint uiOffset, ESeekOrigin eOrigin = SEEK_ORIGIN_START);
	virtual size_t		GetLength() const						{ return size_t(m_uiSize); }

	virtual bool		WriteByte(char c) = 0;
	virtual bool		WriteInt16(short c, bool bUseBigEndian) = 0;
	virtual bool		WriteInt32(int c, bool bUseBigEndian) = 0;
	virtual bool		WriteInt64(LONGLONG c, bool bUseBigEndian) = 0;
	virtual bool		WriteString(const string &sText) = 0;
	virtual bool		WriteString(const wstring &sText) = 0;

	virtual void		Flush() {}
};
//=============================================================================

#endif //__C_FILE_H__

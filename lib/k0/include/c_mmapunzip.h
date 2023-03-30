// (C)2008 S2 Games
// c_mmapunzip.h
// Memory mapped based unzipper
//=============================================================================
#ifndef __C_MMAPUNZIP_H__
#define __C_MMAPUNZIP_H__

//=============================================================================
// Headers
//=============================================================================
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif
#include "c_exception.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
// This is required for a proper fread, but lowers performance very slightly.
// The following structures are read directly from the zip/s2z
//-----------------------------------------------------------------------------
#ifdef _WIN32
#pragma pack(push, ali1, 1)
#else
#pragma pack(push, 1)
#endif

struct SArchiveLocalInfo
{
	unsigned int signature;
	unsigned short minVersion;
	unsigned short generalFlags;
	unsigned short compression;
	unsigned short lastModTime;
	unsigned short lastModDate;
	unsigned int crc32;
	unsigned int compressed;
	unsigned int uncompressed;
	unsigned short filenameLength;
	unsigned short extraLength;
};

struct SArchiveCentralInfo
{
	unsigned int signature;
	unsigned short diskNum;
	unsigned short diskNum2;
	unsigned short numEntries;
	unsigned short numEntries2;
	unsigned int centralDirSize;
	unsigned int centralDirOffset;
	unsigned short commentLength;
};

struct SArchiveFileInfo
{
	unsigned int signature;
	unsigned short version;
	unsigned short version2;
	unsigned short bitflags;
	unsigned short compression;
	unsigned short lastmodTime;
	unsigned short lastmodDate;
	unsigned int crc32;
	unsigned int compressed;
	unsigned int uncompressed;
	unsigned short filenameLength;
	unsigned short extraLength;
	unsigned short commentLength;
	unsigned short diskNumberStart;
	unsigned short internalAttrib;
	unsigned int externalAttrib;
	unsigned int relativeOffset;
};

#ifdef _WIN32
#pragma pack(pop, ali1)
#else
#pragma pack(pop)
#endif

// This structure is what's referenced in our hashmap
struct SZippedFile
{
	const char*	pBuffer;
	bool		bCompressed;
	uint		uiSize;
	uint		uiRawSize;
	uint		uiCRC32;
};

typedef vector<SZippedFile>			ZFVector;
typedef ZFVector::iterator			ZFVector_it;
typedef ZFVector::const_iterator	ZFVector_cit;
typedef ZFVector::reverse_iterator	ZFVector_rit;

typedef hash_map<tstring, SZippedFile*> ZFMap;
typedef pair<tstring, SZippedFile*>		ZFMap_pair;
typedef ZFMap::iterator					ZFMap_it;
typedef ZFMap::const_iterator			ZFMap_cit;

#define MAX_KEEPINMEM_SIZE	0

class CCompressedFile;
//=============================================================================

//=============================================================================
// CMMapUnzip
//=============================================================================
class CMMapUnzip
{
private:
	// Here's our hashmap and the associated private function to add files to it
	ZFMap		m_mapFiles;
	tsvector	m_vFileNames;

	// The Operating System-specific variables and functions because HANDLE is used under Windows...
	#ifdef _WIN32
	HANDLE			m_hFile;
	HANDLE			m_hMappedFile;
	#else
	int				m_iFile;
	#endif

	const char*		m_pData;
	uint			m_uiSize;

	bool			m_bMemory;

	void	Initialize();
	uint	SearchCentralDir();
	void	AddZippedFile(const tstring &sFilename, SArchiveFileInfo* info);

public:
	~CMMapUnzip();
	CMMapUnzip();

	bool		Open(const tstring &sPath);
	bool		Open(const char *pBuffer, uint uiSize);
	void		Close();

	// Returns whether the unzip was successful.
	inline bool				IsOpen() const							{ return (m_pData != NULL); }

	// Check if a specific file exists, or prevent it from being preloaded, etc.
	inline const tsvector&	GetFileList()							{ return m_vFileNames; }
	inline bool				FileExists(const tstring &sFilename)	{ return m_mapFiles.find(LowerString(sFilename)) != m_mapFiles.end(); }

	// Possibly the most important function of them all: the one which actually opens files!
	uint		OpenUnzipFile(const tstring &sFilename, char *&pBuffer);

	uint		GetCompressedFile(const tstring &sFilename, CCompressedFile &cFile);
};
//=============================================================================

//=============================================================================
// CCompressedFile
//=============================================================================
class CCompressedFile
{
private:
	const char *m_pData;
	
	uint	m_uiCRC32;
	uint	m_uiStoredSize;
	uint	m_uiRawSize;
	int		m_iLevel;

public:
	~CCompressedFile()	{}
	CCompressedFile()
		: m_pData(NULL)
		, m_uiCRC32(0)
		, m_uiStoredSize(0)
		, m_uiRawSize(0)
		, m_iLevel(0)
	{}
	CCompressedFile(const char *pData, uint uiCRC32, uint uiStoredSize, uint uiRawSize, int iLevel)
		: m_pData(pData)
		, m_uiCRC32(uiCRC32)
		, m_uiStoredSize(uiStoredSize)
		, m_uiRawSize(uiRawSize)
		, m_iLevel(iLevel)
	{}

	const char*		GetData() const			{ return m_pData; }
	uint			GetCRC32() const		{ return m_uiCRC32; }
	uint			GetStoredSize() const	{ return m_uiStoredSize; }
	uint			GetRawSize() const		{ return m_uiRawSize; }
	int				GetLevel() const		{ return m_iLevel; }
};
//=============================================================================

#endif //__C_MMAPUNZIP_H__

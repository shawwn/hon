// (C)2008 S2 Games
// c_mmapunzip.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k0_common.h"

#include "c_mmapunzip.h"

#include "../ext/zlib/zlib.h"
//=============================================================================

/*====================
  CMMapUnzip::AddZippedFile
  ====================*/
void	CMMapUnzip::AddZippedFile(const tstring &sFilename, SArchiveFileInfo* info)
{
	if (sFilename.empty())
		return;

	// Ignore directory entries
	if (sFilename[sFilename.length() - 1] == _T('/'))
		return;

	SZippedFile *pFile(new SZippedFile);
	pFile->pBuffer = &m_pData[LittleInt(info->relativeOffset) + sizeof(SArchiveLocalInfo) + LittleShort(info->filenameLength) + LittleShort(info->extraLength)];
	pFile->bCompressed = (LittleShort(info->compression) != 0);
	pFile->uiSize = LittleInt(info->compressed);
	pFile->uiRawSize = LittleInt(info->uncompressed);
	pFile->uiCRC32 = LittleInt(info->crc32);

#if 0
	if (!pFile->bCompressed || pFile->uiRawSize == 1)
		K2System.DebugBreak();
#endif

	if (pFile->uiSize == -1)
	{
		pFile->uiSize = 0;
		EX_ERROR(_T("CMMapUnzip::AddZippedFile() - Invalid file length detected"));
	}

	if (pFile->uiRawSize == -1)
	{
		pFile->uiRawSize = 0;
		EX_ERROR(_T("CMMapUnzip::AddZippedFile() - Invalid file length detected"));
	}

	m_mapFiles.insert(ZFMap_pair(sFilename, pFile));
	m_vFileNames.push_back(sFilename);
}


/*====================
  CMMapUnzip::SearchCentralDir
  ====================*/
uint	CMMapUnzip::SearchCentralDir()
{
	// Central directory is at the end of the file
	// right before the global comment. The global
	// comment has a maximum size of 64k

	int iEnd(MAX<int>(m_uiSize - 66000, 3));

	// Scan for signature
	for (int i(m_uiSize - 1); i >= iEnd; --i)
	{
		if (m_pData[i - 0] == 0x06 &&
			m_pData[i - 1] == 0x05 &&
			m_pData[i - 2] == 0x4b &&
			m_pData[i - 3] == 0x50)
		{
			// Signature found; return its position
			return i - 3;
		}
	}
	
	return 0;
}


/*====================
  CMMapUnzip::~CMMapUnzip
  ====================*/
CMMapUnzip::~CMMapUnzip()
{
	Close();
}


/*====================
  CMMapUnzip::CMMapUnzip
  ====================*/
CMMapUnzip::CMMapUnzip()
: m_bMemory(false)
#ifdef _WIN32
, m_hFile(INVALID_HANDLE_VALUE)
, m_hMappedFile(INVALID_HANDLE_VALUE)
#else
, m_iFile(-1)
#endif
, m_pData(NULL)
, m_uiSize(0)
{
}


/*====================
  CMMapUnzip::CMMapUnzip
  ====================*/
bool	CMMapUnzip::Open(const tstring &sPath)
{
	bool bIsOpen(IsOpen());
	assert(!bIsOpen);
	if (bIsOpen)
		return false;

	assert(m_pData == NULL);
	m_pData = NULL;
	assert(m_uiSize == 0);
	m_uiSize = 0;

	// Setup memory mapping
	#ifdef _WIN32
	// Windows version

	assert(m_hFile == INVALID_HANDLE_VALUE);
	m_hFile = INVALID_HANDLE_VALUE;
	assert(m_hMappedFile == INVALID_HANDLE_VALUE);
	m_hMappedFile = INVALID_HANDLE_VALUE;

	m_hFile = CreateFile(sPath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);
	if	(m_hFile == INVALID_HANDLE_VALUE)
	{
		Close();
		return false;
	}

	// Determine the archive's filesize
	m_uiSize = GetFileSize(m_hFile, NULL);

	if (m_uiSize == 0)
	{
		Close();
		return false;
	}

	m_hMappedFile = CreateFileMapping(m_hFile, NULL, PAGE_READONLY, 0, 0, NULL);
	if (m_hMappedFile == INVALID_HANDLE_VALUE)
	{
		Close();
		return false;
	}

	m_pData = static_cast<const char *>(MapViewOfFile(m_hMappedFile, FILE_MAP_READ, 0, 0, 0));
	#else
	// Linux version
	m_iFile = open(sPath.c_str(), O_RDONLY);
	if (m_iFile == -1)
	{
		Close();
		return false;
	}
	
	// set files to close on exec
	long flags;
	if ((flags = fcntl(m_iFile, F_GETFD, 0)) == -1)
		flags = 0;

	if (fcntl(m_iFile, F_SETFD, flags | FD_CLOEXEC) == -1)
	{
		Close();
		return false;
	}

	// Determine the archive's filesize
	struct stat buf;
	if (fstat(m_iFile, &buf) == -1)
	{
		Close();
		return false;
	}

	m_uiSize = buf.st_size;

	if (m_uiSize == 0)
	{
		Close();
		return false;
	}

	m_pData = static_cast<const char *>(mmap(NULL, m_uiSize, PROT_READ, MAP_PRIVATE, m_iFile, 0));
	if (m_pData == static_cast<const char *>(MAP_FAILED))
	{
		m_pData = NULL;
		Close();
		return false;
	}
	#endif

	Initialize();
	return true;
}


/*====================
  CMMapUnzip::CMMapUnzip
  ====================*/
bool	CMMapUnzip::Open(const char *pBuffer, uint uiSize)
{
	bool bIsOpen(IsOpen());
	assert(!bIsOpen);
	if (bIsOpen)
		return false;

	m_bMemory = true;
	m_pData = pBuffer;
	m_uiSize = uiSize;

	if (m_pData == NULL || m_uiSize == 0)
	{
		Close();
		return false;
	}

	Initialize();
	return true;
}


/*====================
  CMMapUnzip::Close
  ====================*/
void	CMMapUnzip::Close()
{
	if (m_bMemory)
	{
		m_pData = NULL;
		m_uiSize = 0;
		m_bMemory = false;
	}
	else
	{
		for (ZFMap_it it(m_mapFiles.begin()), itEnd(m_mapFiles.end()); it != itEnd; ++it)
		{
			delete it->second;
		}
		m_mapFiles.clear();

		// Cleanup file mapping and handles
#ifdef _WIN32
		if (m_pData != NULL)
		{
			UnmapViewOfFile(m_pData);
			m_pData = NULL;
		}
		if (m_hMappedFile != INVALID_HANDLE_VALUE)
		{
			CloseHandle(m_hMappedFile);
			m_hMappedFile = INVALID_HANDLE_VALUE;
		}
		if (m_hFile != INVALID_HANDLE_VALUE)
		{
			CloseHandle(m_hFile);
			m_hFile = INVALID_HANDLE_VALUE;
		}
#else
		if(m_pData != NULL)
		{
			munmap((void*)m_pData, m_uiSize);
			m_pData = NULL;
		}
		if (m_iFile != -1)
		{
			close(m_iFile);
			m_iFile = -1;
		}
#endif
	}
}


/*====================
  CMMapUnzip::Initialize
  ====================*/
void	CMMapUnzip::Initialize()
{
	// Locate the the central directory
	uint uiDirPos(SearchCentralDir());
	if (uiDirPos == 0)
		return;

	// Check that the file hasn't been truncated
	if (m_uiSize - uiDirPos < sizeof(SArchiveCentralInfo))
		return;

	// Initialize the central directory
	const SArchiveCentralInfo *pCentralInfo(reinterpret_cast<const SArchiveCentralInfo*>(&m_pData[uiDirPos]));

	// Prepare the various temporary variables for the loop...
	uint uiPos(LittleInt(pCentralInfo->centralDirOffset));
	SArchiveFileInfo *pInfo(NULL);
	char szFilename[1024];

	while (true)
	{
		// Check that the file hasn't been truncated
		if (m_uiSize - uiPos < sizeof(SArchiveFileInfo))
			break;

		// Point to individual file info
		pInfo = (SArchiveFileInfo*)&m_pData[uiPos];
		uiPos += sizeof(SArchiveFileInfo);

		// Check if the signature matches; if not, the file headers are over (normal termination)
		if (LittleInt(pInfo->signature) != 0x02014b50)
			break;

		uint uiFilenameLength(MIN<uint>(LittleShort(pInfo->filenameLength), 1023));
		
		// Copy filename into temporary buffer
		memcpy(szFilename, &m_pData[uiPos], uiFilenameLength);
	
		// Convert the filename to lowercase
		szFilename[uiFilenameLength] = 0;
		for (uint ui(0); ui < uiFilenameLength; ++ui)
			szFilename[ui] = tolower(szFilename[ui]);

		// Advance
		uiPos += LittleShort(pInfo->filenameLength);
		uiPos += LittleShort(pInfo->extraLength);

		// Add file to the FileSystem (if some criterias are met, see function)
		AddZippedFile(StringToTString(szFilename), pInfo);
	}
}


/*====================
  CMMapUnzip::OpenUnzipFile
  ====================*/
uint	CMMapUnzip::OpenUnzipFile(const tstring &sFilename, char *&pBuffer)
{
	try
	{
		pBuffer = NULL;

		if (sFilename.empty())
			return 0;

		// Check the archive actually loaded properly by verifying the file member
		if (m_pData == NULL)
			return 0;

		// Convert the filename to lowercase and look it up in our hashmap
		tstring sLowerFilename(LowerString(sFilename));
		ZFMap_it file_entry(m_mapFiles.find(sLowerFilename));
		if (file_entry == m_mapFiles.end())
			return 0;

		// Ensure that we don't exceed the maximum array size somehow...
		if (file_entry->second->uiRawSize >= 0x7FFFFFFF)
		{
			EX_ERROR(_T("Unzip exceeded maximum array size!"));
			return 0;
		}

		// Allocate the buffer based on the uncompressed file size
		pBuffer = new char[file_entry->second->uiRawSize];

		// If the data is compressed, we need to uncompress it now (only zlib is supported)
		if (file_entry->second->bCompressed)
		{
			int ret;
			z_stream_s Dstrm;

			// Initialize the decompression stream's structure
			Dstrm.zalloc = Z_NULL;
			Dstrm.zfree = Z_NULL;
			Dstrm.opaque = Z_NULL;
			inflateInit2(&Dstrm, -15);
			ret = inflateReset(&Dstrm);
			if (ret < 0 || ret == 2)
			{
				// Failure, abort
				inflateEnd(&Dstrm);
				SAFE_DELETE_ARRAY(pBuffer);
				return 0;
			}

			// Give the structure information over the data's size and location
			Dstrm.next_in = (byte*)file_entry->second->pBuffer;
			Dstrm.next_out = (byte*)pBuffer;
			Dstrm.avail_in = file_entry->second->uiSize;
			Dstrm.avail_out = file_entry->second->uiRawSize;

			// This function will do the actual decompression
			ret = inflate(&Dstrm, Z_SYNC_FLUSH);
			if (ret < 0 || ret == 2)
			{
				// Failure, abort
				inflateEnd(&Dstrm);
				SAFE_DELETE_ARRAY(pBuffer);
				return 0;
			}

			// Finished decompressing data
			inflateEnd(&Dstrm);
		}
		else
		{
			// If the data isn't compressed, just copy it from buffer to buffer
			memcpy(pBuffer, file_entry->second->pBuffer, file_entry->second->uiRawSize);
		}

		// And finally, return the file's rawsize for the data in pBuffer
		return file_entry->second->uiRawSize;
	}
	catch (CException &ex)
	{
		ex.Process(_T("CMMapUnzip::OpenUnzipFile() - "), NO_THROW);
		return 0;
	}
}


/*====================
  CMMapUnzip::GetCompressedFile
  ====================*/
uint	CMMapUnzip::GetCompressedFile(const tstring &sFilename, CCompressedFile &cFile)
{
	try
	{
		if (sFilename.empty())
			return 0;

		// Check the archive actually loaded properly by verifying the file member
		if (m_pData == NULL)
			return 0;

		// Convert the filename to lowercase and look it up in our hashmap
		tstring sLowerFilename(LowerString(sFilename));
		if (sLowerFilename[0] == _T('/'))
			sLowerFilename = sLowerFilename.substr(1);
		ZFMap_it file_entry(m_mapFiles.find(sLowerFilename));
		if (file_entry == m_mapFiles.end())
			return 0;

		// Ensure that we don't exceed the maximum array size somehow...
		if (file_entry->second->uiRawSize >= 0x7FFFFFFF)
		{
			EX_ERROR(_T("Unzip exceeded maximum array size!"));
			return 0;
		}

		cFile = CCompressedFile
		(
			file_entry->second->pBuffer,
			file_entry->second->uiCRC32,
			file_entry->second->uiSize,
			file_entry->second->uiRawSize,
			file_entry->second->bCompressed ? 1 : 0
		);

		return file_entry->second->uiRawSize;
	}
	catch (CException &ex)
	{
		ex.Process(_T("CMMapUnzip::OpenUnzipFile() - "), NO_THROW);
		return 0;
	}
}

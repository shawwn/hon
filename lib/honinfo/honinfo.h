// (C)2010 S2 Games
// honinfo.h
//
//=============================================================================
#ifndef __HONINFO_H__
#define __HONINFO_H__

#include "honinfo_common.h"

#ifndef _WIN32
#error "Windows only."
#endif

//=============================================================================
// file access flags
//=============================================================================
const uint				EF_EXISTS(0x1);			// the file exists on disk.
const uint				EF_READ_ACCESS(0x2);	// the file exists and can be read.
const uint				EF_WRITE_ACCESS(0x4);	// the file exists and can be written to.

//=============================================================================
// struct SHonVersion
//=============================================================================
struct SHonVersion
{
	// current version info.
	uint				major_version;
	uint				minor_version;
	uint				micro_version;
	uint				hotfix_version;

	SHonVersion();
};

//=============================================================================
// struct SHonManifestFile
//=============================================================================
struct SHonManifestFile
{
	uint				size;		// the size of the file in bytes.
	tstring				path;		// e.g. "base/core/shaders/common/common.h"
	tstring				checksum;	// e.g. "56099b82"
	tstring				version;	// e.g. "0.1.0.0"

	SHonManifestFile();
};
typedef vector< SHonManifestFile >		HonManifestFilesVec;

//=============================================================================
// struct SHonManifest
//=============================================================================
struct SHonManifest
{
	// the current version of HoN, e.g. "1.0.10.0"
	SHonVersion			version;
	tstring				versionStr;

	// the "os", usually "wac" for release builds.
	tstring				os;

	// the platform architecture, e.g. "i686"
	tstring				arch;

	// the manifest file entries.
	HonManifestFilesVec	files;
};

//=============================================================================
// public API
//=============================================================================

// retrieves info about the file located at 'filepath'.
//		return value - a combination of EF_EXISTS, EF_READ_ACCESS,
//			and EF_WRITE_ACCESS.
extern uint			HonInfo_GetFileInfo( const tstring& filepath );

// returns true if the Heroes of Newerth installation folder could be located.
//		return value - the installation path,
//			e.g. "C:\Program Files (x86)\Heroes of Newerth\"
extern tstring		HonInfo_GetInstallPath();

// parses the manifest file.
//		installPath - the Heroes of Newerth installation path,
//			e.g. "C:\Program Files (x86)\Heroes of Newerth\"
extern bool			HonInfo_ParseManifest( SHonManifest& outManifest, const tstring& installPath );


#endif //__HONINFO_H__
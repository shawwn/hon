// (C)2010 Shawn Presser
// main.cpp
//
//=============================================================================

// Windows headers.
#include <windows.h>

// C headers.
#include <stdio.h>

// honinfo headers.
#include "../honinfo/honinfo.h"

// install file headers.
#include "installfiles/hon.h"
#include "installfiles/k2.h"
#include "installfiles/vid_d3d9.h"
#include "installfiles/vid_gl2.h"
#include "installfiles/cgame.h"
#include "installfiles/game.h"
#include "installfiles/game_shared.h"

// forward declaratinos.
struct SInstallFile;

// declarations.
bool			Installer_RestoreGameFileBackups( const tstring& installPath, const vector< SInstallFile >& files );

// globals.
tstring					g_installPath;
vector< SInstallFile >	g_installFiles;

//=============================================================================
// the version of HoN you are hotswapping
//=============================================================================
#define HON_MAJOR_VERSION	1
#define HON_MINOR_VERSION	0
#define HON_MICRO_VERSION	10
#define HON_HOTFIX_VERSION	0

#define MAKE_HON_VERSION_STR_( a, b, c, d )	L#a L"." L#b L"." L#c L"." L#d
#define MAKE_HON_VERSION_STR( a, b, c, d )	MAKE_HON_VERSION_STR_( a, b, c, d )
#define HON_VERSION_STR						MAKE_HON_VERSION_STR( HON_MAJOR_VERSION, HON_MINOR_VERSION, HON_MICRO_VERSION, HON_HOTFIX_VERSION )

//=============================================================================
// private functions
//=============================================================================


/*====================
  PrintF
  ====================*/
void			PrintF( const wchar_t* fmt, ... )
{
	wchar_t buf[ 5*1024 ];
	const uint bufSize( 5*1024 );

	va_list args;
	va_start( args, fmt );
	_vsnwprintf_s( buf, bufSize, _TRUNCATE, fmt, args );
	va_end( args );

	MessageBox( NULL, buf, L"", MB_SYSTEMMODAL | MB_SETFOREGROUND | MB_TOPMOST | MB_OK );
}


/*====================
  Fatal
  ====================*/
void			Fatal()
{
	Installer_RestoreGameFileBackups( g_installPath, g_installFiles );
	exit( -1 );
}


/*====================
  Prompt
  ====================*/
bool			Prompt( const wchar_t* fmt, const wchar_t* caption, ... )
{
	wchar_t buf[ 5*1024 ];
	const uint bufSize( 5*1024 );

	va_list args;
	va_start( args, fmt );
	_vsnwprintf_s( buf, bufSize, _TRUNCATE, fmt, args );
	va_end( args );

	if ( MessageBox( NULL, buf, caption, MB_SYSTEMMODAL | MB_SETFOREGROUND | MB_TOPMOST | MB_YESNO ) != IDYES )
		return false;

	return true;
}


/*====================
  Fatal
  ====================*/


/*====================
  Installer_FileExists
  ====================*/
static bool		Installer_FileExists( const tstring& path )
{
	if ( ( HonInfo_GetFileInfo( path ) & EF_EXISTS ) != 0 )
		return true;
	return false;
}


/*====================
  Installer_ReadFile
  ====================*/
static byte*	Installer_ReadFile( uint& outSize, const tstring& path )
{
	byte* result(NULL);
	HANDLE hFile(INVALID_HANDLE_VALUE);
	DWORD readSize(0);

	outSize = 0;

	// open the file.
	hFile = CreateFile( path.c_str(),
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_READONLY,
		NULL );

	if ( hFile == INVALID_HANDLE_VALUE )
		return NULL;

	outSize = GetFileSize( hFile, NULL );
	result = (byte*)malloc( outSize );

	// read the file.
	if ( !ReadFile( hFile,
		(void*)result,
		outSize,
		&readSize,
		NULL ) )
	{
		assert( false );
		goto fail;
	}

	// verify that we've read the entire file.
	assert( outSize == readSize );
	if ( outSize != readSize )
		goto fail;

	// close the file.
	if ( hFile != INVALID_HANDLE_VALUE )
		CloseHandle( hFile );

	return result;

fail:
	outSize = 0;
	if ( result )
		free( result );

	if ( hFile != INVALID_HANDLE_VALUE )
		CloseHandle( hFile );
	return NULL;
}


/*====================
  Installer_WriteFile
  ====================*/
static bool		Installer_WriteFile( const tstring& path, const byte* data, uint dataSize )
{
	HANDLE hFile(INVALID_HANDLE_VALUE);
	DWORD writeSize(0);
	bool exists(false);

	// sanity check:  the .install file should not exist at this point, so if it does, abort.
	exists = Installer_FileExists( path );
	assert( !exists );
	if ( exists )
	{
		PrintF( L"Installation failed (technical error).  Message:\n\n"
			L"WriteFile: File should not exist: %s\n",
			path.c_str() );
			Fatal();
	}

	// open the file.
	hFile = CreateFile( path.c_str(),
		GENERIC_WRITE,
		0, // do not share
		NULL,
		CREATE_ALWAYS, // always create a new file.
		FILE_ATTRIBUTE_NORMAL,
		NULL );

	if ( hFile == INVALID_HANDLE_VALUE )
		return NULL;

	// read the file.
	if ( !WriteFile( hFile,
		(void*)data,
		dataSize,
		&writeSize,
		NULL ) )
	{
		assert( false );
		goto fail;
	}

	// verify that we've written the entire file.
	assert( dataSize == writeSize );
	if ( dataSize != writeSize )
		goto fail;

	// close the file.
	CloseHandle( hFile );
	return true;

fail:
	if ( hFile != INVALID_HANDLE_VALUE )
		CloseHandle( hFile );
	return NULL;
}


/*====================
  Installer_RenameFile
  ====================*/
static bool		Installer_RenameFile( const tstring& from, const tstring& to, bool fatalError = false )
{
#if 1
	if ( MoveFile( from.c_str(), to.c_str() ) != 0 )
		return true;

	if ( fatalError )
	{
		assert( false );
		PrintF( L"Installation failed (technical error).  Message:\n\n"
			L"Could not rename file.\n"
			L"From: %s\n"
			L"To: %s\n",
			from.c_str(),
			to.c_str() );
		exit( -1 );
	}
	return false;
#else
	bool exists(false);
	byte* buf(NULL);
	uint bufSize(0);

	// sanity check:  the "to" file should not exist at this point, so if it does, abort.
	exists = Installer_FileExists( to );
	assert( !exists );
	if ( exists )
	{
		PrintF( L"Installation failed (technical error).  Message:\n\n"
			L"RenameFile:  File should not exist: %s\n",
			to.c_str() );
		exit( -1 );
		return false;
	}

	// read the file into memory.
	buf = Installer_ReadFile( bufSize, from );
	if ( buf == NULL )
	{
		if ( fatalError )
		{
			assert( buf != NULL );
			PrintF( L"Installation failed (technical error).  Message:\n\n"
				L"RenameFile:  File doesn't exist: %s\n",
				from.c_str() );
			exit( -1 );
		}
		return false;
	}

	// write the file to disk.
	if ( !Installer_WriteFile( to, buf, bufSize ) )
	{
		PrintF( L"WriteFile:  could not write file %s\n"
			L"(Are you out of harddrive space?)\n", to.c_str() );
		return false;
	}
	
	// delete the source file.
	if ( !DeleteFile( from.c_str() ) )
	{
		// if the source file could not be deleted, then delete the destination file
		// and abort with an error.
		bool bDeletedDest = DeleteFile( to.c_str() ) != 0;
		assert( bDeletedDest );
		if ( bDeletedDest )
		{
			PrintF( L"Installation failed (technical error).  Message:\n\n"
				L"RenameFile:  Could not delete source file: %s\n",
				from.c_str() );
		}
		else
		{
			PrintF( L"Installation failed (technical error).  Message:\n\n"
				L"RenameFile:  Could not delete source file: %s\n"
				L"(Also could not delete the destination file: %s)\n",
				from.c_str(),
				to.c_str() );
		}
		exit(-1);
		return false;
	}

	return true;
#endif
}


//=============================================================================
// struct SInstallFile
//=============================================================================
struct SInstallFile
{
	tstring		file_name;
	uint		file_size;
	const byte*	file_data;


	/*====================
	  SInstallFile::SInstallFile
	  ====================*/
	SInstallFile()
		: file_size( 0 )
		, file_data( NULL )
	{ }


	/*====================
	  SInstallFile::SInstallFile
	  ====================*/
	SInstallFile( const tstring& name, uint size, const byte* data )
		: file_name( name )
		, file_size( size )
		, file_data( data )
	{
	}
};


/*====================
  Installer_TestFilePermissions
  ====================*/
bool			Installer_TestFilePermissions( const tstring& installPath, const vector< SInstallFile >& files )
{
	for ( size_t i = 0; i < files.size(); ++i )
	{
		tstring file( installPath + files[i].file_name );
		uint fileFlags = HonInfo_GetFileInfo( file );
		if ( !( fileFlags & EF_WRITE_ACCESS ) )
			return false;
	}

	return true;
}


/*====================
  Installer_DeleteExistingInstallFiles
  ====================*/
bool			Installer_DeleteExistingInstallFiles( const tstring& installPath, const vector< SInstallFile >& files )
{
	for ( size_t i = 0; i < files.size(); ++i )
	{
		const SInstallFile& file( files[i] );
		tstring path( installPath + file.file_name + L".install" );

		if ( Installer_FileExists( path ) )
		{
			// if we can't delete it, then fail.
			if ( DeleteFile( path.c_str() ) == 0 )
			{
				PrintF( L"Installation failed (technical error).  Message:\n\n"
					L"Could not delete existing install file: %s\n",
					path.c_str() );
				Fatal();
			}
		}
	}

	return true;
}


/*====================
  Installer_BackupGameFiles
  ====================*/
bool			Installer_BackupGameFiles( const tstring& installPath, const vector< SInstallFile >& files )
{
	for ( size_t i = 0; i < files.size(); ++i )
	{
		const SInstallFile& file( files[i] );
		tstring path( installPath + file.file_name );

		// rename the file.
		if ( !Installer_RenameFile( path, path + L".backup" ) )
		{
			PrintF( L"Installation failed (technical error).  Message:\n\n"
				L"Could not backup file: %s\n",
				path.c_str() );
			Fatal();
		}
	}

	return true;
}


/*====================
  Installer_DeleteBackups
  ====================*/
bool			Installer_DeleteBackups( const tstring& installPath, const vector< SInstallFile >& files )
{
	for ( size_t i = 0; i < files.size(); ++i )
	{
		const SInstallFile& file( files[i] );
		tstring path( installPath + file.file_name + L".backup" );

		// delete the backup.
		if ( DeleteFile( path.c_str() ) == 0 )
		{
			PrintF( L"Installation failed: Could not delete backup file: %s\n"
				L"(HoN is probably still running.)\n",
				path.c_str() );
			Fatal();
		}
	}

	return true;
}


/*====================
  Installer_RestoreGameFileBackups
  ====================*/
bool			Installer_RestoreGameFileBackups( const tstring& installPath, const vector< SInstallFile >& files )
{
	bool result( true );

	for ( size_t i = 0; i < files.size(); ++i )
	{
		const SInstallFile& file( files[i] );
		tstring path( installPath + file.file_name );

		// if the backup doesn't exist, continue.
		if ( !Installer_FileExists( path + L".backup" ) )
			continue;

		// if the existing game file exists...
		if ( Installer_FileExists( path ) )
		{
			// try to rename it.
			//if ( !Installer_RenameFile( path, path + L".old" ) )
			{
				// try to delete it.
				if ( DeleteFile( path.c_str() ) == 0 )
				{
					PrintF( L"Installation failed (technical error).  Message:\n\n"
						L"Could not restore from backup: %s\n",
						path.c_str() );
					result = false;
				}
			}
		}

		// rename k2.dll.backup to k2.dll, etc.
		if ( !Installer_RenameFile( path + L".backup", path ) )
		{
			PrintF( L"Installation failed (technical error).  Message:\n\n"
				L"Could not backup file: %s\n",
				path.c_str() );
			result = false;
		}
	}

	return result;
}


/*====================
  Installer_WriteInstallFiles
  ====================*/
bool			Installer_WriteInstallFiles( const tstring& installPath, const vector< SInstallFile >& files )
{
	for ( size_t i = 0; i < files.size(); ++i )
	{
		const SInstallFile& file( files[i] );
		tstring path( installPath + file.file_name + L".install" );
		if ( !Installer_WriteFile( path, file.file_data, file.file_size ) )
		{
			PrintF( L"Failed:  could not write file %s\n"
				L"(Are you out of harddrive space?)\n", path.c_str() );
			return false;
		}
	}

	return true;
}


/*====================
  Installer_FinalizeInstallFiles
  ====================*/
bool			Installer_FinalizeInstallFiles( const tstring& installPath, const vector< SInstallFile >& files )
{
	for ( size_t i = 0; i < files.size(); ++i )
	{
		const SInstallFile& file( files[i] );
		tstring path( installPath + file.file_name );
		if ( !Installer_RenameFile( path + L".install", path ) )
		{
			PrintF( L"Installation failed (technical error).  Message:\n\n"
				L"Could not finalize install file: %s\n",
				path.c_str() );
			Fatal();
		}
	}

	return true;
}


/*====================
  WinMain
  ====================*/
int WINAPI		WinMain( HINSTANCE, HINSTANCE, LPSTR, int )
{
	tstring installPath( HonInfo_GetInstallPath() );
	if ( installPath.empty() )
	{
		PrintF( L"Could not locate your Heroes of Newerth installation directory.\n" );
		return 0;
	}

	SHonManifest manifest;
	if ( !HonInfo_ParseManifest( manifest, installPath ) )
	{
		PrintF( L"Could not read from your Heroes of Newerth installation directory.\n" );
		return 0;
	}

	if ( manifest.version.major_version != HON_MAJOR_VERSION ||
		manifest.version.minor_version != HON_MINOR_VERSION ||
		manifest.version.micro_version != HON_MICRO_VERSION ||
		manifest.version.hotfix_version != HON_HOTFIX_VERSION )
	{
		PrintF( 
			L"Could not install patch (unexpected HoN version).\n\n"
			L"Expected HoN version: %s\n"
			L"Your version: %s\n",
			HON_VERSION_STR,
			manifest.versionStr.c_str() );
		return 0;
	}

	if ( !Prompt( L"Install Heroes of Newerth hotfix?", L"Install?" ) )
		return 0;

	PrintF( L"Close HoN before proceeding.\n" );

	vector< SInstallFile > files;
	files.push_back( SInstallFile( L"hon.exe", sizeof( g_hon ), (byte*)g_hon ) );
	files.push_back( SInstallFile( L"k2.dll", sizeof( g_k2 ), (byte*)g_k2 ) );
	files.push_back( SInstallFile( L"vid_d3d9.dll", sizeof( g_vid_d3d9 ), (byte*)g_vid_d3d9 ) );
	files.push_back( SInstallFile( L"vid_gl2.dll", sizeof( g_vid_gl2 ), (byte*)g_vid_gl2 ) );
	files.push_back( SInstallFile( L"game\\cgame.dll", sizeof( g_cgame ), (byte*)g_cgame ) );
	files.push_back( SInstallFile( L"game\\game.dll", sizeof( g_game ), (byte*)g_game ) );
	files.push_back( SInstallFile( L"game\\game_shared.dll", sizeof( g_game_shared ), (byte*)g_game_shared ) );

	g_installPath = installPath;
	g_installFiles = files;

	// if the previous installation was incomplete, restore the previous binaries.
	if ( !Installer_RestoreGameFileBackups( installPath, files ) )
		return 0;

	// ensure that we can overwrite the game files.
	if ( !Installer_TestFilePermissions( installPath, files ) )
	{
		PrintF( L"Installation failed: Could not write to your HoN directory.\n\n"
			L"(Is HoN closed?  Open task manager and terminate hon.exe, if it exists)\n" );
		return 0;
	}

	// delete any existing .install files.
	if ( !Installer_DeleteExistingInstallFiles( installPath, files ) )
		return 0;

	// write the files to the game dir (without overwriting the game files).
	if ( !Installer_WriteInstallFiles( installPath, files ) )
		return 0;

	// rename the existing binaries to *.backup
	Installer_BackupGameFiles( installPath, files );

	// rename k2.dll.install to k2.dll, etc.
	Installer_FinalizeInstallFiles( installPath, files );

	// we can now safely delete the backup files.
	Installer_DeleteBackups( installPath, files );

	PrintF( L"Done.\n" );

	return 0;
}

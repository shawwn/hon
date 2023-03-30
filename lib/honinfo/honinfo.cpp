// (C)2010 S2 Games
// honinfo.cpp
//
//=============================================================================
#include "honinfo_common.h"
#include "honinfo.h"

// Windows header.
#include <windows.h>

// tinyxml header.
#include "tinyxml.h"

//=============================================================================
// class TemporaryWorkingDirectory
//=============================================================================
class TemporaryWorkingDirectory
{
private:
	TCHAR		_prevWorkingDir[ 2048 ];
	bool		_active;
	
public:


	/*====================
	  TemporaryWorkingDirectory::TemporaryWorkingDirectory
	  ====================*/
	TemporaryWorkingDirectory()
		: _active( false )
	{
	}


	/*====================
	  TemporaryWorkingDirectory::~TemporaryWorkingDirectory
	  ====================*/
	~TemporaryWorkingDirectory()
	{
		if ( _active )
		{
			if ( !SetCurrentDirectory( _prevWorkingDir ) )
			{
				assert( false );
			}
		}
	}


	/*====================
	  TemporaryWorkingDirectory::Set
	  ====================*/
	bool		Set( const tstring& dir )
	{
		assert( !_active );
		if ( _active )
			return false;

		if ( GetCurrentDirectory( 2048, _prevWorkingDir ) == 0 )
		{
			assert( false );
			return false;
		}

		if ( !SetCurrentDirectory( dir.c_str() ) )
		{
			assert( false );
			return false;
		}

		_active = true;
		return true;
	}
};

/*====================
SHonVersion::SHonVersion
====================*/
SHonVersion::SHonVersion()
: major_version( 0 )
, minor_version( 0 )
, micro_version( 0 )
, hotfix_version( 0 )
{
}


/*====================
SHonManifestFile::SHonManifestFile
====================*/
SHonManifestFile::SHonManifestFile()
: size( 0 )
{
}

/*====================
  StrReplace
  ====================*/
static void		StrReplaceAll( tstring& str, const tstring& replace, const tstring& with )
{
	size_t pos = str.find( replace );
	if ( pos == tstring::npos )
		return;

	str.replace( pos, pos + replace.size(), with );
	StrReplaceAll( str, replace, with );
}


/*====================
  StrSanitizeFolderPath
  ====================*/
static void		StrSanitizeFolderPath( tstring& path )
{
	assert( !path.empty() );
	if ( path.empty() )
		return;

	// replace forward slashes with backslashes.
	StrReplaceAll( path, L"/", L"\\" );

	// compact multiple backslashes.
	StrReplaceAll( path, L"\\\\", L"\\" );

	// ensure that the last segment is not a filename.
	size_t lastSlashPos = path.find_last_of( L'\\' );
	assert( lastSlashPos != tstring::npos );
	if ( lastSlashPos != tstring::npos )
	{
		size_t dotPos = path.find( L'.', lastSlashPos );
		if ( dotPos != tstring::npos )
			path.erase( lastSlashPos );
	}

	// ensure that the folder path ends with a backslash.
	if ( *path.rbegin() != L'\\' )
		path.append( L"\\" );
}


/*====================
  TestFilePaths
  ====================*/
static bool			TestFilePaths( tstring& outPath, uint outFileInfo, const vector< tstring >& paths )
{
	// test each path.
	for ( size_t i = 0; i < paths.size(); ++i )
	{
		const tstring& path( paths[i] );
		uint fileInfo = HonInfo_GetFileInfo( path );

		// if the file exists, we're done.
		if ( ( fileInfo & EF_EXISTS ) != 0 )
		{
			outPath = path;
			outFileInfo = fileInfo;
			return true;
		}
	}

	// no match.
	return false;
}


/*====================
  Reg_QueryValue
	allocates and returns a registry value.

		outValueType - REG_BINARY, REG_DWORD, REG_DWORD_LITTLE_ENDIAN, REG_DWORD_BIG_ENDIAN,
						REG_EXPAND_SZ, REG_LINK, REG_MULTI_SZ, or REG_NONE.

		outValueSize - the size of the returned buffer.

  ====================*/
static void*		Reg_QueryValue( uint& outValueType, uint& outValueSize, HKEY key, const tstring& name )
{
	DWORD type(0);
	DWORD dataSize(0);
	byte* ret(NULL);
	LONG result;

	outValueType = 0;
	outValueSize = 0;
	
	// get the size of the value.
	result = RegQueryValueEx( key,
		name.c_str(),
		NULL,
		&type,
		NULL,
		&dataSize );

	if ( result == ERROR_FILE_NOT_FOUND )
		goto fail;

	assert( dataSize != 0 );
	if ( dataSize == 0 )
		goto fail;

	// allocate a buffer to hold the value.
	ret = (byte*)malloc( dataSize );

	// query the value.
	result = RegQueryValueEx( key,
		name.c_str(),
		NULL,
		&type,
		ret,
		&dataSize );
	assert( result != ERROR_FILE_NOT_FOUND );
	if ( result == ERROR_FILE_NOT_FOUND )
		goto fail;

	outValueType = (uint)type;
	outValueSize = (uint)dataSize;
	return ret;

fail:
	// cleanup.
	if ( ret )
		free( ret );
	return NULL;
}


#if 0
/*====================
  Reg_QueryString
  ====================*/
static bool		Reg_QueryString( tstring& outVal, HKEY key, const tstring& name )
{
	DWORD type(0);
	DWORD dataSize(0);
	byte* ret(NULL);
	LONG result;

	outVal.clear();
	
	// get the size of the value.
	result = RegGetValue( key,
		NULL,
		name.c_str(),
		RRF_RT_REG_SZ,
		&type,
		NULL,
		&dataSize );

	if ( result != ERROR_SUCCESS )
		return false;

	assert( dataSize != 0 );
	if ( dataSize == 0 )
		return false;

	// allocate a buffer to hold the value.
	ret = (byte*)malloc( dataSize );

	// query the value.
	result = RegGetValue( key,
		NULL,
		name.c_str(),
		RRF_RT_REG_SZ,
		&type,
		(void*)ret,
		&dataSize );

	assert( result == ERROR_SUCCESS );
	if ( result != ERROR_SUCCESS )
		goto fail;

	assert( dataSize > 0 );
	if ( dataSize == 0 )
		goto fail;

	outVal = tstring( (const wchar_t*)ret, ( dataSize / sizeof(wchar_t) ) - 1 );

	// cleanup.
	if ( ret )
		free( ret );
	return true;

fail:
	outVal.clear();

	// cleanup.
	if ( ret )
		free( ret );
	return false;
}
#else
/*====================
  Reg_QueryString
  ====================*/
static bool		Reg_QueryString( tstring& outVal, HKEY key, const tstring& name )
{
	void* buf(NULL);
	uint type(0);
	uint size(0);
	bool ret(false);

	outVal.clear();

	buf = Reg_QueryValue( type, size, key, name );
	if ( !buf )
		goto done;

	if ( type != REG_SZ )
		goto done;

	outVal = tstring( (const wchar_t*)buf );
	ret = true;

done:
	if ( buf )
		free( buf );
	return ret;
}
#endif


/*====================
  HonInfo_GetRegistryInstallLocation
  ====================*/
static bool			HonInfo_GetRegistryInstallLocation( tstring& outLocation )
{
	HKEY key(0);
	bool ret(false);

	// try to open the registry key.
	LONG code = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
		L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\hon",
		0,
		KEY_READ,
		&key );
	if ( code != ERROR_SUCCESS )
	{
		code = RegOpenKeyEx( HKEY_CURRENT_USER,
			L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\hon",
			0,
			KEY_READ,
			&key );
		if ( code != ERROR_SUCCESS )
			goto done;
	}

	// read the proper subkey.
	if ( Reg_QueryString( outLocation, key, L"InstallLocation" ) )
		ret = true;

done:
	if ( key )
		RegCloseKey( key );
	return ret;
}


/*====================
  ReadEntireFile
  ====================*/
static byte*	ReadEntireFile( uint& outSize, const tstring& path )
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
  ToWideStr
  ====================*/
static bool		ToWideStr( tstring& outResult, const char* str )
{
	DWORD num(0);
	DWORD code(0);
	wchar_t* text(NULL);

	outResult.clear();

	if ( !str )
		return false;

	// get the size of the string by setting the 4th parameter to -1
	num = MultiByteToWideChar( CP_ACP, 0, str, -1, NULL, 0 );

	// allocate space for wide char string.
	text = (wchar_t*)malloc( sizeof(wchar_t)*num );

	// convert.
	code = MultiByteToWideChar( CP_ACP, 0, str, -1, text, num );
	assert( code == num );
	if ( code != num )
		goto fail;

	outResult = tstring( text );

	// cleanup.
	free( text );
	return true;

fail:
	outResult.clear();
	free( text );
	return false;
}


/*====================
  HonInfo_GetFileInfo
	returns a combination of EF_EXISTS, EF_READ_ACCESS and EF_WRITE_ACCESS.
  ====================*/
uint		HonInfo_GetFileInfo( const tstring& path )
{
	uint result(0);

	// check for existance.
	if ( _waccess( path.c_str(), 0 ) != -1 )
	{
		result |= EF_EXISTS;

		// check for read access.
		if ( _waccess( path.c_str(), 4 ) != -1 )
			result |= EF_WRITE_ACCESS;

		// check for write access.
		if ( _waccess( path.c_str(), 2 ) != -1 )
			result |= EF_WRITE_ACCESS;
	}

	return result;
}


/*====================
  CurrentDirString
  ====================*/
static tstring	CurrentDirString()
{
	TCHAR workingDir[ 2048 ];
	tstring result;

	if ( GetCurrentDirectory( 2048, workingDir ) )
	{
		result = tstring( workingDir );
		StrSanitizeFolderPath( result );
	}

	return result;
}


/*====================
  HonInfo_GetInstallPath
  ====================*/
tstring		HonInfo_GetInstallPath()
{
	tstring installPath;
	uint accessFlags(0);

	if ( installPath.empty() )
	{
		accessFlags = 0;

		vector< tstring > paths;

		// test the current directory.
		tstring currentDir( CurrentDirString() );
		if ( !currentDir.empty() )
			paths.push_back( currentDir + L"manifest.xml" );

		// test some common installation paths.
		paths.push_back( L"C:\\Program Files (x86)\\Heroes of Newerth\\manifest.xml" );
		paths.push_back( L"C:\\Program Files\\Heroes of Newerth\\manifest.xml" );
		paths.push_back( L"D:\\Program Files (x86)\\Heroes of Newerth\\manifest.xml" );
		paths.push_back( L"D:\\Program Files\\Heroes of Newerth\\manifest.xml" );
		paths.push_back( L"E:\\Program Files (x86)\\Heroes of Newerth\\manifest.xml" );
		paths.push_back( L"E:\\Program Files\\Heroes of Newerth\\manifest.xml" );
		TestFilePaths( installPath, accessFlags, paths );
	}

	if ( installPath.empty() )
	{
		accessFlags = 0;

		// try to find the installation path in the registry.
		if ( HonInfo_GetRegistryInstallLocation( installPath ) )
		{
			assert( !installPath.empty() );
			if ( !installPath.empty() )
			{
				if ( *installPath.rbegin() != L'\\' )
					installPath += L"\\";

				accessFlags = HonInfo_GetFileInfo( installPath + L"manifest.xml" );
				if ( ( accessFlags & EF_EXISTS ) == 0 )
				{
					accessFlags = 0;
					assert( !"invalid installation path." );
					return L"";
				}
			}
		}
	}

	// sanitize the result path.
	StrSanitizeFolderPath( installPath );
	return installPath;
}


/*====================
HonInfo_ParseManifest
====================*/
extern bool		HonInfo_ParseManifest( SHonManifest& outManifest, const tstring& dirtyInstallPath )
{
	// clear the result.
	outManifest = SHonManifest();

	// verify the install path.
	tstring installPath( dirtyInstallPath );
	StrSanitizeFolderPath( installPath );
	assert( !installPath.empty() );
	if ( installPath.empty() )
		return false;

	// temporarily set the working directory to the install path.
	TemporaryWorkingDirectory workingDir;
	if ( !workingDir.Set( installPath ) )
		return false;

	// parse the manifest file.
	TiXmlDocument doc( "manifest.xml" );
	if ( !doc.LoadFile() )
		return false;

	// find the manifest element.
	TiXmlElement* elemManifest = doc.FirstChildElement( "manifest" );
	assert( elemManifest );
	if ( !elemManifest )
		return false;

	// read HoN's version.
	const char* version = elemManifest->Attribute( "version" );
	ToWideStr( outManifest.versionStr, version );
	assert( version != NULL );
	if ( version != NULL )
	{
		uint versionPiece[4] = { 0, 0, 0, 0 };
		uint idx = 0;

		// parse the version string.
		while ( *version )
		{
			// sanity check.
			assert( *version != '.' );
			assert( idx <= 3 );
			if ( idx > 3 )
				break;

			// parse the current version piece.
			versionPiece[ idx ] = atoi( version );

			// advance to the next version piece.
			while ( *version && *version != '.' )
				++version;
			
			if ( *version && *version == '.' )
			{
				++idx;
				++version;
				continue;
			}
		}

		outManifest.version.major_version = versionPiece[0];
		outManifest.version.minor_version = versionPiece[1];
		outManifest.version.micro_version = versionPiece[2];
		outManifest.version.hotfix_version = versionPiece[3];
	}

	// read other manifest params.
	ToWideStr( outManifest.os, elemManifest->Attribute( "os" ) );
	ToWideStr( outManifest.arch, elemManifest->Attribute( "arch" ) );

	// count the number of manifest files.
	uint numManifestFiles(0);
	TiXmlElement* elemFile = elemManifest->FirstChildElement( "file" );
	while ( elemFile )
	{
		++numManifestFiles;
		elemFile = elemFile->NextSiblingElement( "file" );
	}
	
	// reserve file entries.
	outManifest.files.resize( numManifestFiles );

	// read each manifest file.
	uint idx(0);
	elemFile = elemManifest->FirstChildElement( "file" );
	while ( elemFile )
	{
		// add another manifest file entry.
		SHonManifestFile& info( outManifest.files[idx] );

		// read the file params.
		ToWideStr( info.path, elemFile->Attribute( "path" ) );
		ToWideStr( info.checksum, elemFile->Attribute( "checksum" ) );
		ToWideStr( info.version, elemFile->Attribute( "version" ) );

		// read the file size.
		{
			int i(0);
			if ( elemFile->Attribute( "size", &i ) )
				info.size = i;
		}

		// advance to the next file.
		elemFile = elemFile->NextSiblingElement( "file" );
		++idx;

		// sanity check.
		assert( idx <= numManifestFiles );
		if ( idx == numManifestFiles )
			break;
	}

	elemFile = elemFile;

	return true;
}

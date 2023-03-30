// (C)2010 S2 Games
// main.cpp
//
//=============================================================================

// standard C++ headers.
#include <iostream>

// standard C headers.
#include <conio.h> // getch

// honinfo headers.
#include "honinfo.h"

using std::wcout;
using std::endl;

/*====================
  main
  ====================*/
int		main()
{
	tstring installPath( HonInfo_GetInstallPath() );
	if ( installPath.empty() )
	{
		wcout << L"Could not get HoN info." << endl;
		return 0;
	}

	wcout << L"HoN install path: " << installPath << endl;

	SHonManifest manifest;
	if ( !HonInfo_ParseManifest( manifest, installPath ) )
	{
		wcout << L"Could not parse HoN manifest." << endl;
		return 0;
	}

	wcout << L"HoN manifest version: " << manifest.versionStr << endl;
	wcout << L"There are " << manifest.files.size() << L" manifest files." << endl;
	wcout << L"Press any key to continue." << endl;
	getch();

	return 0;
}

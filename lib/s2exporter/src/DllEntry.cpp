// (C)2005 S2 Games
// DllEntry.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "exporter_common.h"

#include "s2exporter.h"

#include "c_s2exporterclassdesc.h"
//=============================================================================

//=============================================================================
//=============================================================================
static CS2ExporterClassDesc S2ExporterClassDesc;

HINSTANCE hInstance;
int controlsInit = FALSE;
//=============================================================================

/*====================
  DllMain
  ====================*/
BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved)
{
	hInstance = hinstDLL;				// Hang on to this DLL's instance handle.

	if (!controlsInit)
	{
		controlsInit = TRUE;
		InitCustomControls(hInstance);	// Initialize MAX's custom controls
		InitCommonControls();			// Initialize Win95 controls
	}

	return (TRUE);
}


/*====================
  LibDescription
  ====================*/
__declspec(dllexport) const TCHAR* LibDescription()
{
#ifdef _DEBUG
	return GetString(IDS_LIBDESCRIPTION_D);
#else
	return GetString(IDS_LIBDESCRIPTION);
#endif
}


/*====================
  LibNumberClasses

  TODO: Must change this number when adding a new class
  ====================*/
__declspec(dllexport) int LibNumberClasses()
{
	return 1;
}


/*====================
  LibClassDesc
  ====================*/
__declspec(dllexport) ClassDesc* LibClassDesc(int i)
{
	switch(i)
	{
		case 0:
			return &S2ExporterClassDesc;
		default:
			return 0;
	}
}


/*====================
  LibVersion
  ====================*/
__declspec(dllexport) ULONG LibVersion()
{
	return VERSION_3DSMAX;
}


/*====================
  GetString
  ====================*/
TCHAR*	GetString(int id)
{
	static TCHAR buf[256];

	if (hInstance)
		return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;
	return NULL;
}

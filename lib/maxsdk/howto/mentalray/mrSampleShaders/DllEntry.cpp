/*==============================================================================

  file:     DllEntry.cpp

  author:   Daniel Levesque

  created:  3mar2003

  description:

     Entry point for this plugin.

  modified:	


© 2003 Autodesk
==============================================================================*/
#include "mrTwoSidedShader.h"
#include "resource.h"

HINSTANCE hInstance;
int controlsInit = FALSE;

TCHAR *GetString(int id);

BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved)
{
	hInstance = hinstDLL;				// Hang on to this DLL's instance handle.

	if (!controlsInit) {
		controlsInit = TRUE;
		InitCustomControls(hInstance);	// Initialize MAX's custom controls
		InitCommonControls();			// Initialize Win95 controls
	}
			
	return (TRUE);
}

__declspec( dllexport ) const TCHAR* LibDescription()
{
	return GetString(IDS_LIBDESCRIPTION);
}

__declspec( dllexport ) int LibNumberClasses()
{
	return 1;
}

__declspec( dllexport ) ClassDesc* LibClassDesc(int i)
{
	switch(i) {
	case 0: 
		return &mrTwoSidedShader::GetClassDesc();
	default:
		return NULL;
	}
}

__declspec( dllexport ) ULONG LibVersion()
{
	return VERSION_3DSMAX;
}

TCHAR *GetString(int id)
{
	static TCHAR buf[256];

	if (hInstance)
		return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;
	return NULL;
}


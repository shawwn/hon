//**************************************************************************/
// Copyright (c) 1998-2005 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//**************************************************************************/
// DESCRIPTION: Sample ATS Provider DLL Entry
// AUTHOR: Michael Russo - created June 3, 2005
//***************************************************************************/

#include "DllEntry.h"
#include "SampleATSProvider.h"
#include "IATSProviderExport.h"

HINSTANCE hInstance;

BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved)
{
	hInstance = hinstDLL;
			
	return (TRUE);
}

TCHAR *GetString(int id)
{
	static TCHAR buf[256];

	if (hInstance)
		return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;
	return NULL;
}

//
// IATSProviderExports
//
ATSProviderExport DWORD GetATSVersion( void )
{
	return ATS::kATSAPIVersion;
}

ATSProviderExport const TCHAR* GetATSProviderName( void )
{
	return GetString(IDS_PROVIDER_NAME);
}

ATSProviderExport IATSProvider* GetIATSProvider( void )
{	
	SampleATSProvider* sampleProvider = SampleATSProvider::GetSampleATSProviderInstance();

	return static_cast<IATSProvider*>(sampleProvider);
}

ATSProviderExport void ReleaseIATSProvider( IATSProvider *pIATSProvider )
{
}

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
// DESCRIPTION: Sample ATS Provider Header
// AUTHOR: Michael Russo - created June 3, 2005
//***************************************************************************/

#ifndef __SAMPLEATSPROVIDER__H
#define __SAMPLEATSPROVIDER__H

#include "DllEntry.h"
#include "IATSProvider.h"

using namespace ATS;

class SampleATSProvider : public IATSProvider
{

public:
	virtual ~SampleATSProvider();
	static SampleATSProvider*	GetSampleATSProviderInstance();

	// IATSProvider
	const TCHAR*	GetProviderName();

	ATSOption		GetSupportOptions();

	bool			IsInitialized();
	bool			IsProjectOpen();

	ATSResult		Initialize( ATSClientInfo &atsClientInfo, ATSOption atsOptions );
	ATSResult		Uninitialize( ATSOption atsOptions );

	ATSResult		OpenProject( TCHAR *szPath, const TCHAR *szComment, ATSOption atsOptions );
	ATSResult		CloseProject( ATSOption atsOptions );

	ATSResult		Checkin( ATSFileList &atsFileList, const TCHAR *szComment, ATSOption atsOptions );
	ATSResult		Checkout( ATSFileList &atsFileList, const TCHAR *szComment, ATSOption atsOptions );
	ATSResult		UndoCheckout( ATSFileList &atsFileList, ATSOption atsOptions );
	ATSResult		GetLatest( ATSFileList &atsFileList, ATSOption atsOptions );
	ATSResult		GetVersionRange( ATSFileList &atsFileList, ATSVersion &verMin, ATSVersion &verMax, ATSOption atsOptions );
	ATSResult		GetVersion( ATSFileList &atsFileList, ATSVersion ver, ATSOption atsOptions );

	ATSResult		AddFiles( ATSFileList &atsFileList, const TCHAR *szComment, ATSOption atsOptions );

	ATSResult		History( ATSFileList &atsFileList, ATSOption atsOptions );
	ATSResult		Properties( ATSFileList &atsFileList, ATSOption atsOptions );

	ATSResult		GetFileStatus( ATSFileList &atsFileList, ATSOption atsOptions );
	ATSResult		GetLockedFileUserList( ATSFileList &atsFileList, ATSUserList &atsUserList, ATSOption atsOptions);

	ATSResult		ExploreProvider( ATSFileList &atsFileList, ATSExploreProviderParams &exploreParams, ATSOption atsOptions);

	ATSResult		ShowProviderOptions( ATSOption atsOptions );
	ATSResult		LaunchProvider( ATSOption atsOptions );

	ATSResult		GetAssociatedFiles( ATSFileList &atsFileList, ATSOption atsOptions );

	bool			IsATSMSSCCWrapper();

protected:
	SampleATSProvider();

protected:
	static SampleATSProvider*	sInstance;
	bool						mInitialized;
	bool						mProjectOpen;
};


#endif // __SAMPLEATSPROVIDER__H

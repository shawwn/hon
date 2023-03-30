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
// DESCRIPTION: Sample ATS Provider
// AUTHOR: Michael Russo - created June 3, 2005
//***************************************************************************/

#include "DllEntry.h"
#include "SampleATSProvider.h"

///////////////////////////////////////////////////////////////////////////////
//
// consts
//
///////////////////////////////////////////////////////////////////////////////

const int	kVFStatusNone				= 0x000;	// no status
const int	kVFStatusCheckedOut			= 0x001;	// checked out to "this" user
const int	kVFStatusCheckedOutOther	= 0x002;	// checked out to other user
const int	kVFStatusOutOfDate			= 0x004;	// file is out of date
const int	kVFStatusModified			= 0x008;	// file has been locally modified
const int	kVFStatusMissing			= 0x010;	// local file is missing
const int	kVFStatusNotOnServer		= 0x020;	// File is not found on server

const int	kVMFileAssocRecurse			= 0x001;	// Recurse children/parents
const int	kVMFileAssocChildren		= 0x002;	// include children
const int	kVMFileAssocParents			= 0x004;	// include parents

///////////////////////////////////////////////////////////////////////////////
//
// SampleATSProvider
//
///////////////////////////////////////////////////////////////////////////////

SampleATSProvider* SampleATSProvider::sInstance = NULL;

SampleATSProvider::SampleATSProvider() :
	mInitialized(false),
	mProjectOpen(false)
{

}

SampleATSProvider::~SampleATSProvider()
{

}

SampleATSProvider* SampleATSProvider::GetSampleATSProviderInstance()	
{
	if( sInstance == NULL )
		sInstance = new SampleATSProvider();
	return sInstance;
}

//
// IATSProvider
//
const TCHAR* SampleATSProvider::GetProviderName()
{
	return GetString(IDS_PROVIDER_NAME);
}

ATSOption SampleATSProvider::GetSupportOptions()
{
	return kATSSupportNone;
}

bool SampleATSProvider::IsInitialized()
{
	return mInitialized;
}

bool SampleATSProvider::IsProjectOpen()
{
	return mProjectOpen;
}

ATSResult SampleATSProvider::Initialize( ATSClientInfo &atsClientInfo, ATSOption atsOptions )
{
	// Perform login or any other initialization required

	mInitialized = true;

	return kATSResSuccess;
}

ATSResult SampleATSProvider::Uninitialize( ATSOption atsOptions )
{
	// Perform logout or any other cleanup required

	mInitialized = false;

	return kATSResSuccess;
}

ATSResult SampleATSProvider::OpenProject( TCHAR *szPath, const TCHAR *szComment, ATSOption atsOptions )
{
	// Use szPath to determine the workspace of the current project/scene.

	mProjectOpen = true;

	return kATSResSuccess;
}

ATSResult SampleATSProvider::CloseProject( ATSOption atsOptions )
{
	mProjectOpen = false;

	return kATSResSuccess;
}

ATSResult SampleATSProvider::Checkin( ATSFileList &atsFileList, const TCHAR *szComment, ATSOption atsOptions )
{
	// Check Keep Checked Out option
	bool bKeepCheckedOut = (atsOptions & kATSOptKeepCheckedOut) ? true : false;

	for( UINT i=0; i<atsFileList.NumActiveEntries(); i++ ) {
		ATSString strFile = atsFileList.GetActiveFileEntry(i)->GetFile();
		// Checkin strFile
	}

	return kATSResSuccess;
}

ATSResult SampleATSProvider::Checkout( ATSFileList &atsFileList, const TCHAR *szComment, ATSOption atsOptions )
{
	// Check Replace Local Copy option
	bool bReplaceLocalCopy = (atsOptions & kATSOptReplaceLocalCopy) ? true : false;

	for( UINT i=0; i<atsFileList.NumActiveEntries(); i++ ) {
		ATSString strFile = atsFileList.GetActiveFileEntry(i)->GetFile();
		// Checkout strFile
	}

	return kATSResSuccess;
}

ATSResult SampleATSProvider::UndoCheckout( ATSFileList &atsFileList, ATSOption atsOptions )
{
	// Check Replace Local Copy option
	bool bReplaceLocalCopy = (atsOptions & kATSOptReplaceLocalCopy) ? true : false;

	for( UINT i=0; i<atsFileList.NumActiveEntries(); i++ ) {
		ATSString strFile = atsFileList.GetActiveFileEntry(i)->GetFile();
		// undo Checkout strFile
	}

	return kATSResSuccess;
}

ATSResult SampleATSProvider::GetLatest( ATSFileList &atsFileList, ATSOption atsOptions )
{
	for( UINT i=0; i<atsFileList.NumActiveEntries(); i++ ) {
		ATSString strFile = atsFileList.GetActiveFileEntry(i)->GetFile();
		// get latest version of strFile
	}

	return kATSResSuccess;
}

ATSResult SampleATSProvider::GetVersionRange( ATSFileList &atsFileList, ATSVersion &verMin, ATSVersion &verMax, ATSOption atsOptions )
{
	int vMin = 0;
	int vMax = 0;

	for( UINT i=0; i<atsFileList.NumActiveEntries(); i++ ) {
		ATSString strFile = atsFileList.GetActiveFileEntry(i)->GetFile();
		// Get version range of str File
	}

	verMin = vMin;
	verMax = vMax;

	return kATSResSuccess;
}

ATSResult SampleATSProvider::GetVersion( ATSFileList &atsFileList, ATSVersion ver, ATSOption atsOptions )
{
	for( UINT i=0; i<atsFileList.NumActiveEntries(); i++ ) {
		ATSString strFile = atsFileList.GetActiveFileEntry(i)->GetFile();
		// get specified version of strFile
	}
	return kATSResSuccess;
}

ATSResult SampleATSProvider::AddFiles( ATSFileList &atsFileList, const TCHAR *szComment, ATSOption atsOptions )
{
	// Check Keep Checked Out option
	bool bKeepCheckedOut = (atsOptions & kATSOptKeepCheckedOut) ? true : false;

	for( UINT i=0; i<atsFileList.NumActiveEntries(); i++ ) {
		ATSString strFile = atsFileList.GetActiveFileEntry(i)->GetFile();
		// Add strFile
	}

	return kATSResSuccess;
}

ATSResult SampleATSProvider::History( ATSFileList &atsFileList, ATSOption atsOptions )
{
	// Even though ATSFileList is used, it is typical that a provider would only
	// support history for a single file.  This example only supports the first file
	// in the list

	if( atsFileList.NumActiveEntries() == 0 )
		return kATSResError;

	bool bNewFileDownloaded = false;

	ATSString strFile = atsFileList.GetActiveFileEntry(0)->GetFile();
	// Display history dialog for strFile
	// If the history dialog download a new version of a file, we must identify this
	// This example assumes bNewFileDownloaded will reflect whether this is the
	// case or not.

	return bNewFileDownloaded ? kATSResSuccessReload : kATSResSuccess;
}

ATSResult SampleATSProvider::Properties( ATSFileList &atsFileList, ATSOption atsOptions )
{
	// Even though ATSFileList is used, it is typical that a provider would only
	// support property UI for a single file.  This example only supports the first file
	// in the list

	if( atsFileList.NumActiveEntries() == 0 )
		return kATSResError;

	ATSString strFile = atsFileList.GetActiveFileEntry(0)->GetFile();
	// Display property dialog for strFile

	return kATSResSuccess;
}

ATSResult SampleATSProvider::GetFileStatus( ATSFileList &atsFileList, ATSOption atsOptions )
{
	if( atsFileList.NumActiveEntries() == 0 )
		return kATSResSuccess;

	for( UINT i=0; i<atsFileList.NumActiveEntries(); i++ ) {

		//
		// Apply file status
		//

		int iFileStatus = kVFStatusNone;
		ATSString strFile = atsFileList.GetActiveFileEntry(i)->GetFile();

		// Get file status for strFile
		// This example assume iFileStatus will contain the status of the file
		// iFileStatus = GetStatusFromServer( strFile );

		if( iFileStatus & kVFStatusNotOnServer ) {
			if( mProjectOpen )
				atsFileList.GetActiveFileEntry(i)->SetFlagsOut( kATSStatusNotControlled|kATSStatusCanAddFile );
			else
				atsFileList.GetActiveFileEntry(i)->SetFlagsOut( kATSStatusUnknown );
			continue;
		}

		ATSStatus atsStatus = kATSStatusControlled;
		if( iFileStatus & kVFStatusCheckedOut )
			atsStatus |= kATSStatusCheckedOut;
		if( iFileStatus & kVFStatusCheckedOutOther )
			atsStatus |= kATSStatusCheckedOutOther;
		if( iFileStatus & kVFStatusOutOfDate )
			atsStatus |= kATSStatusOutOfDate;
		if( iFileStatus & kVFStatusModified )
			atsStatus |= kATSStatusModified;
		if( iFileStatus & kVFStatusMissing )
			atsStatus |= kATSStatusMissing;

		//
		// Apply "can do" status
		//

		// To Allow:	CanShowHistory and CanShowProperties
		// Criteria:	none - always enable
		atsStatus |= (kATSStatusCanShowHistory|kATSStatusCanShowProperties);

		// To Allow:	CanCheckin and CanUndoCheckout
		// Criteria:	The file is checked out to "this" user
		if( iFileStatus & kVFStatusCheckedOut )
			atsStatus |= (kATSStatusCanCheckin | kATSStatusCanUndoCheckout);

		// To Allow:	CanCheckout
		// Criteria:	1) File is not checked out by "this" user
		//				2) and File is not checked out by "other" user
		//				3) and File is not out of date
		//				4) and File is not missing
		if( ((iFileStatus & kVFStatusCheckedOut) == 0)
			&& ((iFileStatus & kVFStatusCheckedOutOther) == 0) 
			&& ((iFileStatus & kVFStatusOutOfDate) == 0) 
			&& ((iFileStatus & kVFStatusMissing) == 0) ) 
		{
			atsStatus |= kATSStatusCanCheckout;
		}

		// To Allow:	CanGetLatest
		// Criteria:	1) File is out of date
		//				2) or File is missing
		//				3) or File is modified
		if( (iFileStatus & kVFStatusOutOfDate)
			|| (iFileStatus & kVFStatusModified)
			|| (iFileStatus & kVFStatusMissing) )
		{
			atsStatus |= kATSStatusCanGetLatest;
		}

		// Set status
		atsFileList.GetActiveFileEntry(i)->SetFlagsOut( atsStatus );
	}

	return kATSResSuccess;

}

ATSResult SampleATSProvider::GetLockedFileUserList( ATSFileList &atsFileList, ATSUserList &atsUserList, ATSOption atsOptions)
{
	// If your provider does not support this type of information, you can return
	// kATSResErrorNotSupported

	for( UINT i=0; i<atsFileList.NumActiveEntries(); i++ ) {

		ATSUserEntry *pEntry = atsUserList.CreateEntry( _T("") );

		int iFileStatus = kVFStatusNone;
		ATSString strFile = atsFileList.GetActiveFileEntry(i)->GetFile();

		// Get file status for strFile
		// This example assume iFileStatus will contain the status of the file
		// iFileStatus = GetStatusFromServer( strFile );

		if( (iFileStatus & kVFStatusNotOnServer) == 0 )
			continue;

		if( iFileStatus & kVFStatusCheckedOutOther ) {
			ATSString strOtherUserName;
			// Get the username of the "Other" user
			// strOtherUserName = GetOtherUserName( strFile );
			pEntry->SetUserName( strOtherUserName.c_str() );
		}
	}

	return kATSResSuccess;
}

ATSResult SampleATSProvider::ExploreProvider( ATSFileList &atsFileList, ATSExploreProviderParams &exploreParams, ATSOption atsOptions)
{
	// If your provider does not support this feature, you can return
	// kATSResErrorNotSupported

	bool bDialogResult = false;

	// Launch dialog that allows that the user to browse the provider based on parameters
	// with exploreParams.
	// call atsFileList.CreateEntry for each file selected in the dialog
	// bDialogResult = LaunchExplorerDialog( atsFileList, exploreParams );

	if( bDialogResult ) {
		// call atsFileList.BuildActiveList to tag selected files as "active"
		atsFileList.BuildActiveList( kATSStatusActive, true );
		if( exploreParams.GetDownloadFiles() ) {
			// Download the selected files in atsFileList/dialog
		}
	}

	return bDialogResult ? kATSResSuccess : kATSResError;
}

ATSResult SampleATSProvider::ShowProviderOptions( ATSOption atsOptions )
{
	// Display the options dialog for this provider.
	// If a change in options requires a status update (such as changing
	// the local workspace), you should return kATSResSuccessReload.

	return kATSResSuccess;
}

ATSResult SampleATSProvider::LaunchProvider( ATSOption atsOptions )
{
	// This method would typically launch an external client application
	// associated with the provider.

	return kATSResSuccess;
}

ATSResult SampleATSProvider::GetAssociatedFiles( ATSFileList &atsFileList, ATSOption atsOptions )
{
	// If your provider does not support this feature, you can return
	// kATSResErrorNotSupported

	int iFlags = 0;
	if( atsOptions & kATSOptAssocRecurse )
		iFlags |= kVMFileAssocRecurse;
	if( atsOptions & kATSOptAssocChildren )
		iFlags |= kVMFileAssocChildren;
	if( atsOptions & kATSOptAssocParents )
		iFlags |= kVMFileAssocParents;

	for( UINT i=0; i<atsFileList.NumActiveEntries(); i++ ) {

		ATSString strFile = atsFileList.GetActiveFileEntry(i)->GetFile();

		// Step 1. Get associated files for strFile based on iFlags options

		// Step 2. replace the children in the active list with the dependent files:

		// Step 3. Remove any children already associated with this file
		atsFileList.GetActiveFileEntry(i)->RemoveAllChildren();

		// Step 4. Loop through associated files and add then as children to the
		// current file.  Associated files can be added as children with:
		// atsFileList.CreateEntry( szAssociatedFileName, atsFileList.GetActiveFileEntry(i), 0, NULL );
	}

	return kATSResSuccess;
}

bool SampleATSProvider::IsATSMSSCCWrapper()
{
	// All Providers implementing IATSProvider (as this example does) should return
	// false for this method.  ATSMSSCC Wrapper objects are created by ATSCore.
	return false;
}


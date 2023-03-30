//************************************************************************** 
//* Asciiimp.cpp	- Ascii File Importer
//* 
//* NOTE: This is unsupported material - not supported by Kinetix.
//*
//* By Christer Janson
//* Kinetix Development
//*
//* March 19, 1997 CCJ Initial coding
//* November 10, 1997 - CCJ First public release  [Unsupported]
//* 
//*
//* This module contains the DLL startup functions
//*
//* Copyright (c) 1997, All Rights Reserved. 
//***************************************************************************


/*********************************************************************************
REVISION LOG ENTRY
Revision By: rjc
Revised on 4/16/2001 7:19:52 PM
Comments: 16 apr 2001  - rjc : Making changes and upgrades for R4
*********************************************************************************/


/*********************************************************************************
REVISION LOG ENTRY
Revision By: rjc
Revised on 06/29/2000 11:18:37 AM
Comments:  rjc 29 jun 2000 :  Making changes and upgrades for v3.1
*********************************************************************************/
#include "asciiimp.h"

HINSTANCE hInstance;
int controlsInit = FALSE;

#define ASCIIIMP_CLASS_ID1      0x207d2cca
#define ASCIIIMP_CLASS_ID2      0x5df6322c

BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved) 
{
	hInstance = hinstDLL;
	
	if (!controlsInit) {
		controlsInit = TRUE;
		InitCustomControls(hInstance);
		InitCommonControls();
	}
	
	return (TRUE);
}


__declspec( dllexport ) const TCHAR* LibDescription() 
{
	return _T("Ascii Scene Importer");
}

/// MUST CHANGE THIS NUMBER WHEN ADD NEW CLASS 
__declspec( dllexport ) int LibNumberClasses() 
{
	return 1;
}


__declspec( dllexport ) ClassDesc* LibClassDesc(int i) 
{
	switch(i) {
	case 0: return GetAsciiImpDesc();
	default: return 0;
	}
}

__declspec( dllexport ) ULONG LibVersion() 
{
	return VERSION_3DSMAX;
}

class AsciiImpClassDesc:public ClassDesc {
public:
	int                     IsPublic() {return 1;}
	void *                  Create(BOOL loading = FALSE) {return new AsciiImp;} 
	const TCHAR *   ClassName() {return _T("AsciiImp");}
	SClass_ID               SuperClassID() {return SCENE_IMPORT_CLASS_ID;} 
	Class_ID                ClassID() {return Class_ID(ASCIIIMP_CLASS_ID1,
		ASCIIIMP_CLASS_ID2);}
	const TCHAR*    Category() {return _T("Chrutilities");}
};

static AsciiImpClassDesc AsciiImpDesc;
ClassDesc* GetAsciiImpDesc() {return &AsciiImpDesc;}

// Class vars
BOOL AsciiImp::resetScene = FALSE;

AsciiImp::AsciiImp()
{
}

AsciiImp::~AsciiImp()
{
}

int AsciiImp::ExtCount()
{
	return 1;
}

const TCHAR * AsciiImp::Ext(int n)
{
	switch(n) {
	case 0:
		return _T("ASE");
	}
	return _T("");
}

const TCHAR * AsciiImp::LongDesc()
{
	return _T("3D Studio MAX ASCII Scene Importer");
}

const TCHAR * AsciiImp::ShortDesc()
{
	return _T("ASCII Scene Import");
}

const TCHAR * AsciiImp::AuthorName() 
{
	return _T("Christer Janson");
}

const TCHAR * AsciiImp::CopyrightMessage() 
{
	return _T("Copyight (c) 1997 by Kinetix");
}

const TCHAR * AsciiImp::OtherMessage1() 
{
	return _T("");
}

const TCHAR * AsciiImp::OtherMessage2() 
{
	return _T("");
}

unsigned int AsciiImp::Version()
{
	return 100;
}

static BOOL CALLBACK AboutBoxDlgProc(
	HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		CenterWindow(hWnd, GetParent(hWnd)); 
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			EndDialog(hWnd, 1);
			break;
		}
		break;
		default:
			return FALSE;
	}
	return TRUE;
}       

void AsciiImp::ShowAbout(HWND hWnd)
{
	DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_ABOUTBOX),
		hWnd, AboutBoxDlgProc, 0);
}


// Dialog proc
static BOOL CALLBACK ImportDlgProc(
	HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	AsciiImp *imp = (AsciiImp*)GetWindowLong(hWnd,GWL_USERDATA); 
	switch (msg) {
	case WM_INITDIALOG:
		imp = (AsciiImp*)lParam;
		SetWindowLong(hWnd,GWL_USERDATA,lParam); 
		CenterWindow(hWnd, GetParent(hWnd)); 
		CheckDlgButton(hWnd, IDC_RESETSCENE, imp->resetScene); 
		
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			imp->resetScene = IsDlgButtonChecked(hWnd, IDC_RESETSCENE); 

			EndDialog(hWnd, 1);
			break;
		case IDCANCEL:
			EndDialog(hWnd, 0);
			break;
		}
		break;
		default:
			return FALSE;
	}
	return TRUE;
}       

// Dummy function for progress bar
DWORD WINAPI fn(LPVOID arg)
{
	return(0);
}

extern TCHAR* GetToken(FILE* pStream);

void BadFile()
{
	MessageBox(NULL, "Bad file.", "TBD: Error.", MB_OK);
}

// Start the importer!
int AsciiImp::DoImport(const TCHAR *name,ImpInterface *ii,Interface *i, BOOL suppressPrompts) 
{
	int		status = 1;
	int		lastProgress = 0;
	int		progress;
	BOOL	fileValid = FALSE;

	mtlTab.ZeroCount();
	mtlTab.Shrink();

	// Grab the interface pointer.
	ip = i;
	impip = ii;
	
	// Prompt the user with our dialogbox.
	if (!DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_ASCIIIMPORT_DLG),
		ip->GetMAXHWnd(), ImportDlgProc, (LPARAM)this)) {
		return 1;
	}
	
	// Open the stream
	pStream = _tfopen(name,_T("rt"));
	if (!pStream) {
		return 0;
	}

	// Get the file size
	fseek(pStream, 0, SEEK_END);
	long fsize = ftell(pStream);
	fseek(pStream, 0, SEEK_SET);

	if(resetScene) {
		impip->NewScene();
	}

	TCHAR* token;

	
	token = GetToken();
	if (token) {
		if (Compare(token, ID_FILEID)) {
			fileVersion = (int)GetFloat();
			if (fileVersion >= 200 && fileVersion <= 200) {
				fileValid = TRUE;
			}
		}
	}

	if (!fileValid) {
		BadFile();
		fclose(pStream);
		return FALSE;
	}

#ifndef USE_IMPNODES
	ip->DisableSceneRedraw();
#endif

	// Startup the progress bar.
	ip->ProgressStart("Importing file...", TRUE, fn, NULL);

	long fpos;
	while ((token = GetToken())) {

		if (ip->GetCancel()) {
			status = 0;
			break;
		}

		// Update the progress meter
		// We will eliminate flicker by only calling this when it changes.
		fpos = ftell(pStream);
		progress = 100*fpos/fsize;
		if (progress != lastProgress) {
			ip->ProgressUpdate(progress);
		}
		lastProgress = progress;

		DebugPrint("Token: %s\n", token);

		if (Compare(token, ID_SCENE)) {
			ImportSceneParams();
		}
		else if (Compare(token, ID_MATERIAL_LIST)) {
			ImportMaterialList();
		}
		else if (Compare(token, ID_GEOMETRY)) {
			ImportGeomObject();
		}
		else if (Compare(token, ID_SHAPE)) {
			ImportShape();
		}
		else if (Compare(token, ID_HELPER)) {
			ImportHelper();
		}
		else if (Compare(token, ID_CAMERA)) {
			ImportCamera();
		}
		else if (Compare(token, ID_LIGHT)) {
			ImportLight();
		}
		else if (Compare(token, ID_COMMENT)) {
			GetToken();
		}
#ifndef USE_IMPNODES
		// ImpNodes doesn't support group creation!

		else if (Compare(token, ID_GROUP)) {
			groupMgr.BeginGroup(GetString());
			GetToken();		// BlockBegin
			GetToken();		// GroupDummy HelperObject
			GetToken();		// GroupDummy BlockBegin
			SkipBlock();	// GroupDummy
		}
		else if (Compare(token, "}")) {
			groupMgr.EndGroup(ip);
		}
#endif
	}
	
	ip->ProgressEnd();

#ifndef USE_IMPNODES
	ip->EnableSceneRedraw();
#endif

	// Close the stream
	fclose(pStream);
	return status;
}



/**********************************************************************
 *<
	FILE: [!output PROJECT_NAME].cpp

	DESCRIPTION:	Appwizard generated plugin

	CREATED BY: 

	HISTORY: 

 *>	Copyright (c) 2003, All Rights Reserved.
 **********************************************************************/

#include "[!output PROJECT_NAME].h"

#define [!output CLASS_NAME]_CLASS_ID	Class_ID([!output CLASSID1], [!output CLASSID2])

class [!output CLASS_NAME] : public [!output SUPER_CLASS_NAME] {
	public:

		static HWND hParams;
		
		const TCHAR   	*Description        ( ) ;
		
		//TODO: Implement the following methods
		const TCHAR   	*AuthorName         ( ) { return _T("");}
		const TCHAR   	*CopyrightMessage   ( ) { return _T("");}
		UINT           	Version             ( ) { return (VERSION_3DSMAX);}
		//TODO: Return the flags that decsribe the capabilities of the plugin
		DWORD          	Capability          ( ) { return( IMGFLT_FILTER | IMGFLT_MASK | IMGFLT_CONTROL ); }
		
		void           	ShowAbout           ( HWND hWnd );  
		BOOL           	ShowControl         ( HWND hWnd );  
		BOOL           	Render              ( HWND hWnd );
		void			FilterUpdate		( );
		BOOL  			Control	  			(HWND ,UINT ,WPARAM ,LPARAM );

		//Constructor/Destructor
		[!output CLASS_NAME]();
		~[!output CLASS_NAME]();		

};


[!output TEMPLATESTRING_CLASSDESC]

[!if PARAM_MAPS]
[!output TEMPLATESTRING_PARAMBLOCKDESC]
[!endif]


BOOL CALLBACK [!output CLASS_NAME]CtrlDlgProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam) {
	static [!output CLASS_NAME] *f = NULL;
	if (message == WM_INITDIALOG) 
		f = ([!output CLASS_NAME] *)lParam;
	if (f) 
		return (f->Control(hWnd,message,wParam,lParam));
	else
		return(FALSE);
}


static BOOL CALLBACK AboutCtrlDlgProc(
		HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch(msg) {
		
		case WM_INITDIALOG:
			CenterWindow(hWnd,GetParent(hWnd));
			return TRUE;
		
		case WM_COMMAND:
		  switch (LOWORD(wParam)) {
			 
			 case IDOK:              
				  EndDialog(hWnd,1);
				  break;
		  }
		  return TRUE;
	}
	return FALSE;
}

//--- [!output CLASS_NAME] -------------------------------------------------------
[!output CLASS_NAME]::[!output CLASS_NAME]()
{

}

[!output CLASS_NAME]::~[!output CLASS_NAME]()
{

}

const TCHAR *[!output CLASS_NAME]::Description()
{
	//TODO: Return the description of the filter
	return _T("");
}

BOOL [!output CLASS_NAME]::ShowControl(HWND hWnd) {
	return (DialogBoxParam(
		hInstance,
		MAKEINTRESOURCE(IDD_PANEL),
		hWnd,
		(DLGPROC)[!output CLASS_NAME]CtrlDlgProc,
		(LPARAM)this));
}

BOOL [!output CLASS_NAME]::Control(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam) 
{
	//TODO: Implement the message handling code here
	switch (message) {
		case WM_INITDIALOG: {
			//-- Make Dialogue Interactive				
			MakeDlgInteractive(hWnd);
			HWND hWndParent = TheManager->AppWnd();
			CenterWindow(hWnd,hWndParent);
			break;
			}
		case WM_CLOSE:
			DestroyWindow(hWnd);
			break;
		return TRUE;
	}
		

		return FALSE;
}

void [!output CLASS_NAME]::FilterUpdate()
{
	//This method is called when an instance of the filter ir created or updated
}

void [!output CLASS_NAME]::ShowAbout(HWND hWnd)
{
	  DialogBoxParam(
		 hInstance,
		 MAKEINTRESOURCE(IDD_ABOUT),
		 hWnd,
		 (DLGPROC)AboutCtrlDlgProc,
		 (LPARAM)this);
}


BOOL [!output CLASS_NAME]::Render(HWND hWnd)
{
	//TODO: Implement the image altering stuff here
	return TRUE;
}

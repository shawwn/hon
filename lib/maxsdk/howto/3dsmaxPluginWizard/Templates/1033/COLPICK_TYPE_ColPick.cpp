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

class [!output CLASS_NAME]ModelessDlg;
class [!output CLASS_NAME]Dlg
{

public:
	HWND	hwOwner;
	HWND	hwPanel;
	
	DWORD	origRGB;
    DWORD	currentRGB;

	IPoint2 curPos;
	TSTR	colName;

	HSVCallback *callback;
	static IPoint2 initPos;
	BOOL	objColor;
	[!output CLASS_NAME]ModelessDlg *sm;


	// Modeless and Modal support
	BOOL	Modeless;
	int		StartModal();
	HWND	StartModeless();


	// Construct and initialize
	[!output CLASS_NAME]Dlg(HWND hOwner, DWORD col,  IPoint2* pos,
			HSVCallback *cb, TCHAR *name, int objClr = 0, [!output CLASS_NAME]ModelessDlg *smp = NULL);
	~[!output CLASS_NAME]Dlg();
	void DoPaint(HWND hWnd);


	// Modify the dialog's settings
	void SetNewColor (DWORD color, TCHAR *name);
	void ModifyColor (DWORD color);
	DWORD GetColor();
	IPoint2 GetPosition();
	void InstallNewCB(DWORD col, HSVCallback *pcb, TCHAR *name);
};

class [!output CLASS_NAME]ModelessDlg: public ColorPicker 
{
	HWND hwnd;
	[!output CLASS_NAME]Dlg *colDlg;

public:
		[!output CLASS_NAME]ModelessDlg(HWND hwndOwner, DWORD initColor, IPoint2* pos, HSVCallback* callback, TCHAR *name, int objClr);
		~[!output CLASS_NAME]ModelessDlg();

		void SetNewColor (DWORD color, TCHAR *name);
		void ModifyColor (DWORD color);
		void InstallNewCB(DWORD col, HSVCallback *pcb, TCHAR *name);

		DWORD GetColor();
		IPoint2 GetPosition();
		void Destroy();
};



class [!output CLASS_NAME] : public [!output SUPER_CLASS_NAME] {
	public:

		static HWND hParams;

	// Do Modal dialog
	int ModalColorPicker(
		HWND hwndOwner, 		// owning window
		DWORD *lpc,				// pointer to color to be edited
	    IPoint2 *spos, 			// starting position, set to ending position
	    HSVCallback *callBack,	// called when color changes
		TCHAR *name				// name of color being edited
	    );

	// Create Modeless dialog.
	ColorPicker *CreateColorPicker(
		HWND hwndOwner,   
		DWORD initColor,  
		IPoint2* spos,    
		HSVCallback *pcallback,
		TCHAR *name, 	  
		BOOL isObjectColor);

	const TCHAR *	ClassName() {  return GetString(IDS_CLASS_NAME); 	}	

	INT_PTR Execute(int cmd, ULONG_PTR arg1=0, ULONG_PTR arg2=0, ULONG_PTR arg3=0) {return 0; } 	
	Class_ID ClassID() { return [!output CLASS_NAME]_CLASS_ID; }
	void DeleteThis(){delete this;}
		
		//Constructor/Destructor

		[!output CLASS_NAME]();
		~[!output CLASS_NAME]();		

};


[!output TEMPLATESTRING_CLASSDESC]

[!if PARAM_MAPS]
[!output TEMPLATESTRING_PARAMBLOCKDESC]
[!endif]


BOOL CALLBACK [!output CLASS_NAME]OptionsDlgProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam) {
	static [!output CLASS_NAME] *imp = NULL;

	switch(message) {
		case WM_INITDIALOG:
			imp = ([!output CLASS_NAME] *)lParam;
			CenterWindow(hWnd,GetParent(hWnd));
			return TRUE;

		case WM_CLOSE:
			EndDialog(hWnd, 0);
			return TRUE;
	}
	return FALSE;
}


[!output CLASS_NAME]::[!output CLASS_NAME]()
{

}

[!output CLASS_NAME]::~[!output CLASS_NAME]()
{

}

IPoint2 [!output CLASS_NAME]Dlg::initPos(354,94);

void [!output CLASS_NAME]Dlg::DoPaint(HWND hWnd)
{
	// TODO: Handle the painting here
}

int [!output CLASS_NAME]Dlg::StartModal()
{
	Modeless = FALSE;
	return (int)DialogBoxParam(
		hInstance, 
		MAKEINTRESOURCE(IDD_PANEL), 
		hwOwner, 
		[!output CLASS_NAME]OptionsDlgProc,
		(LPARAM)this);
}

HWND [!output CLASS_NAME]Dlg::StartModeless()
{
	Modeless = TRUE;
	return (HWND)CreateDialogParam(
		hInstance, 
		MAKEINTRESOURCE(IDD_PANEL), 
		hwOwner, 
		[!output CLASS_NAME]OptionsDlgProc,
		(LPARAM)this);
}



[!output CLASS_NAME]Dlg::[!output CLASS_NAME]Dlg(HWND hOwner, DWORD col,  IPoint2* pos,
								HSVCallback *cb, TCHAR *name, int objClr, [!output CLASS_NAME]ModelessDlg *smp)
{
	hwOwner = hOwner;
	origRGB = currentRGB = col;
	if(pos) curPos = initPos = *pos;
	callback = cb;
	colName = name;
	Modeless = FALSE;
	hwPanel = NULL;
	objColor = objClr?TRUE:FALSE;
	sm = smp;
}

[!output CLASS_NAME]Dlg::~[!output CLASS_NAME]Dlg()
{
	if(hwPanel) hwPanel = NULL;
}


void [!output CLASS_NAME]Dlg::SetNewColor (DWORD color, TCHAR *name)
{
	colName = name;
	origRGB = currentRGB = color;
	SetWindowText(hwPanel,colName);
	InvalidateRect(hwPanel,NULL,FALSE);
}

void [!output CLASS_NAME]Dlg::ModifyColor (DWORD color)
{
	currentRGB=color;
	InvalidateRect(hwPanel,NULL,FALSE);
}

DWORD [!output CLASS_NAME]Dlg::GetColor()
{
	return currentRGB;
}

IPoint2 [!output CLASS_NAME]Dlg::GetPosition()
{
	return curPos;
}

void [!output CLASS_NAME]Dlg::InstallNewCB(DWORD col, HSVCallback *pcb, TCHAR *name)
{
	if (callback) 
		callback->ColorChanged(currentRGB,1);
	callback = pcb;
    currentRGB = origRGB = col;
	colName = name;
	SetWindowText(hwPanel,colName);
	InvalidateRect(hwPanel,NULL,FALSE);
}

ColorPicker* [!output CLASS_NAME]::CreateColorPicker(HWND hwndOwner, DWORD initColor,
													IPoint2* spos, HSVCallback *pcallback,
													TCHAR *name, int objClr) 
{
	return new [!output CLASS_NAME]ModelessDlg(hwndOwner,initColor,spos,pcallback,name,objClr);
}	


int [!output CLASS_NAME]::ModalColorPicker(HWND hwndOwner, DWORD *lpc, IPoint2 *spos,
										  HSVCallback *callBack, TCHAR *name) 
{
	int res = 0;
	[!output CLASS_NAME]Dlg colDlg(hwndOwner,*lpc,spos,callBack,name);
	res = colDlg.StartModal();

	if(lpc)		*lpc = colDlg.GetColor();
	if(spos)	*spos = colDlg.GetPosition();

	return res;
}


/*===========================================================================*\
 |	[!output CLASS_NAME]ModelessDlg methods
\*===========================================================================*/


[!output CLASS_NAME]ModelessDlg::[!output CLASS_NAME]ModelessDlg(HWND hwndOwner, DWORD initColor, 
								   IPoint2* pos, HSVCallback* callback, 
								   TCHAR *name, int objClr)
{
	hwnd = NULL; colDlg = NULL;
	colDlg = new [!output CLASS_NAME]Dlg(hwndOwner,initColor,pos,callback,name,objClr,this);
	hwnd = colDlg->StartModeless();
}


void [!output CLASS_NAME]ModelessDlg::Destroy() {
	DestroyWindow(hwnd);
}

[!output CLASS_NAME]ModelessDlg::~[!output CLASS_NAME]ModelessDlg() {
	Destroy();
	if(colDlg) delete colDlg;
	hwnd = NULL; colDlg = NULL;
}


// Transactions with the dialog handler
void [!output CLASS_NAME]ModelessDlg::SetNewColor (DWORD color, TCHAR *name)
{
	colDlg->SetNewColor(color,name);
}

void [!output CLASS_NAME]ModelessDlg::ModifyColor (DWORD color)
{
	colDlg->ModifyColor(color);
}

void [!output CLASS_NAME]ModelessDlg::InstallNewCB(DWORD col, HSVCallback *pcb, TCHAR *name)
{
	colDlg->InstallNewCB(col,pcb,name);
}

DWORD [!output CLASS_NAME]ModelessDlg::GetColor()
{
	return colDlg->GetColor();
}

IPoint2 [!output CLASS_NAME]ModelessDlg::GetPosition()
{
	return colDlg->GetPosition();
}

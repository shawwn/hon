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

		HWND			hPanel;
		ITVUtility		*iu;
		Interface		*ip;
		
		void BeginEditParams(Interface *ip,ITVUtility *iu);
		void EndEditParams(Interface *ip,ITVUtility *iu);
		
		void Init(HWND hWnd);
		void Destroy(HWND hWnd);

		void TrackSelectionChanged();
		void NodeSelectionChanged();
		void KeySelectionChanged();
		void TimeSelectionChanged();
		void MajorModeChanged();
		void TrackListChanged();
		
		void DeleteThis() { }		
		
		//Constructor/Destructor
		[!output CLASS_NAME]();
		~[!output CLASS_NAME]();		

};

static [!output CLASS_NAME] the[!output CLASS_NAME];

[!output TEMPLATESTRING_CLASSDESC]

[!if PARAM_MAPS]
[!output TEMPLATESTRING_PARAMBLOCKDESC]
[!endif]

static BOOL CALLBACK [!output CLASS_NAME]DlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
		case WM_INITDIALOG:
			the[!output CLASS_NAME].Init(hWnd);
			CenterWindow(hWnd,GetParent(hWnd));
			break;

		case WM_DESTROY:
			the[!output CLASS_NAME].Destroy(hWnd);
			break;

		case WM_COMMAND:
			break;

		case WM_CLOSE:
			DestroyWindow(hWnd);
			break;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:
			the[!output CLASS_NAME].ip->RollupMouseMessage(hWnd,msg,wParam,lParam); 
			break;

		default:
			return FALSE;
	}
	return TRUE;
}

[!output CLASS_NAME]::[!output CLASS_NAME]()
{
	iu = NULL;
	ip = NULL;	
	hPanel = NULL;
}

[!output CLASS_NAME]::~[!output CLASS_NAME]()
{

}

void [!output CLASS_NAME]::BeginEditParams(Interface *ip,ITVUtility *iu)
{
	this->iu = iu;
	this->ip = ip;
	hPanel = CreateDialogParam(
		hInstance,
		MAKEINTRESOURCE(IDD_PANEL),
		iu->GetTrackViewHWnd(),
		[!output CLASS_NAME]DlgProc,
		(LONG)this);
}

void [!output CLASS_NAME]::EndEditParams(Interface *ip,ITVUtility *iu)
{
	this->iu = NULL;
	this->ip = NULL;
	ip->DeleteRollupPage(hPanel);
	hPanel = NULL;
}

void [!output CLASS_NAME]::TrackSelectionChanged()
{

}

void [!output CLASS_NAME]::NodeSelectionChanged()
{

}

void [!output CLASS_NAME]::KeySelectionChanged()
{

}

void [!output CLASS_NAME]::TimeSelectionChanged()
{

}

void [!output CLASS_NAME]::MajorModeChanged()
{

}

void [!output CLASS_NAME]::TrackListChanged()
{

}

void [!output CLASS_NAME]::Init(HWND hWnd)
{

}

void [!output CLASS_NAME]::Destroy(HWND hWnd)
{

}


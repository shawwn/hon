
#include "PaintDeformTest.h"



////////////////////////////////////////////////////////////////////////////////////////
//
// For ParamMap2 Create an instance of a class derived from ParamMap2UserDlgProc.  This 
// is used to provide special processing of controls in the rollup page.-- NH 
// The original use of this has been replaced by the use of PBAccessor
//
////////////////////////////////////////////////////////////////////////////////////////


BOOL PaintDeformTestDlgProc::DlgProc(
		TimeValue t,IParamMap2 *map,
		HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch (msg) 
		{	
		case WM_INITDIALOG:
			{
			mod->InitUI(hWnd);
			break;
			}
		case WM_DESTROY:
			{
			mod->DestroyUI(hWnd);
			break;
			}
		case WM_COMMAND:
			switch (LOWORD(wParam)) 
				{
				//case IDC_SEL_SPHERE:				
					//mod->flags |= CONTROL_UNIFORM|CONTROL_HOLD;
					//mod->NotifyDependents(FOREVER,SELECT_CHANNEL,REFMSG_CHANGE);
					//break;

				case IDC_PAINT:
					mod->Paint();
					break;
				case IDC_PAINTOPTIONS:
					mod->PaintOptions();
					break;
				}
		}

	return FALSE;
	
}


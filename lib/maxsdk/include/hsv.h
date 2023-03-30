/**********************************************************************
 *<
	FILE: hsv.h

	DESCRIPTION:

	CREATED BY: Dan Silva

	HISTORY:

	NH 16|05|03 - Added AColor support.

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#ifndef __HSV__H
#define __HSV__H


#define MAXCOLORS 16


// This callback proc gets called after every mouse button up to tell you the
// new color, if you want to do interactive update.

class HSVCallback {
	public:
		virtual	void ButtonDown() {}
		virtual	void ButtonUp(BOOL accept) {}
		virtual	void ColorChanged(DWORD col, BOOL buttonUp) {}
		virtual void ColorChanged(AColor col, BOOL ButtonUp) {}
		virtual	void BeingDestroyed(IPoint2 pos)=0;	// gets called when picker is closed: 
	};

// Put up the Modal dialog.
extern CoreExport INT_PTR HSVDlg_Do(
		// WIN64 Cleanup: Shuler
	HWND hwndOwner, 		// owning window
	DWORD *lpc,				// pointer to color to be edited
    IPoint2 *spos, 			// starting position, set to ending position
    HSVCallback *callBack,	// called when color changes
	TCHAR *name				// name of color being edited
    );

// NH 16|05|03 Added this method to support AColor.
extern CoreExport INT_PTR HSVDlg_Do(
									// WIN64 Cleanup: Shuler
									HWND hwndOwner, 		// owning window
									AColor *lpc,				// pointer to color to be edited
									IPoint2 *spos, 			// starting position, set to ending position
									HSVCallback *callBack,	// called when color changes
									TCHAR *name				// name of color being edited
									);

CoreExport void RGBtoHSV (DWORD rgb, int *ho, int *so, int *vo);
CoreExport DWORD HSVtoRGB (int H, int S, int V);
CoreExport void HSVtoHWBt (int h, int s, int v, int *ho, int *w, int *bt);
CoreExport void HWBttoHSV (int h, int w, int bt, int *ho, int *s, int *v);

// RB: Added floating point versions
class Color;
CoreExport Color RGBtoHSV(Color rgb);
CoreExport Color HSVtoRGB(Color hsv);

class AColor;

// MODELESS Version

class ColorPicker : public InterfaceServer {
	protected:
	~ColorPicker() {}
	public:
		ColorPicker() {}
		virtual void ModifyColor (DWORD color)=0;
		virtual void SetNewColor (DWORD color, TCHAR *name)=0;  
		virtual DWORD GetColor()=0;
		virtual IPoint2 GetPosition()=0;
		virtual void Destroy()=0;  // remove window and delete ColorPicker.
		virtual void InstallNewCB(DWORD col, HSVCallback *pcb, TCHAR *name)=0;
		virtual void RefreshUI() {}  // Called when display gamma changes


		// NH 16|05|03 Added this method to support AColor. 
		virtual void ModifyColor(AColor color){}
		// NH 16|05|03 Added this method to support AColor. 
		virtual void SetNewColor(AColor, TCHAR * name){}
		// NH 16|05|03 Added this method to support AColor. THis will return the AColor from the picker.
		virtual AColor GetAColor(){return AColor(0,0,0,0);}
		// NH 16|05|03 Added this method to support AColor. 
		virtual void InstallNewCB(AColor col, HSVCallback *pcb, TCHAR *name){}
		
	};

// Create A Modeless Color Picker
CoreExport ColorPicker *CreateColorPicker(HWND hwndOwner, DWORD initColor,
	 IPoint2* spos, HSVCallback *pcallback, TCHAR *name, int objClr=0);


// Create A Modeless Color Picker
// NH 16|05|03 Added this method to support AColor.
CoreExport ColorPicker *CreateColorPicker(HWND hwndOwner, AColor initColor,
	 IPoint2* spos, HSVCallback *pcallback, TCHAR *name, int objClr=0);
	 
CoreExport void SetCPInitPos(IPoint2 &pos);
CoreExport IPoint2 GetCPInitPos(void);	

#define WM_ADD_COLOR	(WM_USER+2321)	// wParam = color

//--------------------------------------------------------------------------
// Pluggable color picker class ( COLPICK_CLASS_ID )
//--------------------------------------------------------------------------


class ColPick : public InterfaceServer {
	public:
	// Do Modal dialog
	virtual INT_PTR ModalColorPicker(
			// WIN64 Cleanup: Shuler
		HWND hwndOwner, 		// owning window
		DWORD *lpc,				// pointer to color to be edited
	    IPoint2 *spos, 			// starting position, set to ending position
	    HSVCallback *callBack,	// called when color changes
		TCHAR *name				// name of color being edited
	    )=0;
	
	virtual INT_PTR ModalColorPicker(
		// WIN64 Cleanup: Shuler
		HWND hwndOwner, 		// owning window
		AColor *lpc,				// pointer to color to be edited
		IPoint2 *spos, 			// starting position, set to ending position
		HSVCallback *callBack,	// called when color changes
		TCHAR *name				// name of color being edited
		){return 0;}

	// Create Modeless dialog.
	virtual	ColorPicker *CreateColorPicker(
		HWND hwndOwner,   // owning window
		DWORD initColor,  // inital value of color
		IPoint2* spos,    // starting position of dialog
		HSVCallback *pcallback, // call back when color changes
		TCHAR *name, 	  // name of color being edited
		BOOL isObjectColor=FALSE)=0;

	virtual	ColorPicker *CreateColorPicker(
		HWND hwndOwner,   // owning window
		AColor initColor,  // inital value of color
		IPoint2* spos,    // starting position of dialog
		HSVCallback *pcallback, // call back when color changes
		TCHAR *name, 	  // name of color being edited
		BOOL isObjectColor=FALSE){return NULL;}

	virtual const TCHAR *ClassName()=0;
	virtual Class_ID ClassID()=0;
	virtual void DeleteThis()=0;
	virtual INT_PTR Execute(int cmd, ULONG_PTR arg1=0, ULONG_PTR arg2=0, ULONG_PTR arg3=0)=0; 
	};

//--------------------------------------------------------------------------
// the class id for the default color picker is Class_ID(DEFAULT_COLPICK_CLASS_ID,0)

#define DEFAULT_COLPICK_CLASS_ID 1

//--------------------------------------------------------------------------
// These are used by the MAX to plug in the current color picker.  
// Developers should not need to access these.
CoreExport ColPick *SetCurColPick(ColPick *colpick);
CoreExport ColPick *GetCurColPick();
//--------------------------------------------------------------------------
   
//--------------------------------------------------------------------------


#endif

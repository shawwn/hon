// (C)2007 S2 Games
// ui_linux.h
//=============================================================================
#ifndef UI_LINUX_H_
#define UI_LINUX_H_

struct UIAPI
{
	// Initialize the UI, display window, cleans up if fails
	// returns:	0 = success
	//			-1 = failure
	int		(*Init)(const char* sTitle, const char* sMessage);
	
	// Cleanup all resources, close windows, etc
	void	(*Cleanup)(void);
	
	// these shouldn't fail...
	void	(*SetTitle)(const char* sTitle);	// Set window title
	void	(*SetMessage)(const char* sMessage);// Set message
	void	(*SetProgress)(float fProgress);	// Set updater progress [0.0,1.0]
	
	// Display error message, wait for user to acknowledge
	void	(*ErrorMessage)(const char* sError);
	
	// Update the UI, handle events
	// returns:	0 = no reaction needed
	//			1 = quit/cancel message received
	int		(*Update)(void);
};

// return the filled in API, 0 = success, -1 = error?
int GetUIAPI(struct UIAPI* api);

typedef int (*PFNGETUIAPI)(struct UIAPI*) ;

#endif /*UI_LINUX_H_*/

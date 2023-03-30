//DLLMain.cpp

#include "maxscrpt.h"
HINSTANCE hInstance;
extern void IntervalArrayInit ();
// ========================================================
// Grab onto this DLL's instance handle
BOOL WINAPI DllMain(HINSTANCE DLLhinst, DWORD fdwReason, LPVOID lpvReserved)
{
	switch(fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		hInstance = DLLhinst;
		break;
	}
	return TRUE;
}
__declspec(dllexport) void LibInit()
{
	//TODO: Put any code for initializing your plugin here. 
	//In this case it is IntervalArrayInit(), which will be defined in IntervalArray.cpp,
	//However in this example IntervalArrayInit() is empty.
	IntervalArrayInit();
}
__declspec(dllexport) const TCHAR* LibDescription()
{
	//TODO: Put code in here telling what your plugin does.
	return _T("Interval Array Function");
}
__declspec(dllexport) ULONG LibVersion()
{
	//Return the version of the Max SDK
	return VERSION_3DSMAX;
}

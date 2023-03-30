/*==============================================================================

  file:     Utility.cpp

  author:   Daniel Levesque

  created:  16 June 2003

  description:

    Implementation of utility functions for the mrObjects plugin.

  modified:	


© 2004 Autodesk
==============================================================================*/

#include <Windows.h>
#include "Utility.h"

TCHAR *GetString(int id)
{
	static TCHAR buf[256];

    extern HINSTANCE hInstance;

	if (hInstance)
		return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;
	return NULL;
}

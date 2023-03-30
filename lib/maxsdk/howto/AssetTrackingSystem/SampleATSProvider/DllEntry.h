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
// DESCRIPTION: Sample ATS Provider DLL Entry Header
// AUTHOR: Michael Russo - created June 3, 2005
//***************************************************************************/

#ifndef __DLLENTRY__H
#define __DLLENTRY__H

#include <windows.h>
#include <commctrl.h>
#include <tchar.h>
#include "resource.h"

extern TCHAR *GetString(int id);
extern HINSTANCE hInstance;

#endif // __DLLENTRY__H

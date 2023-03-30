/**********************************************************************
 *<
	FILE: PolyTest.h

	DESCRIPTION:

	CREATED BY: Steve Anderson, based on mods.h

	HISTORY: Created 2001

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#ifndef __BASIC_OPS__H
#define __BASIC_OPS__H

#include "Max.h"
#include "resource.h"

// Modifiers hereabouts:
extern ClassDesc* GetPolyTestModDesc();

TCHAR *GetString(int id);

// in PolyTest.cpp
extern HINSTANCE hInstance;

// For 'Supports Object of Type' rollups
extern INT_PTR CALLBACK DefaultSOTProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);

#define BIGFLOAT	float(9999999)

#define NEWSWMCAT	_T("Modifiers")

#endif


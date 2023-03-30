// (C)2008 S2 Games
// c_timer_win32.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k0_common.h"

#include "c_timer.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <mmsystem.h>
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
//=============================================================================

/*====================
  CTimer::CTimer
  ====================*/
CTimer::CTimer()
{
	// Set system start time so timers counts from 0
	LARGE_INTEGER ll;
	::QueryPerformanceCounter(&ll);
	m_llStartTicks = ll.QuadPart;
}


/*====================
  CTimer::GetMilliseconds
  ====================*/
uint	CTimer::GetMilliseconds()
{
	return uint(((GetTicks() - m_llStartTicks)) / (GetFrequency() / 1000));
}


/*====================
  CTimer::GetMilliseconds
  ====================*/
uint	CTimer::GetMicroseconds()
{
	return uint(((GetTicks() - m_llStartTicks)) / (GetFrequency() / 1000000));
}


/*====================
  CTimer::GetTicks
  ====================*/
LONGLONG	CTimer::GetTicks()
{
	LARGE_INTEGER ll;
	QueryPerformanceCounter(&ll);
	return ll.QuadPart;
}


/*====================
  CTimer::GetFrequency
  ====================*/
LONGLONG	CTimer::GetFrequency()
{
	LARGE_INTEGER ll;
	QueryPerformanceFrequency(&ll);
	return ll.QuadPart;
}

#endif	//_WIN32

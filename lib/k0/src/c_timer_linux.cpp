// (C)2008 S2 Games
// c_timer_win32.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k0_common.h"

#include "c_timer.h"

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
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	m_llStartTicks = ts.tv_sec * 1000000ll + ts.tv_nsec / 1000ull;
}


/*====================
  CTimer::GetMilliseconds
  ====================*/
uint	CTimer::GetMilliseconds()
{
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	return uint((ts.tv_sec * 1000ull + ts.tv_nsec / 1000000ull)-m_llStartTicks / 1000ull);
}


/*====================
  CTimer::GetMilliseconds
  ====================*/
uint	CTimer::GetMicroseconds()
{
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	return uint((ts.tv_sec * 1000000ull + ts.tv_nsec / 1000ull)-m_llStartTicks);
}


/*====================
  CTimer::GetTicks
  ====================*/
LONGLONG	CTimer::GetTicks()
{
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	return (ts.tv_sec * 100000000ull + ts.tv_nsec / 10ull);
}


/*====================
  CTimer::GetFrequency
  ====================*/
LONGLONG	CTimer::GetFrequency()
{
	return 100000000;
}

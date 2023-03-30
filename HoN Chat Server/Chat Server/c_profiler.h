// (C)2009 S2 Games
// c_profiler.h
//
//=============================================================================
#ifndef __C_PROFILER_H__
#define __C_PROFILER_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_core.h"

#include "c_profilenode.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
#ifdef NO_PROFILE
#define PROFILE(s)
#define START_PROFILE(s)
#define END_PROFILE
#else //NO_PROFILE
#define PROFILE(s)			CProfileSample __sample(m_pProfiler, s);
#define START_PROFILE(s)	{ CProfileSample __sample(m_pProfiler, s);
#define END_PROFILE			}
#endif //NO_PROFILE
//=============================================================================

//=============================================================================
// CProfiler
//=============================================================================
class CProfiler
{
private:
	DECLARE_CORE_API;

	bool			m_bStartRequested;
	bool			m_bStopRequested;
	bool			m_bActive;

	CProfileNode	m_nodeRoot;
	CProfileNode*	m_pCurrentNode;

public:
	~CProfiler()	{}
	CProfiler();

	void	Initialize(CCore *pCore);

	void	StartFrame();
	void	EndFrame();

	void	RequestStartProfiling()		{ m_bStartRequested = true; }
	void	RequestStopProfiling()		{ m_bStopRequested = true; }

	void	StartProfiling();
	void	EndProfiling();

	void	ResetData();
	void	PrintReport();

	uint	StartSample(const char *szName);
	void	EndSample(uint uiHandle);
};
//=============================================================================

#endif //__C_PROFILER_H__

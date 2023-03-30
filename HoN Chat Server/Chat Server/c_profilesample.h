// (C)2009 S2 Games
// c_profilesample.h
//
//=============================================================================
#ifndef __C_PROFILESAMPLE_H__
#define __C_PROFILESAMPLE_H__

//=============================================================================
// CProfileSample
//=============================================================================
class CProfileSample
{
private:
	CProfiler*	m_pProfiler;
	uint		m_uiHandle;

	CProfileSample();

public:
	~CProfileSample()
	{
		m_pProfiler->EndSample(m_uiHandle);
	}
	
	CProfileSample(CProfiler *pProfiler, const char *szName) :
	m_pProfiler(pProfiler),
	m_uiHandle(m_pProfiler->StartSample(szName))
	{}
};
//=============================================================================

#endif //__C_PROFILESAMPLE_H__

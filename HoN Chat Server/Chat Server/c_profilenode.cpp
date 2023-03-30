// (C)2009 S2 Games
// c_profilenode.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "chatserver_common.h"

#include "c_profilenode.h"
#include "c_system.h"
#include "c_console.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
uint CProfileNode::s_uiDepth(0);
//=============================================================================

/*====================
  CProfileNode::CProfileNode
  ====================*/
CProfileNode::CProfileNode(const char *szName, CProfileNode *pParent) :
m_szName(szName),
m_pParent(pParent),

m_llStartTime(0),
m_llRecursiveCalls(0),

m_llCallsThisFrame(0),
m_llTimeThisFrame(0),

m_llTotalCalls(0),
m_llTotalTime(0),
m_llUnloggedTime(0),

m_llLongestTime(0),
m_llMaxFrameCalls(0)
{
}


/*====================
  CProfileNode::NewFrame
  ====================*/
void	CProfileNode::NewFrame()
{
	m_llStartTime = 0;
	m_llRecursiveCalls = 0;

	m_llCallsThisFrame = 0;
	m_llTimeThisFrame = 0;

	for (uint ui(0); ui < m_vChildNodes.size(); ++ui)
		m_vChildNodes[ui]->NewFrame();
}


/*====================
  CProfileNode::Reset
  ====================*/
void	CProfileNode::Reset()
{
	m_llStartTime = 0;
	m_llRecursiveCalls = 0;

	m_llCallsThisFrame = 0;
	m_llTimeThisFrame = 0;

	m_llTotalCalls = 0;
	m_llTotalTime = 0;
	m_llUnloggedTime = 0;

	m_llLongestTime = 0;
	m_llMaxFrameCalls = 0;

	for (uint ui(0); ui < m_vChildNodes.size(); ++ui)
		m_vChildNodes[ui]->Reset();
}


/*====================
  CProfileNode::PrintReport
  ====================*/
void	CProfileNode::PrintReport(CConsole *pConsole, CSystem *pSystem)
{
	string sIndent(s_uiDepth, ' ');

	const LONGLONG llFrequency(pSystem->GetFrequency());

	if (m_llTotalCalls > 0)
	{
		// Name
		pConsole->Perf() << XtoS(sIndent + m_szName, FMT_ALIGNLEFT, 50);
		
		// Calls
		pConsole->Perf() << XtoW(m_llTotalCalls, FMT_NONE, 9);
		if (m_pParent != NULL && m_pParent->m_llTotalCalls != 0)
			pConsole->Perf() << XtoW(L" (" + XtoA(m_llTotalCalls / m_pParent->m_llTotalCalls) + L")", FMT_NONE, 9);
		else
			pConsole->Perf() <<XtoW(WSNULL, FMT_NONE, 9);
		
		m_llUnloggedTime = m_llTotalTime;
		for (uint ui(0); ui < m_vChildNodes.size(); ++ui)
			m_llUnloggedTime -= m_vChildNodes[ui]->m_llTotalTime;

		// Time
		pConsole->Perf() << XtoW((double(m_llTotalTime) / m_llTotalCalls) / llFrequency * 1000.0, FMT_NONE, 7, 2);
		if (m_pParent != NULL && m_pParent->m_llTotalCalls != 0)
			pConsole->Perf() << XtoW(L"(" + XtoW((m_llTotalTime / double(m_pParent->m_llTotalTime)) * 100.0f, FMT_NONE, 0, 1) + L"%)", FMT_NONE, 9);
		else
			pConsole->Perf() << XtoW(WSNULL, FMT_NONE, 9);
		pConsole->Perf() << XtoW(L"[" + XtoW((double(m_llUnloggedTime) / m_llTotalCalls) / llFrequency * 1000.0, FMT_NONE, 0, 2) + L"]", FMT_NONE, 8);
		pConsole->Perf() << XtoW(L"[" + XtoW((m_llUnloggedTime / double(m_llTotalTime)) * 100.0f, FMT_NONE, 0, 1) + L"%]", FMT_NONE, 9);
		pConsole->Perf() << XtoW(double(m_llLongestTime) / llFrequency * 1000.0, FMT_NONE, 7, 2);
		
		pConsole->Perf() << newl;
	}

	++s_uiDepth;

	for (uint ui(0); ui < m_vChildNodes.size(); ++ui)
		m_vChildNodes[ui]->PrintReport(pConsole, pSystem);

	--s_uiDepth;
}


/*====================
  CProfileNode::GetCondensedData
  ====================*/
void	CProfileNode::GetCondensedData(map<string, SProfileData> &mapTotals)
{
	mapTotals[m_szName].sName = m_szName;
	mapTotals[m_szName].bNotLeaf = (mapTotals[m_szName].bNotLeaf || !m_vChildNodes.empty());
	mapTotals[m_szName].llLongestTime = MAX(m_llLongestTime, mapTotals[m_szName].llLongestTime);
	mapTotals[m_szName].llTotalCalls += m_llTotalCalls;
	mapTotals[m_szName].llTotalTime += m_llTotalTime;
	mapTotals[m_szName].llUnloggedTime += m_llUnloggedTime;

	for (uint ui(0); ui < m_vChildNodes.size(); ++ui)
		m_vChildNodes[ui]->GetCondensedData(mapTotals);
}

// (C)2009 S2 Games
// c_profiler.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "chatserver_common.h"

#include "c_profiler.h"
#include "c_console.h"
//=============================================================================

/*====================
  CProfiler::CProfiler
  ====================*/
CProfiler::CProfiler() :
NULL_CORE_API,
m_bActive(false),
m_bStartRequested(false),
m_bStopRequested(false),
m_nodeRoot("root", NULL),
m_pCurrentNode(&m_nodeRoot)
{
}


/*====================
  CProfiler::Initialize
  ====================*/
void	CProfiler::Initialize(CCore *pCore)
{
	INIT_CORE_API(pCore);

	m_bActive = false;
	m_bStartRequested = false;
	m_bStopRequested = false;
}


/*====================
  CProfiler::StartFrame
  ====================*/
void	CProfiler::StartFrame()
{
	if (!m_bActive)
	{
		if (m_bStartRequested)
			StartProfiling();
		else
			return;
	}

	m_nodeRoot.NewFrame();
	m_nodeRoot.StartTimer(m_pSystem);
}


/*====================
  CProfiler::EndFrame
  ====================*/
void	CProfiler::EndFrame()
{
	if (!m_bActive)
		return;

	m_nodeRoot.EndTimer(m_pSystem);

	if (m_bStopRequested)
		EndProfiling();
}


/*====================
  CProfiler::StartProfiling
  ====================*/
void	CProfiler::StartProfiling()
{
	m_bActive = true;
	m_bStartRequested = false;
	ResetData();
}


/*====================
  CProfiler::EndProfiling
  ====================*/
void	CProfiler::EndProfiling()
{
	m_bActive = false;
	m_bStartRequested = false;
	m_bStopRequested = false;
	PrintReport();
}


/*====================
  CProfiler::ResetData
  ====================*/
void	CProfiler::ResetData()
{
	m_pCurrentNode = &m_nodeRoot;
	m_pCurrentNode->Reset();
}


/*====================
  CProfiler::PrintReport
  ====================*/
bool	SortMax(SProfileData &A, SProfileData &B)
{
	if (A.bNotLeaf == B.bNotLeaf)
		return A.llLongestTime > B.llLongestTime;
	if (A.bNotLeaf)
		return false;
	return true;
}

bool	SortUnlogged(SProfileData &A, SProfileData &B)
{
	if (A.llTotalCalls == 0)
		return false;
	if (B.llTotalCalls == 0)
		return true;
	return (A.llUnloggedTime / A.llTotalCalls) > (B.llUnloggedTime / B.llTotalCalls);
}

bool	SortTotalTime(SProfileData &A, SProfileData &B)
{
	if (A.bNotLeaf && B.bNotLeaf)
		return A.llUnloggedTime > B.llUnloggedTime;
	else if (A.bNotLeaf)
		return A.llUnloggedTime > B.llTotalTime;
	else if (B.bNotLeaf)
		return A.llTotalTime > B.llUnloggedTime;
	else
		return A.llTotalTime > B.llTotalTime;
}

bool	SortMostCalls(SProfileData &A, SProfileData &B)
{
	if (A.bNotLeaf == B.bNotLeaf)
		return A.llTotalCalls > B.llTotalCalls;
	if (A.bNotLeaf)
		return false;
	return true;
}

void	CProfiler::PrintReport()
{
	wstring sDateTime(m_pCore->GetDateTimeString());
	for (uint ui(0); ui < sDateTime.size(); ++ui)
	{
		if (sDateTime[ui] == L'/')
			sDateTime[ui] = L'-';
		if (sDateTime[ui] == L':')
			sDateTime[ui] = L'.';
	}

	m_pConsole->Perf().SetLogFile(L"./logs/perf-" + sDateTime + L".log");
	m_pConsole->Perf() << XtoW(L"        ", FMT_ALIGNLEFT, 50) << XtoW(L"      Calls       ", FMT_NONE, 18) << XtoW(L"                  Time                 ", FMT_NONE, 40) << newl;
	m_pConsole->Perf() << XtoW(L"Function", FMT_ALIGNLEFT, 50) << XtoW(L"Total  (Per Frame)", FMT_NONE, 18) << XtoW(L"Average  (% Parent) [This] [% this] Max", FMT_NONE, 40) << newl;
	m_pConsole->Perf() << XtoW(L"--------", FMT_ALIGNLEFT, 50) << XtoW(L"------------------", FMT_NONE, 18) << XtoW(L"---------------------------------------", FMT_NONE, 40) << newl;
	m_nodeRoot.PrintReport(m_pConsole, m_pSystem);
	m_pConsole->Perf() << newl;
	
	// Retrieve condensed totals by node
	const double dFrequency(1000.0 / m_pSystem->GetFrequency());

	map<string, SProfileData> mapTotals;
	m_nodeRoot.GetCondensedData(mapTotals);
	vector<SProfileData> vTotals;
	for (map<string, SProfileData>::iterator it(mapTotals.begin()), itEnd(mapTotals.end()); it != itEnd; ++it)
		vTotals.push_back(it->second);
	
	m_pConsole->Perf() << L"Max call times (leaf nodes only)" << newl;
	m_pConsole->Perf() << L"--------------------------------" << newl;
	std::sort(vTotals.begin(), vTotals.end(), SortMax);
	for (vector<SProfileData>::iterator it(vTotals.begin()), itEnd(vTotals.end()); it != itEnd && !it->bNotLeaf; ++it)
		m_pConsole->Perf() << XtoS(it->sName, FMT_ALIGNLEFT, 50) << XtoW(it->llLongestTime * dFrequency, FMT_NONE, 8, 4) << newl;
	m_pConsole->Perf() << newl;

	m_pConsole->Perf() << L"Average call times" << newl;
	m_pConsole->Perf() << L"------------------" << newl;
	std::sort(vTotals.begin(), vTotals.end(), SortUnlogged);
	for (vector<SProfileData>::iterator it(vTotals.begin()), itEnd(vTotals.end()); it != itEnd; ++it)
	{
		if (it->llTotalCalls > 0)
			m_pConsole->Perf() << XtoS(it->sName, FMT_ALIGNLEFT, 50) << XtoW(it->llUnloggedTime * dFrequency / it->llTotalCalls, FMT_NONE, 8, 4) << newl;
	}
	m_pConsole->Perf() << newl;

	m_pConsole->Perf() << L"Total time" << newl;
	m_pConsole->Perf() << L"----------" << newl;
	std::sort(vTotals.begin(), vTotals.end(), SortTotalTime);
	for (vector<SProfileData>::iterator it(vTotals.begin()), itEnd(vTotals.end()); it != itEnd; ++it)
	{
		if (it->bNotLeaf)
			m_pConsole->Perf() << XtoS(it->sName, FMT_ALIGNLEFT, 50) << XtoW(it->llUnloggedTime * dFrequency, FMT_DELIMIT, 9, 0) << XtoW(L"(" + XtoW(it->llUnloggedTime / double(m_nodeRoot.GetTotalTime()) * 100.0f, FMT_NONE, 0, 1) + L"%)", FMT_NONE, 9) << newl;
		else
			m_pConsole->Perf() << XtoS(it->sName, FMT_ALIGNLEFT, 50) << XtoW(it->llTotalTime * dFrequency, FMT_DELIMIT, 9, 0) << XtoW(L"(" + XtoW(it->llTotalTime / double(m_nodeRoot.GetTotalTime()) * 100.0f, FMT_NONE, 0, 1) + L"%)", FMT_NONE, 9) << newl;
	}
	m_pConsole->Perf() << newl;

	m_pConsole->Perf() << L"Most called" << newl;
	m_pConsole->Perf() << L"-----------" << newl;
	std::sort(vTotals.begin(), vTotals.end(), SortMostCalls);
	for (vector<SProfileData>::iterator it(vTotals.begin()), itEnd(vTotals.end()); it != itEnd && !it->bNotLeaf; ++it)
	{
		m_pConsole->Perf() << XtoS(it->sName, FMT_ALIGNLEFT, 50) << XtoW(it->llTotalCalls, FMT_DELIMIT, 9);
		float fAverage(it->llTotalCalls / m_nodeRoot.GetTotalCalls());
		if (fAverage >= 1.0f)
			m_pConsole->Perf() << XtoW(fAverage, FMT_DELIMIT, 9, 0);
		else
			m_pConsole->Perf() << XtoW(L"(" + XtoW(fAverage, FMT_DELIMIT, 0, 1) + L"%)", FMT_NONE, 9);
		m_pConsole->Perf() << newl;
	}
	m_pConsole->Perf() << newl;

	m_pConsole->Perf().CloseLogFile();
}


/*====================
  CProfiler::StartSample
  ====================*/
uint	CProfiler::StartSample(const char *szName)
{
	if (!m_bActive)
		return 0;

	m_pCurrentNode = m_pCurrentNode->StartSample(szName, m_pSystem, m_pMemManager);
	return 0;
}


/*====================
  CProfiler::EndSample
  ====================*/
void	CProfiler::EndSample(uint uiHandle)
{
	if (!m_bActive)
		return;

	m_pCurrentNode = m_pCurrentNode->EndSample(m_pSystem);
}

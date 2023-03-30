// (C)2009 S2 Games
// c_profilenode.h
//
//=============================================================================
#ifndef __C_PROFILENODE_H__
#define __C_PROFILENODE_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_system.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
struct SProfileData
{
	string		sName;
	bool		bNotLeaf;
	LONGLONG	llTotalCalls;
	LONGLONG	llTotalTime;
	LONGLONG	llLongestTime;
	LONGLONG	llUnloggedTime;

	SProfileData() :
	bNotLeaf(false),
	llTotalCalls(0),
	llTotalTime(0),
	llLongestTime(0),
	llUnloggedTime(0)
	{}
};
//=============================================================================

//=============================================================================
// CProfileNode
//=============================================================================
class CProfileNode
{
private:
	typedef vector<const char*>		NameVector;
	typedef NameVector::iterator	NameVector_it;

	typedef vector<CProfileNode*>	NodeVector;
	typedef NodeVector::iterator	NodeVector_it;

	static uint		s_uiDepth;

	const char*		m_szName;
	CProfileNode*	m_pParent;

	NameVector		m_vChildNames;
	NodeVector		m_vChildNodes;

	LONGLONG		m_llStartTime;
	LONGLONG		m_llRecursiveCalls;

	LONGLONG		m_llCallsThisFrame;
	LONGLONG		m_llTimeThisFrame;

	LONGLONG		m_llTotalCalls;
	LONGLONG		m_llTotalTime;
	LONGLONG		m_llUnloggedTime;

	LONGLONG		m_llLongestTime;
	LONGLONG		m_llMaxFrameCalls;

	CProfileNode();

public:
	~CProfileNode()	{}
	CProfileNode(const char *szName, CProfileNode *pParent);

	LONGLONG	GetTotalTime() const	{ return m_llTotalTime; }
	LONGLONG	GetTotalCalls() const	{ return m_llTotalCalls; }

	inline void	StartTimer(const CSystem *pSystem)
	{
		++m_llCallsThisFrame;
		++m_llTotalCalls;
		m_llStartTime = pSystem->GetTicks();
	}

	inline void	EndTimer(const CSystem *pSystem)
	{
		m_llTimeThisFrame = pSystem->GetTicks() - m_llStartTime;
		m_llTotalTime += m_llTimeThisFrame;
		m_llLongestTime = MAX(m_llLongestTime, m_llTimeThisFrame);
	}

	inline CProfileNode*	StartSample(const char *szName, CSystem *pSystem, CMemManager *pMemManager);
	inline CProfileNode*	EndSample(const CSystem *pSystem);
	
	void	NewFrame();
	void	Reset();
	void	PrintReport(CConsole *pConsole, CSystem *pSystem);
	
	static bool	Pred(CProfileNode *pElem1, CProfileNode *pElem2)
	{
		return pElem1->m_szName < pElem2->m_szName;
	}

	void	GetCondensedData(map<string, SProfileData> &mapTotals);
};
//=============================================================================

/*====================
  CProfileNode::StartSample
  ====================*/
inline CProfileNode*	CProfileNode::StartSample(const char *szName, CSystem *pSystem, CMemManager *pMemManager)
{
	if (m_szName == szName)
	{
		++m_llCallsThisFrame;
		++m_llRecursiveCalls;
		return this;
	}

	NameVector_it itName(lower_bound(m_vChildNames.begin(), m_vChildNames.end(), szName));

	if (itName == m_vChildNames.end() || *itName != szName)
	{
		m_vChildNames.push_back(szName);
		sort(m_vChildNames.begin(), m_vChildNames.end());

		m_vChildNodes.push_back(NEW(pMemManager) CProfileNode(szName, this));
		sort(m_vChildNodes.begin(), m_vChildNodes.end(), CProfileNode::Pred);
		
		itName = lower_bound(m_vChildNames.begin(), m_vChildNames.end(), szName);
	}

	uint uiIndex(itName - m_vChildNames.begin());

	m_vChildNodes[uiIndex]->StartTimer(pSystem);
	return m_vChildNodes[uiIndex];
}


/*====================
  CProfileNode::EndSample
  ====================*/
inline CProfileNode*	CProfileNode::EndSample(const CSystem *pSystem)
{
	if (m_llRecursiveCalls > 0)
	{
		--m_llRecursiveCalls;
		return this;
	}

	EndTimer(pSystem);
	return m_pParent;
}

#endif //__C_PROFILENODE_H__

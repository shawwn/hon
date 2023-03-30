// (C)2005 S2 Games
// c_heap.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "chatserver_common.h"

#include "c_heap.h"
#include "c_memmanager.h"
#include "c_console.h"
//=============================================================================

/*====================
  CHeap::~CHeap
  ====================*/
CHeap::~CHeap()
{
#ifdef DEBUG_MEM
	assert(m_pHead == NULL);
	assert(m_pTail == NULL);
	assert(m_ullTotalAllocations - m_ullTotalDeallocations == 0);
	assert(m_ullTotalAllocated - m_ullTotalDeallocated == 0);
#endif
	m_pMemManager->UnregisterHeap(this);
}


/*====================
  CHeap::CHeap
  ====================*/
CHeap::CHeap(CMemManager *pMemManager, const char *szName, size_t zPoolSize) :
m_pMemManager(pMemManager),
#ifdef DEBUG_MEM
m_ullTotalAllocations(0),
m_ullTotalDeallocations(0),
m_ullTotalAllocated(0),
m_ullTotalDeallocated(0),
m_pHead(NULL),
m_pTail(NULL),
#endif //DEBUG_MEM

m_zPoolSize(zPoolSize),
m_pPool(NULL),
m_pMarker(NULL),
m_pPoolTail(NULL)
{
	assert(m_pMemManager != NULL);
	assert(strlen(szName) < HEAP_MAX_NAME_LEN);

	memset(m_szName, 0, HEAP_MAX_NAME_LEN + 1);
	STRNCPY_S(m_szName, HEAP_MAX_NAME_LEN, szName, HEAP_MAX_NAME_LEN);

	m_uiID = m_pMemManager->RegisterHeap(this);

	if (m_zPoolSize > 0)
	{
		m_pPool = NEW(m_pMemManager) char[m_zPoolSize];
		m_pPoolTail = m_pMarker = m_pPool;
	}
}


/*====================
  CHeap::Allocate
  ====================*/
void*	CHeap::Allocate(size_t zSize)
{
#ifdef DEBUG_MEM
	++m_ullTotalAllocations;
	m_ullTotalAllocated += zSize - (sizeof(SMemHeader) + sizeof(MEM_END_TAG));

	if (m_pPool == NULL)
		return malloc(zSize);
#else
	zSize += sizeof(CHeap*);

	if (m_pPool == NULL)
	{
		CHeap **ppHeap; ppHeap = (CHeap**)malloc(zSize + sizeof(CHeap*));
		*ppHeap = this;
		return ppHeap + 1;
	}
#endif

	if (zSize > m_zPoolSize)
		CSystem::Fatal(L"Attempted to allocate " + XtoW(INT_SIZE(zSize)) + L" bytes from heap " + SingleToWide(m_szName) + L" with a pool size of " + XtoW(INT_SIZE(m_zPoolSize)));

	if (m_pMarker + zSize > m_pPool + m_zPoolSize)
		m_pMarker = m_pPool;

#ifdef DEBUG_MEM
	void *pRet = m_pMarker;
	m_pMarker += zSize;
#else
	CHeap **pRet = (CHeap**)m_pMarker;
	*pRet = this;
	pRet += 1;
	m_pMarker += zSize + sizeof(CHeap*);
#endif

	return pRet;
}


/*====================
  CHeap::Free
  ====================*/
#ifdef DEBUG_MEM
void	CHeap::Free(void *p, size_t z)
{
	++m_ullTotalDeallocations;
	m_ullTotalDeallocated += z;

	if (m_pPool == NULL)
	{
		free(p);
	}
	else
	{
		if (m_pPoolTail == p)
			m_pPoolTail += z;
	}
}
#endif


/*====================
  CHeap::Reallocate
  ====================*/
#ifndef DEBUG_MEM
void*	CHeap::Reallocate(void *pTarget, size_t zSize)
{
	if (!HasPool())
	{
		CHeap **ppHeap = ((CHeap**)pTarget - 1);
		return (CHeap**)realloc(ppHeap, zSize) + 1;
	}

	CSystem::Fatal(L"CHeap::Reallocate() is not yet implemented for pools!");

	/*
	char *pRet(m_pMarker);
	char *pDest(pRet);
	char *pSrc((char*)pTarget);

	CHeap **pRet = (CHeap**)m_pMarker;
	*pRet = this;
	pRet += 1;
	memcpy(pRet, pTarget, zSize);

	m_pMarker += zSize;
	return pRet;
	/**/
	return NULL;
}
#endif


/*====================
  CHeap::Validate
  ====================*/
#ifdef DEBUG_MEM
bool	CHeap::Validate()
{
	size_t zCount(0);

	SMemHeader *p(m_pHead);
	while (p != NULL)
	{
		if (p->pHeap != this)
			CSystem::DebugBreak();
		if (p->uiMarker != MEM_START_TAG)
			CSystem::DebugBreak();
		if (*(uint*)((char*)p + p->zSize + sizeof(SMemHeader)) != MEM_END_TAG)
			CSystem::DebugBreak();
		if (p->pHeapPrev == NULL && p != m_pTail)
			CSystem::DebugBreak();

		++zCount;
		p = p->pHeapPrev;
	}

	if (zCount != m_ullTotalAllocations - m_ullTotalDeallocations)
		CSystem::DebugBreak();

	return true;
}
#endif


/*====================
  CHeap::PrintAllocations
  ====================*/
#ifdef DEBUG_MEM
void	CHeap::PrintAllocations(CConsole *pConsole)
{
	if (pConsole == NULL)
		return;

	pConsole->Admin()
		<< _T("Heap ") << m_szName << _T(" has ")
		<< m_ullTotalAllocated - m_ullTotalDeallocated << _T(" bytes in ")
		<< m_ullTotalAllocations - m_ullTotalDeallocations << _T(" active allocations")
		<< newl;
}
#endif

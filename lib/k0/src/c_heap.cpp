// (C)2008 S2 Games
// c_heap.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k0_common.h"

#include "c_heap.h"

#include "c_memmanager.h"
//=============================================================================

/*====================
  CHeap::~CHeap
  ====================*/
CHeap::~CHeap()
{
#ifdef K0_DEBUG_MEM
	if (m_zTotalAllocations != m_zTotalDeallocations)
		Console.Warn << _T("Heap ") << m_szName << _T(" has ") <<
		INT_SIZE(m_zTotalAllocated - m_zTotalDeallocated) << _T(" bytes in ") <<
		INT_SIZE(m_zTotalAllocations - m_zTotalDeallocations) << _T(" active allocations") << newl;
#endif //K0_DEBUG_MEM
	MemManager.UnregisterHeap(this);
}


/*====================
  CHeap::CHeap
  ====================*/
CHeap::CHeap(const char *szName, size_t zPoolSize, EReservedHeap eHeap) :
#ifdef K0_DEBUG_MEM
m_zTotalAllocations(0),
m_zTotalDeallocations(0),
m_zTotalAllocated(0),
m_zTotalDeallocated(0),
m_pHead(NULL),
m_pTail(NULL),
#endif //K0_DEBUG_MEM

m_zPoolSize(zPoolSize),
m_pPool(NULL),
m_pMarker(NULL)
{
	assert(strlen(szName) < HEAP_MAX_NAME_LEN);
	MemManager.Set(m_szName, 0, HEAP_MAX_NAME_LEN);
	STRNCPY_S(m_szName, HEAP_MAX_NAME_LEN, szName, HEAP_MAX_NAME_LEN);

	m_uiID = MemManager.RegisterHeap(this, eHeap);

	if (m_zPoolSize > 0)
	{
		m_pPool = new char[m_zPoolSize];
		m_pMarker = m_pPool;
	}
}


/*====================
  CHeap::Allocate
  ====================*/
void*	CHeap::Allocate(size_t z)
{
#ifdef K0_DEBUG_MEM
	++m_zTotalAllocations;
	m_zTotalAllocated += z - (sizeof(SMemHeader) + sizeof(MEM_END_TAG));
#endif //K0_DEBUG_MEM

	if (m_pPool == NULL)
		return malloc(z);

	if (z > m_zPoolSize)
		EX_FATAL(_T("Attempted to allocate ") + XtoA((uint)z) + _T(" bytes from heap ")
					+ XtoA(m_szName) + _T(" with a pool size of ") + XtoA((uint)m_zPoolSize));

	if (m_pMarker + z > m_pPool + m_zPoolSize)
	{
#ifdef K0_DEBUG_MEM
		Console.Mem << _T("Heap ") << m_szName << _T(" recycled") << newl;
#endif //K0_DEBUG_MEM
		m_pMarker = m_pPool;
	}

	void *pRet = m_pMarker;
	m_pMarker += z;
	return pRet;
}


/*====================
  CHeap::Free
  ====================*/
void	CHeap::Free(void *p, size_t z)
{
#ifdef K0_DEBUG_MEM
	++m_zTotalDeallocations;
	m_zTotalDeallocated += z;
#endif

	if (m_pPool == NULL)
		free(p);
}

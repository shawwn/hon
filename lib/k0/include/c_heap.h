// (C)2005 S2 Games
// c_heap.h
//
//=============================================================================
#ifndef __C_HEAP_H__
#define __C_HEAP_H__

//=============================================================================
// Definitions
//=============================================================================
//#define K0_DEBUG_MEM
//#define K0_DEBUG_MEM_EX

const uint MEM_DEBUG_MAX_FILE_NAME_LENGTH(48);

struct SMemHeader
{
	uint				uiMarker;
	size_t				zSize;
#ifdef K0_DEBUG_MEM_EX
	char				szFile[MEM_DEBUG_MAX_FILE_NAME_LENGTH];
	short				nLine;
	uint				uiTimeStamp;
#endif //K0_DEBUG_MEM_EX
	class CHeap			*pHeap;
	struct SMemHeader	*pHeapPrev;
	struct SMemHeader	*pHeapNext;
	struct SMemHeader	*pPrev;
	struct SMemHeader	*pNext;
};

const uint MAX_HEAPS(32);
enum EReservedHeap
{
	HEAP_CLIENT_GAME,

	NUM_RESERVED_HEAPS
};

const size_t HEAP_MAX_NAME_LEN(16);
//=============================================================================

//=============================================================================
// CHeap
//=============================================================================
class K2_API CHeap
{
private:
	char	m_szName[HEAP_MAX_NAME_LEN];
	uint	m_uiID;

#ifdef K0_DEBUG_MEM
	size_t	m_zTotalAllocations;
	size_t	m_zTotalDeallocations;
	size_t	m_zTotalAllocated;
	size_t	m_zTotalDeallocated;

	// Allocation list
	SMemHeader*		m_pHead;
	SMemHeader*		m_pTail;
#endif //K0_DEBUG_MEM

	CHeap();
	CHeap(CHeap&);
	CHeap&	operator=(CHeap&);

	size_t	m_zPoolSize;
	char*	m_pPool;
	char*	m_pMarker;

public:
	~CHeap();
	CHeap(const char *szName, size_t zPool = 0, EReservedHeap eHeap = NUM_RESERVED_HEAPS);

	void*		Allocate(size_t z);
	void		Free(void *p, size_t z);

	const char*	GetName()				{ return m_szName; }
	uint		GetID() const			{ return m_uiID; }

	bool		HasPool() const			{ return m_pPool != NULL; }
	float		GetPoolUsage() const	{ return (m_pMarker - m_pPool) / float(m_zPoolSize); }

#ifdef K0_DEBUG_MEM
	size_t		GetAllocCount()			{ return m_zTotalAllocations; }
	size_t		GetAllocSize()			{ return m_zTotalAllocated; }
	size_t		GetDeallocCount()		{ return m_zTotalDeallocations; }
	size_t		GetDeallocSize()		{ return m_zTotalDeallocated; }

	SMemHeader*	GetHead()				{ return m_pHead; }
	SMemHeader*	GetTail()				{ return m_pTail; }
	void		SetHead(SMemHeader *p)	{ m_pHead = p; }
	void		SetTail(SMemHeader *p)	{ m_pTail = p; }
#endif //K0_DEBUG_MEM
};
//=============================================================================

#endif //__C_HEAP_H__

// (C)2005 S2 Games
// c_heap.h
//
//=============================================================================
#ifndef __C_HEAP_H__
#define __C_HEAP_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_core.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
const uint MEM_DEBUG_MAX_FILE_NAME_LENGTH(24);

enum EAllocationType
{
	ALLOCATE_NORMAL,
	ALLOCATE_ARRAY,
	ALLOCATE_STL,
	ALLOCATE_CURL
};

struct SMemHeader
{
	uint				uiMarker;
	int					iTag;
	size_t				zSize;
	EAllocationType		eType;
	CMemManager*		pMemManager;
	char				szFile[MEM_DEBUG_MAX_FILE_NAME_LENGTH];
	ushort				unLine;
	uint				uiTimeStamp;
	class CHeap*		pHeap;
	struct SMemHeader*	pHeapPrev;
	struct SMemHeader*	pHeapNext;
	struct SMemHeader*	pPrev;
	struct SMemHeader*	pNext;
	struct SMemHeader*	pTrackPrev;
	struct SMemHeader*	pTrackNext;
	SMemHeader*			pHeaderStart;
};

const uint MAX_HEAPS(32);
const size_t HEAP_MAX_NAME_LEN(14);
//=============================================================================

//=============================================================================
// CHeap
//=============================================================================
class CHeap
{
private:
	CMemManager*	m_pMemManager;

	char			m_szName[HEAP_MAX_NAME_LEN + 1];
	uint			m_uiID;

#ifdef DEBUG_MEM
	ULONGLONG		m_ullTotalAllocations;
	ULONGLONG		m_ullTotalDeallocations;
	ULONGLONG		m_ullTotalAllocated;
	ULONGLONG		m_ullTotalDeallocated;

	// Allocation list
	SMemHeader*		m_pHead;
	SMemHeader*		m_pTail;
#endif //DEBUG_MEM

	size_t			m_zPoolSize;
	char*			m_pPool;
	char*			m_pMarker;
	char*			m_pPoolTail;

	CHeap();
	CHeap(CHeap&);
	CHeap&	operator=(CHeap&);

public:
	~CHeap();
	CHeap(CMemManager *pMemManager, const char *szName, size_t zPool = 0);

	void*		Allocate(size_t zSize);

#ifdef DEBUG_MEM
	void		Free(void *p, size_t z);
#else
	inline void	Free(void *p)					{ if (!HasPool()) free(p); }
	void*		Reallocate(void *pTarget, size_t zSize);
#endif

	inline const char*	GetName()				{ return m_szName; }
	inline uint			GetID() const			{ return m_uiID; }

	inline bool			HasPool() const			{ return m_pPool != NULL; }
	inline float		GetPoolUsage() const	{ return (m_pMarker - m_pPool) / float(m_zPoolSize); }

#ifdef DEBUG_MEM
	ULONGLONG	GetAllocCount()					{ return m_ullTotalAllocations; }
	ULONGLONG	GetAllocSize()					{ return m_ullTotalAllocated; }
	ULONGLONG	GetDeallocCount()				{ return m_ullTotalDeallocations; }
	ULONGLONG	GetDeallocSize()				{ return m_ullTotalDeallocated; }

	SMemHeader*	GetHead()						{ return m_pHead; }
	SMemHeader*	GetTail()						{ return m_pTail; }
	void		SetHead(SMemHeader *p)			{ m_pHead = p; }
	void		SetTail(SMemHeader *p)			{ m_pTail = p; }

	bool		Validate();
	void		PrintAllocations(CConsole *pConsole);
#endif //DEBUG_MEM
};
//=============================================================================

#endif //__C_HEAP_H__

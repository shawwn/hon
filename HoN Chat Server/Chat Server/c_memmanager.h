// (C)2005 S2 Games
// c_memmanager.h
//
//=============================================================================
#ifndef __C_MEMMANAGER_H__
#define __C_MEMMANAGER_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_core.h"

#include "c_heap.h"
#include <cstring>
#include <cstdlib>
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CConsole;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
const uint MEM_START_TAG	(0xaaaa1111);
const uint MEM_END_TAG		(0xffff9999);
const byte MEM_INIT_TAG		(0xc3);
const byte MEM_FREE_TAG		(0xb2);

#ifndef errno_t
typedef int errno_t;
#endif

#ifdef DEBUG_MEM
#define NEW(memmanager)				new(memmanager, __FILE__, __LINE__)
#define HEAP_NEW(memmanager, heap)	new(memmanager, heap, __FILE__, __LINE__)
#else // DEBUG_MEM
#define NEW(memmanager)				new(memmanager)
#define HEAP_NEW(memmanager, heap)	new(memmanager, heap)
#endif // DEBUG_MEM
//=============================================================================

//=============================================================================
// CMemManager
//=============================================================================
class CMemManager
{
	DECLARE_CORE_API;

private:
	CHeap*				m_pDefaultHeap;
	CHeap*				m_apHeaps[MAX_HEAPS];

	// Stats
#ifdef DEBUG_MEM
	static ULONGLONG	s_ullCommonAllocs;
	static ULONGLONG	s_ullCommonDeallocs;
	static ULONGLONG	s_ullCommonAllocSize;
	static ULONGLONG	s_ullCommonDeallocSize;

	ULONGLONG			m_ullTotalAllocs;
	ULONGLONG			m_ullTotalAllocSize;
	ULONGLONG			m_ullTotalDeallocs;
	ULONGLONG			m_ullTotalDeallocSize;
	ULONGLONG			m_ullMemoryManagerOverhead;

	ULONGLONG			m_ullTrackAllocs;
	ULONGLONG			m_ullTrackAllocSize;
	ULONGLONG			m_ullTrackDeallocs;
	ULONGLONG			m_ullTrackDeallocSize;

	uint				m_uiTimeStamp;

	// Allocation list
	SMemHeader*			m_pHead;
	SMemHeader*			m_pTail;

	SMemHeader*			m_pTrackHead;
	SMemHeader*			m_pTrackTail;

	static SMemHeader*	s_pCommonHead;
	static SMemHeader*	s_pCommonTail;
#endif //DEBUG_MEM

public:
	~CMemManager()	{}
	CMemManager();

	void						SetDefaultHeap(CHeap *pHeap)											{ m_pDefaultHeap = pHeap; }
	CHeap*						GetDefaultHeap() const													{ return m_pDefaultHeap; }

	bool						Initialize(CCore *pCore);
	void						SynchronizeCoreAPI();

	uint						RegisterHeap(CHeap *pHeap);
	void						UnregisterHeap(CHeap *pHeap);
	CHeap*						AllocateHeap(const char *szName);

#ifdef DEBUG_MEM
	static void*				AllocateCommon(size_t zSize, EAllocationType eType);
	inline void*				Allocate(size_t zSize, EAllocationType eType)													{ return Allocate(zSize, eType, m_pDefaultHeap, "", 0); }
	inline void*				Allocate(size_t zSize, EAllocationType eType, CHeap* pHeap)										{ return Allocate(zSize, eType, pHeap, "", 0); }
	inline void*				Allocate(size_t zSize, EAllocationType eType, const char *szFile, ushort unLine)				{ return Allocate(zSize, eType, m_pDefaultHeap, szFile, unLine); }
	void*						Allocate(size_t zSize, EAllocationType eType, CHeap* pHeap, const char *szFile, ushort unLine);
	void						Deallocate(SMemHeader *pHeader, EAllocationType eType);
	void*						Reallocate(void *pTarget, size_t zSize, EAllocationType eType)														{ return Reallocate(pTarget, zSize, eType, m_pDefaultHeap, "", 0); }
	void*						Reallocate(void *pTarget, size_t zSize, EAllocationType eType, CHeap* pHeap)										{ return Reallocate(pTarget, zSize, eType, pHeap, "", 0); }
	void*						Reallocate(void *pTarget, size_t zSize, EAllocationType eType, const char *szFile, ushort unLine)					{ return Reallocate(pTarget, zSize, eType, m_pDefaultHeap, szFile, unLine); }
	void*						Reallocate(void *pTarget, size_t zSize, EAllocationType eType, CHeap* pHeap, const char *szFile, ushort unLine);
	static void					DeallocateCommon(void *pTarget, EAllocationType eType);
#else
	static inline void*			AllocateCommon(size_t zSize)											{ void **p; p = (void**)malloc(zSize + sizeof(void*)); *p = NULL; return p + 1; }
	inline void*				Allocate(size_t zSize)													{ return Allocate(zSize, m_pDefaultHeap); }
	inline void*				Allocate(size_t zSize, CHeap *pHeap)									{ return pHeap->Allocate(zSize); }
	inline void*				Reallocate(void *pTarget, size_t zSize)									{ return Reallocate(pTarget, zSize, m_pDefaultHeap); }
	inline void*				Reallocate(void *pTarget, size_t zSize, CHeap *pHeap)					{ return pHeap->Reallocate(pTarget, zSize); }
	static void					DeallocateCommon(void *pTarget);
#endif

#ifdef DEBUG_MEM
	void						Validate();
	void						PrintStats();
	void						PrintTrackingStats();
	void						PrintAllocations(const char *szHeapName = NULL, uint uiTime = -1);
	void						PrintAllocationsNoDuplicates(const char *szHeapName = NULL, uint uiTime = -1);
	void						ResetTracking();
	void						PrintTaggedAllocations(int iTag);

	ULONGLONG					GetActiveAllocs()														{ return m_ullTotalAllocs - m_ullTotalDeallocs; }
	ULONGLONG					GetActiveAllocsSize()													{ return m_ullTotalAllocSize - m_ullTotalDeallocSize; }
#endif //DEBUG_MEM
};
//=============================================================================

/*====================
  CMemManager::DeallocateCommon
  ====================*/
#ifdef DEBUG_MEM
inline void	CMemManager::DeallocateCommon(void *pTarget, EAllocationType eType)
{
	if (pTarget == NULL)
		return;

	SMemHeader *pHeader(*((SMemHeader**)pTarget - 1));
	if (pHeader->pMemManager != NULL)
	{
		pHeader->pMemManager->Deallocate(pHeader, eType);
		return;
	}

	assert(pHeader->uiMarker == MEM_START_TAG);
	assert(pHeader->pHeaderStart = pHeader);
	assert(*(uint*)((char*)pHeader + sizeof(SMemHeader) + pHeader->zSize) == MEM_END_TAG);
	assert(pHeader->eType == eType);
	
	++s_ullCommonDeallocs;
	s_ullCommonDeallocSize += pHeader->zSize;

	if (pHeader->pNext != NULL)
		pHeader->pNext->pPrev = pHeader->pPrev;
	if (pHeader->pPrev != NULL)
		pHeader->pPrev->pNext = pHeader->pNext;
	if (s_pCommonHead == pHeader)
		s_pCommonHead = pHeader->pPrev;
	if (s_pCommonTail == pHeader)
		s_pCommonTail = pHeader->pNext;

	free(pHeader);
}
#else
inline void	CMemManager::DeallocateCommon(void *pTarget)
{
	if (pTarget == NULL)
		return;

	CHeap **ppHeap = ((CHeap**)pTarget - 1);
	if (*ppHeap == NULL)
		free(ppHeap);
	else
		(*ppHeap)->Free(ppHeap);
}
#endif

//=============================================================================
// CSTLAllocator
//=============================================================================
template<class T>
class CSTLAllocator : public std::allocator<T>
{
private:
	CSTLAllocator();

public:
	typedef typename std::allocator<T>::size_type	size_type;
	typedef typename std::allocator<T>::pointer		pointer;
	
	CMemManager*	m_pMemManager;
	CHeap*			m_pHeap;

	template<class _Other>
	struct rebind
	{
		typedef CSTLAllocator<_Other> other;
	};

	CSTLAllocator(CMemManager *pMemManager) :
	m_pMemManager(pMemManager),
	m_pHeap(pMemManager->GetDefaultHeap())
	{}
	
	CSTLAllocator(CMemManager *pMemManager, CHeap *pHeap) :
	m_pMemManager(pMemManager),
	m_pHeap(pHeap)
	{}
	
	template<class _Other>
	CSTLAllocator(const CSTLAllocator<_Other> &other) :
	m_pMemManager(other.m_pMemManager),
	m_pHeap(other.m_pHeap)
	{}
	
	CSTLAllocator(const CSTLAllocator &__a) :
	std::allocator<T>(__a),
	m_pMemManager(__a.m_pMemManager),
	m_pHeap(__a.m_pHeap)
	{}

	~CSTLAllocator() {}

#ifndef __GNUC__
	template<class _Other>
	allocator<T>& operator=(const allocator<_Other>&)
	{
		// assign from a related allocator (do nothing)
		return (*this);
	}
#endif

#ifdef DEBUG_MEM
	pointer	allocate(size_type _Count)				{ return (pointer)m_pMemManager->Allocate(_Count * sizeof(T), ALLOCATE_STL, m_pHeap, "<stl>", 0); }
	pointer allocate(size_type _Count, const void*)	{ return (pointer)m_pMemManager->Allocate(_Count * sizeof(T), ALLOCATE_STL, m_pHeap, "<stl>", 0); }
	void	deallocate(pointer _Ptr, size_type)		{ CMemManager::DeallocateCommon(_Ptr, ALLOCATE_STL); }
#else
	pointer	allocate(size_type _Count)				{ return (pointer)m_pMemManager->Allocate(_Count * sizeof(T), m_pHeap); }
	pointer allocate(size_type _Count, const void*)	{ return (pointer)m_pMemManager->Allocate(_Count * sizeof(T), m_pHeap); }
	void	deallocate(pointer _Ptr, size_type)		{ CMemManager::DeallocateCommon(_Ptr); }
#endif
};
//=============================================================================

/*====================
  operator new
  ====================*/
#ifdef DEBUG_MEM
inline void*	operator new(size_t zSize)																				{ return CMemManager::AllocateCommon(zSize, ALLOCATE_NORMAL); }
inline void*	operator new(size_t zSize, CMemManager *pMemManager, const char *szFile, ushort unLine)					{ return pMemManager->Allocate(zSize, ALLOCATE_NORMAL, szFile, unLine); }
inline void*	operator new(size_t zSize, CMemManager *pMemManager, CHeap *pHeap, const char *szFile, ushort unLine)	{ return pMemManager->Allocate(zSize, ALLOCATE_NORMAL, pHeap, szFile, unLine); }
#else
inline void*	operator new(size_t zSize)																				{ return CMemManager::AllocateCommon(zSize); }
inline void*	operator new(size_t zSize, CMemManager *pMemManager)													{ return pMemManager->Allocate(zSize); }
inline void*	operator new(size_t zSize, CMemManager *pMemManager, CHeap *pHeap)										{ return pMemManager->Allocate(zSize, pHeap); }
#endif


/*====================
  operator new[]
  ====================*/
#ifdef DEBUG_MEM
inline void*	operator new[](size_t zSize)																			{ return CMemManager::AllocateCommon(zSize, ALLOCATE_ARRAY); }
inline void*	operator new[](size_t zSize, CMemManager *pMemManager, const char *szFile, ushort unLine)				{ return pMemManager->Allocate(zSize, ALLOCATE_ARRAY, szFile, unLine); }
inline void*	operator new[](size_t zSize, CMemManager *pMemManager, CHeap *pHeap, const char *szFile, ushort unLine)	{ return pMemManager->Allocate(zSize, ALLOCATE_ARRAY, pHeap, szFile, unLine); }
#else
inline void*	operator new[](size_t zSize)																			{ return CMemManager::AllocateCommon(zSize); }
inline void*	operator new[](size_t zSize, CMemManager *pMemManager)													{ return pMemManager->Allocate(zSize); }
inline void*	operator new[](size_t zSize, CMemManager *pMemManager, CHeap *pHeap)									{ return pMemManager->Allocate(zSize, pHeap); }
#endif


/*====================
  operator delete
  ====================*/
#ifdef DEBUG_MEM
inline void	operator delete(void *p)																					{ CMemManager::DeallocateCommon(p, ALLOCATE_NORMAL); }
inline void	operator delete(void *p, CMemManager *pMemManager, const char *szFile, ushort unLine)						{ CMemManager::DeallocateCommon(p, ALLOCATE_NORMAL); }
inline void	operator delete(void *p, CMemManager *pMemManager, CHeap *pHeap, const char *szFile, ushort unLine)			{ CMemManager::DeallocateCommon(p, ALLOCATE_NORMAL); }
#else
inline void	operator delete(void *p)																					{ CMemManager::DeallocateCommon(p); }
inline void	operator delete(void *p, CMemManager *pMemManager)															{ CMemManager::DeallocateCommon(p); }
inline void	operator delete(void *p, CMemManager *pMemManager, CHeap *pHeap)											{ CMemManager::DeallocateCommon(p); }
#endif


/*====================
  operator delete[]
  ====================*/
#ifdef DEBUG_MEM
inline void	operator delete[](void *p)																					{ CMemManager::DeallocateCommon(p, ALLOCATE_ARRAY); }
inline void	operator delete[](void *p, CMemManager *pMemManager, const char *szFile, ushort unLine)						{ CMemManager::DeallocateCommon(p, ALLOCATE_ARRAY); }
inline void	operator delete[](void *p, CMemManager *pMemManager, CHeap *pHeap, const char *szFile, ushort unLine)		{ CMemManager::DeallocateCommon(p, ALLOCATE_ARRAY); }
#else
inline void	operator delete[](void *p)																					{ CMemManager::DeallocateCommon(p); }
inline void	operator delete[](void *p, CMemManager *pMemManager)														{ CMemManager::DeallocateCommon(p); }
inline void	operator delete[](void *p, CMemManager *pMemManager, CHeap *pHeap)											{ CMemManager::DeallocateCommon(p); }
#endif

#endif //__C_MEMMANAGER_H__

// (C)2005 S2 Games
// c_memmanager.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "chatserver_common.h"

#include "c_memmanager.h"
#include "c_heap.h"
#include "c_console.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
#ifdef DEBUG_MEM
ULONGLONG CMemManager::s_ullCommonAllocs(0);
ULONGLONG CMemManager::s_ullCommonDeallocs(0);
ULONGLONG CMemManager::s_ullCommonAllocSize(0);
ULONGLONG CMemManager::s_ullCommonDeallocSize(0);

SMemHeader*	CMemManager::s_pCommonHead(NULL);
SMemHeader*	CMemManager::s_pCommonTail(NULL);
#endif

CVAR(int, mem_tag, 0);
//=============================================================================

/*====================
  CMemManager::CMemManager
  ====================*/
CMemManager::CMemManager() :
NULL_CORE_API,

#ifdef DEBUG_MEM
m_ullTotalAllocs(0),
m_ullTotalAllocSize(0),
m_ullTotalDeallocs(0),
m_ullTotalDeallocSize(0),
m_ullMemoryManagerOverhead(0),

m_ullTrackAllocs(0),
m_ullTrackAllocSize(0),
m_ullTrackDeallocs(0),
m_ullTrackDeallocSize(0),

m_uiTimeStamp(0),

m_pHead(NULL),
m_pTail(NULL),

m_pTrackHead(NULL),
m_pTrackTail(NULL),
#endif //DEBUG_MEM

m_pDefaultHeap(NULL)
{
	memset(m_apHeaps, 0, sizeof(m_apHeaps));
}


/*====================
  CMemManager::Initialize
  ====================*/
bool	CMemManager::Initialize(CCore *pCore)
{
	assert(m_pDefaultHeap != NULL);

	m_pCore = pCore;
	assert(m_pCore != NULL);

	return true;
}


/*====================
  CMemManager::SynchronizeCoreAPI
  ====================*/
void	CMemManager::SynchronizeCoreAPI()
{
	INIT_CORE_API(m_pCore);
}


/*====================
  CMemManager::RegisterHeap
  ====================*/
uint	CMemManager::RegisterHeap(CHeap *pHeap)
{
	uint uiHeapID(0);
	while (uiHeapID < MAX_HEAPS && m_apHeaps[uiHeapID] != NULL)
		++uiHeapID;

	if (uiHeapID >= MAX_HEAPS)
		CSystem::Fatal(L"No free slot for heap: " + SingleToWide(pHeap->GetName()));

	m_apHeaps[uiHeapID] = pHeap;
	return uiHeapID;
}


/*====================
  CMemManager::UnregisterHeap
  ====================*/
void	CMemManager::UnregisterHeap(CHeap *pHeap)
{
	for (uint ui(0); ui < MAX_HEAPS; ++ui)
	{
		if (m_apHeaps[ui] == NULL)
			continue;

		if (m_apHeaps[ui]->GetID() == pHeap->GetID())
		{
			m_apHeaps[ui] = NULL;
			return;
		}
	}
}


/*====================
  CMemManager::AllocateHeap
  ====================*/
CHeap*	CMemManager::AllocateHeap(const char *szName)
{
	uint uiHeapID(0);
	while (uiHeapID < MAX_HEAPS && m_apHeaps[uiHeapID] != NULL)
		++uiHeapID;

	if (uiHeapID >= MAX_HEAPS)
		return NULL;

	CHeap *pNewHeap(NEW(this) CHeap(this, szName));
	m_apHeaps[uiHeapID] = pNewHeap;

	return pNewHeap;
}


/*====================
  CMemManager::AllocateCommon

  Tracks allocations that don't belong to a specific memory manager.
  ====================*/
#ifdef DEBUG_MEM
void*	CMemManager::AllocateCommon(size_t zSize, EAllocationType eType)
{

	++s_ullCommonAllocs;
	s_ullCommonAllocSize += zSize;
	
	SMemHeader *pHeader;
	pHeader = (SMemHeader*)malloc(zSize + sizeof(SMemHeader) + sizeof(MEM_END_TAG));
	
	memset(pHeader, 0, sizeof(SMemHeader));
	pHeader->uiMarker = MEM_START_TAG;
	pHeader->iTag = -1;
	pHeader->zSize = zSize;
	pHeader->eType = eType;
	pHeader->uiTimeStamp = INVALID_TIME;
	pHeader->pHeaderStart = pHeader;
	
	if (s_pCommonHead == NULL)
		s_pCommonHead = pHeader;
	
	if (s_pCommonTail != NULL)
		s_pCommonTail->pNext = pHeader;
	pHeader->pPrev = s_pCommonTail;
	s_pCommonTail = pHeader;

	pHeader->pNext = NULL;
	
	char *pBlock((char*)(pHeader + 1));
	memset(pBlock, MEM_INIT_TAG, zSize);
	*(uint*)(pBlock + zSize) = MEM_END_TAG;

	return pBlock;
}
#endif


/*====================
  CMemManager::Allocate
  ====================*/
#ifdef DEBUG_MEM
void*	CMemManager::Allocate(size_t z, EAllocationType eType, CHeap* pHeap, const char *szFile, ushort unLine)
{
#ifdef VALIDATE_MEM
	Validate();
#endif

	char *p = (char*)pHeap->Allocate(sizeof(SMemHeader) + z + sizeof(MEM_END_TAG));
	assert (p != NULL);

	SMemHeader *pe = (SMemHeader*)p;
	pe->uiMarker = MEM_START_TAG;

	// Find the start of the file name, stripping the path
	const char *pNextSlash(strchr(szFile, '\\'));
	while (pNextSlash != NULL)
	{
		szFile = pNextSlash + 1;
		pNextSlash = strchr(szFile, '\\');
	}

	// Store filename and line number
	memset(pe->szFile, 0, MEM_DEBUG_MAX_FILE_NAME_LENGTH);
	if (szFile != NULL)
		STRNCPY_S(pe->szFile, MEM_DEBUG_MAX_FILE_NAME_LENGTH, szFile, _TRUNCATE);
	pe->unLine = unLine;
	
	pe->uiTimeStamp = m_pCore->GetMilliseconds();

	pe->zSize = z;
	pe->eType = eType;
	pe->pHeap = pHeap;
	pe->pMemManager = this;

	if (m_pHead != NULL)
		m_pHead->pNext = pe;
	pe->pPrev = m_pHead;
	pe->pNext = NULL;
	m_pHead = pe;
	if (m_pTail == NULL)
		m_pTail = m_pHead;

	if (pHeap->GetHead() != NULL)
		pHeap->GetHead()->pHeapNext = pe;
	pe->pHeapPrev = pHeap->GetHead();
	pe->pHeapNext = NULL;
	pHeap->SetHead(pe);
	if (pHeap->GetTail() == NULL)
		pHeap->SetTail(pHeap->GetHead());

	if (m_pTrackHead != NULL)
		m_pTrackHead->pTrackNext = pe;
	pe->pTrackPrev = m_pTrackHead;
	pe->pTrackNext = NULL;
	m_pTrackHead = pe;
	if (m_pTrackTail == NULL)
		m_pTrackTail = m_pTrackHead;

	pe->iTag = mem_tag;
	pe->pHeaderStart = pe;

	p += sizeof(SMemHeader);

	memset(p, MEM_INIT_TAG, z);
	*(uint*)(p + z) = MEM_END_TAG;

	++m_ullTotalAllocs;
	m_ullTotalAllocSize += z;
	++m_ullTrackAllocs;
	m_ullTrackAllocSize += z;
	m_ullMemoryManagerOverhead += sizeof(SMemHeader) + sizeof(MEM_END_TAG);

#ifdef VALIDATE_MEM
	Validate();
#endif

	return p;
}
#endif


/*====================
  CMemManager::Deallocate
  ====================*/
#ifdef DEBUG_MEM
void	CMemManager::Deallocate(SMemHeader *pHeader, EAllocationType eType)
{
#ifdef VALIDATE_MEM
	Validate();
#endif

	// Check for header damage
	assert(pHeader->uiMarker == MEM_START_TAG);
	assert(pHeader->pHeaderStart == pHeader);
	assert(pHeader->eType == eType);

	// Remove from linked list of all allocations
	if (pHeader->pNext != NULL)
		pHeader->pNext->pPrev = pHeader->pPrev;
	if (pHeader->pPrev != NULL)
		pHeader->pPrev->pNext = pHeader->pNext;
	if (m_pHead == pHeader)
		m_pHead = pHeader->pPrev;
	if (m_pTail == pHeader)
		m_pTail = pHeader->pNext;
	
	// Remove from linked list of this heap
	if (pHeader->pHeapNext != NULL)
		pHeader->pHeapNext->pHeapPrev = pHeader->pHeapPrev;
	if (pHeader->pHeapPrev != NULL)
		pHeader->pHeapPrev->pHeapNext = pHeader->pHeapNext;
	if (pHeader->pHeap->GetHead() == pHeader)
		pHeader->pHeap->SetHead(pHeader->pHeapPrev);
	if (pHeader->pHeap->GetTail() == pHeader)
		pHeader->pHeap->SetTail(pHeader->pHeapNext);

	// Remove from linked list of currently tracked allocations
	if (pHeader->pTrackNext != NULL)
		pHeader->pTrackNext->pTrackPrev = pHeader->pTrackPrev;
	if (pHeader->pTrackPrev != NULL)
		pHeader->pTrackPrev->pTrackNext = pHeader->pTrackNext;
	if (m_pTrackHead == pHeader)
		m_pTrackHead = pHeader->pTrackPrev;
	if (m_pTrackTail == pHeader)
		m_pTrackTail = pHeader->pTrackNext;

	// Check that end tag hasn't been damaged by an overrun
	assert(*(uint*)((char*)pHeader + sizeof(SMemHeader) + pHeader->zSize) == MEM_END_TAG);

	// Update stats
	++m_ullTotalDeallocs;
	m_ullTotalDeallocSize += pHeader->zSize;
	++m_ullTrackDeallocs;
	m_ullTrackDeallocSize += pHeader->zSize;
	m_ullMemoryManagerOverhead -= sizeof(SMemHeader) + sizeof(MEM_END_TAG);

	size_t zSize(pHeader->zSize);
	CHeap *pHeap(pHeader->pHeap);
	memset(pHeader, MEM_FREE_TAG, sizeof(SMemHeader) + zSize + sizeof(MEM_END_TAG));
	pHeap->Free(pHeader, zSize);

#ifdef VALIDATE_MEM
	Validate();
#endif
}
#endif


/*====================
  CMemManager::Reallocate
  ====================*/
#ifdef DEBUG_MEM
void*	CMemManager::Reallocate(void *pTarget, size_t zSize, EAllocationType eType, CHeap* pHeap, const char *szFile, ushort unLine)
{
	if (pTarget == NULL)
	{
		if (zSize > 0)
			return Allocate(zSize, eType, pHeap, szFile, unLine);
		
		return NULL;
	}
	
	if (zSize == 0)
	{
		DeallocateCommon(pTarget, eType);
		return NULL;
	}

	void *pNew(Allocate(zSize, eType, pHeap, szFile, unLine));
	if (pNew == NULL)
	{
		DeallocateCommon(pTarget, eType);
		return NULL;
	}

	SMemHeader *pHeader(*((SMemHeader**)pTarget - 1));
	size_t zOld(pHeader->zSize);
	memcpy(pNew, pTarget, zOld);
	
	DeallocateCommon(pTarget, eType);
	return pNew;
}
#endif


#ifdef DEBUG_MEM
/*====================
  CMemManager::PrintStats
  ====================*/
void	CMemManager::PrintStats()
{
	m_pConsole->Admin()
		<< L"----------------------------------------" << newl
		<< L"Tptal Physical:  " << XtoW(GetByteStringW(m_pSystem->GetTotalPhysicalMemory()), FMT_NONE, 7) << newl
		<< L"Free Physical:   " << XtoW(GetByteStringW(m_pSystem->GetFreePhysicalMemory()), FMT_NONE, 7) << newl
		<< L"Total Virtual:   " << XtoW(GetByteStringW(m_pSystem->GetTotalVirtualMemory()), FMT_NONE, 7) << newl
		<< L"Free Virtual:    " << XtoW(GetByteStringW(m_pSystem->GetFreeVirtualMemory()), FMT_NONE, 7) << newl
		<< L"Total Page:      " << XtoW(GetByteStringW(m_pSystem->GetTotalPageFile()), FMT_NONE, 7) << newl
		<< L"Free Page:       " << XtoW(GetByteStringW(m_pSystem->GetFreePageFile()), FMT_NONE, 7) << newl
		<< L"Process:         " << XtoW(GetByteStringW(m_pSystem->GetProcessMemoryUsage()), FMT_NONE, 7) << newl
		<< L"Process Virtual: " << XtoW(GetByteStringW(m_pSystem->GetProcessVirtualMemoryUsage()), FMT_NONE, 7) << newl
		<< L"----------------------------------------" << newl
		<< L"Allocated:   " << XtoW(m_ullTotalAllocs, FMT_DELIMIT, 14) << L" " << XtoW(GetByteStringW(m_ullTotalAllocSize), FMT_NONE, 9) << newl
		<< L"Deallocated: " << XtoW(m_ullTotalDeallocs, FMT_DELIMIT, 14) << L" " << XtoW(GetByteStringW(m_ullTotalDeallocSize), FMT_NONE, 9) << newl
		<< L"Active:      " << XtoW(GetActiveAllocs(), FMT_DELIMIT, 14) << L" " << XtoW(GetByteStringW(GetActiveAllocsSize()), FMT_NONE, 9) << newl
		<< L"Common:      " << XtoW(s_ullCommonAllocs - s_ullCommonDeallocs, FMT_DELIMIT, 14) << L" " << XtoW(GetByteStringW(s_ullCommonAllocSize - s_ullCommonDeallocSize), FMT_NONE, 9) << newl
		<< L"Overhead:    " << XtoW(GetByteStringW(m_ullMemoryManagerOverhead), FMT_NONE, 9) << newl
		<< L"Total:       " << XtoW(GetByteStringW(m_ullMemoryManagerOverhead + GetActiveAllocsSize() + (s_ullCommonAllocSize - s_ullCommonDeallocSize)), FMT_NONE, 9) << newl
		<< L"----------------------------------------" << newl;

	for (uint ui(0); ui < MAX_HEAPS; ++ui)
	{
		CHeap *pHeap(m_apHeaps[ui]);
		if (pHeap == NULL)
			continue;

		m_pConsole->Admin()
			<< XtoW(L"[" + SingleToWide(pHeap->GetName()) + L"]", FMT_ALIGNLEFT, 13)
			<< XtoW(pHeap->GetAllocCount() - pHeap->GetDeallocCount(), FMT_DELIMIT, 9) << L" "
			<< XtoW(GetByteStringW(pHeap->GetAllocSize() - pHeap->GetDeallocSize()), FMT_NONE, 9)
			<< newl;
	}
}


/*====================
  CMemManager::PrintTrackingStats
  ====================*/
void	CMemManager::PrintTrackingStats()
{
	size_t zAllocs(m_ullTrackAllocs);
	size_t zDeallocs(m_ullTrackDeallocs);
	size_t zAllocSize(m_ullTrackAllocSize);
	size_t zDeallocSize(m_ullTrackDeallocSize);

	/*
	SMemHeader *pHeader(m_pTrackTail);
	SMemHeader *pHeaderEnd(m_pTrackHead);

	while (pHeader != NULL && pHeader != pHeaderEnd)
	{
		Console.Mem << _T("#") << pHeader->uiSequence << SPACE << pHeader->pHeap->GetName() << SPACE << INT_SIZE(pHeader->zSize) << newl;
		pHeader = pHeader->pTrackNext;
	}
	/**/

	m_pConsole->Admin() << L"Total allocations: " << INT_SIZE(zAllocs) << L" (" << INT_SIZE(zAllocSize) << L" bytes)" << newl
		<< L"Total deallocations: " << INT_SIZE(zDeallocs) << L" (" << INT_SIZE(zDeallocSize) << L" bytes)" << newl
		<< L"Active alloations: " << INT_SIZE(zAllocs - zDeallocs) << L" (" << INT_SIZE(zAllocSize - zDeallocSize) << L" bytes)" << newl;
}


/*====================
  CMemManager::PrintAllocations
  ====================*/
void	CMemManager::PrintAllocations(const char *szHeapName, uint uiTime)
{
	SMemHeader *pHeader(m_pTail);
	SMemHeader *pStop(m_pHead);
	uint uiTotal(0);
	bool bFoundHeap(false);
	if (szHeapName != NULL && strlen(szHeapName) > 0)
	{
		for (int i(0); i < MAX_HEAPS; ++i)
		{
			if (strcmp(szHeapName, m_apHeaps[i]->GetName()) == 0)
			{
				pHeader = m_apHeaps[i]->GetTail();
				pStop = m_apHeaps[i]->GetHead();
				bFoundHeap = true;
				break;
			}
		}
	}

	if (szHeapName != NULL && !bFoundHeap)
		return;

	if (uiTime == -1)
		uiTime = m_uiTimeStamp;

	while (pHeader != NULL && pHeader != pStop)
	{
		if (pHeader->uiTimeStamp >= uiTime)
		{
			m_pConsole->Admin()
				<< XtoW(L"[" + SingleToWide(pHeader->pHeap->GetName()) + L"]", FMT_ALIGNLEFT, HEAP_MAX_NAME_LEN + 2) << L" "
				<< XtoW(GetByteStringW(INT_SIZE(pHeader->zSize)), FMT_NONE, 8) << L" ";
			
			if (pHeader->szFile[0] != 0)
			{
				m_pConsole->Admin() << pHeader->szFile;
				if (pHeader->unLine != 0)
					m_pConsole->Admin() << L", " << pHeader->unLine;
			}
			
			m_pConsole->Admin() << newl;

			uiTotal += INT_SIZE(pHeader->zSize);
		}

		if (bFoundHeap)
			pHeader = pHeader->pHeapNext;
		else
			pHeader = pHeader->pNext;
	}

	m_pConsole->Admin() << L"Total allocated: " << GetByteStringW(uiTotal) << newl;
}


/*====================
  CMemManager::PrintAllocationsNoDuplicates
  ====================*/
void	CMemManager::PrintAllocationsNoDuplicates(const char *szHeapName, uint uiTime)
{
	SMemHeader *pHeader(m_pTail);
	SMemHeader *pStop(m_pHead);
	uint uiTotal(0);
	uint uiTotalNoFile(0);
	bool bFoundHeap(false);
	if (szHeapName != NULL && strlen(szHeapName) > 0)
	{
		for (int i(0); i < MAX_HEAPS; ++i)
		{
			if (strcmp(szHeapName, m_apHeaps[i]->GetName()) == 0)
			{
				pHeader = m_apHeaps[i]->GetTail();
				pStop = m_apHeaps[i]->GetHead();
				bFoundHeap = true;
				break;
			}
		}
	}

	if (szHeapName != NULL && !bFoundHeap)
		return;

	if (uiTime == -1)
		uiTime = m_uiTimeStamp;

	map<string, map<uint, uint> > mapAllocations;

	while (pHeader != NULL && pHeader != pStop)
	{
		if (pHeader->uiTimeStamp >= uiTime)
		{
			if (strlen(pHeader->szFile) != 0)
			{
				map<string, map<uint, uint> >::iterator findit(mapAllocations.find(pHeader->szFile));

				if (findit == mapAllocations.end())
				{
					mapAllocations.insert(pair<string, map<uint, uint> >(pHeader->szFile, map<uint, uint>()));
					findit = mapAllocations.find(pHeader->szFile);
				}

				map<uint, uint>::iterator lineit(findit->second.find(pHeader->unLine));

				if (lineit == findit->second.end())
				{
					findit->second.insert(pair<uint, uint>(pHeader->unLine, 0));
					lineit = findit->second.find(pHeader->unLine);
				}

				lineit->second += INT_SIZE(pHeader->zSize);
			}
			else
				uiTotalNoFile += INT_SIZE(pHeader->zSize);

			uiTotal += INT_SIZE(pHeader->zSize);
		}

		if (bFoundHeap)
			pHeader = pHeader->pHeapNext;
		else
			pHeader = pHeader->pNext;
	}

	for (map<string, map<uint, uint> >::iterator fileit(mapAllocations.begin()); fileit != mapAllocations.end(); fileit++)
	{
		for (map<uint, uint>::iterator lineit(fileit->second.begin()); lineit != fileit->second.end(); lineit++)
			m_pConsole->Admin() << L"File: " << fileit->first << L" Line: " << lineit->first << L" Total Size: " << lineit->second << newl;
	}

	m_pConsole->Admin() << L"Allocated not using K2_NEW: " << uiTotalNoFile << newl;
	m_pConsole->Admin() << L"Total allocated: " << uiTotal << newl;
}


/*====================
  CMemManager::ResetTracking
  ====================*/
void	CMemManager::ResetTracking()
{
	m_ullTrackAllocs = m_ullTrackDeallocs = 0;
	m_ullTrackAllocSize = m_ullTrackDeallocSize = 0;
	m_uiTimeStamp = m_pCore->GetMilliseconds();
	
	SMemHeader *pHeader(m_pTrackTail);
	while (pHeader != NULL)
	{
		if (pHeader->pTrackPrev != NULL)
			pHeader->pTrackPrev->pTrackNext = NULL;
		pHeader->pTrackPrev = NULL;
		pHeader = pHeader->pTrackNext;
	}

	m_pTrackHead = NULL;
	m_pTrackTail = NULL;
}


/*====================
  CMemManager::PrintTaggedAllocations
  ====================*/
void	CMemManager::PrintTaggedAllocations(int iTag)
{
	SMemHeader *pHeader(m_pTail);

	while (pHeader != NULL)
	{
		if (pHeader->iTag == iTag)
			m_pConsole->Admin() << pHeader->pHeap->GetName() << SPACE << INT_SIZE(pHeader->zSize) << newl;
		pHeader = pHeader->pNext;
	}
}


/*====================
  CMemManager::Validate
  ====================*/
void	CMemManager::Validate()
{
	for (uint ui(0); ui < MAX_HEAPS; ++ui)
	{
		if (m_apHeaps[ui] == NULL)
			continue;

		m_apHeaps[ui]->Validate();
	}
}


/*--------------------
  MemValidate
  --------------------*/
CMD(MemValidate, 0)
{
	pCore->GetMemManager()->Validate();
}


/*--------------------
  MemInfo
  --------------------*/
CMD(MemInfo, 0)
{
	pCore->GetMemManager()->PrintStats();
}


/*--------------------
  MemTrackInfo
  --------------------*/
CMD(MemTrackInfo, 0)
{
	pCore->GetMemManager()->PrintTrackingStats();
}


/*--------------------
  MemResetTracking
  --------------------*/
CMD(MemResetTracking, 0)
{
	pCore->GetMemManager()->ResetTracking();
}


/*--------------------
  MemPrintTagged
  --------------------*/
CMD(MemPrintTagged, 1)
{
	pCore->GetMemManager()->PrintTaggedAllocations(AtoI(vArgList[0]));
}


/*--------------------
  PrintAllocations
  --------------------*/
CMD(PrintAllocations, 0)
{
	if (vArgList.size() >= 2)
		pCore->GetMemManager()->PrintAllocations(WideToSingle(vArgList[0]).c_str(), AtoI(vArgList[1]));
	else if (!vArgList.empty())
		pCore->GetMemManager()->PrintAllocations(WideToSingle(vArgList[0]).c_str());
	else
		pCore->GetMemManager()->PrintAllocations();
}


/*--------------------
  PrintAllocationsNoDuplicates
  --------------------*/
CMD(PrintAllocationsNoDuplicates, 0)
{
	if (vArgList.size() >= 2)
		pCore->GetMemManager()->PrintAllocationsNoDuplicates(WideToSingle(vArgList[0]).c_str(), AtoI(vArgList[1]));
	else if (!vArgList.empty())
		pCore->GetMemManager()->PrintAllocationsNoDuplicates(WideToSingle(vArgList[0]).c_str());
	else
		pCore->GetMemManager()->PrintAllocationsNoDuplicates();
}
#endif //DEBUG_MEM

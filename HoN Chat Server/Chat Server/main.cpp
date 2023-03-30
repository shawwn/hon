// (C)2008 S2 Games
// main.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "chatserver_common.h"

#include "c_core.h"
#include "c_memmanager.h"

#include "curl/curl.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
CMemManager*	g_pMemManager(NULL);
#ifdef CURL_CUSTOM_MEM_MANAGER
CHeap*			g_pCURLHeap(NULL);

static void*	Malloc(size_t zSize)				{ return g_pMemManager->Allocate(zSize, ALLOCATE_CURL, g_pCURLHeap); }
static void		Free(void *p)						{ g_pMemManager->DeallocateCommon(p, ALLOCATE_CURL); }
static void*	Realloc(void *pOld, size_t zSize)	{ return g_pMemManager->Reallocate(pOld, zSize, ALLOCATE_CURL, g_pCURLHeap); }
static char*	StrDup(const char *szOld)
{
	size_t zLen(strlen(szOld) + 1);
	char *szNew((char*)g_pMemManager->Allocate(zLen, ALLOCATE_CURL, g_pCURLHeap));
	memcpy(szNew, szOld, zLen);
	return szNew;
}
static void*	CAlloc(size_t zCount, size_t zSize)	{ void *pNew(g_pMemManager->Allocate(zCount * zSize, ALLOCATE_CURL, g_pCURLHeap)); memset(pNew, 0, zCount * zSize); return pNew; }
#endif
//=============================================================================

/*====================
  wmain
  ====================*/
int wmain(int argc, _TCHAR* argv[])
{
	CMemManager MemManager;
	g_pMemManager = &MemManager;
	CHeap heapDefault(&MemManager, "default");
	MemManager.SetDefaultHeap(&heapDefault);

#ifdef CURL_CUSTOM_MEM_MANAGER
	CHeap heapCURL(&MemManager, "curl");
	g_pCURLHeap = &heapCURL;

	curl_global_init_mem(CURL_GLOBAL_ALL, Malloc, Free, Realloc, StrDup, CAlloc);
#else
	curl_global_init(CURL_GLOBAL_ALL);
#endif

	CCore core(&MemManager);
	if (!core.Initialize())
		return EXIT_INIT_FAILED;

	int iReturn(core.Execute());

	curl_global_cleanup();

	return iReturn;
}

// (C)2008 S2 Games
// c_httpmanager.h
//
//=============================================================================
#ifndef __C_HTTPMANAGER_H__
#define __C_HTTPMANAGER_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_core.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CHTTPRequest;
class CPHPData;
typedef void CURL;
typedef void CURLM;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
enum ENotifyType
{
	NOTIFY_TYPE_UNKNOWN,
	NOTIFY_TYPE_BUDDY_ADDER,
	NOTIFY_TYPE_BUDDY_ADDED,
	NOTIFY_TYPE_BUDDY_REMOVER,
	NOTIFY_TYPE_BUDDY_REMOVED,
	NOTIFY_TYPE_CLAN_RANK,
	NOTIFY_TYPE_CLAN_ADD,
	NOTIFY_TYPE_CLAN_REMOVE,
};

const uint INVALID_REQUEST_INDEX(-1);

typedef CSTLAllocator<CHTTPRequest*>				HTTPRequestList_alloc;
typedef list<CHTTPRequest*, HTTPRequestList_alloc>	HTTPRequestList;
typedef HTTPRequestList::iterator					HTTPRequestList_it;
//=============================================================================

//=============================================================================
// CHTTPManager
//=============================================================================
class CHTTPManager
{
private:
	DECLARE_STL_VECTOR_TYPES(Request, CHTTPRequest*);
	DECLARE_STL_DEQUE_TYPES(Request, CHTTPRequest*);
	DECLARE_STL_MAP_TYPES(CurlResult, CURL*, CHTTPRequest*);

	DECLARE_CORE_API;

	CHeap*			m_pHeap;
	CHeap*			m_pSTLHeap;

	CURLM*			m_pCurlMulti;

	CurlResultMap*	m_pResults;

	RequestVector*	m_pRequests;
	RequestDeque*	m_pReleasedRequests;
	uivector		m_vAvailableRequestIDs;

	uint			m_uiTotalRequests;
	uint			m_uiFailedRequests;

public:
	~CHTTPManager();
	CHTTPManager();

	inline void		IncrementFailedRequests()	{ ++m_uiFailedRequests; }

	bool			Initialize(CCore *pCore);
	void			Frame();
	void			Shutdown();

	CHTTPRequest*	SpawnRequest();
	void			SendRequest(CHTTPRequest *pRequest);
	void			KillRequest(CHTTPRequest *pRequest);
	void			ReleaseRequest(CHTTPRequest *pRequest);
	
	void			Reboot();

	void			PrintUsage() const;
};
//=============================================================================
#endif	//__C_HTTPMANAGER_H__
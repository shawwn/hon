// (C)2008 S2 Games
// c_httpmanager.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "chatserver_common.h"

#include "c_httpmanager.h"
#include "c_httprequest.h"
#include "c_phpdata.h"
#include "c_console.h"
#include "c_clientmanager.h"
#include "c_netmanager.h"

#include "curl/curl.h"
//=============================================================================

/*====================
  CHTTPManager::CHTTPManager
  ====================*/
CHTTPManager::~CHTTPManager()
{
	Shutdown();

	delete m_pResults;
	delete m_pRequests;
	delete m_pReleasedRequests;
	
	delete m_pSTLHeap;
	delete m_pHeap;
}


/*====================
  CHTTPManager::CHTTPManager
  ====================*/
CHTTPManager::CHTTPManager() :
NULL_CORE_API,
m_pHeap(NULL),
m_pSTLHeap(NULL),
m_pCurlMulti(NULL),
m_pResults(NULL),
m_pRequests(NULL),
m_pReleasedRequests(NULL),
m_uiTotalRequests(0),
m_uiFailedRequests(0)
{
}


/*====================
  CHTTPManager::Initialize
  ====================*/
bool	CHTTPManager::Initialize(CCore *pCore)
{
	INIT_CORE_API(pCore);
	
	m_pHeap = m_pMemManager->AllocateHeap("http");
	m_pSTLHeap = m_pMemManager->AllocateHeap("http_stl");

	m_pResults = HEAP_NEW(m_pMemManager, m_pHeap) CurlResultMap(std::less<CURL*>(), CurlResultMap_alloc(m_pMemManager, m_pSTLHeap));
	m_pRequests = HEAP_NEW(m_pMemManager, m_pHeap) RequestVector(RequestVector_alloc(m_pMemManager, m_pSTLHeap));
	m_pReleasedRequests = HEAP_NEW(m_pMemManager, m_pHeap) RequestDeque(RequestDeque_alloc(m_pMemManager, m_pSTLHeap));

	m_pCurlMulti = curl_multi_init();
	if (m_pCurlMulti == NULL)
		return  false;

	return true;
}


/*====================
  CHTTPManager::Frame
  ====================*/
void	CHTTPManager::Frame()
{
	PROFILE("CHTTPManager::Frame")

	CURLMcode result(CURLM_CALL_MULTI_PERFORM);
	while (result == CURLM_CALL_MULTI_PERFORM)
	{
		int iProcessCount(0);
		result = curl_multi_perform(m_pCurlMulti, &iProcessCount);
		if (result != CURLE_OK && result != CURLM_CALL_MULTI_PERFORM)
			m_pConsole->Admin() << _T("CHTTPManager::Frame() - ") << curl_multi_strerror(result) << newl;
	}

	// Grab messages
	for (;;)
	{
		int iMessageCount(0);
		CURLMsg *pMsg(curl_multi_info_read(m_pCurlMulti, &iMessageCount));
		if (pMsg == NULL)
			break;

		// CURL docs say this is the only valid message right now
		if (pMsg->msg != CURLMSG_DONE)
			continue;

		CurlResultMap_it itRequest(m_pResults->find(pMsg->easy_handle));
		if (itRequest == m_pResults->end())
			continue;

		itRequest->second->Completed(pMsg->data.result);
		curl_multi_remove_handle(m_pCurlMulti, pMsg->easy_handle);
	}

	// Expire dormant requests
	if (!m_pReleasedRequests->empty())
	{
		CHTTPRequest *pRequest(m_pReleasedRequests->front());
		if (m_pCore->GetSeconds() - pRequest->GetTimeStamp() > 30)
		{
			m_pReleasedRequests->pop_front();
			KillRequest(pRequest);
		}
	}
}


/*====================
  CHTTPManager::Shutdown
  ====================*/
void	CHTTPManager::Shutdown()
{
	for (RequestVector_it itRequest(m_pRequests->begin()), itEnd(m_pRequests->end()); itRequest != itEnd; ++itRequest)
	{
		CHTTPRequest *pRequest(*itRequest);
		if (pRequest == NULL)
			continue;

		curl_multi_remove_handle(m_pCurlMulti, pRequest->GetCURL());
		curl_easy_cleanup(pRequest->GetCURL());

		delete *itRequest;
	}
	m_pRequests->clear();

	if (m_pCurlMulti != NULL)
	{
		curl_multi_cleanup(m_pCurlMulti);
		m_pCurlMulti = NULL;
	}
}


/*====================
  CHTTPManager::SpawnRequest
  ====================*/
CHTTPRequest*	CHTTPManager::SpawnRequest()
{
	PROFILE("CHTTPManager::SpawnRequest");

	if (!m_pReleasedRequests->empty())
	{
		CHTTPRequest *pRequest(m_pReleasedRequests->back());
		m_pReleasedRequests->pop_back();
		m_pResults->insert(CurlResultMap_pair(pRequest->GetCURL(), pRequest));
		return pRequest;
	}

	CURL *pCurlEasy(curl_easy_init());
	if (pCurlEasy == NULL)
		return NULL;

	uint uiID(INT_SIZE(m_pRequests->size()));
	if (!m_vAvailableRequestIDs.empty())
		uiID = m_vAvailableRequestIDs.back();

	CHTTPRequest *pNewRequest(HEAP_NEW(m_pMemManager, m_pHeap) CHTTPRequest(m_pCore, pCurlEasy, uiID));
	if (pNewRequest == NULL)
		return NULL;

	if (m_vAvailableRequestIDs.empty())
	{
		m_pRequests->push_back(pNewRequest);
	}
	else
	{	
		m_pRequests->at(uiID) = pNewRequest;
		m_vAvailableRequestIDs.pop_back();
	}
	
	m_pResults->insert(CurlResultMap_pair(pCurlEasy, pNewRequest));
	return pNewRequest;
}


/*====================
  CHTTPManager::SendRequest
  ====================*/
void	CHTTPManager::SendRequest(CHTTPRequest *pRequest)
{
	PROFILE("CHTTPManager::SendRequest")

	++m_uiTotalRequests;

	CURLMcode code(curl_multi_add_handle(m_pCurlMulti, pRequest->GetCURL()));
	if (code != CURLM_OK)
		pRequest->Completed(code);
}


/*====================
  CHTTPManager::KillRequest
  ====================*/
void	CHTTPManager::KillRequest(CHTTPRequest *pRequest)
{
	PROFILE("CHTTPManager::KillRequest")

	if (pRequest == NULL)
		return;

	m_pResults->erase(pRequest->GetCURL());

	curl_multi_remove_handle(m_pCurlMulti, pRequest->GetCURL());
	curl_easy_cleanup(pRequest->GetCURL());

	uint uiID(pRequest->GetID());
	delete m_pRequests->at(uiID);
	m_pRequests->at(uiID) = NULL;
	m_vAvailableRequestIDs.push_back(uiID);
}


/*====================
  CHTTPManager::ReleaseRequest
  ====================*/
void	CHTTPManager::ReleaseRequest(CHTTPRequest *pRequest)
{
	PROFILE("CHTTPManager::ReleaseRequest")

	if (pRequest == NULL)
		return;

	m_pResults->erase(pRequest->GetCURL());

	curl_multi_remove_handle(m_pCurlMulti, pRequest->GetCURL());
	pRequest->Reset();
	pRequest->SetTargetURL(SNULL);
	pRequest->SetTimeStamp(m_pCore->GetSeconds());

	m_pReleasedRequests->push_back(pRequest);
}


/*====================
  CHTTPManager::Reboot
  ====================*/
void	CHTTPManager::Reboot()
{
	// Kill all requests
	for (RequestVector_it itRequest(m_pRequests->begin()), itEnd(m_pRequests->end()); itRequest != itEnd; ++itRequest)
	{
		CHTTPRequest *pRequest(*itRequest);
		if (pRequest == NULL)
			continue;

		curl_multi_remove_handle(m_pCurlMulti, pRequest->GetCURL());
		curl_easy_cleanup(pRequest->GetCURL());

		delete *itRequest;
	}

	m_pRequests->clear();
	m_pReleasedRequests->clear();
	m_pResults->clear();
	m_vAvailableRequestIDs.clear();

	// Shutdown the library
	if (m_pCurlMulti != NULL)
	{
		curl_multi_cleanup(m_pCurlMulti);
		m_pCurlMulti = NULL;
	}

	curl_global_cleanup();

	// Initialize a new instance of the library
	curl_global_init(CURL_GLOBAL_ALL);

	m_pCurlMulti = curl_multi_init();
	if (m_pCurlMulti == NULL)
		CSystem::Fatal(L"Could not re-initialize CURL");

	// Notify all other dependant modules that their requests are no longer valid
	m_pClientManager->InvalidateAllClientRequests();
	m_pClientManager->SpawnNewHeartbeatRequest();
	m_pNetManager->FlushPendingConnections();
}


/*====================
  CHTTPManager::PrintUsage
  ====================*/
void	CHTTPManager::PrintUsage() const
{
	uint uiActive(INT_SIZE((m_pRequests->size() - m_pReleasedRequests->size()) - m_vAvailableRequestIDs.size()));

	m_pConsole->Admin()
		<< newl
		<< L"HTTP Requests" << newl
		<< L"-------------" << newl
		<< L"Active:        " << XtoA(uiActive, FMT_DELIMIT, 5) << L" / " << XtoA(INT_SIZE(m_pRequests->size()), FMT_DELIMIT) << newl
		<< L"Standby:       " << XtoA(INT_SIZE(m_pReleasedRequests->size()), FMT_DELIMIT, 5) << newl
		<< L"Dead:          " << XtoA(INT_SIZE(m_vAvailableRequestIDs.size()), FMT_DELIMIT, 5) << newl
		<< L"Total:         " << XtoA(m_uiTotalRequests, FMT_DELIMIT, 5) << newl
		<< L"Failed:        " << XtoA(m_uiFailedRequests, FMT_DELIMIT, 5) << L" (" << XtoA(100.0f * (m_uiFailedRequests / float(m_uiTotalRequests)), FMT_NONE, 0, 2) << L"%)" << newl;
}

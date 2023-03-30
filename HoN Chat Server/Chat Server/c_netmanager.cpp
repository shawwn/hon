// (C)2008 S2 Games
// c_netmanager.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "chatserver_common.h"

#include "c_netmanager.h"
#include "c_httprequest.h"
#include "c_phpdata.h"
#include "c_pendingconnection.h"
#include "c_peer.h"
#include "c_console.h"
#include "c_matchmaker.h"
#include "c_gamemanager.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
CVAR(string, net_masterServerAddress, "masterserver.hon.s2games.com");
//=============================================================================

/*====================
  CNetManager::~CNetManager
  ====================*/
CNetManager::~CNetManager()
{
	m_sockListen.Close();

	for (PendingConnectionList_it it(m_pPendingConnections->begin()); it != m_pPendingConnections->end(); ++it)
		delete *it;
	delete m_pPendingConnections;

	m_NetDriver.Shutdown();

	delete m_pSTLHeap;
	delete m_pHeap;
}


/*====================
  CNetManager::CNetManager
  ====================*/
CNetManager::CNetManager() :
NULL_CORE_API,
m_pHeap(NULL),
m_pSTLHeap(NULL),
m_bListening(false),
m_unListenPort(0),
m_unRequestedListenPort(0),
m_uiReinitTime(INVALID_TIME),
m_uiConnectionCount(0),
m_uiSuccessfulConnectionCount(0)
{
}


/*====================
  CNetManager::Initialize
  ====================*/
bool	CNetManager::Initialize(CCore *pCore)
{
	INIT_CORE_API(pCore);

	m_pHeap = m_pMemManager->AllocateHeap("net");
	m_pSTLHeap = m_pMemManager->AllocateHeap("net_stl");

	m_pPendingConnections = HEAP_NEW(m_pMemManager, m_pHeap) PendingConnectionList(PendingConnectionList_alloc(m_pMemManager, m_pSTLHeap));

	if (!m_NetDriver.Initialize(pCore))
		return false;
	
	// Lookup public ip address
	CHTTPRequest *pRequest(m_pHTTPManager->SpawnRequest());
	pRequest->SetTargetURL(net_masterServerAddress.Get() + SERVER_REQUEST_SCRIPT "?f=whatismyip");
	
	for (int i(0); i < 5; ++i)
	{
		pRequest->SendRequest();
		pRequest->Wait();

		if (pRequest->GetStatus() == HTTP_REQUEST_SUCCESS)
		{
			m_sPublicAddress = WideToSingle(pRequest->GetResponse());
			if (IsIPAddress(m_sPublicAddress))
				break;

			m_sPublicAddress.clear();
		}

		m_pSystem->Sleep(1000);
	}
	
	m_pHTTPManager->KillRequest(pRequest);

	m_pConsole->Admin() << L"Public IP: " << m_sPublicAddress << newl;
	return true;
}


/*====================
  CNetManager::Reinitialize
  ====================*/
void	CNetManager::Reinitialize()
{
	PROFILE("CNetManager::Reinitialize")

	if (!OpenListenSocket(m_unRequestedListenPort))
	{
		m_uiReinitTime = m_pCore->GetSeconds() + 5;
		return;
	}

	m_uiReinitTime = INVALID_TIME;
}


/*====================
  CNetManager::OpenListenSocket
  ====================*/
bool	CNetManager::OpenListenSocket(ushort unPort)
{
	if (m_bListening)
	{
		m_pConsole->Admin() << L"Already listening on port " << m_unListenPort << L", ignoring new listen request." << newl;
		return false;
	}

	if (unPort == 0)
	{
		for (unPort = 11031; unPort < 11131; ++unPort)
		{
			if (m_sockListen.Init(&m_NetDriver, K2_SOCKET_TCP, unPort))
			{
				m_unListenPort = m_sockListen.GetRecvPort();
				break;
			}
		}
	}
	else
	{
		m_unRequestedListenPort = unPort;
		if (m_sockListen.Init(&m_NetDriver, K2_SOCKET_TCP, unPort))
			m_unListenPort = m_sockListen.GetRecvPort();

		if (m_unListenPort != m_unRequestedListenPort)
		{
			m_pConsole->Admin() << L"Failed to open requested port: " << m_unRequestedListenPort << newl;
			Disconnect();
			m_uiReinitTime = m_pCore->GetSeconds() + 5;
			return false;
		}
	}

	if (!m_sockListen.IsInitialized() || !m_sockListen.OpenListenPort())
	{
		m_pConsole->Admin() << L"Failed to open requested port: " << m_unRequestedListenPort << newl;
		Disconnect();
		m_uiReinitTime = m_pCore->GetSeconds() + 5;
		return false;
	}

	m_bListening = true;
	m_pConsole->Admin() << L"Listening on port: " << m_unListenPort << newl;
	return true;
}


/*====================
  CNetManager::Frame
  ====================*/
void	CNetManager::Frame()
{
	PROFILE("CNetManager::Frame")

	// Check for reinitializing socket
	if (m_uiReinitTime != INVALID_TIME)
	{
		if (m_uiReinitTime <= m_pCore->GetSeconds())
			Reinitialize();
		return;
	}

	// Check for socket errors
	if (m_sockListen.HasError())
	{
		m_pConsole->Std() << L"An error occured while listening for connections. Attempting reinitialize in 5 seconds." << newl;
		Disconnect();
		m_uiReinitTime = m_pCore->GetSeconds() + 5;
		return;
	}

	// Check for new connection requests
	while (m_sockListen.DataWaiting())
		AcceptConnection();

	// Loop through the clients that have requested a connection and check status
	PendingConnectionList_it itConnection(m_pPendingConnections->begin());
	PendingConnectionList_it itEnd(m_pPendingConnections->end());
	while (itConnection != itEnd)
	{
		if (!(*itConnection)->Frame())
		{
			delete *itConnection;
			STL_ERASE(*m_pPendingConnections, itConnection);
			itEnd = m_pPendingConnections->end();
			continue;
		}

		++itConnection;
	}
}


/*====================
  CNetManager::AcceptConnection
  ====================*/
void	CNetManager::AcceptConnection()
{
	PROFILE("CNetManager::AcceptConnection")

	CSocket *pNewSocket(m_sockListen.AcceptConnection(m_pMemManager));
	if (pNewSocket == NULL || !pNewSocket->IsInitialized())
	{
		delete pNewSocket;
		return;
	}

	CPendingConnection *pNewConnection(HEAP_NEW(m_pMemManager, m_pHeap) CPendingConnection(pNewSocket, m_pCore));
	m_pPendingConnections->push_back(pNewConnection);
	++m_uiConnectionCount;
}


/*====================
  CNetManager::Connect
  ====================*/
CSocket*	CNetManager::Connect(const string &sAddress, ushort unPort, ESocketType eType)
{
	CSocket *pSocket(HEAP_NEW(m_pMemManager, m_pHeap) CSocket);
	if (!pSocket->Init(&m_NetDriver, eType) || !pSocket->SetSendAddr(sAddress, unPort))
	{
		delete pSocket;
		return NULL;
	}

	return pSocket;
}


/*====================
  CNetManager::Disconnect
  ====================*/
void	CNetManager::Disconnect()
{
	m_sockListen.Close();
	m_unListenPort = 0;
	m_bListening = false;

	m_uiConnectionCount = 0;

	FlushPendingConnections();

	m_pConsole->Admin() << L"Chat server is now offline." << newl;
}


/*====================
  CNetManager::FlushPendingConnections
  ====================*/
void	CNetManager::FlushPendingConnections()
{
	for (PendingConnectionList_it it(m_pPendingConnections->begin()); it != m_pPendingConnections->end(); ++it)
		delete *it;

	m_pPendingConnections->clear();
}


/*====================
  CNetManager::PrintUsage
  ====================*/
void	CNetManager::PrintUsage()
{
	m_pConsole->Admin()
		<< newl
		<< L"Connections     " << newl
		<< L"--------------- " << newl
		<< L"Public address: " << m_sPublicAddress << newl
		<< L"Listen port:    " << m_unListenPort << newl
		<< L"Pending:        " << XtoA(INT_SIZE(m_pPendingConnections->size()), FMT_DELIMIT) << newl
		<< L"Total:          " << XtoA(m_uiConnectionCount, FMT_DELIMIT) << L" (" << XtoA(m_uiConnectionCount / SecToMin(m_pCore->GetSeconds()), FMT_NONE, 0, 2) << L" per min)" << newl
		<< L"Successful:     " << XtoA(m_uiSuccessfulConnectionCount, FMT_DELIMIT) << L" (" << XtoA(100.0f * (m_uiSuccessfulConnectionCount / float(m_uiConnectionCount)), FMT_NONE, 0, 2) << L"%)" << newl;
}

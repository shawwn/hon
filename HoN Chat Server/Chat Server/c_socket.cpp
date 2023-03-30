// (C)2005 S2 Games
// c_socket.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "chatserver_common.h"

#include "c_socket.h"
#include "c_netdriver.h"
//=============================================================================

/*====================
  CSocket::~CSocket
  ====================*/
CSocket::~CSocket()
{
	delete m_pSendAddr;
	Close();
}


/*====================
  CSocket::CSocket
  ====================*/
CSocket::CSocket() :
m_pNetDriver(NULL),
m_bInitialized(false),
m_eType(K2_SOCKET_INVALID),
m_uiSocket(0),

m_uiRecvAddress(0),
m_wRecvPort(0),
m_wSendPort(0),

m_pSendAddr(NULL),

m_bIsLocalConnection(false),
m_bIsLANConnection(false)
{
}


/*====================
  CSocket::Init
  ====================*/
bool	CSocket::Init(CNetDriver *pNetDriver, ESocketType eType, word wPort, bool bBlocking)
{
	m_pNetDriver = pNetDriver;
	m_uiSocket = 0;
	m_eType = eType;
	m_wRecvPort = wPort;

	if (m_pNetDriver == NULL)
		return false;

	if (!m_pNetDriver->OpenPort(m_uiSocket, m_wRecvPort, eType, bBlocking))
		return false;

	m_bInitialized = true;
	return true;
}

bool	CSocket::Init(const CSocket *pSocket, uint uiNewSocket)
{
	if (pSocket == NULL)
	{
		m_bInitialized = false;
		return false;
	}

	m_pNetDriver = pSocket->m_pNetDriver;
	m_uiSocket = uiNewSocket;
	m_bInitialized = pSocket->m_bInitialized;
	m_eType = pSocket->m_eType;

	m_sRecvAddress = pSocket->m_sRecvAddress;
	m_uiRecvAddress = pSocket->m_uiRecvAddress;
	m_wRecvPort = pSocket->m_wRecvPort;

	return m_bInitialized;
}


/*====================
  CSocket::SetSendAddr
  ====================*/
bool	CSocket::SetSendAddr(const string &sAddress, word wPort)
{
	if (!m_bInitialized)
		return false;

	m_sSendAddress = sAddress;
	if (wPort != 0)
		m_wSendPort = wPort;

	if (!m_pNetDriver->SetSendAddress(m_sSendAddress, m_wSendPort, m_pSendAddr, m_bIsLocalConnection, m_bIsLANConnection))
		return false;

	if (RequiresConnection() && !m_pNetDriver->Connect(m_uiSocket, m_pSendAddr))
		return false;

	return true;
}


/*====================
  CSocket::OpenListenPort
  ====================*/
bool	CSocket::OpenListenPort(int iMaxConnectionsWaiting)
{
	if (!m_bInitialized)
		return false;

	if (RequiresConnection())
		return m_pNetDriver->StartListening(m_uiSocket, iMaxConnectionsWaiting);

	return false;
}


/*====================
  CSocket::AcceptConnection
  ====================*/
CSocket*	CSocket::AcceptConnection(CMemManager *pMemManager)
{
	if (!m_bInitialized || !RequiresConnection() || pMemManager == NULL)
		return NULL;

	CSocket *pNewSocket(NEW(pMemManager) CSocket);
	if (pNewSocket == NULL)
		return NULL;

	uint uiNewSocket;
	if (m_pNetDriver->AcceptConnection(m_uiSocket, uiNewSocket, m_sRecvAddress, m_uiRecvAddress, m_wRecvPort))
		pNewSocket->Init(this, uiNewSocket);

	return pNewSocket;
}

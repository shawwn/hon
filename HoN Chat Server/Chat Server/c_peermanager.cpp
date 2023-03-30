// (C)2009 S2 Games
// c_peermanager.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "chatserver_common.h"

#include "c_peermanager.h"
#include "c_peer.h"
#include "c_console.h"
#include "c_netmanager.h"
//=============================================================================

/*====================
  CPeerManager::~CPeerManager
  ====================*/
CPeerManager::~CPeerManager()
{
	for (PeerList_it itPeer(m_pPendingPeers->begin()), itEnd(m_pPendingPeers->end()); itPeer != itEnd; ++itPeer)
		delete *itPeer;
	delete m_pPendingPeers;

	for (PeerMap_it itPeer(m_pPeers->begin()), itEnd(m_pPeers->end()); itPeer != itEnd; ++itPeer)
		delete itPeer->second;
	delete m_pPeers;

	delete m_pSTLHeap;
	delete m_pHeap;
}


/*====================
  CPeerManager::CPeerManager
  ====================*/
CPeerManager::CPeerManager() :
NULL_CORE_API,
m_pHeap(NULL),
m_pSTLHeap(NULL),
m_uiID(0),
m_uiNextID(1),
m_pPendingPeers(NULL),
m_pPeers(NULL)
{
}


/*====================
  CPeerManager::Initialize
  ====================*/
bool	CPeerManager::Initialize(CCore *pCore)
{
	INIT_CORE_API(pCore);

	m_pHeap = m_pMemManager->AllocateHeap("peer");
	m_pSTLHeap = m_pMemManager->AllocateHeap("peer_stl");

	m_pPendingPeers = HEAP_NEW(m_pMemManager, m_pSTLHeap) PeerList(PeerList_alloc(m_pMemManager, m_pSTLHeap));
	m_pPeers = HEAP_NEW(m_pMemManager, m_pHeap) PeerMap(std::less<uint>(), PeerList_alloc(m_pMemManager, m_pSTLHeap));

	m_sKey.clear();
	for (int i(0); i < 20; ++i)
		m_sKey += char(M_Random('A', 'Z'));

	return true;
}


/*====================
  CPeerManager::Frame
  ====================*/
void	CPeerManager::Frame()
{
	PROFILE("CPeerManager::Frame")

	// Process pending outgoing connections
	for (PeerList_it itPeer(m_pPendingPeers->begin()), itEnd(m_pPendingPeers->end()); itPeer != itEnd; )
	{
		CPeer *pPeer(*itPeer);
		
		if (!pPeer->Frame())
		{
			SAFE_DELETE(pPeer);
			STL_ERASE(*m_pPendingPeers, itPeer);
			continue;
		}

		if (pPeer->GetState() == PEER_STATE_CONNECTED)
		{
			STL_ERASE(*m_pPendingPeers, itPeer);
			continue;
		}

		++itPeer;
	}

	// Receive data from all active peers
	for (PeerMap_it itPeer(m_pPeers->begin()), itEnd(m_pPeers->end()); itPeer != itEnd; ++itPeer)
	{
		CPeer *pPeer(itPeer->second);
		if (pPeer != NULL)
			pPeer->Frame();
	}
}


/*====================
  CPeerManager::AddPeer
  ====================*/
void	CPeerManager::AddPeer(CSocket *pSocket, CPacket &pkt)
{
	PROFILE("CPeerManager::AddPeer")

	string sAddress(pkt.ReadString());
	ushort unPort(pkt.ReadShort());
	string sKey(pkt.ReadString());
	uint uiID(pkt.ReadInt());

	PeerMap_it itPeer(m_pPeers->find(uiID));

	// Check that the key is valid and not in use
	if (sKey != m_sKey || pkt.HasFaults() || (itPeer != m_pPeers->end() && itPeer->second->GetState() != PEER_STATE_IDLE))
	{
		CPacket pkt;
		pkt << PEER_CMD_GO_AWAY;
		pSocket->SendPacket(pkt);
		m_pConsole->Admin() << L"Rejected peer with key: " << sKey << newl;

		SAFE_DELETE(pSocket);
		return;
	}

	CPeer *pNewPeer(HEAP_NEW(m_pMemManager, m_pHeap) CPeer(m_pCore));
	if (pNewPeer == NULL)
	{
		CPacket pkt;
		pkt << PEER_CMD_GO_AWAY;
		pSocket->SendPacket(pkt);
		m_pConsole->Admin() << L"Rejected peer with key: " << sKey << newl;

		SAFE_DELETE(pSocket);
		return;
	}

	if (uiID == INVALID_PEER_ID)
		pNewPeer->SetID(m_uiNextID++);
	else
		pNewPeer->SetID(uiID);

	m_pPeers->insert(PeerMap_pair(uiID, pNewPeer));

	pNewPeer->SetSocket(pSocket);
	pNewPeer->SetAddress(sAddress);
	pNewPeer->SetPort(unPort);
	pNewPeer->SetState(PEER_STATE_CONNECTED);
	pNewPeer->SendWelcome(GetID());

	m_pConsole->Admin() << L"Peer #" << pNewPeer->GetID() << L" connected." << newl;
}


/*====================
  CPeerManager::Connect
  ====================*/
void	CPeerManager::Connect(const string &sAddress, ushort unPort, const string &sKey)
{
	PROFILE("CPeerManager::Connect")

	Disconnect();
	m_sKey = sKey;

	CSocket *pSocket(m_pNetManager->Connect(sAddress, unPort, K2_SOCKET_TCP));
	if (pSocket == NULL)
		return;

	CPeer *pNewPeer(HEAP_NEW(m_pMemManager, m_pHeap) CPeer(m_pCore));
	if (pNewPeer == NULL)
	{
		SAFE_DELETE(pSocket);
		return;
	}

	pNewPeer->SetSocket(pSocket);
	pNewPeer->SetState(PEER_STATE_CONNECTING);
	pNewPeer->SetExpireTime(m_pCore->GetSeconds() + 15);

	m_pPendingPeers->push_back(pNewPeer);
}

void	CPeerManager::Connect(CPeer *pPeer)
{
	PROFILE("CPeerManager::Connect")

	/*pPeer->Disconnect();
	CSocket *pSocket(m_pNetManager->Connect(pPeer->GetAddress(), pPeer->GetPort(), K2_SOCKET_TCP));
	if (pSocket == NULL)
		return;

	pPeer->SetSocket(pSocket);
	pPeer->SetState(PEER_STATE_CONNECTING);
	pPeer->SetKey(GetKey());
	pPeer->SetExpireTime(g_core.GetSeconds() + 15);

	m_vActivePeers[pPeer->GetID()] = NULL;
	m_pPendingPeers->push_back(pPeer);*/
}


/*====================
  CPeerManager::Connected
  ====================*/
void	CPeerManager::Connected(uint uiLocalID, CPeer *pNewPeer)
{
	PROFILE("CPeerManager::Connected");

	m_pPeers->insert(PeerMap_pair(pNewPeer->GetID(), pNewPeer));
	m_uiID = uiLocalID;

	m_pConsole->Admin() << L"Joined peer network as node #" << GetID() << newl;
}


/*====================
  CPeerManager::SendPeerList
  ====================*/
void	CPeerManager::SendPeerList(CPeer *pTargetPeer)
{
	pTargetPeer->GetSendPacket() << PEER_CMD_PEER_INFO << m_uiID << m_pNetManager->GetPublicAddress() << m_pNetManager->GetListenPort();

	for (PeerMap_it itPeer(m_pPeers->begin()), itEnd(m_pPeers->end()); itPeer != itEnd; ++itPeer)
		pTargetPeer->SendPeerInfo(itPeer->second);

	pTargetPeer->GetSendPacket() << PEER_CMD_END_PEER_INFO;
}


/*====================
  CPeerManager::BroadcastPeerData
  ====================*/
void	CPeerManager::BroadcastPeerData(CPeer *pPeer)
{
	for (PeerMap_it itPeer(m_pPeers->begin()), itEnd(m_pPeers->end()); itPeer != itEnd; ++itPeer)
	{
		CPeer *pTargetPeer(itPeer->second);
		if (pTargetPeer != NULL &&
			pTargetPeer->GetState() == PEER_STATE_CONNECTED)
			pTargetPeer->SendPeerInfo(pPeer);
	}
}


/*====================
  CPeerManager::GetPeer
  ====================*/
CPeer*	CPeerManager::GetPeer(uint uiID)
{
	PROFILE("CPeerManager::GetPeer")

	PeerMap_it itPeer(m_pPeers->find(uiID));
	if (itPeer == m_pPeers->end())
		return NULL;

	return itPeer->second;
}


/*====================
  CPeerManager::UpdatePeer
  ====================*/
void	CPeerManager::UpdatePeer(CPacket &pkt)
{
	uint uiID(pkt.ReadInt());
	string sAddress(pkt.ReadString());
	ushort unPort(pkt.ReadShort());

	if (pkt.HasFaults())
		return;
	
	m_pConsole->Admin() << L"Received peer info for peer #" << uiID << newl;

	if (uiID == m_uiID)
		return;

	CPeer *pPeer(NULL);
	PeerMap_it itPeer(m_pPeers->find(uiID));
	if (itPeer == m_pPeers->end())
	{
		pPeer = HEAP_NEW(m_pMemManager, m_pHeap) CPeer(m_pCore);
		if (pPeer == NULL)
			return;

		m_pPeers->insert(PeerMap_pair(uiID, pPeer));
	}
	else
	{
		pPeer = itPeer->second;
	}

	pPeer->SetID(uiID);
	pPeer->SetAddress(sAddress);
	pPeer->SetPort(unPort);
}


/*====================
  CPeerManager::Disconnect
  ====================*/
void	CPeerManager::Disconnect()
{
	PROFILE("CPeerManager::Disconnect")

	for (PeerList_it itPeer(m_pPendingPeers->begin()), itEnd(m_pPendingPeers->end()); itPeer != itEnd; ++itPeer)
		SAFE_DELETE(*itPeer);
	m_pPendingPeers->clear();

	for (PeerMap_it itPeer(m_pPeers->begin()), itEnd(m_pPeers->end()); itPeer != itEnd; ++itPeer)
		SAFE_DELETE(itPeer->second);
	m_pPeers->clear();
}


/*====================
  CPeerManager::RelayWhisper
  ====================*/
void	CPeerManager::RelayWhisper(const string &sSenderUTF8, const wstring &sTarget, const string &sMessageUTF8)
{
	for (PeerMap_it itPeer(m_pPeers->begin()), itEnd(m_pPeers->end()); itPeer != itEnd; ++itPeer)
	{
		CPeer *pTargetPeer(itPeer->second);
		if (pTargetPeer != NULL && pTargetPeer->GetState() == PEER_STATE_CONNECTED)
		{
			pTargetPeer->GetSendPacket() << PEER_CMD_WHISPER;
			pTargetPeer->GetSendPacket().WriteUTF8String(sSenderUTF8);
			pTargetPeer->GetSendPacket() << sTarget;
			pTargetPeer->GetSendPacket().WriteUTF8String(sMessageUTF8);
		}
	}
}


/*====================
  CPeerManager::PrintUsage
  ====================*/
void	CPeerManager::PrintUsage() const
{
	m_pConsole->Admin() << newl
		<< L"Peers       " << newl
		<< L"----------- " << newl
		<< L"Local ID:   " << GetID() << newl
		<< L"Key:        " << GetKey() << newl;

	for (PeerMap_cit itPeer(m_pPeers->begin()), itEnd(m_pPeers->end()); itPeer != itEnd; ++itPeer)
	{
		CPeer *pPeer(itPeer->second);
		if (pPeer == NULL)
		{
			m_pConsole->Admin() << L"#" << XtoW(itPeer->first, FMT_NONE, 2) << L" [NULL]" << newl;
			continue;
		}

		m_pConsole->Admin() << L"#" << XtoW(pPeer->GetID(), FMT_NONE, 2);
		switch (pPeer->GetState())
		{
		case PEER_STATE_IDLE:					m_pConsole->Admin() << L" [IDLE]                   "; break;
		case PEER_STATE_DISCONNECTED:			m_pConsole->Admin() << L" [DISCONNECTED]           "; break;
		case PEER_STATE_CONNECTING:				m_pConsole->Admin() << L" [CONNECTING]             "; break;
		case PEER_STATE_AUTHORIZING:			m_pConsole->Admin() << L" [AUTHORIZING]            "; break;
		case PEER_STATE_RECEIVING_PEER_INFO:	m_pConsole->Admin() << L" [RECEIVING_PEER_INFO]    "; break;
		case PEER_STATE_RECEIVING_CHANNEL_INFO:	m_pConsole->Admin() << L" [RECEIVING_CHANNEL_INFO] "; break;
		case PEER_STATE_RECEIVING_CLIENT_INFO:	m_pConsole->Admin() << L" [RECEIVING_CLIENT_INFO]  "; break;
		case PEER_STATE_CONNECTED:				m_pConsole->Admin() << L" [CONNECTED]              "; break;
		}

		if (pPeer->GetState() > PEER_STATE_IDLE)
			m_pConsole->Admin() << XtoS(pPeer->GetAddress() + ":" + XtoS(pPeer->GetPort()), FMT_ALIGNLEFT, 22);

		m_pConsole->Admin() << newl;
	}
}

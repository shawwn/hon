// (C)2008 S2 Games
// c_clientmanager.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "chatserver_common.h"

#include "c_clientmanager.h"
#include "c_channel.h"
#include "c_httprequest.h"
#include "c_phpdata.h"
#include "c_console.h"
#include "c_clanmanager.h"
//=============================================================================

//=============================================================================
//Declarations
//=============================================================================
EXTERN_CVAR(string, net_masterServerAddress);
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
CVAR(bool,	net_sendHeartbeat,				false);
CVAR(bool,	net_sendPlayerCounts,			false);
CVAR(uint,	net_heartbeatInterval,			60);
CVAR(uint,	client_longSessionThreshold,	300);
//=============================================================================

/*====================
  CClientManager::CClientManager
  ====================*/
CClientManager::CClientManager() :
NULL_CORE_API,
m_pHeap(NULL),
m_pSTLHeap(NULL),

m_uiNextMessageTime(INVALID_TIME),
m_uiNextHeartbeat(INVALID_TIME),
m_pHeartbeat(NULL),

m_pActiveClients(NULL),
m_pAccountToClientID(NULL),
m_pNameToClientID(NULL),

m_uiTotalUniqueClients(0),
m_uiTotalOnline(0),
m_uiTotalInGame(0),
m_llTotalSessionLength(0),
m_llTotalSessions(0),
m_llTotalLongSessions(0),
m_llTotalLongSessionLength(0),

m_uiVirtualClientCount(0),

m_pClanInvites(NULL),
m_pClanCreates(NULL),

m_pPlayerCounts(NULL),
m_pHTTPRequests(NULL)
{
}


/*====================
  CClientManager::~CClientManager
  ====================*/
CClientManager::~CClientManager()
{
	for (ClientVector_it it(m_pActiveClients->begin()); it != m_pActiveClients->end(); ++it)
		delete *it;
	delete m_pActiveClients;

	delete m_pAccountToClientID;
	delete m_pNameToClientID;

	delete m_pClanInvites;
	delete m_pClanCreates;

	delete m_pPlayerCounts;

	delete m_pHTTPRequests;

	delete m_pSTLHeap;
	delete m_pHeap;
}


/*====================
  CClientManager::SendHeartbeat
  ====================*/
void	CClientManager::SendHeartbeat()
{
	PROFILE("CClientManager::SendHeartbeat");

	m_pHeartbeat->ClearVariables();
	m_pHeartbeat->AddVariable(L"f", L"chat_online");
	m_pHeartbeat->AddVariable(L"online", m_uiTotalOnline);
	m_pHeartbeat->AddVariable(L"in_game", m_uiTotalInGame);
	m_pHeartbeat->AddVariable(L"seen", m_uiTotalUniqueClients);
	//m_pHeartbeat->AddVariable(L"avg_session", INT_FLOOR(m_llTotalSessionLength / float(m_llTotalSessions)));

	m_pHeartbeat->SendPostRequest();

	m_uiNextHeartbeat += net_heartbeatInterval;
}


/*====================
  CClientManager::Initialize
  ====================*/
void	CClientManager::Initialize(CCore *pCore)
{
	INIT_CORE_API(pCore);

	m_pHeap = m_pMemManager->AllocateHeap("client");
	m_pSTLHeap = m_pMemManager->AllocateHeap("client_stl");

	m_pActiveClients = HEAP_NEW(m_pMemManager, m_pHeap) ClientVector(ClientVector_alloc(m_pMemManager, m_pSTLHeap));
	m_pAccountToClientID = HEAP_NEW(m_pMemManager, m_pHeap) AccountClientMap(std::less<uint>(), AccountClientMap_alloc(m_pMemManager, m_pSTLHeap));
	m_pNameToClientID = HEAP_NEW(m_pMemManager, m_pHeap) NameClientMap(std::less<wstring>(), NameClientMap_alloc(m_pMemManager, m_pSTLHeap));

	m_pClanInvites = HEAP_NEW(m_pMemManager, m_pHeap) ClanInviteMap(std::less<uint>(), ClanInviteMap_alloc(m_pMemManager, m_pSTLHeap));
	m_pClanCreates = HEAP_NEW(m_pMemManager, m_pHeap) ClanCreateMap(std::less<uint>(), ClanCreateMap_alloc(m_pMemManager, m_pSTLHeap));

	m_pPlayerCounts = HEAP_NEW(m_pMemManager, m_pHeap) PlayerCountMap(std::less<uint>(), PlayerCountMap_alloc(m_pMemManager, m_pSTLHeap));

	m_pHTTPRequests = HEAP_NEW(m_pMemManager, m_pHeap) HTTPRequestList(HTTPRequestList_alloc(m_pMemManager, m_pSTLHeap));

	SpawnNewHeartbeatRequest();

	m_uiNextMessageTime = m_pCore->GetMilliseconds() + 3500;
	m_uiNextHeartbeat = m_pCore->GetSeconds();
}


/*====================
  CClientManager::Frame
  ====================*/
void	CClientManager::Frame()
{
	PROFILE("CClientManager::Frame");

	bool bDecrementMessages(false);
	if (m_uiNextMessageTime < m_pCore->GetMilliseconds())
	{
		m_uiNextMessageTime += 3500;
		bDecrementMessages = true;
	}

	// Heartbeat
	if (net_sendHeartbeat && m_pCore->GetSeconds() >= m_uiNextHeartbeat)
		SendHeartbeat();

	for (uint uiClient(0); uiClient < m_pActiveClients->size(); ++uiClient)
	{
		CClient *pClient(m_pActiveClients->at(uiClient));
		if (pClient == NULL)
			continue;

		if (bDecrementMessages)
			pClient->DecrementRequestCounter();

		if (!pClient->Frame())
		{
			RemoveClient(pClient);
			continue;
		}

		if (!pClient->CheckTimeout())
		{
			RemoveClient(pClient);
			continue;
		}

		if (pClient->UserCountUpdateNeeded())
			pClient->SendUserCount(m_uiTotalOnline);
	}

	// Check clan creation expiry times
	ClanCreateMap_it itCreate(m_pClanCreates->begin());
	ClanCreateMap_it itCreateEnd(m_pClanCreates->end());
	while (itCreate != itCreateEnd)
	{
		if (itCreate->second.uiRecvTime + 120000 < m_pCore->GetMilliseconds())
		{
			STL_ERASE(*m_pClanCreates, itCreate);
			continue;
		}

		++itCreate;
	}

	// Check clan invite expiry times
	ClanInviteMap_it itInvite(m_pClanInvites->begin());
	ClanInviteMap_it itInviteEnd(m_pClanInvites->end());
	while (itInvite != itInviteEnd)
	{
		if (itInvite->second.uiRecvTime + 120000 < m_pCore->GetMilliseconds())
		{
			STL_ERASE(*m_pClanInvites, itInvite);
			continue;
		}

		++itInvite;
	}

	// Check for completed requests
	HTTPRequestList_it itRequest(m_pHTTPRequests->begin());
	HTTPRequestList_it itEnd(m_pHTTPRequests->end());
	while (itRequest != itEnd)
	{
		if ((*itRequest)->GetStatus() == HTTP_REQUEST_SENDING)
		{
			++itRequest;
			continue;
		}

		m_pHTTPManager->ReleaseRequest(*itRequest);
		STL_ERASE(*m_pHTTPRequests, itRequest);
		itEnd = m_pHTTPRequests->end();
	}
}


/*====================
  CClientManager::AddClanMemberSuccess
  ====================*/
void	CClientManager::AddClanMemberSuccess(CClient *pClient)
{
	PROFILE("CClientManager::AddClanMemberSuccess");

	if (pClient == NULL)
		return;

	ClanInviteMap_it itClanInvite(m_pClanInvites->find(pClient->GetAccountID()));
	if (itClanInvite == m_pClanInvites->end())
		return;

	// Get clan entry or create one if this is the first time we've seen this clan
	CClan *pClan(m_pClanManager->GetClan(itClanInvite->second.uiClanID));
	if (pClan == NULL)
	{
		pClan = m_pClanManager->AddClan(itClanInvite->second.uiClanID);
		if (pClan == NULL)
			return;

		pClan->SetName(itClanInvite->second.sClan);
		pClan->SetTag(itClanInvite->second.sTag);
	}

	pClient->RemoveFlags(CLIENT_IS_OFFICER | CLIENT_IS_CLAN_LEADER);
	pClient->SetClan(pClan);

	CBufferDynamic buffer;
	buffer << CHAT_CMD_NEW_CLAN_MEMBER << pClient->GetAccountID();
	pClan->Broadcast(buffer, pClient);

	buffer << pClan->GetClanID() << pClan->GetNameUTF8() << byte(0) << pClan->GetTagUTF8() << byte(0);
	pClient->Send(buffer);

	buffer.Clear();
	buffer << CHAT_CMD_NAME_CHANGE << pClient->GetAccountID() << pClient->GetNameUTF8() << byte(0);
	pClient->SendToPeers(buffer);

	m_pClanInvites->erase(itClanInvite);
}


/*====================
  CClientManager::AddClanMemberFailed
  ====================*/
void	CClientManager::AddClanMemberFailed(CClient *pClient)
{
	PROFILE("CClientManager::AddClanMemberFailed");

	if (pClient == NULL)
		return;

	ClanInviteMap_it it(m_pClanInvites->find(pClient->GetAccountID()));
	if (it == m_pClanInvites->end())
		return;

	CBufferDynamic buf;
	buf << CHAT_CMD_CLAN_ADD_FAIL_UNKNOWN << pClient->GetNameUTF8() << byte(0);
	
	pClient->Send(buf);
	
	CClient *pTarget(GetClientFromAccountID(it->second.uiOriginID));
	if (pTarget != NULL)
		pTarget->Send(buf);

	m_pClanInvites->erase(it);
}


/*====================
  CClientManager::CreateClanFailed
  ====================*/
void	CClientManager::CreateClanFailed(uint uiAccountID)
{
	PROFILE("CClientManager::CreateClanFailed");

	CClient *pClient(GetClientFromAccountID(uiAccountID));

	if (pClient == NULL)
		return;

	ClanCreateMap_it it(m_pClanCreates->find(uiAccountID));
	if (it == m_pClanCreates->end())
		return;

	CBufferDynamic buf;
	buf << CHAT_CMD_CLAN_CREATE_FAIL_UNKNOWN;
	pClient->Send(buf);

	m_pClanCreates->erase(it);
}


/*====================
  CClientManager::CreateClanFailedClanName
  ====================*/
void	CClientManager::CreateClanFailedClanName(uint uiAccountID)
{
	PROFILE("CClientManager::CreateClanFailedClanName");

	CClient *pClient(GetClientFromAccountID(uiAccountID));

	if (pClient == NULL)
		return;

	ClanCreateMap_it it(m_pClanCreates->find(uiAccountID));
	if (it == m_pClanCreates->end())
		return;

	CBufferDynamic buf;
	buf << CHAT_CMD_CLAN_CREATE_FAIL_NAME;
	pClient->Send(buf);

	m_pClanCreates->erase(it);
}


/*====================
  CClientManager::CreateClanFailedTag
  ====================*/
void	CClientManager::CreateClanFailedTag(uint uiAccountID)
{
	PROFILE("CClientManager::CreateClanFailedTag");

	CClient *pClient(GetClientFromAccountID(uiAccountID));

	if (pClient == NULL)
		return;

	ClanCreateMap_it it(m_pClanCreates->find(uiAccountID));
	if (it == m_pClanCreates->end())
		return;

	CBufferDynamic buf;
	buf << CHAT_CMD_CLAN_CREATE_FAIL_TAG;
	pClient->Send(buf);

	m_pClanCreates->erase(it);
}


/*====================
  CClientManager::CreateClanSuccess
  ====================*/
void	CClientManager::CreateClanSuccess(uint uiAccountID, uint uiClanID)
{
	PROFILE("CClientManager::CreateClanSuccess");

	CClient *pClient(GetClientFromAccountID(uiAccountID));
	if (pClient == NULL)
		return;

	ClanCreateMap_it it(m_pClanCreates->find(uiAccountID));
	if (it == m_pClanCreates->end())
		return;

	CClan *pClan(m_pClanManager->GetClan(uiClanID));
	if (pClan == NULL)
	{
		pClan = m_pClanManager->AddClan(uiClanID);
		if (pClan == NULL)
			return;

		pClan->SetName(it->second.sClan);
		pClan->SetTag(it->second.sTag);
	}

	pClient->SetClan(pClan);
	pClient->RemoveFlags(CLIENT_IS_OFFICER | CLIENT_IS_CLAN_LEADER);
	pClient->AddFlags(CLIENT_IS_CLAN_LEADER);

	CChannel *pChannel(m_pChannelManager->CreateClanChannel(pClan));
	if (pChannel != NULL)
		pChannel->AddClient(pClient, true);

	CBufferDynamic bufSend;

	bufSend << CHAT_CMD_CLAN_CREATE_COMPLETE << CHAT_CMD_NEW_CLAN_MEMBER << uiAccountID << uiClanID << pClan->GetNameUTF8() << byte(0) << pClan->GetTagUTF8() << byte(0);
	pClient->Send(bufSend);
	bufSend.Clear();

	CBufferDynamic buf;
	buf << CHAT_CMD_UPDATE_STATUS << uiAccountID << pClient->GetStatusByte() << pClient->GetFlags() << uiClanID << pClan->GetNameUTF8() << byte(0);

	if (pClient->GetStatus() > CLIENT_STATUS_CONNECTED)
		buf << pClient->GetServerAddressUTF8() << byte(0);

	if (pClient->GetStatus() == CLIENT_STATUS_IN_GAME)
		buf << pClient->GetServerNameUTF8() << byte(0) << pClient->GetMatchID();

	for (int i(0); i < 4; ++i)
	{
		uint uiTargetAccountID(it->second.uiTarget[i]);

		CClient *pTarget(GetClientFromAccountID(uiTargetAccountID));
		if (pTarget == NULL)
			continue;

		pTarget->RemoveFlags(CLIENT_IS_OFFICER | CLIENT_IS_CLAN_LEADER);
		pTarget->AddFlags(CLIENT_IS_OFFICER);
		pTarget->SetClan(pClan);

		if (pChannel != NULL)
			pChannel->AddClient(pTarget, true);

		bufSend << CHAT_CMD_NEW_CLAN_MEMBER << uiTargetAccountID << uiClanID << pClan->GetNameUTF8() << byte(0) << pClan->GetTagUTF8() << byte(0);
		pTarget->Send(bufSend);
		bufSend.Clear();

		buf << CHAT_CMD_UPDATE_STATUS << uiTargetAccountID << pTarget->GetStatusByte() << pTarget->GetFlags() << uiClanID << pClan->GetNameUTF8() << byte(0);

		if (pTarget->GetStatus() > CLIENT_STATUS_CONNECTED)
			buf << pTarget->GetServerAddressUTF8() << byte(0);

		if (pTarget->GetStatus() == CLIENT_STATUS_IN_GAME)
			buf << pTarget->GetServerNameUTF8() << byte(0) << pTarget->GetMatchID();
	}

	pClan->Broadcast(buf);

	buf.Clear();
	buf << CHAT_CMD_NEW_CLAN_MEMBER << uiAccountID;

	for (int i(0); i < 4; ++i)
	{
		CClient *pTarget(GetClientFromAccountID(it->second.uiTarget[i]));
		if (pTarget == NULL)
			continue;

		pTarget->Send(buf);
	}

	for (int i(0); i < 4; ++i)
	{
		buf.Clear();
		buf << CHAT_CMD_NEW_CLAN_MEMBER << it->second.uiTarget[i];
		pClient->Send(buf);

		for (int z(0); z < 4; ++z)
		{
			if (z == i)
				continue;

			CClient *pTarget(GetClientFromAccountID(it->second.uiTarget[z]));
			if (pTarget == NULL)
				continue;

			pTarget->Send(buf);
		}
	}

	bufSend.Clear();
	bufSend << CHAT_CMD_NAME_CHANGE << pClient->GetAccountID() << pClient->GetNameUTF8() << byte(0);
	pClient->SendToPeers(bufSend);
	
	for (uint uiTarget(0); uiTarget < 4; ++uiTarget)
	{
		CClient *pTarget(GetClientFromAccountID(it->second.uiTarget[uiTarget]));
		if (pTarget == NULL)
			continue;

		bufSend.Clear();
		bufSend << CHAT_CMD_NAME_CHANGE << pClient->GetAccountID() << pClient->GetNameUTF8() << byte(0);
		pClient->SendToPeers(bufSend);
	}

	m_pClanCreates->erase(it);
}


/*====================
  CClientManager::RequestBuddyAddSuccess
  ====================*/
void	CClientManager::RequestBuddyAddSuccess(const uint uiAccountID, const wstring sAccountNickName, const uint uiAccountNotifyID, const wstring sAccountNotification, const uint uiBuddyAccountID, const wstring sBuddyNickName, const uint uiBuddyNotifyID, const wstring sBuddyNotification)
{
	PROFILE("CClientManager::RequestBuddyAddSuccess");

	CClient *pClient(GetClientFromAccountID(uiAccountID));
	if (pClient == NULL)
		return;
	
	CBufferDynamic bufSend;
	
	// send byte(1) for the adder, byte(2) for the added
	bufSend << CHAT_CMD_REQUEST_BUDDY_ADD_RESPONSE << byte(1) << uiAccountNotifyID << WStringToUTF8(sBuddyNickName) << byte(0);
	pClient->Send(bufSend);
	bufSend.Clear();

	CClient *pTarget(GetClientFromAccountID(uiBuddyAccountID));
	if (pTarget == NULL)
		return;	
	
	bufSend << CHAT_CMD_REQUEST_BUDDY_ADD_RESPONSE << byte(2) << uiBuddyNotifyID << WStringToUTF8(sAccountNickName) << byte(0);
	pTarget->Send(bufSend);
	bufSend.Clear();	
}


/*====================
  CClientManager::RequestBuddyAddFail
  ====================*/
void	CClientManager::RequestBuddyAddFail(const uint uiType, const uint uiAccountID, const uint uiAccountNotifyID, const wstring sBuddyNickName)
{
	PROFILE("CClientManager::RequestBuddyAddFail");

	CClient *pClient(GetClientFromAccountID(uiAccountID));
	if (pClient == NULL)
		return;
	
	CBufferDynamic bufSend;
	
	if (uiType == 0)
	{		
		// send byte(0) for a general failure
		bufSend << CHAT_CMD_REQUEST_BUDDY_ADD_RESPONSE << byte(0) << uiAccountNotifyID << WStringToUTF8(sBuddyNickName) << byte(0);
		pClient->Send(bufSend);
	}
	else if (uiType == 3)
	{
		// send byte(3) for a duplicate buddy request
		bufSend << CHAT_CMD_REQUEST_BUDDY_ADD_RESPONSE << byte(3) << 0 << WStringToUTF8(sBuddyNickName) << byte(0);
		pClient->Send(bufSend);	
	}	
	else if (uiType == 4)
	{
		// send byte(4) if they were ignored by their requested buddy
		bufSend << CHAT_CMD_REQUEST_BUDDY_ADD_RESPONSE << byte(4) << 0 << WStringToUTF8(sBuddyNickName) << byte(0);
		pClient->Send(bufSend);	
	}	
}


/*====================
  CClientManager::RequestBuddyApproveSuccess
  ====================*/
void	CClientManager::RequestBuddyApproveSuccess(const uint uiAccountID, const wstring sAccountNickName, const uint uiAccountNotifyID, const wstring sAccountNotification, const uint uiBuddyAccountID, const wstring sBuddyNickName, const uint uiBuddyNotifyID, const wstring sBuddyNotification)
{
	PROFILE("CClientManager::RequestBuddyApproveSuccess");

	CClient *pApprover(GetClientFromAccountID(uiAccountID));
	if (pApprover == NULL)
		return;
	
	CBufferDynamic bufSend;
	
	// send byte(1) for the approver, byte(2) for the approved
	bufSend << CHAT_CMD_REQUEST_BUDDY_APPROVE_RESPONSE << byte(1) << uiBuddyAccountID << uiAccountNotifyID << WStringToUTF8(sBuddyNickName) << byte(0);
	pApprover->Send(bufSend);
	bufSend.Clear();

	CClient *pApproved(GetClientFromAccountID(uiBuddyAccountID));
	if (pApproved == NULL)
		return;	
	
	bufSend << CHAT_CMD_REQUEST_BUDDY_APPROVE_RESPONSE << byte(2) << uiAccountID << uiBuddyNotifyID << WStringToUTF8(sAccountNickName) << byte(0);
	pApproved->Send(bufSend);
	bufSend.Clear();
	
		
	// Add the new buddy to the approver's buddy list
	pApprover->AddBuddy(uiBuddyAccountID);

	// Update the approver's user on the new buddies status (online/offline)
	bufSend << CHAT_CMD_UPDATE_STATUS << uiBuddyAccountID << pApproved->GetStatusByte() << pApproved->GetFlags() << pApproved->GetClanID() << pApproved->GetClanNameUTF8() << byte(0);

	if (pApproved->GetStatus() > CLIENT_STATUS_CONNECTED)
		bufSend << pApproved->GetServerAddressUTF8() << byte(0);

	if (pApproved->GetStatus() == CLIENT_STATUS_IN_GAME)
		bufSend << pApproved->GetServerNameUTF8() << byte(0) << pApproved->GetMatchID();

	pApprover->Send(bufSend);
	bufSend.Clear();
	
		
	// Add the new buddy to the approved's buddy list
	pApproved->AddBuddy(uiAccountID);

	// Update the approved's user on the new buddies status (online/offline)
	bufSend << CHAT_CMD_UPDATE_STATUS << uiAccountID << pApprover->GetStatusByte() << pApprover->GetFlags() << pApprover->GetClanID() << pApprover->GetClanNameUTF8() << byte(0);

	if (pApprover->GetStatus() > CLIENT_STATUS_CONNECTED)
		bufSend << pApprover->GetServerAddressUTF8() << byte(0);

	if (pApprover->GetStatus() == CLIENT_STATUS_IN_GAME)
		bufSend << pApprover->GetServerNameUTF8() << byte(0) << pApprover->GetMatchID();

	pApproved->Send(bufSend);
}


/*====================
  CClientManager::RequestBuddyApproveFail
  ====================*/
void	CClientManager::RequestBuddyApproveFail(const uint uiAccountID, const wstring sAccountNickName, const uint uiAccountNotifyID, const uint uiBuddyAccountID, const wstring sBuddyNickName, const uint uiBuddyNotifyID)
{
	PROFILE("CClientManager::RequestBuddyApproveFail");

	CClient *pClient(GetClientFromAccountID(uiAccountID));
	if (pClient == NULL)
		return;
	
	CBufferDynamic bufSend;
	
	// send byte(0) for the approver since there was a failure
	bufSend << CHAT_CMD_REQUEST_BUDDY_APPROVE_RESPONSE << byte(0) << uiAccountNotifyID << WStringToUTF8(sBuddyNickName) << byte(0);
	pClient->Send(bufSend);
}


/*====================
  CClientManager::Broadcast
  ====================*/
void	CClientManager::Broadcast(const IBuffer &buffer, byte yFlags)
{
	// FIXME: Probably best to stagger this rather than send it all out at once
	for (uint uiIndex(0); uiIndex < m_pActiveClients->size(); ++uiIndex)
	{
		CClient *pClient(m_pActiveClients->at(uiIndex));
		if (pClient != NULL && !pClient->HasAnyFlags(yFlags))
			pClient->Send(buffer);
	}
}


/*====================
  CClientManager::AddVirtualClient
  ====================*/
CClient*	CClientManager::AddVirtualClient()
{
	CClient *pNewClient(AddClient(1000000000 + m_uiVirtualClientCount, L"VClient" + XtoW(m_uiVirtualClientCount), NULL));
	if (pNewClient == NULL)
		return NULL;

	pNewClient->SetRating(MATCH_POOL_SOLO, 1400.0f + rand() % 200);
	pNewClient->SetMatchCount(MATCH_POOL_SOLO, rand() % 100);
	pNewClient->SetRating(MATCH_POOL_TEAM, 1400.0f + rand() % 200);
	pNewClient->SetMatchCount(MATCH_POOL_TEAM, rand() % 100);
	pNewClient->AddFlags(CLIENT_VIRTUAL);
	int iTotal(pNewClient->GetMatchCount(MATCH_POOL_SOLO) + pNewClient->GetMatchCount(MATCH_POOL_TEAM) + rand() % 100 + 1);
	pNewClient->SetTotalMatches(iTotal);
	pNewClient->SetDisconnectCount((rand() % iTotal) * 0.1f);

	++m_uiVirtualClientCount;

	return pNewClient;
}


/*====================
  CClientManager::SpawnNewHeartbeatRequest
  ====================*/
void	CClientManager::SpawnNewHeartbeatRequest()
{
	m_pHeartbeat = m_pHTTPManager->SpawnRequest();
	m_pHeartbeat->SetTargetURL(net_masterServerAddress.Get() + SERVER_REQUEST_SCRIPT);
}


/*====================
  CClientManager::InvalidateAllClientRequests
  ====================*/
void	CClientManager::InvalidateAllClientRequests()
{
	for (ClientVector_it itClient(m_pActiveClients->begin()), itEnd(m_pActiveClients->end()); itClient != itEnd; ++itClient)
	{
		CClient *pClient(*itClient);
		if (pClient == NULL)
			continue;

		pClient->InvalidateAllRequests();
	}
}


/*====================
  CClientManager::SendConnectionCountsUpdate
  ====================*/
void	CClientManager::SendConnectionCountsUpdate(uint uiAddress, SPlayerCount &count)
{
	if (!net_sendPlayerCounts)
		return;

	in_addr addr;
	addr.S_un.S_addr = uiAddress;

	CHTTPRequest *pRequest(m_pHTTPManager->SpawnRequest());
	pRequest->SetTargetURL(net_masterServerAddress.Get() + CLIENT_REQUEST_SCRIPT);
	pRequest->AddVariable(L"f", L"ip_capacity");
	pRequest->AddVariable(L"ip", inet_ntoa(addr));
	pRequest->AddVariable(L"paid", count.uiPaid);
	pRequest->AddVariable(L"free", count.uiOther);
	pRequest->SendPostRequest();
	m_pHTTPRequests->push_back(pRequest);
}


/*====================
  CClientManager::IncrementConnectionCounts
  ====================*/
void	CClientManager::IncrementConnectionCounts(uint uiAddr, bool bIsPaid)
{
	PlayerCountMap_it itFind(m_pPlayerCounts->find(uiAddr));
	SPlayerCount &count(itFind == m_pPlayerCounts->end() ? (*m_pPlayerCounts)[uiAddr] : itFind->second);
	if (bIsPaid)
		++count.uiPaid;
	else
		++count.uiOther;

	SendConnectionCountsUpdate(uiAddr, count);
}


/*====================
  CClientManager::DecrementConnectionCounts
  ====================*/
void	CClientManager::DecrementConnectionCounts(uint uiAddr, bool bIsPaid)
{
	PlayerCountMap_it itFind(m_pPlayerCounts->find(uiAddr));
	SPlayerCount &count(itFind == m_pPlayerCounts->end() ? (*m_pPlayerCounts)[uiAddr] : itFind->second);
	if (bIsPaid)
		--count.uiPaid;
	else
		--count.uiOther;

	SendConnectionCountsUpdate(uiAddr, count);
}


/*====================
  CClientManager::PrintUsage
  ====================*/
void	CClientManager::PrintUsage()
{
	uint uiActiveCount(0);
	uint aStatusCounts[NUM_CLIENT_STATUSES];
	memset(aStatusCounts, 0, sizeof(uint) * NUM_CLIENT_STATUSES);

	for (uint uiClient(0); uiClient < m_pActiveClients->size(); ++uiClient)
	{
		if (m_pActiveClients->at(uiClient) == NULL)
			continue;

		++uiActiveCount;
		++aStatusCounts[m_pActiveClients->at(uiClient)->GetStatus()];
	}

	m_pConsole->Admin()
		<< newl
		<< L"Clients          " << newl
		<< L"---------------- " << newl
		<< L"Active:          " << XtoA(uiActiveCount, FMT_DELIMIT, 7) << " / " << XtoA(INT_SIZE(m_pActiveClients->size()), FMT_DELIMIT) << newl
		<< L"---------------- " << newl
		<< L"Offline:         " << XtoA(aStatusCounts[CLIENT_STATUS_DISCONNECTED], FMT_DELIMIT, 7) << newl
		<< L"Chatting:        " << XtoA(aStatusCounts[CLIENT_STATUS_CONNECTED], FMT_DELIMIT, 7) << newl
		<< L"Loading:         " << XtoA(aStatusCounts[CLIENT_STATUS_JOINING_GAME], FMT_DELIMIT, 7) << newl
		<< L"In game:         " << XtoA(aStatusCounts[CLIENT_STATUS_IN_GAME], FMT_DELIMIT, 7) << newl
		<< L"---------------- " << newl
		<< L"Average Session: " << GetTimeStringW(SecToMs(m_llTotalSessionLength / float(m_llTotalSessions)), -1, 0, TIME_STRING_ALPHA_SEPERATOR)
			<< L" (" << GetTimeStringW(SecToMs(m_llTotalLongSessionLength / float(m_llTotalLongSessions)), -1, 0, TIME_STRING_ALPHA_SEPERATOR) << L")" << newl
		<< L"Total online:    " << XtoA(m_uiTotalOnline, FMT_DELIMIT, 7) << newl
		<< L"Total in game:   " << XtoA(m_uiTotalInGame, FMT_DELIMIT, 7) << newl;
}


/*====================
  CClientManager::PrintPlayerCounts
  ====================*/
void	CClientManager::PrintPlayerCounts()
{
	for (PlayerCountMap_it itAddr(m_pPlayerCounts->begin()), itEnd(m_pPlayerCounts->end()); itAddr != itEnd; ++itAddr)
	{
		in_addr addr;
		addr.S_un.S_addr = itAddr->first;
		m_pConsole->Admin() << inet_ntoa(addr) << L" " << itAddr->second.uiPaid << L" " << itAddr->second.uiOther << newl;
	}
}


/*====================
  CClientManager::PrintPlayerCountTotals
  ====================*/
void	CClientManager::PrintPlayerCountTotals()
{
	uint uiTotalPaid(0);
	uint uiTotalOther(0);
	for (PlayerCountMap_it itAddr(m_pPlayerCounts->begin()), itEnd(m_pPlayerCounts->end()); itAddr != itEnd; ++itAddr)
	{
		uiTotalPaid += itAddr->second.uiPaid;
		uiTotalOther += itAddr->second.uiOther;
	}

	m_pConsole->Admin() << L"Total paid: " << uiTotalPaid << newl;
	m_pConsole->Admin() << L"Total other: " << uiTotalOther << newl;
	m_pConsole->Admin() << L"Total: " << (uiTotalPaid + uiTotalOther) << L" (" << m_uiTotalOnline << L")" << newl;
}


/*====================
  CClientManager::AddClient
  ====================*/
CClient*	CClientManager::AddClient(uint uiAccountID, const wstring &sName, CSocket *pSocket)
{
	PROFILE("CClientManager::AddClient");

	// Kill existing client using this account ID, if any
	CClient *pOldClient(GetClientFromAccountID(uiAccountID));
	if (pOldClient != NULL)
		RemoveClient(pOldClient);

	IncrementOnlineCount();

	// Allocate new client
	CClient *pClient(HEAP_NEW(m_pMemManager, m_pHeap) CClient(m_pCore, m_pHeap, pSocket));

	pClient->SetAccountID(uiAccountID);
	pClient->SetName(sName);
	pClient->SetSessionStart(m_pCore->GetSeconds());

	// Assign client id
	if (m_vAvailableActiveIDs.empty())
	{
		pClient->SetClientID(INT_SIZE(m_pActiveClients->size()));
		m_pActiveClients->push_back(pClient);
	}
	else
	{
		pClient->SetClientID(m_vAvailableActiveIDs.back());
		m_pActiveClients->at(pClient->GetClientID()) = pClient;
		m_vAvailableActiveIDs.pop_back();
	}

	// Add to account ID map
	AccountClientMap_it itAccountID(m_pAccountToClientID->find(uiAccountID));
	if (itAccountID == m_pAccountToClientID->end())
		m_pAccountToClientID->insert(AccountClientMap_pair(pClient->GetAccountID(), pClient->GetClientID()));
	else
		itAccountID->second = pClient->GetClientID();

	// Add to name map
	NameClientMap_it itName(m_pNameToClientID->find(pClient->GetNameLower()));
	if (itName == m_pNameToClientID->end())
		m_pNameToClientID->insert(NameClientMap_pair(pClient->GetNameLower(), pClient->GetClientID()));
	else
		itName->second = pClient->GetClientID();

	// Send "looking for clan" updates if they are the clan leader
/*	if (pClient->yFlags & CLIENT_IS_CLAN_LEADER)
	{
		svector vLFC;

		for (ClientMap_it it(m_mapClients.begin()); it != m_mapClients.end(); it++)
		{
			if (!(it->second->yFlags & CLIENT_LOOKING_FOR_CLAN))
				continue;

			vLFC.push_back(it->second->sName);
		}

		if (vLFC.size() > 0)
		{
			bufSend << CHAT_CMD_MULT_LOOKING_FOR_CLAN << INT_SIZE(vLFC.size());

			for (svector_it it(vLFC.begin()); it != vLFC.end(); it++)
				bufSend << TStringToUTF8(*it) << byte(0);

			pClient->Send(bufSend);
		}
	}*/

	return pClient;
}


/*====================
  CClientManager::GetClientFromAccountID
  ====================*/
CClient*	CClientManager::GetClientFromAccountID(uint uiAccountID)
{
	PROFILE("CClientManager::GetClientFromAccountID");

	AccountClientMap_it it(m_pAccountToClientID->find(uiAccountID));
	if (it == m_pAccountToClientID->end())
		return NULL;

	return m_pActiveClients->at(it->second);
}


/*====================
  CClientManager::GetClientFromName
  ====================*/
CClient*	CClientManager::GetClientFromName(const wstring &sName)
{
	PROFILE("CClientManager::GetClientFromName");

	// FIXME: Clan tags should not be a part of the name!
	// Remove clan tag
	size_t zPos(sName.find(L"]"));
	if (zPos != wstring::npos)
	{
		NameClientMap_it it(m_pNameToClientID->find(sName.substr(zPos + 1)));
		if (it == m_pNameToClientID->end())
			return NULL;

		return m_pActiveClients->at(it->second);
	}

	NameClientMap_it it(m_pNameToClientID->find(sName));
	if (it == m_pNameToClientID->end())
		return NULL;

	return m_pActiveClients->at(it->second);
}


/*====================
  CClientManager::RemoveClient
  ====================*/
void	CClientManager::RemoveClient(CClient *pClient)
{
	PROFILE("CClientManager::RemoveClient");

	if (pClient == NULL)
		return;

	m_pConsole->Std() << _T("Client '") << pClient->GetName() << _T("' (ID: ") << XtoA(pClient->GetAccountID()) << _T(") has been disconnected.") << newl;

	if (pClient->HasAllFlags(CLIENT_LOOKING_FOR_CLAN))
	{
		CBufferDynamic bufSend;
		bufSend << CHAT_CMD_NOT_LOOKING_FOR_CLAN << pClient->GetNameUTF8() << byte(0);

		// FIXME: Big ugly loop
		for (uint uiClientID(0); uiClientID < m_pActiveClients->size(); ++uiClientID)
		{
			CClient *pTargetClient(m_pActiveClients->at(uiClientID));
			if (pTargetClient == NULL || !(pTargetClient->HasAllFlags(CLIENT_IS_CLAN_LEADER)))
				continue;

			pTargetClient->Send(bufSend);
		}
	}

	// Remove from lists and make id available
	m_pActiveClients->at(pClient->GetClientID()) = NULL;
	m_vAvailableActiveIDs.push_back(pClient->GetClientID());

	m_pAccountToClientID->erase(pClient->GetAccountID());
	m_pNameToClientID->erase(pClient->GetNameLower());

	// Update session values
	uint uiSessionLength(m_pCore->GetSeconds() - pClient->GetSessionStart());
	++m_llTotalSessions;
	m_llTotalSessionLength += uiSessionLength;
	if (uiSessionLength > client_longSessionThreshold)
	{
		++m_llTotalLongSessions;
		m_llTotalLongSessionLength += uiSessionLength;
	}
	
	delete pClient;
}


/*====================
  CClientManager::HandleClanInvite
  ====================*/
bool	CClientManager::HandleClanInvite(CClient *pClient, CPacket &pkt)
{
	PROFILE("CClientManager::HandleClanInvite");

	const wstring sUser(pkt.ReadWString());

	if(!pClient->IncrementRequestCounter())
		return true;
		
	CBufferDynamic buf;
		
	if (sUser.empty())
	{
		buf << CHAT_CMD_CLAN_ADD_FAIL_ONLINE;
		pClient->Send(buf);
		return true;
	}

	CClient *pUser(GetClientFromName(LowerString(sUser)));
		
	// The specified user is either not existent, is disconnected, or is invisible, respond saying they are offline
	if (pUser == NULL || pUser->GetStatus() < CLIENT_STATUS_CONNECTED || pUser->GetChatModeType() == CHAT_MODE_INVISIBLE)
	{
		buf << CHAT_CMD_CLAN_ADD_FAIL_ONLINE;
		pClient->Send(buf);
		return true;
	}
	
	// The user was found, check if they are in a clan, if so reject this and respond saying they are already in a clan
	if (pUser->HasClan())
	{
		buf << CHAT_CMD_CLAN_ADD_FAIL_CLAN;
		pClient->Send(buf);
		return true;	
	}	
	
	if (pUser->GetChatModeType() == CHAT_MODE_DND)
	{
		// Notify the sender that user is DND and do not pass pass the message to the receiver
		buf << CHAT_CMD_CHAT_MODE_AUTO_RESPONSE << CHAT_MODE_DND << pUser->GetNameUTF8() << byte(0) << WStringToUTF8(pUser->GetChatModeTypeReason()) << byte(0);
		pClient->Send(buf);
		return true;
	}

	CClan *pClan(pClient->GetClan());
	
	// We couldn't gather the clan information associated with this user, they probably shouldn't be initiating this request
	if (pClan == NULL)
	{
		buf << CHAT_CMD_CLAN_ADD_FAIL_UNKNOWN;
		pClient->Send(buf);
		return true;
	}

	// We found the clan, but they either aren't in it or they don't have the privileges to invite someone new to it
	if (!pClient->HasClan() || !(pClient->HasAnyFlags(CLIENT_IS_OFFICER | CLIENT_IS_CLAN_LEADER)))
	{
		buf << CHAT_CMD_CLAN_ADD_FAIL_PERMS;
		pClient->Send(buf);
		return true;
	}

	// This user has already been (recently) invited to a clan, respond with error message
	ClanInviteMap_it findit(m_pClanInvites->find(pUser->GetAccountID()));
	if (findit != m_pClanInvites->end())
	{
		buf << CHAT_CMD_CLAN_ADD_FAIL_INVITED;
		pClient->Send(buf);
		return true;
	}

	for (ClanCreateMap_it itClan(m_pClanCreates->begin()), itEnd(m_pClanCreates->end()); itClan != itEnd; ++itClan)
	{
		if (itClan->second.uiFounderAccountID == pUser->GetAccountID())
		{
			buf << CHAT_CMD_CLAN_ADD_FAIL_INVITED;
			pClient->Send(buf);
			return true;
		}

		for (int i(0); i < 4; ++i)
		{
			if (itClan->second.uiTarget[i] == pUser->GetAccountID())
			{
				buf << CHAT_CMD_CLAN_ADD_FAIL_INVITED;
				pClient->Send(buf);
				return true;
			}
		}
	}

	SClanInvite &invite(m_pClanInvites->insert(ClanInviteMap_pair(pUser->GetAccountID(), SClanInvite())).first->second);
	invite.uiRecvTime = m_pCore->GetMilliseconds();
	invite.uiTargetID = pUser->GetAccountID();
	invite.uiOriginID = pClient->GetAccountID();
	invite.uiClanID = pClan->GetClanID();
	invite.sClan = pClan->GetName();
	invite.sTag = pClan->GetTag();
	invite.bCreateClan = false;

	buf << CHAT_CMD_CLAN_ADD_MEMBER << pClient->GetNameUTF8() << byte(0) << pClan->GetNameUTF8() << byte(0);
	pUser->Send(buf);

	return true;
}


/*====================
  CClientManager::HandleClanInviteRejected
  ====================*/
bool	CClientManager::HandleClanInviteRejected(CClient *pClient, CPacket &pkt)
{
	PROFILE("CClientManager::HandleClanInviteRejected");

	ClanInviteMap_it findit(m_pClanInvites->find(pClient->GetAccountID()));

	if (findit == m_pClanInvites->end())
	{
		for (ClanCreateMap_it it(m_pClanCreates->begin()); it != m_pClanCreates->end(); it++)
		{
			bool bFound(false);

			for (int i(0); i < 4; i++)
			{
				if (it->second.uiTarget[i] == pClient->GetAccountID())
				{
					bFound = true;
					break;
				}
			}

			if (bFound)
			{
				CBufferDynamic buf;
				buf << CHAT_CMD_CLAN_CREATE_REJECT << pClient->GetNameUTF8() << byte(0);
				CClient *pTarget(GetClientFromAccountID(it->second.uiFounderAccountID));
				if (pTarget != NULL)
					pTarget->Send(buf);

				m_pClanCreates->erase(it);
				break;
			}
		}

		return true;
	}

	CBufferDynamic buf;
	buf << CHAT_CMD_CLAN_ADD_REJECTED << pClient->GetNameUTF8() << byte(0);
	CClient *pTarget(GetClientFromAccountID(findit->second.uiOriginID));
	if (pTarget != NULL)
		pTarget->Send(buf);

	m_pClanInvites->erase(findit);

	return true;
}


/*====================
  CClientManager::HandleClanInviteAccepted
  ====================*/
bool	CClientManager::HandleClanInviteAccepted(CClient *pClient, CPacket &pkt)
{
	PROFILE("CClientManager::HandleClanInviteAccepted");

	ClanInviteMap_it findit(m_pClanInvites->find(pClient->GetAccountID()));

	if (findit == m_pClanInvites->end())
	{
		for (ClanCreateMap_it it(m_pClanCreates->begin()); it != m_pClanCreates->end(); ++it)
		{
			bool bAllAccepted(true);
			bool bFound(false);

			for (int i(0); i < 4; i++)
			{
				if (it->second.uiTarget[i] == pClient->GetAccountID())
				{
					it->second.bAccepted[i] = true;
					bFound = true;
				}

				if (!it->second.bAccepted[i])
					bAllAccepted = false;
			}

			if (bFound)
			{
				if (bAllAccepted)
				{
					CClient *pTarget(GetClientFromAccountID(it->second.uiFounderAccountID));
					if (pTarget != NULL)
						pTarget->CreateClan(it->second);
				}
				else
				{
					CBufferDynamic buf;
					buf << CHAT_CMD_CLAN_CREATE_ACCEPT << pClient->GetNameUTF8() << byte(0);
					CClient *pTarget(GetClientFromAccountID(it->second.uiFounderAccountID));
					if (pTarget != NULL)
						pTarget->Send(buf);
				}

				break;
			}
		}

		return true;
	}

	CClient *pOrigin(GetClientFromAccountID(findit->second.uiOriginID));

	if (pOrigin == NULL)
	{
		if (findit != m_pClanInvites->end())
			m_pClanInvites->erase(findit);

		return true;
	}

	pClient->SendClanInvite(pOrigin);
	return true;
}


/*====================
  CClientManager::HandleCreateClan
  ====================*/
bool	CClientManager::HandleCreateClan(CClient *pClient, CPacket &pkt)
{
	PROFILE("CClientManager::HandleCreateClan");

	wstring sName(pkt.ReadWString());
	wstring sTag(pkt.ReadWString());
	
	if(!pClient->IncrementRequestCounter())
		return true;	

	// Lookup requested members
	wstring asMemberNames[NUM_REQUIRED_CLAN_MEMBERS];
	CClient *apMembers[NUM_REQUIRED_CLAN_MEMBERS];
	for (uint ui(0); ui < NUM_REQUIRED_CLAN_MEMBERS; ++ui)
	{
		asMemberNames[ui] = pkt.ReadWString();
		apMembers[ui] = GetClientFromName(LowerString(asMemberNames[ui]));
	}

	if (pClient == NULL)
		return true;

	// Validate requested members
	for (uint ui(0); ui < NUM_REQUIRED_CLAN_MEMBERS; ++ui)
	{
		if (apMembers[ui] == NULL || apMembers[ui]->GetAccountID() == INVALID_ACCOUNT_ID)
		{
			CBufferDynamic buf;
			buf << CHAT_CMD_CLAN_CREATE_FAIL_FIND << TStringToUTF8(asMemberNames[ui]) << byte(0);
			pClient->Send(buf);
			return true;
		}

		if (pClient->HasClan())
		{
			CBufferDynamic buf;
			buf << CHAT_CMD_CLAN_CREATE_FAIL_CLAN << pClient->GetNameUTF8() << byte(0);
			pClient->Send(buf);
			return true;
		}
			
	}

	if (pClient->HasClan())
	{
		CBufferDynamic buf;
		buf << CHAT_CMD_CLAN_CREATE_FAIL_CLAN << pClient->GetNameUTF8() << byte(0);
		pClient->Send(buf);
		return true;
	}

	ClanInviteMap_it findit(m_pClanInvites->find(pClient->GetAccountID()));
	if (findit != m_pClanInvites->end())
	{
		CBufferDynamic buf;
		buf << CHAT_CMD_CLAN_CREATE_FAIL_INVITE << pClient->GetNameUTF8() << byte(0);
		pClient->Send(buf);

		return true;
	}

	if (sName.empty() || sTag.empty() || sTag.size() > 4)
	{
		CBufferDynamic buf;
		buf << CHAT_CMD_CLAN_CREATE_FAIL_PARAM;
		pClient->Send(buf);

		return true;
	}


	for (uint ui(0); ui < NUM_REQUIRED_CLAN_MEMBERS; ++ui)
	{
		ClanInviteMap_it itInvite(m_pClanInvites->find(apMembers[ui]->GetAccountID()));
		if (itInvite == m_pClanInvites->end())
			continue;

		CBufferDynamic buf;
		buf << CHAT_CMD_CLAN_CREATE_FAIL_INVITE << WStringToUTF8(asMemberNames[ui]) << byte(0);
		pClient->Send(buf);
		return true;
	}

	for (ClanCreateMap_it createit(m_pClanCreates->begin()); createit != m_pClanCreates->end(); createit++)
	{
		if (createit->second.uiFounderAccountID == pClient->GetAccountID() ||
			createit->second.uiFounderAccountID == apMembers[0]->GetAccountID() ||
			createit->second.uiFounderAccountID == apMembers[1]->GetAccountID() ||
			createit->second.uiFounderAccountID == apMembers[2]->GetAccountID() ||
			createit->second.uiFounderAccountID == apMembers[3]->GetAccountID())
		{
			CClient *pTarget(GetClientFromAccountID(createit->second.uiFounderAccountID));

			if (pTarget != NULL)
			{
				CBufferDynamic buf;
				buf << CHAT_CMD_CLAN_CREATE_FAIL_INVITE << pTarget->GetNameUTF8() << byte(0);
				pClient->Send(buf);
			}

			return true;
		}

		for (uint ui(0); ui < NUM_REQUIRED_CLAN_MEMBERS; ++ui)
		{
			if (createit->second.uiTarget[ui] == pClient->GetAccountID() ||
				createit->second.uiTarget[ui] == apMembers[0]->GetAccountID() ||
				createit->second.uiTarget[ui] == apMembers[1]->GetAccountID() ||
				createit->second.uiTarget[ui] == apMembers[2]->GetAccountID() ||
				createit->second.uiTarget[ui] == apMembers[3]->GetAccountID())
			{
				CClient *pTarget(GetClientFromAccountID(createit->second.uiTarget[ui]));

				if (pTarget != NULL)
				{
					CBufferDynamic buf;
					buf << CHAT_CMD_CLAN_CREATE_FAIL_INVITE << pTarget->GetNameUTF8() << byte(0);
					pClient->Send(buf);
				}

				return true;
			}
		}
	}

	SClanCreate &create(m_pClanCreates->insert(ClanCreateMap_pair(pClient->GetAccountID(), SClanCreate())).first->second);
	create.uiFounderAccountID = pClient->GetAccountID();
	create.uiRecvTime = m_pCore->GetMilliseconds();
	create.sClan = sName;
	create.sTag = sTag;

	CBufferDynamic buffer;
	buffer << CHAT_CMD_CLAN_ADD_MEMBER << pClient->GetNameUTF8() << byte(0) << TStringToUTF8(sName) << byte(0);

	for (uint ui(0); ui < NUM_REQUIRED_CLAN_MEMBERS; ++ui)
	{
		create.uiTarget[ui] = apMembers[ui]->GetAccountID();
		create.bAccepted[ui] = false;
		apMembers[ui]->Send(buffer);
	}

	return true;
}

// (C)2009 S2 Games
// c_pendingconnection.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "chatserver_common.h"

#include "c_pendingconnection.h"
#include "c_peer.h"
#include "c_httprequest.h"
#include "c_phpdata.h"
#include "c_netmanager.h"
#include "c_peermanager.h"
#include "c_clanmanager.h"
#include "c_console.h"
#include "c_httpmanager.h"
#include "c_clientmanager.h"
#include "c_gameservermanager.h"
#include "c_gameserver.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
EXTERN_CVAR(string, net_masterServerAddress);
//=============================================================================

/*====================
  CPendingConnection::~CPendingConnection
  ====================*/
CPendingConnection::~CPendingConnection()
{
	delete m_pSocket;
	m_pHTTPManager->ReleaseRequest(m_pRequest);
}


/*====================
  CPendingConnection::CPendingConnection
  ====================*/
CPendingConnection::CPendingConnection(CSocket *pSocket, CCore *pCore) :
NULL_CORE_API,
m_pSocket(pSocket),
m_pRequest(NULL),
m_eState(PENDING_HANDSHAKE),
m_uiTimeStamp(INVALID_TIME),
m_uiAccountID(INVALID_ACCOUNT_ID)
{
	INIT_CORE_API(pCore);
	
	m_uiTimeStamp = m_pCore->GetSeconds();

	m_pConsole->Net() << m_pSocket->GetRecvAddress() << L" - CONNECT" << newl;
}


/*====================
  CPendingConnection::HandshakeFrame
  ====================*/
bool	CPendingConnection::HandshakeFrame()
{
	PROFILE("CPendingConnection::HandshakeFrame");

	// Try to receive a packet, return false only if there is a failure
	CPacket	pktRecv;
	int iRecvLength(m_pSocket->ReceivePacket(pktRecv));
	if (iRecvLength < 1)
		return iRecvLength == K2_SOCKET_EMPTY;

	// If a packet was received, check for a valid connection request
	ushort unCmd(pktRecv.ReadShort(NET_CHAT_INVALID));

	if (pktRecv.HasFaults())
		return false;

	switch (unCmd)
	{
	case PEER_CMD_HELLO:
		// Pass socket to peer
		m_pPeerManager->AddPeer(m_pSocket, pktRecv);
		m_pSocket = NULL;
		return false;

	case NET_CHAT_CL_CONNECT:
		return RequestClientAuth(pktRecv);

	case NET_CHAT_GS_CONNECT:
		return RequestServerAuth(pktRecv);

	case NET_CHAT_INVALID:
	default:
		return false;
	}

	return false;
}


/*====================
  CPendingConnection::ClientAuthFrame
  ====================*/
bool	CPendingConnection::ClientAuthFrame()
{
	PROFILE("CPendingConnection::ClientAuthFrame");

	if (m_pRequest->IsActive())
		return true;

	if (!m_pRequest->WasSuccessful())
	{
		m_pConsole->Net() << m_pSocket->GetRecvAddress() << L" - REQUEST FAILED" << newl;
		return false;
	}

	ProcessClientAuthResponse();
	return false;
}


/*====================
  CPendingConnection::ServerAuthFrame
  ====================*/
bool	CPendingConnection::ServerAuthFrame()
{
	PROFILE("CPendingConnection::ServerAuthFrame");

	ProcessServerAuthResponse();
	return false;
}


/*====================
  CPendingConnection::ProcessClientAuthResponse
  ====================*/
void	CPendingConnection::ProcessClientAuthResponse()
{
	CPHPData phpResponse(m_pRequest->GetResponse());

	wstring sName(phpResponse.GetString(L"nickname"));
	if (!phpResponse.IsValid() || !phpResponse.GetString(L"error").empty() || phpResponse.GetVar(L"error") != NULL || sName.empty())
	{
		m_pConsole->Net() << m_pSocket->GetRecvAddress() << L" - BAD RESPONSE: " << m_pRequest->GetResponse() << newl;
		m_pHTTPManager->ReleaseRequest(m_pRequest);
		m_pRequest = NULL;
		return;
	}

	m_pHTTPManager->ReleaseRequest(m_pRequest);
	m_pRequest = NULL;

	m_pConsole->Net() << m_pSocket->GetRecvAddress() << L" - GOOD RESPONSE" << newl;

	uint uiAccountID(phpResponse.GetInteger(L"account_id"));

	// Create a new client object
	CClient *pClient(m_pClientManager->AddClient(uiAccountID, sName, m_pSocket));
	
	// set the clients chat mode type to be 'available' or 'invisible', defaults to 0 (available) if nothing sent
	pClient->SetChatModeType(phpResponse.GetInteger(L"chat_mode_type"));

	// The socket is now the clients responsibility
	m_pSocket = NULL;

	const CPHPData *pSubResponse(phpResponse.GetVar(L"stats"));
	if (pSubResponse != NULL)
	{
		pSubResponse = pSubResponse->GetVar(0);
		if (pSubResponse != NULL)
		{
			pClient->SetRating(MATCH_POOL_SOLO, pSubResponse->GetFloat(L"rnk_amm_solo_rating"));
			pClient->SetRating(MATCH_POOL_TEAM, pSubResponse->GetFloat(L"rnk_amm_team_rating"));
			pClient->SetMatchCount(MATCH_POOL_SOLO, pSubResponse->GetInteger(L"rnk_amm_solo_count"));
			pClient->SetMatchCount(MATCH_POOL_TEAM, pSubResponse->GetInteger(L"rnk_amm_team_count"));
			pClient->SetTotalMatches(pSubResponse->GetInteger(L"acc_games_played") + pSubResponse->GetInteger(L"rnk_games_played"));
			pClient->SetDisconnectCount(pSubResponse->GetInteger(L"acc_discos") + pSubResponse->GetInteger(L"rnk_discos"));
		}
	}

	// Account type
	pSubResponse = phpResponse.GetVar(L"check_perms");
	if (pSubResponse != NULL)
	{
		uint uiAccountType(pSubResponse->GetInteger(L"account_type"));
		switch (uiAccountType)
		{
		default:
		case ACCOUNT_DISABLED:
		case ACCOUNT_SERVER:
			m_pClientManager->RemoveClient(pClient);
			return;

		case ACCOUNT_DEMO:
			pClient->SetAccountType(ACCOUNT_DEMO);
			pClient->SetPaidAccount(false);
			break;

		case ACCOUNT_REGULAR:
			pClient->SetAccountType(ACCOUNT_REGULAR);
			pClient->SetPaidAccount(true);
			break;

		case ACCOUNT_PREMIUM:
			pClient->SetAccountType(ACCOUNT_PREMIUM);
			pClient->AddFlags(CLIENT_IS_PREMIUM);
			pClient->SetPaidAccount(true);
			break;

		case ACCOUNT_STAFF:
			pClient->SetAccountType(ACCOUNT_STAFF);
			pClient->AddFlags(CLIENT_IS_STAFF);
			pClient->SetPaidAccount(true);
			break;

		case ACCOUNT_GM:
			pClient->SetAccountType(ACCOUNT_GM);
			pClient->SetPaidAccount(true);
			break;
		}
	}

	// Grab buddy list
	const CPHPData *pBuddyList(phpResponse.GetVar(L"buddy_list"));
	if (pBuddyList != NULL && pBuddyList->GetVar(0) != NULL)
	{
		pBuddyList = pBuddyList->GetVar(0);

		uint uiArrayNum(0);
		const CPHPData *pBuddy(pBuddyList->GetVar(uiArrayNum++));
		while (pBuddy != NULL)
		{
			pClient->AddBuddy(pBuddy->GetInteger(L"buddy_id"));
			pBuddy = pBuddyList->GetVar(uiArrayNum++);
		}
	}

	// Rank
	wstring sClanRank(LowerString(phpResponse.GetString(L"rank")));
	if (sClanRank == L"officer")
		pClient->AddFlags(CLIENT_IS_OFFICER);
	else if (sClanRank == L"leader")
		pClient->AddFlags(CLIENT_IS_CLAN_LEADER);

	// Clan
	uint uiClanID(phpResponse.GetInteger(L"clan_id", INVALID_CLAN_ID));
	if (uiClanID != INVALID_CLAN_ID)
	{
		CClan *pClan(m_pClanManager->GetClan(uiClanID));
		if (pClan == NULL)
		{
			pClan = m_pClanManager->AddClan(uiClanID);
			if (pClan != NULL)
			{
				pClan->SetClanID(uiClanID);
				pClan->SetName(phpResponse.GetString(L"name"));
				pClan->SetTag(phpResponse.GetString(L"tag"));
			}
		}

		pClient->SetClan(pClan);
	}

	pClient->SetCookie(m_sCookie);
	pClient->SetToken(m_sToken);

	pClient->KeepAlive();
	pClient->SetOnline();
	m_pNetManager->IncrementSuccessfulConnectionCount();
}


/*====================
  CPendingConnection::ProcessServerAuthResponse
  ====================*/
void	CPendingConnection::ProcessServerAuthResponse()
{
	CGameServer *pGameServer(m_pGameServerManager->AddGameServer(0, m_pSocket));
	m_pSocket = NULL;

	pGameServer->SetOnline();
	m_pNetManager->DecrementConnectionCount();	// Only count clients
}


/*====================
  CPendingConnection::Frame
  ====================*/
bool	CPendingConnection::Frame()
{
	PROFILE("CPendingConnection::Frame");

	// Check for timeout
	if (m_pCore->GetSeconds() - m_uiTimeStamp >= 5)
	{
		m_pConsole->Net() << m_pSocket->GetRecvAddress() << L" - TIME OUT" << newl;
		return false;
	}

	switch (m_eState)
	{
	case PENDING_HANDSHAKE:		return HandshakeFrame();
	case PENDING_CLIENT_AUTH:	return ClientAuthFrame();
	case PENDING_SERVER_AUTH:	return ServerAuthFrame();
	}

	return false;
}


/*====================
  CPendingConnection::RequestClientAuth
  ====================*/
bool	CPendingConnection::RequestClientAuth(CPacket &pkt)
{
	PROFILE("CPendingConnection::RequestClientAuth");

	m_uiAccountID = pkt.ReadInt(INVALID_ACCOUNT_ID);
	m_sCookie = pkt.ReadString();
	m_sToken = pkt.ReadString();
	uint uiVersion(pkt.ReadInt(0));
	uint uiChatModeType(pkt.ReadInt(0));

	m_pConsole->Net() << m_pSocket->GetRecvAddress() << L" - CLIENT: " << m_uiAccountID << L" " << m_sCookie << L" v" << uiVersion << L" " << uiChatModeType << newl;

	// Check for version mismatch
	if (uiVersion != CHAT_PROTOCOL_VERSION)
	{
		CPacket pktReject;
		pktReject << NET_CHAT_CL_REJECT << CHAT_CLIENT_REJECT_BAD_VERSION;
		m_pSocket->SendPacket(pktReject);
		return false;
	}

	// Check for missing credentials
	if (m_sCookie.empty())
	{
		CPacket pktReject;
		pktReject << NET_CHAT_CL_REJECT << CHAT_CLIENT_REJECT_AUTH_FAILED;
		m_pSocket->SendPacket(pktReject);
		return false;
	}

	m_pRequest = m_pHTTPManager->SpawnRequest();
	if (m_pRequest == NULL)
		return false;

	m_pRequest->SetTargetURL(net_masterServerAddress.Get() + CLIENT_REQUEST_SCRIPT);
	m_pRequest->AddVariable(L"f", L"chatclient_connect");
	m_pRequest->AddVariable(L"account_id", XtoS(m_uiAccountID));
	m_pRequest->AddVariable(L"cookie", m_sCookie);
	m_pRequest->AddVariable(L"chat_mode_type", XtoS(uiChatModeType));
	m_pRequest->AddVariable(L"token", m_sToken);
	m_pRequest->SendPostRequest();

	m_eState = PENDING_CLIENT_AUTH;
	return true;
}


/*====================
  CPendingConnection::RequestServerAuth
  ====================*/
bool	CPendingConnection::RequestServerAuth(CPacket &pkt)
{
	PROFILE("CPendingConnection::RequestServerAuth");

	m_uiAccountID = pkt.ReadInt(INVALID_ACCOUNT_ID);
	m_sCookie = pkt.ReadString();
	uint uiVersion(pkt.ReadInt(0));

	m_pConsole->Net() << m_pSocket->GetRecvAddress() << L" - GAME SERVER: " << m_uiAccountID << L" " << m_sCookie << L" v" << uiVersion << newl;

	// Check for version mismatch
	if (uiVersion != CHAT_PROTOCOL_VERSION)
	{
		CPacket pktReject;
		pktReject << NET_CHAT_GS_REJECT << SERVER_REJECT_BAD_VERSION;
		m_pSocket->SendPacket(pktReject);
		return false;
	}

	// Check for missing credentials
	if (m_sCookie.empty())
	{
		CPacket pktReject;
		pktReject << NET_CHAT_GS_REJECT << SERVER_REJECT_AUTH_FAILED;
		m_pSocket->SendPacket(pktReject);
		return false;
	}

	/*
	m_pRequest = m_pHTTPManager->SpawnRequest();
	if (m_pRequest == NULL)
		return false;

	m_pRequest->SetTargetURL(net_masterServerAddress.Get() + SERVER_REQUEST_SCRIPT);
	m_pRequest->AddVariable(L"f", L"chatclient_connect");
	m_pRequest->AddVariable(L"account_id", XtoS(m_uiAccountID));
	m_pRequest->AddVariable(L"cookie", m_sCookie);
	m_pRequest->SendPostRequest();
	/**/

	m_eState = PENDING_SERVER_AUTH;
	return true;
}

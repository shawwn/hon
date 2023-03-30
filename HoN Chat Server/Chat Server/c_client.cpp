// (C)2009 S2 Games
// c_client.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "chatserver_common.h"

#include "c_client.h"
#include "c_clientmanager.h"
#include "c_phpdata.h"
#include "c_channel.h"
#include "c_peer.h"
#include "c_httprequest.h"
#include "c_console.h"
#include "c_matchmaker.h"
#include "c_match.h"
#include "c_peermanager.h"
#include "c_gameserver.h"
#include "c_gameservermanager.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
EXTERN_CVAR(bool, matchmaker_disabled);
EXTERN_CVAR(string, net_masterServerAddress);
EXTERN_CVAR(wstring, matchmaker_allowedRegions);

CVAR(uint, client_floodThreshold,			5);
CVAR(uint, client_maxChannels,				8);
CVAR(uint, client_timeout,					120);
CVAR(bool, client_stayInGeneralChannels,	false);

CVAR(float,	stat_leaveThreshold,			0.08f);
//=============================================================================

/*====================
  CClient::~CClient
  ====================*/
CClient::~CClient()
{
	SetOffline();
}


/*====================
  CClient::CClient
  ====================*/
CClient::CClient(CCore *pCore, CHeap *pHeap, CSocket *pSocket) :
NULL_CORE_API,
m_pHeap(pHeap),

m_lHTTPRequests(HTTPRequestList_alloc(pCore->GetMemManager(), pCore->GetClientManager()->GetSTLHeap())),
m_pPendingMatch(NULL),
m_uiSessionStart(0),
m_eStatus(CLIENT_STATUS_DISCONNECTED),
m_uiChatModeType(CHAT_MODE_AVAILABLE),
m_uiClientID(INVALID_CLIENT_ID),
m_uiPeerID(INVALID_PEER_ID),
m_pClan(NULL),

m_iMatches(0),
m_iDisconnects(0),

m_yFlags(0),
m_bIsPaidAccount(false),

m_uiAccountID(INVALID_ACCOUNT_ID),
m_pSocket(pSocket),
m_uiLastRecvTime(INVALID_TIME),
m_bSentPing(false),
m_uiRequestCount(0),
m_uiMatchID(-1),
m_uiLastUserCountUpdate(INVALID_TIME),
m_uiChannelListIndex(INVALID_CHANNEL),
m_bChannelListWait(true),
m_bChannelListSub(false),
m_yChannelListSequence(0)
{
	INIT_CORE_API(pCore);

	for (uint uiIndex(0); uiIndex < NUM_MATCH_POOLS; ++uiIndex)
	{
		m_aRating[uiIndex] = 0;
		m_aMatchCount[uiIndex] = 0;
		m_aInitialRatingSet[uiIndex] = true;
	}
}


/*====================
  CClient::IncrementRequestCounter
  ====================*/
bool	CClient::IncrementRequestCounter()
{
	PROFILE("CClient::IncrementRequestCounter")

	++m_uiRequestCount;
	if (m_uiRequestCount > client_floodThreshold)
	{
		m_pConsole->Std() << GetName() << L" - Flooding." << newl;
		//g_core.BanClient(GetAccountID(), 300000, BAN_REASON_FLOODING);
		return false;
	}

	return true;
}


/*====================
  CClient::Disconnect
  ====================*/
void	CClient::Disconnect()
{
	PROFILE("CClient::Disconnect")

	if (m_pSocket == NULL)
		return;

	if (m_pSocket->IsConnected())
	{
		m_bufferSend.Clear();
		m_bufferSend << CHAT_CMD_DISCONNECTED;
		Send(m_bufferSend);
		m_pSocket->Close();
	}

	delete m_pSocket;
	m_pSocket = NULL;
	m_eStatus = CLIENT_STATUS_DISCONNECTED;
}


/*====================
  CClient::VerifyNotification
  ====================*/
void	CClient::VerifyNotification(uint uiKey)
{
	if (m_eStatus < CLIENT_STATUS_CONNECTED)
		return;

	CHTTPRequest *pRequest(m_pHTTPManager->SpawnRequest());
	if (pRequest == NULL)
		return;

	pRequest->SetType(CLIENT_REQUEST_NOTIFICATION);
	pRequest->SetTargetURL(net_masterServerAddress.Get() + CLIENT_REQUEST_SCRIPT);

	pRequest->AddVariable(L"f", L"check_notification");
	pRequest->AddVariable(L"cookie", m_sCookie);
	pRequest->AddVariable(L"key", uiKey);

	pRequest->SendPostRequest();
	m_lHTTPRequests.push_back(pRequest);
}


/*====================
  CClient::ProcessNotification
  ====================*/
void	CClient::ProcessNotification(const wstring &sResult)
{
	CPHPData phpResponse(sResult);
	const CPHPData *pNotify(phpResponse.GetVar(L"check_notification"));
	if (pNotify == NULL)
		return;

	wstring sType(LowerString(pNotify->GetString(L"type")));
	byte yType(NOTIFY_TYPE_UNKNOWN);
	if (sType == L"added")
		yType = NOTIFY_TYPE_BUDDY_ADDED;
	else if (sType == L"b_add")
		yType = NOTIFY_TYPE_BUDDY_ADDER;
	else if (sType == L"rem")
		yType = NOTIFY_TYPE_BUDDY_REMOVED;
	else if (sType ==L"b_rem")
		yType = NOTIFY_TYPE_BUDDY_REMOVER;
	else if (sType == L"c_rank")
		yType = NOTIFY_TYPE_CLAN_RANK;
	else if (sType == L"c_add")
		yType = NOTIFY_TYPE_CLAN_ADD;
	else if (sType == L"c_rem")
		yType = NOTIFY_TYPE_CLAN_REMOVE;

	if (yType == NOTIFY_TYPE_UNKNOWN)
		return;

	if (yType < NOTIFY_TYPE_CLAN_RANK)
		SendNotification(pNotify->GetString(L"message"), yType);
	else
		NotifyClan(yType, pNotify->GetString(L"message"));
}


/*====================
  CClient::ProcessHTTPRequests
  ====================*/
void	CClient::ProcessHTTPRequests()
{
	PROFILE("CClient::ProcessHTTPRequests")

	if (m_lHTTPRequests.empty())
		return;

	for (HTTPRequestList_it itRequest(m_lHTTPRequests.begin()), itEnd(m_lHTTPRequests.end()); itRequest != itEnd; )
	{
		switch ((*itRequest)->GetStatus())
		{
		case HTTP_REQUEST_SENDING:
			++itRequest;
			break;

		default:
		case HTTP_REQUEST_IDLE:
		case HTTP_REQUEST_ERROR:
			m_pHTTPManager->ReleaseRequest(*itRequest);
			STL_ERASE(m_lHTTPRequests, itRequest);
			itEnd = m_lHTTPRequests.end();
			break;

		case HTTP_REQUEST_SUCCESS:
			switch ((*itRequest)->GetType())
			{
			case CLIENT_REQUEST_NOTIFICATION:
				ProcessNotification((*itRequest)->GetResponse());
				break;

			case CLIENT_REQUEST_CLAN_ADD:
				ProcessClanInviteResponse((*itRequest)->GetResponse());
				break;

			case CLIENT_REQUEST_CLAN_CREATE:
				ProcessCreateClanResponse((*itRequest)->GetResponse());
				break;

			case CLIENT_REQUEST_USER_INFO_IN_GAME:
				ProcessUserInfoInGameResponse((*itRequest)->GetResponse());
				break;

			case CLIENT_REQUEST_UPDATE_RATING:
				ProcessUpdateRatingResponse((*itRequest)->GetResponse(), false);
				break;

			case CLIENT_REQUEST_JOIN_MATCHMAKING_POOL:
				ProcessUpdateRatingResponse((*itRequest)->GetResponse(), true);
				break;
				
			case CLIENT_REQUEST_BUDDY_ADD:
				ProcessRequestBuddyAddResponse((*itRequest)->GetResponse());
				break;				
				
			case CLIENT_REQUEST_BUDDY_APPROVE:
				ProcessRequestBuddyApproveResponse((*itRequest)->GetResponse());
				break;				
			}
			
			m_pHTTPManager->ReleaseRequest(*itRequest);
			STL_ERASE(m_lHTTPRequests, itRequest);
			itEnd = m_lHTTPRequests.end();
			break;
		}
	}
}


/*====================
  CClient::Frame
  ====================*/
bool	CClient::Frame()
{
	PROFILE("CClient::Frame");

	ProcessHTTPRequests();

	if (HasAllFlags(CLIENT_VIRTUAL))
		return true;

	if (m_pSocket == NULL)
		return false;

	// Send channel listing
	if (!m_bChannelListWait && m_uiChannelListIndex != INVALID_CHANNEL)
	{
		if (m_bChannelListSub)
		{
			m_pChannelManager->SendChannelSublisting(this);
			m_bChannelListWait = true;
		}
		else
		{
			m_pChannelManager->SendChannelListing(this);
			m_bChannelListWait = true;
		}
	}

	CPacket pktRecv;
	int iRecvLength(m_pSocket->ReceivePacket(pktRecv));
	if (iRecvLength < 1)
		return iRecvLength == K2_SOCKET_EMPTY;

	m_uiLastRecvTime = m_pCore->GetSeconds();

	bool bClientOk(true);
	while (!pktRecv.DoneReading() && !pktRecv.HasFaults() && bClientOk)
	{
		ushort unCmd(pktRecv.ReadShort());

		switch (unCmd)
		{
		case NET_CHAT_PONG:
			m_bSentPing = false;
			break;

		case NET_CHAT_CL_GET_CHANNEL_LIST:
			if (m_uiChannelListIndex == INVALID_CHANNEL)
			{
				m_bChannelListWait = false;
				m_uiChannelListIndex = 0;
				m_bChannelListSub = false;
			}
			break;

		case NET_CHAT_CL_CHANNEL_LIST_ACK:
			m_bChannelListWait = false;
			break;

		case NET_CHAT_CL_GET_CHANNEL_SUBLIST:
			bClientOk = HandleChannelSublist(pktRecv);
			break;

		case NET_CHAT_CL_CHANNEL_SUBLIST_ACK:
			bClientOk = HandleChannelSublistAck(pktRecv);
			break;

		case NET_CHAT_CL_GET_USER_STATUS:
			bClientOk = HandleGetUserStatus(pktRecv);
			break;

		case CHAT_CMD_CHANNEL_MSG:
			bClientOk = HandleChannelMessage(pktRecv);
			break;

		case CHAT_CMD_WHISPER:
			bClientOk = HandleWhisper(pktRecv);
			break;

		case CHAT_CMD_IM:
			bClientOk = HandleIM(pktRecv);
			break;

		case CHAT_CMD_REQUEST_BUDDY_ADD:
			bClientOk = HandleRequestBuddyAdd(pktRecv);
			break;

		case CHAT_CMD_REQUEST_BUDDY_APPROVE:
			bClientOk = HandleRequestBuddyAddApprove(pktRecv);
			break;

		case CHAT_CMD_REQUEST_BUDDY_REMOVE:
			bClientOk = HandleBuddyRemoveNotification(pktRecv);
			break;

		case CHAT_CMD_JOINING_GAME:
			bClientOk = HandleJoiningGame(pktRecv);
			break;

		case CHAT_CMD_JOINED_GAME:
			bClientOk = HandleJoinedGame(pktRecv);
			break;

		case CHAT_CMD_LEFT_GAME:
			bClientOk = HandleLeftGame();
			break;

		case CHAT_CMD_CLAN_WHISPER:
			bClientOk = HandleClanWhisper(pktRecv);
			break;

		case CHAT_CMD_CLAN_PROMOTE_NOTIFY:
			bClientOk = HandleClanPromoteNotification(pktRecv);
			break;

		case CHAT_CMD_CLAN_DEMOTE_NOTIFY:
			bClientOk = HandleClanDemoteNotification(pktRecv);
			break;

		case CHAT_CMD_CLAN_REMOVE_NOTIFY:
			bClientOk = HandleClanRemoveNotification(pktRecv);
			break;

		case CHAT_CMD_LOOKING_FOR_CLAN:
			bClientOk = HandleLookingForClan();
			break;

		case CHAT_CMD_NOT_LOOKING_FOR_CLAN:
			bClientOk = HandleNotLookingForClan();
			break;

		case CHAT_CMD_JOIN_CHANNEL:
			bClientOk = HandleJoinChannel(pktRecv);
			break;

		case CHAT_CMD_WHISPER_BUDDIES:
			bClientOk = HandleWhisperBuddies(pktRecv);
			break;

		case CHAT_CMD_LEAVE_CHANNEL:
			bClientOk = HandleLeaveChannel(pktRecv);
			break;

		case CHAT_CMD_INVITE_USER_ID:
			bClientOk = HandleInviteIDToServer(pktRecv);
			break;

		case CHAT_CMD_INVITE_USER_NAME:
			bClientOk = HandleInviteNameToServer(pktRecv);
			break;

		case CHAT_CMD_INVITE_REJECTED:
			bClientOk = HandleInviteRejected(pktRecv);
			break;

		case CHAT_CMD_USER_INFO:
			bClientOk = HandleUserInfo(pktRecv);
			break;

		case CHAT_CMD_CHANNEL_TOPIC:
			bClientOk = HandleChannelTopic(pktRecv);
			break;

		case CHAT_CMD_CHANNEL_KICK:
			bClientOk = HandleChannelKick(pktRecv);
			break;

		case CHAT_CMD_CHANNEL_BAN:
			bClientOk = HandleChannelBan(pktRecv);
			break;

		case CHAT_CMD_CHANNEL_UNBAN:
			bClientOk = HandleChannelUnban(pktRecv);
			break;

		case CHAT_CMD_CHANNEL_SILENCE_USER:
			bClientOk = HandleChannelSilenceUser(pktRecv);
			break;

		case CHAT_CMD_CHANNEL_PROMOTE:
			bClientOk = HandleChannelPromote(pktRecv);
			break;

		case CHAT_CMD_CHANNEL_DEMOTE:
			bClientOk = HandleChannelDemote(pktRecv);
			break;

		case CHAT_CMD_MESSAGE_ALL:
			bClientOk = HandleMessageAll(pktRecv);
			break;

		case CHAT_CMD_CHANNEL_SET_AUTH:
			bClientOk = HandleChannelSetAuth(pktRecv);
			break;

		case CHAT_CMD_CHANNEL_REMOVE_AUTH:
			bClientOk = HandleChannelRemoveAuth(pktRecv);
			break;

		case CHAT_CMD_CHANNEL_ADD_AUTH_USER:
			bClientOk = HandleChannelAddAuthUser(pktRecv);
			break;

		case CHAT_CMD_CHANNEL_REM_AUTH_USER:
			bClientOk = HandleChannelRemoveAuthUser(pktRecv);
			break;

		case CHAT_CMD_CHANNEL_LIST_AUTH:
			bClientOk = HandleChannelListAuth(pktRecv);
			break;

		case CHAT_CMD_CHANNEL_SET_PASSWORD:
			bClientOk = HandleChannelSetPassword(pktRecv);
			break;

		case CHAT_CMD_JOIN_CHANNEL_PASSWORD:
			bClientOk = HandleChannelJoinPassword(pktRecv);
			break;

		case CHAT_CMD_CLAN_ADD_MEMBER:
			bClientOk = m_pClientManager->HandleClanInvite(this, pktRecv);
			break;

		case CHAT_CMD_CLAN_ADD_REJECTED:
			bClientOk = m_pClientManager->HandleClanInviteRejected(this, pktRecv);
			break;

		case CHAT_CMD_CLAN_ADD_ACCEPTED:
			bClientOk = m_pClientManager->HandleClanInviteAccepted(this, pktRecv);
			break;

		case CHAT_CMD_CLAN_CREATE_REQUEST:
			bClientOk = m_pClientManager->HandleCreateClan(this, pktRecv);
			break;

		case NET_CHAT_CL_REQUEST_AUTO_MATCH:
			bClientOk = HandleRequestAutoMatch(pktRecv);
			break;

		case NET_CHAT_CL_CANCEL_AUTO_MATCH:
			bClientOk = HandleCancelAutoMatch(pktRecv);
			break;
			
		case CHAT_CMD_CHAT_ROLL:
			bClientOk = HandleChatRoll(pktRecv);
			break;

		case CHAT_CMD_CHAT_EMOTE:
			bClientOk = HandleChatEmote(pktRecv);
			break;
			
		case CHAT_CMD_SET_CHAT_MODE_TYPE:
			bClientOk = HandleSetChatModeType(pktRecv);
			break;

		case CHAT_CMD_REQUEST_MATCHMAKING_INFO:
			bClientOk = HandleRequestMatchmakingInfo(pktRecv);
			break;

		default:
			m_pConsole->Std() << L"Invalid data from user " << GetName() << newl
				<< L"cmd: " << SHORT_HEX_STR(unCmd) << L" " << pktRecv.GetReadPos() << L"/" << pktRecv.GetLength() << newl;
			bClientOk = false;
			break;
		}
	}

	return bClientOk;
}


/*====================
  CClient::CheckTimeout
  ====================*/
bool	CClient::CheckTimeout()
{
	PROFILE("CClient::CheckTimeout");

	if (m_eStatus < CLIENT_STATUS_CONNECTED || HasAllFlags(CLIENT_VIRTUAL))
		return true;

	if (m_pCore->GetSeconds() - m_uiLastRecvTime >= client_timeout)
		return false;

	if (!m_bSentPing && m_pCore->GetSeconds() - m_uiLastRecvTime >= client_timeout / 2)
		SendPing();

	return true;
}


/*====================
  CClient::Send
  ====================*/
void	CClient::Send(const IBuffer &buffer)
{
	PROFILE("CClient::Send");

	if (HasAllFlags(CLIENT_VIRTUAL))
		return;

	if (m_eStatus < CLIENT_STATUS_CONNECTED)
		return;

	if (buffer.GetLength() > MAX_PACKET_SIZE)
	{
		m_pConsole->Std() << L"Packet overflow for client #" << m_uiClientID << L" (" << GetName() << L")" << newl;
		return;
	}

	CPacket pkt;
	pkt.Write(buffer.Get(), buffer.GetLength());
	m_pSocket->SendPacket(pkt);
}


/*====================
  CClient::SendPing
  ====================*/
void	CClient::SendPing()
{
	PROFILE("CClient::SendPing");

	m_bSentPing = true;

	// Ping user to see if they're still alive
	m_bufferSend.Clear();
	m_bufferSend << NET_CHAT_PING;
	Send(m_bufferSend);

	m_pConsole->Std() << m_sNameLower << L" - Ping?" << newl;
}


/*====================
  CClient::SendUserCount
  ====================*/
void	CClient::SendUserCount(uint uiUserCount)
{
	PROFILE("CClient::SendUserCount");

	m_bufferSend.Clear();
	m_bufferSend << CHAT_CMD_PLAYER_COUNT << uiUserCount;

	Send(m_bufferSend);

	m_uiLastUserCountUpdate = m_pCore->GetMilliseconds();
}


/*====================
  CClient::SendNotification
  ====================*/
void	CClient::SendNotification(const wstring &sMessage, byte yType)
{
	PROFILE("CClient::SendNotification");

	if (m_eStatus < CLIENT_STATUS_CONNECTED)
		return;

	m_bufferSend.Clear();
	m_bufferSend << CHAT_CMD_NOTIFICATION << yType << WStringToUTF8(sMessage) << byte(0);

	Send(m_bufferSend);
}


/*====================
  CClient::SetClan
  ====================*/
void	CClient::SetClan(CClan *pClan)
{
	if (m_pClan == pClan)
		return;

	if (m_pClan != NULL)
		m_pClan->RemoveClient(this);

	m_pClan = pClan;

	if (m_pClan == NULL)
	{
		m_sNameUTF8 = WStringToUTF8(m_sName);
		RemoveFlags(CLIENT_IS_OFFICER | CLIENT_IS_CLAN_LEADER);
		return;
	}

	m_sNameUTF8 = "[" + m_pClan->GetTagUTF8() + "]" + WStringToUTF8(m_sName);
	m_pClan->AddClient(this);
}


/*====================
  CClient::SetName
  ====================*/
void	CClient::SetName(const wstring &sName)
{
	PROFILE("CClient::SetName");

	m_sName = sName;
	
	if (m_pClan == NULL)
		m_sNameUTF8 = WStringToUTF8(sName);
	else
		m_sNameUTF8 = "[" + m_pClan->GetTagUTF8() + "]" + WStringToUTF8(sName);

	m_sNameLower = LowerString(sName);
}


/*====================
  CClient::IsLeaver
  ====================*/
bool	CClient::IsLeaver() const
{
	if (m_iMatches == 0)
		return false;

	if (m_iMatches >= 100)
		return (m_iDisconnects / float(m_iMatches)) > stat_leaveThreshold + 0.001f;

	float fStartGame(0.0f);
	float fStartLeaves(3.0f);

	float fEndGame(80.0f);
	float fEndLeaves(stat_leaveThreshold * 100.0f);

	float fLerp(CLAMP(ILERP<float>(m_iMatches, fStartGame, fEndGame), 0.0f, 1.0f));
	fLerp = CLAMP(LERP(fLerp, fStartLeaves, fEndLeaves) / m_iMatches, 0.0f, 1.0f);

	return (m_iDisconnects / float(m_iMatches)) > fLerp + 0.001f;
}


/*====================
  CClient::PrintInfo
  ====================*/
void	CClient::PrintInfo() const
{
	m_pConsole->Admin()
		<< L"Client #" << m_uiClientID << newl
		<< L"---------------" << newl
		<< L"Name:          " << m_sName << newl
		<< L"Account ID:    " << m_uiAccountID << newl
		<< L"Peer ID:       " << m_uiPeerID << newl
		<< L"Session:       " << GetTimeStringW(SecToMs(m_pCore->GetSeconds() - m_uiSessionStart)) << newl
		<< L"Solo Count:    " << m_aMatchCount[MATCH_POOL_SOLO] << newl
		<< L"Solo Rating:   " << m_aRating[MATCH_POOL_SOLO] << newl
		<< L"Team Count:    " << m_aMatchCount[MATCH_POOL_TEAM] << newl
		<< L"Team Rating:   " << m_aRating[MATCH_POOL_TEAM] << newl
		<< L"Total Matches: " << m_iMatches << newl
		<< L"Disconnects:   " << m_iDisconnects << newl
		<< L"Leaver:        " << IsLeaver() << newl;
	
	if (m_pClan != NULL)
		m_pConsole->Admin() << L"Clan:       " << m_pClan->GetName() << L" (" << m_pClan->GetClanID() << L")" << newl;
	
	m_pConsole->Admin() << L"Flags:      ";
	if (HasAllFlags(CLIENT_IS_OFFICER)) m_pConsole->Admin() << L"[OFFICER] ";
	if (HasAllFlags(CLIENT_IS_CLAN_LEADER)) m_pConsole->Admin() << L"[CLAN LEADER] ";
	if (HasAllFlags(CLIENT_LOOKING_FOR_CLAN)) m_pConsole->Admin() << L"[LOOKING FOR CLAN] ";
	if (HasAllFlags(CLIENT_IS_STAFF)) m_pConsole->Admin() << L"[STAFF] ";
	if (HasAllFlags(CLIENT_IS_PREMIUM)) m_pConsole->Admin() << L"[PREMIUM] ";
	m_pConsole->Admin() << newl;

	m_pConsole->Admin() << L"Status:     ";
	switch (m_eStatus)
	{
	case CLIENT_STATUS_DISCONNECTED:	m_pConsole->Admin() << L"Offline" << newl; return;
	case CLIENT_STATUS_CONNECTED:		m_pConsole->Admin() << L"Online" << newl; break;
	case CLIENT_STATUS_JOINING_GAME:	m_pConsole->Admin() << L"Loading..." << newl; break;
	case CLIENT_STATUS_IN_GAME:			m_pConsole->Admin() << L"In match #" << m_uiMatchID << newl; break;
	}

	if (m_setChannels.empty())
		return;

	m_pConsole->Admin()
		<< newl
		<< L"Channels" << newl
		<< L"--------------" << newl;
	for (uiset_cit itChannel(m_setChannels.begin()), itEnd(m_setChannels.end()); itChannel != itEnd; ++itChannel)
	{
		CChannel *pChannel(m_pChannelManager->GetChannel(*itChannel));
		if (pChannel == NULL)
			continue;

		m_pConsole->Admin() << L"[" << pChannel->GetChannelID() << L"] " << UTF8ToWString(pChannel->GetNameUTF8()) << newl;
	}
}


/*====================
  CClient::SetOnline
  ====================*/
void	CClient::SetOnline()
{
	PROFILE("CClient::SetOnline");

	m_eStatus = CLIENT_STATUS_CONNECTED;
	ResetRequestCounter();
	KeepAlive();

	m_bufferSend.Clear();
	m_bufferSend << NET_CHAT_CL_ACCEPT;
	Send(m_bufferSend);

	SendStatusUpdates();
	UpdateStatus();

	m_pConsole->Std() << L"Client '" << m_sNameUTF8 << L"' has connected, ID: " << m_uiAccountID << newl;
	
	// don't connect to any channels because this player wants to remain invisible to others
	if (GetChatModeType() != CHAT_MODE_INVISIBLE)
	{
		CChannel *pChannel(m_pChannelManager->GetDefaultChannel(this));
		if (pChannel != NULL)
			pChannel->AddClient(this);
	}

	m_pClientManager->IncrementConnectionCounts(m_pSocket->GetRecvAddressBin(), m_bIsPaidAccount);
}


/*====================
  CClient::SetOffline
  ====================*/
void	CClient::SetOffline()
{
	PROFILE("CClient::SetOffline");

	// Remove client from pending match, if they are in one
	JoinMatch(NULL);

	for (HTTPRequestList_it itRequest(m_lHTTPRequests.begin()), itEnd(m_lHTTPRequests.end()); itRequest != itEnd; ++itRequest)
		m_pHTTPManager->ReleaseRequest(*itRequest);
	m_lHTTPRequests.clear();

	if (m_eStatus >= CLIENT_STATUS_IN_GAME)
		m_pClientManager->DecrementInGameCount();

	if (m_eStatus >= CLIENT_STATUS_CONNECTED)
	{
		m_pClientManager->DecrementOnlineCount();
		m_pClientManager->DecrementConnectionCounts(m_pSocket->GetRecvAddressBin(), m_bIsPaidAccount);
	}

	m_eStatus = CLIENT_STATUS_DISCONNECTED;
	UpdateStatus();

	LeaveAllChannels();
	Disconnect();
	m_setBuddies.clear();
	SetClan(NULL);
	ClearCurrentMatch();
}


/*====================
  CClient::JoinMatch
  ====================*/
void	CClient::JoinMatch(CMatch *pMatch)
{
	if (m_pPendingMatch != NULL)
		m_pPendingMatch->InvalidateClient(this, (pMatch == NULL) ? L"cancel request" : L"joined new match");
	m_pPendingMatch = pMatch;
}


/*====================
  CClient::SendMatchMakerUpdate
  ====================*/
void	CClient::SendMatchMakerUpdate(byte yStatus, uint uiParamA, uint uiParamB)
{
	m_bufferSend.Clear();
	m_bufferSend << CHAT_CMD_AUTO_MATCH_STATUS << yStatus;
	switch (yStatus)
	{
	case MATCHMAKER_STATUS_IN_POOL:
		m_bufferSend << uiParamA << uiParamB;
		break;

	case MATCHMAKER_STATUS_LOOKING_FOR_SERVER:
		m_bufferSend << uiParamA;
		break;

	case MATCHMAKER_STATUS_WAITING_FOR_SERVER:
		break;
	}

	Send(m_bufferSend);
}


/*====================
  CClient::GetGeneralChannel
  ====================*/
CChannel*	CClient::GetGeneralChannel() const
{
	PROFILE("CClient::GetGeneralChannel");

	for (uiset_cit itChannelID(m_setChannels.begin()), itEnd(m_setChannels.end()); itChannelID != itEnd; ++itChannelID)
	{
		CChannel *pChannel(m_pChannelManager->GetChannel(*itChannelID));
		if (pChannel != NULL && pChannel->HasAllFlags(CHANNEL_FLAG_GENERAL_USE))
			return pChannel;
	}

	return NULL;
}


/*====================
  CClient::LeaveAllChannels
  ====================*/
void	CClient::LeaveAllChannels(uint uiFlags)
{
	PROFILE("CClient::LeaveAllChannels");

	uiset setChannels(m_setChannels);
	for (uiset_it itChannel(setChannels.begin()), itEnd(setChannels.end()); itChannel != itEnd; ++itChannel)
	{
		CChannel *pChannel(m_pChannelManager->GetChannel(*itChannel));
		if (pChannel == NULL || !pChannel->HasAllFlags(uiFlags))
			continue;

		pChannel->RemoveClient(this);
	}
}


/*====================
  CClient::AddChannelsToSet
  ====================*/
void	CClient::AddChannelsToSet(uiset &setChannels, CClient *pViewer) const
{
	PROFILE("CClient::AddChannelsToSet");

	if (pViewer == NULL)
	{
		setChannels.insert(m_setChannels.begin(), m_setChannels.end());
		return;
	}

	for (uiset_cit it(m_setChannels.begin()), itEnd(m_setChannels.end()); it != itEnd; ++it)
	{
		CChannel *pChannel(m_pChannelManager->GetChannel(*it));
		if (pChannel == NULL || !pChannel->CanSee(pViewer))
			continue;
		
		setChannels.insert(*it);
	}
}


/*====================
  CClient::AddChannelMatesToSet
  ====================*/
void	CClient::AddChannelMatesToSet(ClientSet &setPeers) const
{
	PROFILE("CClient::AddChannelMatesToSet");

	for (uiset_cit it(m_setChannels.begin()), itEnd(m_setChannels.end()); it != itEnd; ++it)
	{
		CChannel *pChannel(m_pChannelManager->GetChannel(*it));
		if (pChannel != NULL)
			pChannel->AddClientsToSet(setPeers);
	}
}


/*====================
  CClient::UpdateStatus
  ====================*/
void	CClient::UpdateStatus()
{
	PROFILE("CClient::UpdateStatus");

	// don't send any updates because this player wants to remain invisible to others
	if (GetChatModeType() == CHAT_MODE_INVISIBLE)
		return;
	
	m_bufferSend.Clear();
	m_bufferSend << CHAT_CMD_UPDATE_STATUS << m_uiAccountID << byte(m_eStatus) << m_yFlags << GetClanID() << GetClanNameUTF8() << byte(0);

	if (m_eStatus > CLIENT_STATUS_CONNECTED)
		m_bufferSend << m_sServerAddressPort << byte(0);
			
	if (m_eStatus == CLIENT_STATUS_IN_GAME)
	{		
		// TODO: maybe add status updates for in game tutorial and friends joining arranged games
		// it's a local game they are joining, don't send this for now
		if (CompareNoCase(m_sServerAddressPort.substr(0, 9), LOCALHOST) == 0)
			return;
	
		CGameServer *pJoinedServer;
		
		pJoinedServer = m_pGameServerManager->GetGameServerFromAddress(m_sServerAddressPort);
		if (pJoinedServer != NULL)
		{		
			m_bufferSend << WStringToUTF8(pJoinedServer->GetGameName()) << byte(0)		// Game Name
						 << m_uiMatchID													// MatchID
						 << pJoinedServer->GetArrangedType()							// Arranged Match Type (0 = None, 1 = Match Making, 2 = Tournament, 3 = League)
						 << GetNameUTF8() << byte(0)									// Player Name
						 << WStringToUTF8(pJoinedServer->GetLocation()) << byte(0)		// Region
						 << WStringToUTF8(pJoinedServer->GetGameModeName()) << byte(0)	// Game Mode Name (banningdraft)
						 <<	pJoinedServer->GetTeamSize()								// Team Size
						 << WStringToUTF8(pJoinedServer->GetMapName()) << byte(0)		// Map Name (caldavar)
						 << pJoinedServer->GetTier()									// Tier - Noobs Only (0), Noobs Allowed (1), Pro (2)
						 << pJoinedServer->GetOfficial()								// 0 - Unofficial, 1 - Official w/ stats, 2 - Official w/o stats
						 <<	pJoinedServer->GetNoLeaver()								// No Leavers (1), Leavers (0)
						 << pJoinedServer->GetPrivate()									// Private (1), Not Private (0)
						 << pJoinedServer->GetAllHeroes()								// All Heroes (1), Not All Heroes (0)
						 << pJoinedServer->GetEasyMode()								// Easy Mode (1), Not Easy Mode (0)
						 << pJoinedServer->GetForceRandom()								// Force Random (1), Not Force Random (0)
						 << pJoinedServer->GetAutoBalanced()							// Auto Balanced (1), Non Auto Balanced (0)
						 << pJoinedServer->GetAdvancedOptions()							// Advanced Options	(1), No Advanced Options (0)
						 << pJoinedServer->GetMinPSR()									// Min PSR
						 << pJoinedServer->GetMaxPSR()									// Max PSR
						 << pJoinedServer->GetDevHeroes()								// Dev Heroes (1), Non Dev Heroes (0)
						 << pJoinedServer->GetHardcore();								// Hardcore (1), Non Hardcore (0)
		}			
		else		
			m_bufferSend << m_sGameNameUTF8 << byte(0) << m_uiMatchID;
	}

	SendToPeers(m_bufferSend);

	if (m_pClan != NULL)
		m_pClan->Broadcast(m_bufferSend);
}


/*====================
  CClient::SendStatusUpdates

  This function updates a new user on the status of all of his clan members and buddies, 
  it doesn't actually send status updates of this client
  ====================*/
void	CClient::SendStatusUpdates()
{
	PROFILE("CClient::SendStatusUpdates")
	
	// Update the user on buddy and clan online/offline status
	CBufferDynamic bufSend;
	bufSend << CHAT_CMD_INITIAL_STATUS;

	ClientSet setOnline(std::less<ClientSet::key_type>(), ClientSet_alloc(m_pMemManager, m_pHeap));

	// Add online clan members to the set, only those that are not invisible
	// This is the old way, the new way I made m_pClients public as I was not able to access the Client list from within CClan
	//if (m_pClan != NULL)
		//m_pClan->AddClientsToSet(setOnline);

	if (m_pClan != NULL)
	{
		for (ClientSet_it itClient(m_pClan->m_pClients->begin()), itEnd(m_pClan->m_pClients->end()); itClient != itEnd; ++itClient) 
		{ 
			CClient *pClient(*itClient); 
			if (pClient == NULL || pClient->GetChatModeType() == CHAT_MODE_INVISIBLE) 
				continue; 
				
			setOnline.insert(pClient);
		}	
	}

	// Add online buddies (who were not already added as clan members) to the set
	for (uiset_it itBuddy(m_setBuddies.begin()), itEnd(m_setBuddies.end()); itBuddy != itEnd; ++itBuddy)
	{
		CClient *pTarget(m_pClientManager->GetClientFromAccountID(*itBuddy));
		// only add targets who aren't invisible
		if (pTarget != NULL && pTarget->GetChatModeType() != CHAT_MODE_INVISIBLE)
			setOnline.insert(pTarget);
	}

	// Add the number of clients and client info to the packet
	// Any account IDs included here are online buddies/clan members
	bufSend << INT_SIZE(setOnline.size());

	for (ClientSet_it it(setOnline.begin()), itEnd(setOnline.end()); it != itEnd; ++it)
	{
		bufSend << (*it)->GetAccountID() << (*it)->GetStatusByte() << (*it)->GetFlags();

		if ((*it)->GetStatus() > CLIENT_STATUS_CONNECTED)
		{
			bufSend << (*it)->GetServerAddressUTF8() << byte(0);
			bufSend << (*it)->GetServerNameUTF8() << byte(0);
		}
	}

	Send(bufSend);
}


/*====================
  CClient::SendToPeers
  ====================*/
void	CClient::SendToPeers(const IBuffer &buffer)
{
	PROFILE("CClient::SendToPeers")
	
	// Get a list of users in the same channels as this user
	ClientSet setClients(std::less<ClientSet::key_type>(), ClientSet_alloc(m_pMemManager, m_pHeap));
	AddChannelMatesToSet(setClients);

	// Add buddies
	for (uiset_it itBuddy(m_setBuddies.begin()), itEnd(m_setBuddies.end()); itBuddy != itEnd; ++itBuddy)
	{
		CClient *pClient(m_pClientManager->GetClientFromAccountID(*itBuddy));
		if (pClient == NULL || pClient == this)
			continue;

		setClients.insert(pClient);
	}

	// Add clan mates
	if (m_pClan != NULL)
		m_pClan->AddClientsToSet(setClients);

	// Send to all clients in set
	for (ClientSet_it itClient(setClients.begin()), itEnd(setClients.end()); itClient != itEnd; ++itClient)
	{
		CClient *pClient(*itClient);
		if (pClient == NULL || pClient == this)
			continue;

		pClient->Send(buffer);
	}
}


/*====================
  CClient::UserCountUpdateNeeded
  ====================*/
bool	CClient::UserCountUpdateNeeded()
{
	if (m_uiLastUserCountUpdate == INVALID_TIME)
		return true;

	if (m_uiLastUserCountUpdate + USER_COUNT_UPDATE_DELAY < m_pCore->GetMilliseconds())
		return true;

	return false;
}


/*====================
  CClient::HandleChannelMessage
  ====================*/
bool	CClient::HandleChannelMessage(CPacket &pkt)
{
	PROFILE("CClient::HandleChannelMessage")

	string sMessage(pkt.ReadUTF8String());
	uint uiChannelID(pkt.ReadInt());

	if (!IncrementRequestCounter())
		return false;

	CChannel *pChannel(m_pChannelManager->GetChannel(uiChannelID));
	if (pChannel == NULL)
		return true;

	if (!pChannel->HasClient(this))
		return true;

	if (pChannel->IsSilenced(this))
	{
		m_pConsole->Std() << m_sName << L" is currently silenced." << newl;

		m_bufferSend.Clear();
		m_bufferSend << CHAT_CMD_CHANNEL_SILENCED << uiChannelID;
		Send(m_bufferSend);
		return true;
	}

	if (sMessage.length() > CHAT_MESSAGE_MAX_LENGTH)
		sMessage.resize(CHAT_MESSAGE_MAX_LENGTH);

	m_pConsole->Chatter() << m_sName << L" -> Channel '" << pChannel->GetNameLower() << L"': " << sMessage << newl;

	// Setup a packet to send to all users in the channel
	m_bufferSend.Clear();
	m_bufferSend << CHAT_CMD_CHANNEL_MSG << m_uiAccountID << uiChannelID << sMessage << byte(0);

	pChannel->Broadcast(m_bufferSend, this);

	return true;
}


/*====================
  CClient::HandleWhisper
  ====================*/
bool	CClient::HandleWhisper(CPacket &pkt)
{
	PROFILE("CClient::HandleWhisper")

	wstring sTarget(pkt.ReadWString());
	string sMessageUTF8(pkt.ReadUTF8String());

	if (!IncrementRequestCounter())
		return true;

	if (sMessageUTF8.length() > CHAT_MESSAGE_MAX_LENGTH)
		sMessageUTF8.resize(CHAT_MESSAGE_MAX_LENGTH);

	// Find recipient
	// FIXME: Is it possible to get rid of all name lookups?
	CClient *pTarget(m_pClientManager->GetClientFromName(LowerString(sTarget)));
	if (pTarget == NULL)
	{
		// Notify the player that user doesn't exist
		m_bufferSend.Clear();
		m_bufferSend << CHAT_CMD_WHISPER_FAILED;
		Send(m_bufferSend);
		return true;		
	}

	if (pTarget->GetStatus() < CLIENT_STATUS_CONNECTED || pTarget->GetChatModeType() == CHAT_MODE_INVISIBLE)
	{
		// Notify the player that user doesn't exist
		m_bufferSend.Clear();
		m_bufferSend << CHAT_CMD_WHISPER_FAILED;
		Send(m_bufferSend);
		return true;
	}
	
	if (pTarget->GetChatModeType() == CHAT_MODE_AFK)
	{
		// Notify the sender that user is AFK but still pass the message to the receiver	
		m_bufferSend.Clear();		
		m_bufferSend << CHAT_CMD_CHAT_MODE_AUTO_RESPONSE << CHAT_MODE_AFK << pTarget->GetNameUTF8() << byte(0) << WStringToUTF8(pTarget->GetChatModeTypeReason()) << byte(0);
		Send(m_bufferSend);
	}
	else if (pTarget->GetChatModeType() == CHAT_MODE_DND)
	{
		// Notify the sender that user is DND and do not pass pass the message to the receiver	
		m_bufferSend.Clear();
		m_bufferSend << CHAT_CMD_CHAT_MODE_AUTO_RESPONSE << CHAT_MODE_DND << pTarget->GetNameUTF8() << byte(0) << WStringToUTF8(pTarget->GetChatModeTypeReason()) << byte(0);
		Send(m_bufferSend);
		return true;
	}		

	pTarget->ReceiveWhisper(m_sNameUTF8, sMessageUTF8);
	return true;
}


/*====================
  CClient::HandleIM
  ====================*/
bool	CClient::HandleIM(CPacket &pkt)
{
	PROFILE("CClient::HandleIM")

	wstring sTarget(pkt.ReadWString());
	wstring sMessage(pkt.ReadWString());

	if (!IncrementRequestCounter())
		return true;

	m_pConsole->Chatter() << m_sName << L" -> IM '" << sTarget << L"': " << sMessage << newl;

	// Find recipient
	CClient *pTarget(m_pClientManager->GetClientFromName(LowerString(sTarget)));
	if (pTarget == NULL || pTarget->GetStatus() < CLIENT_STATUS_CONNECTED || pTarget->GetChatModeType() == CHAT_MODE_INVISIBLE)
	{
		// Notify the player that user doesn't exist
		m_bufferSend.Clear();
		m_bufferSend << CHAT_CMD_IM_FAILED << WStringToUTF8(sTarget) << byte(0);
		Send(m_bufferSend);
		return true;
	}

	// Setup a packet to send to the recipient
	m_bufferSend.Clear();
	m_bufferSend << CHAT_CMD_IM << m_sNameUTF8 << byte(0) << WStringToUTF8(sMessage.substr(0, CHAT_MESSAGE_MAX_LENGTH)) << byte(0);
	pTarget->Send(m_bufferSend);

	return true;
}


/*====================
  CClient::HandleRequestBuddyAdd
  ====================*/
bool	CClient::HandleRequestBuddyAdd(CPacket &pkt)
{
	PROFILE("CClient::HandleRequestBuddyAdd")

	const string sBuddyNickName(pkt.ReadUTF8String());	
	
	if (!IncrementRequestCounter())
		return true;	
	
	CHTTPRequest *pRequest(m_pHTTPManager->SpawnRequest());
	if (pRequest == NULL)
		return false;

	pRequest->Reset();
	pRequest->SetType(CLIENT_REQUEST_BUDDY_ADD);
	pRequest->SetTargetURL(net_masterServerAddress.Get() + CLIENT_REQUEST_SCRIPT);
	pRequest->AddVariable(L"f", L"new_buddy2");
	pRequest->AddVariable(L"account_id", m_uiAccountID);	
	pRequest->AddVariable(L"buddynickname", sBuddyNickName);
	pRequest->AddVariable(L"cookie", m_sCookie);
	pRequest->SendPostRequest();
	m_lHTTPRequests.push_back(pRequest);
		
	return true;
}


/*====================
  CClient::HandleRequestBuddyAddApprove
  ====================*/
bool	CClient::HandleRequestBuddyAddApprove(CPacket &pkt)
{
	PROFILE("CClient::HandleRequestBuddyAddApprove")

	const string sBuddyNickName(pkt.ReadUTF8String());
	
	if (!IncrementRequestCounter())
		return true;	
	
	CHTTPRequest *pRequest(m_pHTTPManager->SpawnRequest());
	if (pRequest == NULL)
		return false;

	pRequest->Reset();
	pRequest->SetType(CLIENT_REQUEST_BUDDY_APPROVE);
	pRequest->SetTargetURL(net_masterServerAddress.Get() + CLIENT_REQUEST_SCRIPT);
	pRequest->AddVariable(L"f", L"approve_buddy2");
	pRequest->AddVariable(L"account_id", m_uiAccountID);	
	pRequest->AddVariable(L"buddynickname", sBuddyNickName);
	pRequest->AddVariable(L"cookie", m_sCookie);
	pRequest->SendPostRequest();
	m_lHTTPRequests.push_back(pRequest);

	return true;
}


/*====================
  CClient::HandleBuddyRemoveNotification
  ====================*/
bool	CClient::HandleBuddyRemoveNotification(CPacket &pkt)
{
	PROFILE("CClient::HandleBuddyRemoveNotification")

	//uint uiTargetID(pkt.ReadInt());
	//uint uiNotifyRequester(pkt.ReadInt());
	//uint uiNotifyBuddy(pkt.ReadInt());

	// Remove the buddy from the user's buddy list
	//m_setBuddies.erase(uiTargetID);

	// Retrieve the notification messages
	//VerifyNotification(uiNotifyRequester);

	//CClient *pTarget(m_pClientManager->GetClientFromAccountID(uiTargetID));
	//if (pTarget != NULL)
		//pTarget->VerifyNotification(uiNotifyBuddy);

	return true;
}


/*====================
  CClient::HandleJoiningGame
  ====================*/
bool	CClient::HandleJoiningGame(CPacket &pkt)
{
	PROFILE("CClient::HandleJoiningGame")

	wstring sAddressPort(pkt.ReadWString());
	
	ClearCurrentMatch();
	SetServerAddressPort(sAddressPort);
	SetStatus(CLIENT_STATUS_JOINING_GAME);
	
	if (!client_stayInGeneralChannels)
		LeaveAllChannels(CHANNEL_FLAG_GENERAL_USE);

	UpdateStatus();
	
	m_pConsole->Std() << L"User " << m_sName << L" joining game, IP: " << sAddressPort << newl;

	return true;
}


/*====================
  CClient::HandleJoinedGame
  ====================*/
bool	CClient::HandleJoinedGame(CPacket &pkt)
{
	PROFILE("CClient::HandleJoinedGame")

	SetServerName(pkt.ReadWString());
	SetMatchID(pkt.ReadInt(-1));

	if (m_eStatus < CLIENT_STATUS_IN_GAME)
		m_pClientManager->IncrementInGameCount();

	SetStatus(CLIENT_STATUS_IN_GAME);

	// Clear any old match channels
	LeaveAllChannels(CHANNEL_FLAG_SERVER);

	// Join new match channel
	if (m_uiMatchID != -1)
	{
		CChannel *pChannel(m_pChannelManager->CreateMatchChannel(m_uiMatchID));
		if (pChannel != NULL)
			pChannel->AddClient(this, true);
	}
	
	UpdateStatus();
	
	m_pConsole->Std() << L"User " << m_sName << L" finished joining game '" << m_sGameName << L"', match ID " << m_uiMatchID << L"." << newl;
	
	return true;
}


/*====================
  CClient::HandleLeftGame
  ====================*/
bool	CClient::HandleLeftGame()
{
	PROFILE("CClient::HandleLeftGame");

	if (m_eStatus >= CLIENT_STATUS_IN_GAME)
		m_pClientManager->DecrementInGameCount();

	SetStatus(CLIENT_STATUS_CONNECTED);
	ClearCurrentMatch();

	// don't connect to any default channels because this player wants to remain invisible to others
	if (GetChatModeType() != CHAT_MODE_INVISIBLE)
	{
		CChannel *pChannel(m_pChannelManager->GetDefaultChannel(this));
		if (pChannel != NULL)
			pChannel->AddClient(this);
	}	
	
	UpdateStatus();
	
	m_pConsole->Std() << L"User " << m_sName << L" left the game '" << m_sGameName << L"'." << newl;
	
	return true;
}


/*====================
  CClient::HandleClanWhisper
  ====================*/
bool	CClient::HandleClanWhisper(CPacket &pkt)
{
	PROFILE("CClient::HandleClanWhisper");

	wstring sMessage(pkt.ReadWString());

	if (!IncrementRequestCounter())
		return true;

	m_pConsole->Chatter() << m_sName << L" -> Clan: " << sMessage << newl;

	CClan *pClan(GetClan());
	if (pClan == NULL)
	{
		// Notify the player that he's not in a clan
		m_bufferSend.Clear();
		m_bufferSend << CHAT_CMD_CLAN_WHISPER_FAILED;
		Send(m_bufferSend);
		return true;
	}

	// Setup a packet to send to the recipients
	m_bufferSend.Clear();
	m_bufferSend << CHAT_CMD_CLAN_WHISPER << m_uiAccountID << TStringToUTF8(sMessage.substr(0, CHAT_MESSAGE_MAX_LENGTH)) << byte(0);
	pClan->Broadcast(m_bufferSend, this);
	return true;
}


/*====================
  CClient::HandleClanPromoteNotification
  ====================*/
bool	CClient::HandleClanPromoteNotification(CPacket &pkt)
{
	PROFILE("CClient::HandleClanPromoteNotification")

	uint uiTargetID(pkt.ReadInt());

	if (m_pClan == NULL || !HasAllFlags(CLIENT_IS_CLAN_LEADER))
		return true;

	CClient *pTarget(m_pClientManager->GetClientFromAccountID(uiTargetID));
	if (pTarget == NULL || pTarget->GetClan() != m_pClan)
		return true;

	// Upgrade the user's permissions
	pTarget->AddFlags(CLIENT_IS_OFFICER);
	
	m_bufferSend.Clear();
	m_bufferSend << CHAT_CMD_CLAN_RANK_CHANGE << pTarget->GetAccountID() << byte(CLAN_RANK_OFFICER) << m_uiAccountID;
	m_pClan->Broadcast(m_bufferSend);
	
	return true;
}


/*====================
  CClient::HandleClanRemoveNotification
  ====================*/
bool	CClient::HandleClanRemoveNotification(CPacket &pkt)
{
	PROFILE("CClient::HandleClanRemoveNotification")

	uint uiTargetID(pkt.ReadInt());

	if (m_pClan == NULL || (m_uiAccountID != uiTargetID && !HasAllFlags(CLIENT_IS_CLAN_LEADER)))
		return true;

	CClient *pTarget(m_pClientManager->GetClientFromAccountID(uiTargetID));
	if (pTarget == NULL || pTarget->GetClan() != m_pClan)
		return true;

	m_bufferSend.Clear();
	m_bufferSend << CHAT_CMD_CLAN_RANK_CHANGE << pTarget->GetAccountID() << byte(CLAN_RANK_NONE) << m_uiAccountID;
	m_pClan->Broadcast(m_bufferSend);

	pTarget->SetClan(NULL);

	m_bufferSend.Clear();
	m_bufferSend << CHAT_CMD_NAME_CHANGE << pTarget->GetAccountID() << pTarget->GetNameUTF8() << byte(0);
	pTarget->SendToPeers(m_bufferSend);

	return true;
}


/*====================
  CClient::HandleClanDemoteNotification
  ====================*/
bool	CClient::HandleClanDemoteNotification(CPacket &pkt)
{
	PROFILE("CClient::HandleClanDemoteNotification")

	uint uiTargetID(pkt.ReadInt());

	if (m_pClan == NULL || !HasAllFlags(CLIENT_IS_CLAN_LEADER))
		return true;

	CClient *pTarget(m_pClientManager->GetClientFromAccountID(uiTargetID));
	if (pTarget == NULL || pTarget->GetClan() != m_pClan)
		return true;

	// Degrade the user's permissions
	pTarget->RemoveFlags(CLIENT_IS_OFFICER);
	
	m_bufferSend.Clear();
	m_bufferSend << CHAT_CMD_CLAN_RANK_CHANGE << pTarget->GetAccountID() << byte(CLAN_RANK_MEMBER) << m_uiAccountID;
	m_pClan->Broadcast(m_bufferSend);

	return true;
}


/*====================
  CClient::HandleLookingForClan
  ====================*/
bool	CClient::HandleLookingForClan()
{
	PROFILE("CClient::HandleLookingForClan")

	if (HasAllFlags(CLIENT_LOOKING_FOR_CLAN) || HasClan())
		return true;

	AddFlags(CLIENT_LOOKING_FOR_CLAN);

	// Notify all clan leaders of the update
	m_bufferSend.Clear();
	m_bufferSend << CHAT_CMD_LOOKING_FOR_CLAN << m_sNameUTF8 << byte(0);

	m_pClientManager->Broadcast(m_bufferSend, CLIENT_IS_CLAN_LEADER);
	return true;
}


/*====================
  CClient::HandleNotLookingForClan
  ====================*/
bool	CClient::HandleNotLookingForClan()
{
	PROFILE("CClient::HandleNotLookingForClan")

	if (!HasAllFlags(CLIENT_LOOKING_FOR_CLAN) || HasClan())
		return true;

	RemoveFlags(CLIENT_LOOKING_FOR_CLAN);

	// Notify all clan leaders of the update
	m_bufferSend.Clear();
	m_bufferSend << CHAT_CMD_NOT_LOOKING_FOR_CLAN << m_sNameUTF8 << byte(0);

	m_pClientManager->Broadcast(m_bufferSend, CLIENT_IS_CLAN_LEADER);
	return true;
}


/*====================
  CClient::HandleJoinChannel
  ====================*/
bool	CClient::HandleJoinChannel(CPacket &pkt)
{
	PROFILE("CClient::HandleJoinChannel")

	wstring sChannelName(pkt.ReadWString());

	if (sChannelName.empty())
		return true;

	m_bufferSend.Clear();

	if (GetNumChannels() >= client_maxChannels)
	{
		// User can only join MAX_CHANNELS at a time
		m_bufferSend << CHAT_CMD_MAX_CHANNELS;
		Send(m_bufferSend);
		return true;
	}

	CChannel *pChannel(m_pChannelManager->GetChannel(sChannelName));
	if (pChannel == NULL)
		pChannel = m_pChannelManager->CreatePublicChannel(sChannelName);
	if (pChannel == NULL)
		return true;

	if (pChannel->HasPassword() && !pChannel->IsAdmin(this))
	{
		m_bufferSend << CHAT_CMD_JOIN_CHANNEL_PASSWORD << pChannel->GetNameUTF8() << byte(0);
		Send(m_bufferSend);
		return true;
	}

	pChannel->AddClient(this);
	return true;
}


/*====================
  CClient::HandleWhisperBuddies
  ====================*/
bool	CClient::HandleWhisperBuddies(CPacket &pkt)
{
	PROFILE("CClient::HandleWhisperBuddies")

	wstring sMessage(pkt.ReadWString());

	if (!IncrementRequestCounter())
		return false;

	if (sMessage.empty())
		return true;

	m_pConsole->Chatter() << m_sName << L" -> Buddies: " << sMessage << newl;

	for (uiset_it it(m_setBuddies.begin()); it != m_setBuddies.end(); ++it)
	{
		CClient *pTarget(m_pClientManager->GetClientFromAccountID(*it));
		if (pTarget == NULL)
			continue;
			
		if (pTarget->GetChatModeType() == CHAT_MODE_AFK)
		{
			// Even though this user is AFK, still pass the message to the receiver but don't notify the sender that they are AFK, too spammy
		}
		else if (pTarget->GetChatModeType() == CHAT_MODE_DND)
		{
			// The user is in DND, do not pass pass the message to the receiver and don't notify the sender that they are DND either, too spammy
			continue;
		}			
		
		// Setup a packet to send to the recipients
		m_bufferSend.Clear();
		m_bufferSend << CHAT_CMD_WHISPER_BUDDIES << m_sNameUTF8 << byte(0) << WStringToUTF8(sMessage.substr(0, CHAT_MESSAGE_MAX_LENGTH)) << byte(0);
				
		pTarget->Send(m_bufferSend);
	}

	return true;
}


/*====================
  CClient::HandleLeaveChannel
  ====================*/
bool	CClient::HandleLeaveChannel(CPacket &pkt)
{
	PROFILE("CClient::HandleLeaveChannel")

	wstring sChannel(pkt.ReadWString());

	if (sChannel.empty())
		return true;

	CChannel *pChannel(m_pChannelManager->GetChannel(sChannel));
	if (pChannel == NULL)
		return true;

	pChannel->RemoveClient(this);
	return true;
}


/*====================
  CClient::HandleInviteToServer
  ====================*/
bool	CClient::HandleInviteToServer(CClient *pTarget)
{
	PROFILE("CClient::HandleInviteToServer")

	m_bufferSend.Clear();

	if (pTarget == NULL || pTarget == this || pTarget->GetStatus() < CLIENT_STATUS_CONNECTED)
	{
		// Notify the player that user doesn't exist
		m_bufferSend << CHAT_CMD_INVITE_FAILED_USER;
		Send(m_bufferSend);
		return true;
	}

	if (m_eStatus < CLIENT_STATUS_JOINING_GAME)
	{
		// Notify the player that they cannot invite from their current state
		m_bufferSend << CHAT_CMD_INVITE_FAILED_GAME;
		Send(m_bufferSend);
		return true;
	}

	m_bufferSend << CHAT_CMD_INVITED_TO_SERVER << m_sNameUTF8 << byte(0) << m_uiAccountID << m_sServerAddressPort << byte(0);
	pTarget->Send(m_bufferSend);

	return true;
}


/*====================
  CClient::SpawnMatchMakerInfoRequest
  ====================*/
CHTTPRequest*	CClient::SpawnMatchMakerInfoRequest()
{
	CHTTPRequest *pRequest(m_pHTTPManager->SpawnRequest());
	if (pRequest == NULL)
		return NULL;

	pRequest->SetTargetURL(net_masterServerAddress.Get() + CLIENT_REQUEST_SCRIPT);

	pRequest->AddVariable(L"f", L"get_field_stats");
	pRequest->AddVariable(L"field[]", L"rnk_amm_solo_rating");
	pRequest->AddVariable(L"field[]", L"rnk_amm_team_rating");
	pRequest->AddVariable(L"field[]", L"rnk_amm_solo_count");
	pRequest->AddVariable(L"field[]", L"rnk_amm_team_count");
	pRequest->AddVariable(L"field[]", L"acc_games_played");
	pRequest->AddVariable(L"field[]", L"rnk_games_played");
	pRequest->AddVariable(L"field[]", L"acc_discos");
	pRequest->AddVariable(L"field[]", L"rnk_discos");
	pRequest->AddVariable(L"field[]", L"rnk_amm_solo_pset");
	pRequest->AddVariable(L"field[]", L"rnk_amm_team_pset");
	pRequest->AddVariable(L"account_id[]", m_uiAccountID);

	pRequest->SendPostRequest();
	m_lHTTPRequests.push_back(pRequest);

	return pRequest;
}


/*====================
  CClient::HandleInviteIDToServer
  ====================*/
bool	CClient::HandleInviteIDToServer(CPacket &pkt)
{
	PROFILE("CClient::HandleInviteIDToServer")

	uint uiAccountID(pkt.ReadInt());
	return HandleInviteToServer(m_pClientManager->GetClientFromAccountID(uiAccountID));
}


/*====================
  CClient::HandleInviteNameToServer
  ====================*/
bool	CClient::HandleInviteNameToServer(CPacket &pkt)
{
	PROFILE("CClient::HandleInviteNameToServer")

	wstring sName(pkt.ReadWString());
	return HandleInviteToServer(m_pClientManager->GetClientFromName(LowerString(sName)));
}


/*====================
  CClient::HandleInviteRejected
  ====================*/
bool	CClient::HandleInviteRejected(CPacket &pkt)
{
	PROFILE("CClient::HandleInviteRejected")

	int iAccountID(pkt.ReadInt());
	CClient *pTarget(m_pClientManager->GetClientFromAccountID(iAccountID));

	if (pTarget == NULL || pTarget == this || pTarget->GetStatus() < CLIENT_STATUS_CONNECTED)
		return true;

	m_bufferSend.Clear();
	m_bufferSend << CHAT_CMD_INVITE_REJECTED << m_sNameUTF8 << byte(0) << m_uiAccountID;
	pTarget->Send(m_bufferSend);
	return true;
}


/*====================
  CClient::HandleUserInfo
  ====================*/
bool	CClient::HandleUserInfo(CPacket &pkt)
{
	PROFILE("CClient::HandleUserInfo")

	string sNameUTF8(pkt.ReadUTF8String());

	if (!IncrementRequestCounter())
		return true;

	CClient *pTarget(m_pClientManager->GetClientFromName(LowerString(UTF8ToWString(sNameUTF8))));

	m_bufferSend.Clear();

	// Notify the player that user doesn't exist
	if (pTarget == NULL || pTarget->GetChatModeType() == CHAT_MODE_INVISIBLE)
	{
		m_bufferSend << CHAT_CMD_USER_INFO_NO_EXIST << sNameUTF8 << byte(0);
		Send(m_bufferSend);
		return true;
	}

	switch (pTarget->GetStatus())
	{
	case CLIENT_STATUS_DISCONNECTED:
		//m_bufferSend << CHAT_CMD_USER_INFO_OFFLINE << pTarget->GetNameUTF8() << byte(0) << pTarget->GetLastOnlineUTF8() << byte(0);
		break;

	case CLIENT_STATUS_CONNECTED:
		{
			uiset setChannels;
			pTarget->AddChannelsToSet(setChannels, this);

			m_bufferSend << CHAT_CMD_USER_INFO_ONLINE << pTarget->GetNameUTF8() << byte(0) << INT_SIZE(setChannels.size());
			for (uiset_it it(setChannels.begin()), itEnd(setChannels.end()); it != itEnd; ++it)
			{
				CChannel *pChannel(m_pChannelManager->GetChannel(*it));
				m_bufferSend << pChannel->GetNameUTF8() << byte(0);
			}
			break;
		}

	case CLIENT_STATUS_JOINING_GAME:
		m_bufferSend << CHAT_CMD_USER_INFO_IN_GAME << pTarget->GetNameUTF8() << byte(0) << pTarget->GetServerNameUTF8() << byte(0);
		break;

	case CLIENT_STATUS_IN_GAME:
		{
			CHTTPRequest *pRequest(m_pHTTPManager->SpawnRequest());
			if (pRequest == NULL)
				return false;

			pRequest->Reset();
			pRequest->SetType(CLIENT_REQUEST_USER_INFO_IN_GAME);
			pRequest->SetTargetURL(net_masterServerAddress.Get() + CLIENT_REQUEST_SCRIPT);
			pRequest->AddVariable(L"f", L"get_cgt");
			pRequest->AddVariable(L"nickname", pTarget->GetNameUTF8());
			pRequest->AddVariable(L"match_id", XtoS(pTarget->GetMatchID()));
			pRequest->SendPostRequest();
			m_lHTTPRequests.push_back(pRequest);
			break;
		}
		
	}

	Send(m_bufferSend);
	return true;
}


/*====================
  CClient::HandleChannelTopic
  ====================*/
bool	CClient::HandleChannelTopic(CPacket &pkt)
{
	PROFILE("CClient::HandleChannelTopic")

	uint uiChannelID(pkt.ReadInt());
	wstring sTopic(pkt.ReadWString());

	CChannel *pChannel(m_pChannelManager->GetChannel(uiChannelID));
	if (pChannel != NULL && pChannel->IsAdmin(this, CHAT_CLIENT_ADMIN_OFFICER))
		pChannel->SetTopic(sTopic);

	return true;
}


/*====================
  CClient::HandleChannelKick
  ====================*/
bool	CClient::HandleChannelKick(CPacket &pkt)
{
	PROFILE("CClient::HandleChannelKick")

	uint uiChannelID(pkt.ReadInt());
	uint uiAccountID(pkt.ReadInt());

	CChannel *pChannel(m_pChannelManager->GetChannel(uiChannelID));
	if (pChannel != NULL)
		pChannel->Kick(this, m_pClientManager->GetClientFromAccountID(uiAccountID));

	return true;
}


/*====================
  CClient::HandleChannelBan
  ====================*/
bool	CClient::HandleChannelBan(CPacket &pkt)
{
	PROFILE("CClient::HandleChannelBan")

	uint uiChannelID(pkt.ReadInt());
	wstring sName(pkt.ReadWString());

	CChannel *pChannel(m_pChannelManager->GetChannel(uiChannelID));
	if (pChannel != NULL)
		pChannel->Ban(this, m_pClientManager->GetClientFromName(LowerString(sName)));		

	return true;
}


/*====================
  CClient::HandleChannelUnban
  ====================*/
bool	CClient::HandleChannelUnban(CPacket &pkt)
{
	PROFILE("CClient::HandleChannelUnban")

	uint uiChannelID(pkt.ReadInt());
	wstring sName(pkt.ReadWString());

	CChannel *pChannel(m_pChannelManager->GetChannel(uiChannelID));
	if (pChannel != NULL)
		pChannel->LiftBan(this, m_pClientManager->GetClientFromName(LowerString(sName)));		

	return true;
}


/*====================
  CClient::HandleChannelSilenceUser
  ====================*/
bool	CClient::HandleChannelSilenceUser(CPacket &pkt)
{
	PROFILE("CClient::HandleChannelSilenceUser")

	uint uiChannelID(pkt.ReadInt());
	wstring sName(pkt.ReadWString());
	uint uiDuration(pkt.ReadInt());

	CChannel *pChannel(m_pChannelManager->GetChannel(uiChannelID));
	if (pChannel == NULL)
		return true;

	pChannel->Silence(this, m_pClientManager->GetClientFromName(LowerString(sName)), uiDuration);
	return true;
}


/*====================
  CClient::HandleChannelPromote
  ====================*/
bool	CClient::HandleChannelPromote(CPacket &pkt)
{
	PROFILE("CClient::HandleChannelPromote")

	uint uiChannelID(pkt.ReadInt());
	uint uiAccountID(pkt.ReadInt());

	CChannel *pChannel(m_pChannelManager->GetChannel(uiChannelID));
	if (pChannel != NULL)
		pChannel->Promote(this, m_pClientManager->GetClientFromAccountID(uiAccountID));

	return true;
}


/*====================
  CClient::HandleChannelDemote
  ====================*/
bool	CClient::HandleChannelDemote(CPacket &pkt)
{
	PROFILE("CClient::HandleChannelDemote")

	uint uiChannelID(pkt.ReadInt());
	uint uiAccountID(pkt.ReadInt());

	CChannel *pChannel(m_pChannelManager->GetChannel(uiChannelID));
	if (pChannel != NULL)
		pChannel->Demote(this, m_pClientManager->GetClientFromAccountID(uiAccountID));

	return true;
}


/*====================
  CClient::HandleMessageAll
  ====================*/
bool	CClient::HandleMessageAll(CPacket &pkt)
{
	PROFILE("CClient::HandleMessageAll")

	wstring sMessage(pkt.ReadWString());
	if (pkt.HasFaults())
		m_pConsole->Std() << L"Bad packet in: CClient::HandleMessageAll" << newl;

	if (!HasAllFlags(CLIENT_IS_STAFF))
		return true;

	m_pConsole->Std() << m_sName << L" to ALL: " << sMessage << newl;

	m_bufferSend.Clear();
	m_bufferSend << CHAT_CMD_MESSAGE_ALL << m_sNameUTF8 << byte(0) << WStringToUTF8(sMessage) << byte(0);

	m_pClientManager->Broadcast(m_bufferSend);
	return true;
}


/*====================
  CClient::HandleChannelSetAuth
  ====================*/
bool	CClient::HandleChannelSetAuth(CPacket &pkt)
{
	PROFILE("CClient::HandleChannelSetAuth")

	uint uiChannelID(pkt.ReadInt(INVALID_CHANNEL));
	CChannel *pChannel(m_pChannelManager->GetChannel(uiChannelID));
	if (pChannel != NULL)
		pChannel->EnableAuth(this);

	return true;
}


/*====================
  CClient::HandleChannelRemoveAuth
  ====================*/
bool	CClient::HandleChannelRemoveAuth(CPacket &pkt)
{
	PROFILE("CClient::HandleChannelRemoveAuth")

	uint uiChannelID(pkt.ReadInt(INVALID_CHANNEL));
	CChannel *pChannel(m_pChannelManager->GetChannel(uiChannelID));
	if (pChannel != NULL)
		pChannel->DisableAuth(this);

	return true;
}


/*====================
  CClient::HandleChannelAddAuthUser
  ====================*/
bool	CClient::HandleChannelAddAuthUser(CPacket &pkt)
{
	PROFILE("CClient::HandleChannelAddAuthUser")

	uint uiChannelID(pkt.ReadInt(INVALID_CHANNEL));
	wstring sName(pkt.ReadWString());

	CChannel *pChannel(m_pChannelManager->GetChannel(uiChannelID));
	if (pChannel != NULL)
		pChannel->AddAuth(this, sName);

	return true;
}


/*====================
  CClient::HandleChannelRemoveAuthUser
  ====================*/
bool	CClient::HandleChannelRemoveAuthUser(CPacket &pkt)
{
	PROFILE("CClient::HandleChannelRemoveAuthUser")

	uint uiChannelID(pkt.ReadInt(INVALID_CHANNEL));
	wstring sName(pkt.ReadWString());

	CChannel *pChannel(m_pChannelManager->GetChannel(uiChannelID));
	if (pChannel != NULL)
		pChannel->RemoveAuth(this, sName);

	return true;
}


/*====================
  CClient::HandleChannelListAuth
  ====================*/
bool	CClient::HandleChannelListAuth(CPacket &pkt)
{
	PROFILE("CClient::HandleChannelListAuth")

	uint uiChannelID(pkt.ReadInt(INVALID_CHANNEL));
	CChannel *pChannel(m_pChannelManager->GetChannel(uiChannelID));
	if (pChannel != NULL)
		pChannel->SendAuthList(this);

	return true;
}


/*====================
  CClient::HandleChannelSetPassword
  ====================*/
bool	CClient::HandleChannelSetPassword(CPacket &pkt)
{
	PROFILE("CClient::HandleChannelSetPassword")

	uint uiChannelID(pkt.ReadInt());
	wstring sPassword(pkt.ReadWString());

	CChannel *pChannel(m_pChannelManager->GetChannel(uiChannelID));
	if (pChannel == NULL)
		return true;

	pChannel->SetPassword(this, sPassword);
	return true;
}


/*====================
  CClient::HandleChannelJoinPassword
  ====================*/
bool	CClient::HandleChannelJoinPassword(CPacket &pkt)
{
	PROFILE("CClient::HandleChannelJoinPassword")

	wstring sChannelName(pkt.ReadWString());
	wstring sPassword(pkt.ReadWString());

	CChannel *pChannel(m_pChannelManager->GetChannel(sChannelName));
	if (pChannel == NULL)
		pChannel = m_pChannelManager->CreatePublicChannel(sChannelName);
	if (pChannel == NULL)
		return true;

	pChannel->AddClient(this, false, sPassword);
	return true;
}


/*====================
  CClient::HandleRequestAutoMatch
  ====================*/
bool	CClient::HandleRequestAutoMatch(CPacket &pktRecv)
{
	wstring sRegionList(pktRecv.ReadWString());
	if (pktRecv.HasFaults())
		return false;

	if (matchmaker_disabled || (IsLeaver() && !matchmaker_allowLeavers))
	{
		m_bufferSend.Clear();
		m_bufferSend << NET_CHAT_CL_MATCHMAKER_REJECT << (matchmaker_disabled ? CHAT_MATCHMAKER_REJECT_DISABLED : CHAT_MATCHMAKER_REJECT_LEAVER);
		Send(m_bufferSend);
		return true;
	}

	m_uiRegionFlags = 0;
	wsvector vRegions(TokenizeString(sRegionList, L','));
	for (wsvector_it it(vRegions.begin()), itEnd(vRegions.end()); it != itEnd; ++it)
		m_uiRegionFlags |= m_pMatchMaker->GetRegionFlagFromName(*it);
	
	// Restring regions
	uint uiAllowedRegions(0);
	wsvector vAllowedRegions(TokenizeString(matchmaker_allowedRegions, L' '));
	for (wsvector_it it(vAllowedRegions.begin()), itEnd(vAllowedRegions.end()); it != itEnd; ++it)
		uiAllowedRegions |= m_pMatchMaker->GetRegionFlagFromName(*it);

	m_uiRegionFlags &= uiAllowedRegions;
	
	if (m_uiRegionFlags == 0)
	{
		m_bufferSend.Clear();
		m_bufferSend << NET_CHAT_CL_MATCHMAKER_REJECT << CHAT_MATCHMAKER_REJECT_NO_REGION;
		Send(m_bufferSend);
		return true;
	}

	CHTTPRequest *pRequest(SpawnMatchMakerInfoRequest());
	if (pRequest != NULL)
		pRequest->SetType(CLIENT_REQUEST_JOIN_MATCHMAKING_POOL);

	return true;
}


/*====================
  CClient::HandleCancelAutoMatch
  ====================*/
bool	CClient::HandleCancelAutoMatch(CPacket &pktRecv)
{
	SendMatchMakerUpdate(MATCHMAKER_STATUS_IDLE);
	if (m_pPendingMatch != NULL)
		m_pMatchMaker->IncrementCancelCounter(m_pPendingMatch->GetRegion());
	JoinMatch(NULL);
	return true;
}


/*====================
  CClient::HandleChatRoll
  ====================*/
bool	CClient::HandleChatRoll(CPacket &pktRecv)
{
	PROFILE("CClient::HandleChatRoll")

	string sMessage(pktRecv.ReadUTF8String());
	uint uiChannelID(pktRecv.ReadInt());

	if (!IncrementRequestCounter())
		return false;

	CChannel *pChannel(m_pChannelManager->GetChannel(uiChannelID));
	if (pChannel == NULL)
		return true;

	if (!pChannel->HasClient(this))
		return true;

	if (pChannel->IsSilenced(this))
	{
		m_pConsole->Std() << m_sName << _T(" is currently silenced.") << newl;

		m_bufferSend.Clear();
		m_bufferSend << CHAT_CMD_CHANNEL_SILENCED << uiChannelID;
		Send(m_bufferSend);
		return true;
	}

	if (sMessage.length() > CHAT_MESSAGE_MAX_LENGTH)
		sMessage.resize(CHAT_MESSAGE_MAX_LENGTH);

	m_pConsole->Chatter() << m_sName << L" -> Channel '" << pChannel->GetNameLower() << L"': " << sMessage << newl;

	// Setup a packet to send to all users in the channel
	m_bufferSend.Clear();
	m_bufferSend << CHAT_CMD_CHAT_ROLL << m_uiAccountID << uiChannelID << sMessage << byte(0);

	pChannel->Broadcast(m_bufferSend, this);

	return true;
}


/*====================
  CClient::HandleChatEmote
  ====================*/
bool	CClient::HandleChatEmote(CPacket &pktRecv)
{
	PROFILE("CClient::HandleChatEmote")

	string sMessage(pktRecv.ReadUTF8String());
	uint uiChannelID(pktRecv.ReadInt());

	if (!IncrementRequestCounter())
		return false;

	CChannel *pChannel(m_pChannelManager->GetChannel(uiChannelID));
	if (pChannel == NULL)
		return true;

	if (!pChannel->HasClient(this))
		return true;

	if (pChannel->IsSilenced(this))
	{
		m_pConsole->Std() << m_sName << _T(" is currently silenced.") << newl;

		m_bufferSend.Clear();
		m_bufferSend << CHAT_CMD_CHANNEL_SILENCED << uiChannelID;
		Send(m_bufferSend);
		return true;
	}

	if (sMessage.length() > CHAT_MESSAGE_MAX_LENGTH)
		sMessage.resize(CHAT_MESSAGE_MAX_LENGTH);

	m_pConsole->Chatter() << m_sName << L" -> Channel '" << pChannel->GetNameLower() << L"': " << sMessage << newl;

	// Setup a packet to send to all users in the channel
	m_bufferSend.Clear();
	m_bufferSend << CHAT_CMD_CHAT_EMOTE << m_uiAccountID << uiChannelID << sMessage << byte(0);

	pChannel->Broadcast(m_bufferSend, this);

	return true;
}


/*====================
  CClient::HandleSetChatModeType
  ====================*/
bool	CClient::HandleSetChatModeType(CPacket &pktRecv)
{
	PROFILE("CClient::HandleSetChatModeType");

	uint uiChatModeType(pktRecv.ReadInt());
	wstring sReason(pktRecv.ReadWString());

	if (!IncrementRequestCounter())
		return false;
				
	m_uiChatModeType = uiChatModeType;
	m_sChatModeTypeReason = sReason;

	// Setup a packet to send back to the client
	m_bufferSend.Clear();
	m_bufferSend << CHAT_CMD_SET_CHAT_MODE_TYPE << m_uiChatModeType << WStringToUTF8(m_sChatModeTypeReason) << byte(0);
	Send(m_bufferSend);
	
	return true;
}


/*====================
  CClient::HandleRequestMatchmakingInfo
  ====================*/
bool	CClient::HandleRequestMatchmakingInfo(CPacket &pktRecv)
{
	PROFILE("CClient::HandleRequestMatchmakingInfo");

	CHTTPRequest *pRequest(SpawnMatchMakerInfoRequest());
	if (pRequest != NULL)
		pRequest->SetType(CLIENT_REQUEST_UPDATE_RATING);

	return true;
}


/*====================
  CClient::HandleChannelSublist
  ====================*/
bool	CClient::HandleChannelSublist(CPacket &pkt)
{
	byte ySequence(pkt.ReadByte());
	wstring sSublistHead(pkt.ReadWString());

	if (sSublistHead.length() > 32)
		return true;

	m_bChannelListWait = false;
	m_uiChannelListIndex = 0;
	m_bChannelListSub = true;
	m_sChannelListHead = LowerString(sSublistHead);
	m_yChannelListSequence = ySequence;

	m_bufferSend.Clear();
	m_bufferSend << NET_CHAT_CL_CHANNEL_SUBLIST_START << m_yChannelListSequence << WStringToUTF8(m_sChannelListHead) << byte(0);
	Send(m_bufferSend);

	return true;
}


/*====================
  CClient::HandleChannelSublistAck
  ====================*/
bool	CClient::HandleChannelSublistAck(CPacket &pkt)
{
	byte ySequence(pkt.ReadByte());

	if (ySequence == m_yChannelListSequence)
		m_bChannelListWait = false;

	return true;
}


/*====================
  CClient::ProcessRequestAdminKick
  ====================*/
bool	CClient::ProcessRequestAdminKick(CPacket &pkt)
{
	wstring sName(pkt.ReadWString());
	if (pkt.HasFaults())
		return false;

	if (sName.empty() || !HasAllFlags(CLIENT_IS_STAFF))
		return true;

	CClient *pClient(m_pClientManager->GetClientFromName(sName));
	if (pClient == NULL)
		pClient = m_pClientManager->GetClientFromAccountID(AtoI(sName));
	
	if (pClient != NULL)
		m_pClientManager->RemoveClient(pClient);

	return true;
}


/*====================
  CClient::ReceiveWhisper
  ====================*/
void	CClient::ReceiveWhisper(const string &sSenderNameUTF8, const string &sMessageUTF8)
{
	PROFILE("CClient::ReceiveWhisper");

	m_pConsole->Chatter() << sSenderNameUTF8 << _T(" -> Whisper '") << m_sNameUTF8 << _T("': ") << sMessageUTF8 << newl;

	// Setup a packet to send to the recipient
	m_bufferSend.Clear();
	m_bufferSend << CHAT_CMD_WHISPER << sSenderNameUTF8 << byte(0) << sMessageUTF8 << byte(0);
	Send(m_bufferSend);
}


/*====================
  CClient::SendClanInvite
  ====================*/
void	CClient::SendClanInvite(CClient *pInviter)
{
	PROFILE("CClient::SendClanInvite");

	if (m_eStatus < CLIENT_STATUS_CONNECTED)
		return;

	CHTTPRequest *pRequest(m_pHTTPManager->SpawnRequest());
	if (pRequest != NULL)
	{
		pRequest->SetType(CLIENT_REQUEST_CLAN_ADD);
		pRequest->SetTargetURL(net_masterServerAddress.Get() + CLIENT_REQUEST_SCRIPT);

		pRequest->AddVariable(L"f", L"add_member");
		pRequest->AddVariable(L"member_ck", pInviter->GetCookie());
		pRequest->AddVariable(L"target_ck", m_sCookie);
		pRequest->AddVariable(L"clan_id", pInviter->GetClanID());

		pRequest->SendPostRequest();
		m_lHTTPRequests.push_back(pRequest);
	}
}


/*====================
  CClient::ProcessClanInviteResponse
  ====================*/
void	CClient::ProcessClanInviteResponse(const wstring &sResponse)
{
	PROFILE("CClient::ProcessClanInviteResponse");

	CPHPData phpResponse(sResponse);
	const CPHPData *pError(phpResponse.GetVar(L"error"));
	if (pError != NULL)
	{
		m_pClientManager->AddClanMemberFailed(this);
		return;
	}

	m_pClientManager->AddClanMemberSuccess(this);
}


/*====================
  CClient::CreateClan
  ====================*/
void	CClient::CreateClan(SClanCreate &clan)
{
	PROFILE("CClient::CreateClan");

	if (m_eStatus < CLIENT_STATUS_CONNECTED)
		return;

	string aCookies[4];
	for (int i(0); i < 4; ++i)
	{
		CClient *pClient(m_pClientManager->GetClientFromAccountID(clan.uiTarget[i]));
		if (pClient == NULL)
		{
			m_pClientManager->CreateClanFailed(clan.uiFounderAccountID);
			return;
		}

		aCookies[i] = pClient->GetCookie();
	}

	CHTTPRequest *pRequest(m_pHTTPManager->SpawnRequest());
	if (pRequest != NULL)
	{
		pRequest->SetType(CLIENT_REQUEST_CLAN_CREATE);
		pRequest->SetTargetURL(net_masterServerAddress.Get() + CLIENT_REQUEST_SCRIPT);

		pRequest->AddVariable(L"f", L"create_clan");
		pRequest->AddVariable(L"clan_name", clan.sClan);
		pRequest->AddVariable(L"tag", clan.sTag);
		pRequest->AddVariable(L"leader_ck", m_sCookie);
		for (int i(0); i < 4; ++i)
			pRequest->AddVariable(L"member" + XtoW(i + 1) + L"_ck", aCookies[i]);

		pRequest->SendPostRequest();
		m_lHTTPRequests.push_back(pRequest);
	}
}


/*====================
  CClient::ProcessCreateClanResponse
  ====================*/
void	CClient::ProcessCreateClanResponse(const wstring &sResponse)
{
	CPHPData phpResponse(sResponse);
	const CPHPData *pError(phpResponse.GetVar(L"error"));
	if (pError != NULL)
	{
		m_pClientManager->CreateClanFailed(m_uiAccountID);
		return;
	}

	if (CompareNoCase(phpResponse.GetString(L"create_clan"), L"OK") != 0 || phpResponse.GetString(L"clan_id").empty())
	{
		if (!phpResponse.GetString(L"clan_name").empty())
			m_pClientManager->CreateClanFailedClanName(m_uiAccountID);
		else if (!phpResponse.GetString(L"tag").empty())
			m_pClientManager->CreateClanFailedTag(m_uiAccountID);
		else
			m_pClientManager->CreateClanFailed(m_uiAccountID);

		return;
	}

	m_pClientManager->CreateClanSuccess(m_uiAccountID, phpResponse.GetInteger(L"clan_id"));
}


/*====================
  CClient::ProcessUserInfoInGameResponse
  ====================*/
void	CClient::ProcessUserInfoInGameResponse(const wstring &sResponse)
{
	CPHPData phpResponse(sResponse);
	const CPHPData *pError(phpResponse.GetVar(L"error"));
	if (pError != NULL)
	{
		return;
	}

	wstring sCGT(phpResponse.GetString(L"cgt"));
	wstring sName(phpResponse.GetString(L"nickname"));
	wstring sServerName(phpResponse.GetString(L"matchname"));

	m_bufferSend.Clear();
	m_bufferSend << CHAT_CMD_USER_INFO_IN_GAME << WStringToUTF8(sName) << byte(0) << WStringToUTF8(sServerName) << byte(0) << WStringToUTF8(sCGT) << byte(0);
	Send(m_bufferSend);
}


/*====================
  CClient::ProcessUpdateRatingResponse
  ====================*/
void	CClient::ProcessUpdateRatingResponse(const wstring &sResponse, bool bJoinMatchMaker)
{
	CPHPData phpResponse(sResponse);
	if (!phpResponse.GetBool(L"0"))
		return;

	const CPHPData *pStats(phpResponse.GetVar(L"field_stats"));
	if (pStats == NULL)
		return;
	pStats = pStats->GetVar(XtoW(m_uiAccountID));
	if (pStats == NULL)
		return;

	m_aInitialRatingSet[MATCH_POOL_SOLO] = pStats->GetBool(L"rnk_amm_solo_pset");
	m_aInitialRatingSet[MATCH_POOL_TEAM] = pStats->GetBool(L"rnk_amm_team_pset");
	m_aMatchCount[MATCH_POOL_SOLO] = pStats->GetInteger(L"rnk_amm_solo_count");
	m_aMatchCount[MATCH_POOL_TEAM] = pStats->GetInteger(L"rnk_amm_team_count");
	m_aRating[MATCH_POOL_SOLO] = pStats->GetInteger(L"rnk_amm_solo_rating");
	m_aRating[MATCH_POOL_TEAM] = pStats->GetInteger(L"rnk_amm_team_rating");
	m_iMatches = pStats->GetInteger(L"acc_games_played") + pStats->GetInteger(L"rnk_games_played");
	m_iDisconnects = pStats->GetInteger(L"acc_discos") + pStats->GetInteger(L"rnk_discos");

	m_bufferSend.Clear();
	m_bufferSend
		<< NET_CHAT_CL_MATCHMAKING_INFO
		<< byte(matchmaker_disabled)
		<< GetMatchCount(MATCH_POOL_SOLO)
		<< GetRating(MATCH_POOL_SOLO)
		<< GetMatchCount(MATCH_POOL_TEAM)
		<< GetRating(MATCH_POOL_TEAM)
		<< byte(m_aInitialRatingSet[MATCH_POOL_SOLO])
		<< byte(m_aInitialRatingSet[MATCH_POOL_TEAM]);
	Send(m_bufferSend);
	
	if (bJoinMatchMaker)
		m_pMatchMaker->AddClient(this, m_uiRegionFlags);
}


/*====================
  CClient::ProcessRequestBuddyAddResponse
  ====================*/
void	CClient::ProcessRequestBuddyAddResponse(const wstring &sResponse)
{
	CPHPData phpResponse(sResponse);

	// handle duplicate and ignored responses
	if (CompareNoCase(phpResponse.GetString(L"new_buddy"), L"IGN") == 0 && !phpResponse.GetString(L"buddy_nickname").empty())
	{
		m_pClientManager->RequestBuddyAddFail(4, m_uiAccountID, 0, phpResponse.GetString(L"buddy_nickname"));
		return;
	}
	else if (CompareNoCase(phpResponse.GetString(L"new_buddy"), L"DUP") == 0 && !phpResponse.GetString(L"buddy_nickname").empty())
	{
		m_pClientManager->RequestBuddyAddFail(3, m_uiAccountID, 0, phpResponse.GetString(L"buddy_nickname"));
		return;
	}

	if (phpResponse.GetVar(L"error") != NULL || CompareNoCase(phpResponse.GetString(L"new_buddy"), L"OK") != 0 || 
		phpResponse.GetString(L"account_nickname").empty() || phpResponse.GetInteger(L"account_notify_id") == 0 || 
		phpResponse.GetString(L"account_params").empty() || phpResponse.GetInteger(L"buddy_id") == 0 || 
		phpResponse.GetString(L"buddy_nickname").empty() ||	phpResponse.GetInteger(L"buddy_notify_id") == 0 ||
		phpResponse.GetString(L"buddy_params").empty()
		)
		m_pClientManager->RequestBuddyAddFail(0, m_uiAccountID, phpResponse.GetInteger(L"account_notify_id"), phpResponse.GetString(L"buddy_nickname"));
	else
		m_pClientManager->RequestBuddyAddSuccess(m_uiAccountID, phpResponse.GetString(L"account_nickname"), phpResponse.GetInteger(L"account_notify_id"), phpResponse.GetString(L"account_params"), phpResponse.GetInteger(L"buddy_id"), phpResponse.GetString(L"buddy_nickname"), phpResponse.GetInteger(L"buddy_notify_id"), phpResponse.GetString(L"buddy_params"));
}


/*====================
  CClient::ProcessRequestBuddyApproveResponse
  ====================*/
void	CClient::ProcessRequestBuddyApproveResponse(const wstring &sResponse)
{
	CPHPData phpResponse(sResponse);

	if (phpResponse.GetVar(L"error") != NULL || CompareNoCase(phpResponse.GetString(L"approve_buddy"), L"OK") != 0 || 
		phpResponse.GetString(L"account_nickname").empty() || phpResponse.GetInteger(L"account_notify_id") == 0 || 
		phpResponse.GetString(L"account_params").empty() || phpResponse.GetInteger(L"buddy_id") == 0 || 
		phpResponse.GetString(L"buddy_nickname").empty() ||	phpResponse.GetInteger(L"buddy_notify_id") == 0 ||
		phpResponse.GetString(L"buddy_params").empty()
		)
		m_pClientManager->RequestBuddyApproveFail(m_uiAccountID, phpResponse.GetString(L"account_nickname"), phpResponse.GetInteger(L"account_notify_id"), phpResponse.GetInteger(L"buddy_id"), phpResponse.GetString(L"buddy_nickname"), phpResponse.GetInteger(L"buddy_notify_id"));
	else
		m_pClientManager->RequestBuddyApproveSuccess(m_uiAccountID, phpResponse.GetString(L"account_nickname"), phpResponse.GetInteger(L"account_notify_id"), phpResponse.GetString(L"account_params"), phpResponse.GetInteger(L"buddy_id"), phpResponse.GetString(L"buddy_nickname"), phpResponse.GetInteger(L"buddy_notify_id"), phpResponse.GetString(L"buddy_params"));
}

/*====================
  CClient::HandleGetUserStatus
  ====================*/
bool CClient::HandleGetUserStatus( CPacket &pkt )
{
	wstring sName(pkt.ReadWString());
	CClient *pTarget(m_pClientManager->GetClientFromName(LowerString(sName)));
	if (pTarget == NULL)
	{
		m_bufferSend.Clear();
		m_bufferSend << NET_CHAT_CL_USER_STATUS << WStringToUTF8(sName) << byte(0) << byte(CLIENT_STATUS_DISCONNECTED);
		Send(m_bufferSend);
		return true;
	}

	m_bufferSend.Clear();
	m_bufferSend << NET_CHAT_CL_USER_STATUS << WStringToUTF8(sName) << byte(0) << pTarget->GetStatusByte();
	Send(m_bufferSend);

	return true;
}



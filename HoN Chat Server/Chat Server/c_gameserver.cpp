// (C)2010 S2 Games
// c_gameserver.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "chatserver_common.h"

#include "c_gameserver.h"
#include "c_gameservermanager.h"
#include "c_socket.h"
#include "c_console.h"
#include "c_match.h"
#include "c_clientmanager.h"
#include "c_client.h"
#include "c_matchmaker.h"
//=============================================================================

/*====================
  CGameServer::~CGameServer
  ====================*/
CGameServer::~CGameServer()
{
	Disconnect();

	delete m_pMatch;
}


/*====================
  CGameServer::CGameServer
  ====================*/
CGameServer::CGameServer(CCore *pCore, CSocket *pSocket, uint uiID) :
NULL_CORE_API,
m_uiID(uiID),
m_pSocket(pSocket),
m_uiLastReceiveTime(INVALID_TIME),
m_bSentPing(false),
m_uiServerID(INVALID_ACCOUNT_ID),
m_unPort(0),
m_yStatus(0),
m_pMatch(NULL),
m_bHasLocationIterator(false)
{
	INIT_CORE_API(pCore);
}


/*====================
  CGameServer::ProcessStatusUpdate
  ====================*/
bool	CGameServer::ProcessStatusUpdate(CPacket &pkt)
{
	m_uiServerID = pkt.ReadInt();
	m_sAddress = pkt.ReadString();
	m_unPort = pkt.ReadShort();
	m_sLocation = pkt.ReadWString();
	m_sName = pkt.ReadWString();
	m_yStatus = pkt.ReadByte();
	m_yArrangedType = pkt.ReadByte();	
	m_yOfficial = pkt.ReadByte();
	m_yNoLeaver = pkt.ReadByte();
	m_yPrivate = pkt.ReadByte();
	m_sMapName = pkt.ReadWString();
	m_yTier = pkt.ReadByte();
	m_sGameName = pkt.ReadWString();
	m_sGameModeName = pkt.ReadWString();
	m_yTeamSize = pkt.ReadByte();
	m_yEasyMode = pkt.ReadByte();
	m_yAllHeroes = pkt.ReadByte();
	m_yForceRandom = pkt.ReadByte();
	m_yAutoBalanced = pkt.ReadByte();
	m_yAdvancedOptions = pkt.ReadByte();
	m_unMinPSR = pkt.ReadShort();
	m_unMaxPSR = pkt.ReadShort();
	m_yDevHeroes = pkt.ReadByte();
	m_yHardcore = pkt.ReadByte();

	m_pConsole->Std()
		<< L"SERVER STATUS: #" << m_uiServerID
		<< L" " << m_sAddress << L":" << m_unPort
		<< L" region:" << m_sLocation
		<< L" name:\"" << m_sName << L"\""
		<< L" status:" << m_yStatus
		<< newl;

	m_pGameServerManager->UpdateServerLocation(this);
	m_pGameServerManager->UpdateGameServerInfo(this);

	if (m_yStatus < 1 || m_yStatus > 2)
	{
		delete m_pMatch;
		m_pMatch = NULL;
	}

	return !pkt.HasFaults();
}


/*====================
  CGameServer::ProcessAnnounceMatch
  ====================*/
bool	CGameServer::ProcessAnnounceMatch(CPacket &pkt)
{
	uint uiMatchID(pkt.ReadInt());
	uint uiChallenge(pkt.ReadInt());

	if (pkt.HasFaults())
		return false;

	if (m_pMatch != NULL && m_pMatch->Validate(uiMatchID, uiChallenge))
		m_pMatch->AnnounceMatchReady(m_sAddress, m_unPort);

	return true;
}


/*====================
  CGameServer::ProcessRemindPlayer
  ====================*/
bool	CGameServer::ProcessRemindPlayer(CPacket &pkt)
{
	uint uiAccountID(pkt.ReadInt());
	if (pkt.HasFaults())
		return false;

	if (m_pMatch == NULL)
		return true;

	CClient *pClient(m_pClientManager->GetClientFromAccountID(uiAccountID));
	if (pClient == NULL)
		return true;

	CBufferFixed<24> buffer;
	buffer << CHAT_CMD_AUTO_MATCH_CONNECT << m_pMatch->GetID() << StringToUTF8(m_sAddress) << byte(0) << m_unPort;
	pClient->Send(buffer);

	return true;
}


/*====================
  CGameServer::ProcessReplacePlayer
  ====================*/
bool	CGameServer::ProcessReplacePlayer(CPacket &pkt)
{
	// Read account id of player to be replaced
	uint uiAccountID(pkt.ReadInt());
	if (pkt.HasFaults())
		return false;

	// Ignore if this server has no active match
	if (m_pMatch == NULL)
		return true;

	// Make sure the missing client is no longer associated with this match
	CClient *pClient(m_pClientManager->GetClientFromAccountID(uiAccountID));
	if (pClient != NULL)
	{
		m_pMatch->InvalidateClient(pClient, L"replaced");
		pClient->LeaveMatch(m_pMatch);
	}

	// Request a viable substitute from the matchmaker
	CClient *pSubstitute(m_pMatchMaker->RequestSubstitute(m_pMatch, uiAccountID));
	if (pSubstitute == NULL)
		return true;

	// Place the substitute into this match and notify the game server of success
	m_pMatch->AddClient(pSubstitute);
	SendSubstitution(uiAccountID, pSubstitute->GetAccountID());

	return true;
}


/*====================
  CGameServer::ReadSocket
  ====================*/
bool	CGameServer::ReadSocket()
{
	PROFILE("CGameServer::ReadSocket");

	if (m_pSocket == NULL)
		return false;

	CPacket pktRecv;
	int iRecvLength(m_pSocket->ReceivePacket(pktRecv));
	if (iRecvLength < 1)
		return iRecvLength == K2_SOCKET_EMPTY;

	m_uiLastReceiveTime = m_pCore->GetSeconds();

	bool bConnectionOK(true);
	while (!pktRecv.DoneReading() && !pktRecv.HasFaults() && bConnectionOK)
	{
		ushort unCmd(pktRecv.ReadShort());

		switch (unCmd)
		{
		case NET_CHAT_PING:
			m_pktSend.Clear();
			m_pktSend << NET_CHAT_PONG;
			m_pSocket->SendPacket(m_pktSend);
			//m_pConsole->Admin() << L"PING - Game server #" << m_uiServerID << newl;
			break;

		case NET_CHAT_PONG:
			//m_pConsole->Admin() << L"PONG - Game server #" << m_uiServerID << newl;
			m_bSentPing = false;
			break;

		case NET_CHAT_GS_DISCONNECT:
			Disconnect();
			bConnectionOK = false;
			break;

		case NET_CHAT_GS_STATUS:
			bConnectionOK = ProcessStatusUpdate(pktRecv);
			break;

		case NET_CHAT_GS_ANNOUNCE_MATCH:
			bConnectionOK = ProcessAnnounceMatch(pktRecv);
			break;

		case NET_CHAT_GS_ABANDON_MATCH:
			{
				byte yFailed(pktRecv.ReadByte());

				if (m_pMatch != NULL)
				{
					if (yFailed)
						m_pMatchMaker->IncrementFailCounter();
					delete m_pMatch;
					m_pMatch = NULL;
				}

				bConnectionOK = !pktRecv.HasFaults();
			}
			break;

		case NET_CHAT_GS_MATCH_STARTED:
			break;

		case NET_CHAT_GS_REMIND_PLAYER:
			bConnectionOK = ProcessRemindPlayer(pktRecv);
			break;

		case NET_CHAT_GS_REPLACE_PLAYER:
			bConnectionOK = ProcessReplacePlayer(pktRecv);
			break;

		default:
			bConnectionOK = false;
			break;
		}
	}

	return bConnectionOK;
}


/*====================
  CGameServer::Disconnect
  ====================*/
void	CGameServer::Disconnect()
{
	delete m_pSocket;
	m_pSocket = NULL;

	m_pGameServerManager->RemoveGameServer(this);
}


/*====================
  CGameServer::SetOnline
  ====================*/
void	CGameServer::SetOnline()
{
	m_uiLastReceiveTime = m_pCore->GetSeconds();

	m_pktSend.Clear();
	m_pktSend << NET_CHAT_GS_ACCEPT;
	m_pSocket->SendPacket(m_pktSend);
}


/*====================
  CGameServer::SendPing
  ====================*/
void	CGameServer::SendPing()
{
	if (m_bSentPing)
		return;

	m_bSentPing = true;

	m_pktSend.Clear();
	m_pktSend << NET_CHAT_PING;
	m_pSocket->SendPacket(m_pktSend);
}


/*====================
  CGameServer::SendSubstitution
  ====================*/
void	CGameServer::SendSubstitution(uint uiOldClientID, uint uiNewClientID)
{
	m_pktSend.Clear();
	m_pktSend << NET_CHAT_GS_ROSTER_SUBSTITUTE << uiOldClientID << uiNewClientID;
	m_pSocket->SendPacket(m_pktSend);
}


/*====================
  CGameServer::CreateMatch
  ====================*/
void	CGameServer::CreateMatch(CMatch *pMatch)
{
	m_pMatch = pMatch;
	m_pMatch->Start();

	// Send request
	m_pktSend.Clear();
	m_pMatch->WriteMatchInfo(m_pktSend);

	m_pSocket->SendPacket(m_pktSend);

	m_pConsole->Std() << L"Requesting match creation #" << m_uiID << L" " << m_sAddress << L":" << m_unPort << newl;
}

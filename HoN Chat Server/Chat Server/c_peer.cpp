// (C)2009 S2 Games
// c_peer.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "chatserver_common.h"

#include "c_peer.h"
#include "c_peermanager.h"
#include "c_console.h"
#include "c_netmanager.h"
#include "c_channelmanager.h"
#include "c_clientmanager.h"
//=============================================================================

/*====================
  CPeer::~CPeer
  ====================*/
CPeer::~CPeer()
{
	SAFE_DELETE(m_pSocket);
}


/*====================
  CPeer::CPeer
  ====================*/
CPeer::CPeer(CCore *pCore) :
NULL_CORE_API,
m_pSocket(NULL),
m_eState(PEER_STATE_IDLE),
m_unPort(0),
m_uiID(INVALID_PEER_ID),
m_uiExpireTime(INVALID_TIME),

m_uiCurrentChannelInfo(0)
{
	INIT_CORE_API(pCore);
}


/*====================
  CPeer::Send
  ====================*/
void	CPeer::Send()
{
	PROFILE("CPeer::Send")

	if (m_pktSend.IsEmpty())
		return;

	if (m_pktSend.HasFaults())
	{
		m_pktSend.Clear();
		m_pConsole->Admin() << L"Overflow in send packet for peer: " << m_uiID << newl;
		return;
	}

	m_pSocket->SendPacket(m_pktSend);
	m_pktSend.Clear();
}


/*====================
  CPeer::Receive
  ====================*/
bool	CPeer::Receive()
{
	PROFILE("CPeer::Receive")

	int iReceived(0);
	for (;;)
	{
		iReceived = m_pSocket->ReceivePacket(m_pktRecv);
		if (iReceived < 1)
			return iReceived == K2_SOCKET_EMPTY;

		while (!m_pktRecv.DoneReading())
		{
			ushort unCmd(m_pktRecv.ReadShort());
			switch (unCmd)
			{

			case PEER_CMD_REQUEST_PEER_INFO:
				m_pConsole->Admin() << L"Received peer info request from peer #" << m_uiID << newl;
				m_eState = PEER_STATE_RECEIVING_PEER_INFO;
				m_pPeerManager->SendPeerList(this);
				break;

			case PEER_CMD_PEER_INFO:
				m_pPeerManager->UpdatePeer(m_pktRecv);
				break;

			case PEER_CMD_END_PEER_INFO:
				m_pktSend << PEER_CMD_REQUEST_CHANNEL_INFO;
				m_eState = PEER_STATE_RECEIVING_CHANNEL_INFO;
				break;

			case PEER_CMD_REQUEST_CHANNEL_INFO:
				m_pConsole->Admin() << L"Received channel info request from peer #" << m_uiID << newl;
				m_eState = PEER_STATE_RECEIVING_CHANNEL_INFO;
				break;

			case PEER_CMD_CHANNEL_INFO:
				m_pChannelManager->UpdateChannel(m_pktRecv);
				break;

			case PEER_CMD_END_CHANNEL_INFO:
				//m_pktSend << PEER_CMD_REQUEST_CLIENT_INFO;
				m_eState = PEER_STATE_RECEIVING_CLIENT_INFO;
				break;

			case PEER_CMD_CHANNEL_MESSAGE:
				{
				}
				break;

			case PEER_CMD_WHISPER:
				{
					string sSenderUTF8(m_pktRecv.ReadUTF8String());
					wstring sTarget(m_pktRecv.ReadWString());
					string sMessageUTF8(m_pktRecv.ReadUTF8String());

					CClient *pClient(m_pClientManager->GetClientFromName(LowerStringRef(sTarget)));
					if (pClient != NULL)
						pClient->ReceiveWhisper(sSenderUTF8, sMessageUTF8);
				}
				break;

			default:
				m_pConsole->Admin() << L"Invalid command from peer" << newl;
				return false;
			}

			if (m_pktRecv.HasFaults())
			{
				m_pConsole->Admin() << L"Bad packet from peer" << newl;
				return false;
			}
		}
	}

	return true;
}


/*====================
  CPeer::ConnectingFrame
  ====================*/
bool	CPeer::ConnectingFrame()
{
	PROFILE("CPeer::ConnectingFrame")

	if (m_pCore->GetSeconds() >= m_uiExpireTime)
	{
		Disconnect();
		return false;
	}

	// Wait for connection to establish
	if (!m_pSocket->IsConnected())
		return true;

	// Send "Hello"
	m_pktSend.Clear();
	m_pktSend << PEER_CMD_HELLO << m_pNetManager->GetPublicAddress() << m_pNetManager->GetListenPort() << m_pPeerManager->GetKey() << GetID();
	m_pSocket->SendPacket(m_pktSend);
	m_pktSend.Clear();
	m_eState = PEER_STATE_AUTHORIZING;
	return true;
}


/*====================
  CPeer::AuthorizingFrame
  ====================*/
bool	CPeer::AuthorizingFrame()
{
	PROFILE("CPeer::AuthorizingFrame")

	int iRecvLength(m_pSocket->ReceivePacket(m_pktRecv));
	if (iRecvLength < 1)
		return iRecvLength == K2_SOCKET_EMPTY;

	ushort unCmd(m_pktRecv.ReadShort());
	if (m_pktRecv.HasFaults())
		return false;

	if (unCmd == PEER_CMD_WELCOME)
	{
		uint uiYourID(m_pktRecv.ReadInt());
		uint uiMyID(m_pktRecv.ReadInt());
		m_uiID = uiMyID;
		m_eState = PEER_STATE_RECEIVING_PEER_INFO;
		m_pPeerManager->Connected(uiYourID, this);

		m_pktSend << PEER_CMD_REQUEST_PEER_INFO;
		m_pConsole->Admin() << L"Sending peer info request." << newl;
	}
	else if (unCmd == PEER_CMD_GO_AWAY)
	{
		Disconnect();
		return false;
	}

	return true;
}


/*====================
  CPeer::ConnectedFrame
  ====================*/
bool	CPeer::ConnectedFrame()
{
	PROFILE("CPeer::ConnectedFrame")

	if (!Receive())
	{
		Disconnect();
		return false;
	}

	Send();
	return true;
}


/*====================
  CPeer::ChannelInfoFrame
  ====================*/
bool	CPeer::ChannelInfoFrame()
{
	PROFILE("CPeer::ChannelInfoFrame")

	if (!Receive())
	{
		Disconnect();
		return false;
	}

	m_uiCurrentChannelInfo = m_pChannelManager->AddNextChannelInfo(m_uiCurrentChannelInfo, m_pktSend);

	Send();
	return true;
}


/*====================
  CPeer::Frame
  ====================*/
bool	CPeer::Frame()
{
	PROFILE("CPeer::Frame")

	switch (m_eState)
	{
	case PEER_STATE_IDLE:					return true;
	case PEER_STATE_DISCONNECTED:			m_pPeerManager->Connect(this); return true;
	case PEER_STATE_CONNECTING:				return ConnectingFrame();
	case PEER_STATE_AUTHORIZING:			return AuthorizingFrame();
	case PEER_STATE_RECEIVING_PEER_INFO:	return ConnectedFrame();
	case PEER_STATE_RECEIVING_CHANNEL_INFO:	return ConnectedFrame();
	case PEER_STATE_RECEIVING_CLIENT_INFO:	return ConnectedFrame();
	case PEER_STATE_CONNECTED:				return ConnectedFrame();
	}

	return false;
}


/*====================
  CPeer::Disconnect
  ====================*/
void	CPeer::Disconnect()
{
	SAFE_DELETE(m_pSocket);
	m_eState = PEER_STATE_IDLE;

	m_pConsole->Admin() << L"Peer #" << m_uiID << L" disconnected." << newl;
}


/*====================
  CPeer::SendWelcome
  ====================*/
void	CPeer::SendWelcome(uint uiLocalID)
{
	m_pktSend << PEER_CMD_WELCOME << m_uiID << uiLocalID;
	m_pConsole->Admin() << L"Sending welcome to peer #" << m_uiID << newl;
}


/*====================
  CPeer::SendPeerInfo
  ====================*/
void	CPeer::SendPeerInfo(CPeer *pPeer)
{
	if (pPeer == NULL)
		return;

	m_pktSend << PEER_CMD_PEER_INFO << pPeer->GetID() << pPeer->GetAddress() << pPeer->GetPort();
	m_pConsole->Admin() << L"Sending info to peer #" << m_uiID << L" for peer #" << pPeer->GetID() << newl;
}

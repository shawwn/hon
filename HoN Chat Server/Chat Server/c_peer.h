// (C)2009 S2 Games
// c_peer.h
//
//=============================================================================
#ifndef __C_PEER_H__
#define __C_PEER_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_packet.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CSocket;
class CPeerManager;
class CClientManager;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
enum EPeerState
{
	PEER_STATE_IDLE,
	PEER_STATE_DISCONNECTED,
	PEER_STATE_CONNECTING,
	PEER_STATE_AUTHORIZING,
	PEER_STATE_RECEIVING_PEER_INFO,
	PEER_STATE_RECEIVING_CHANNEL_INFO,
	PEER_STATE_RECEIVING_CLIENT_INFO,
	PEER_STATE_CONNECTED
};

const uint INVALID_PEER_ID(-1);
//=============================================================================

//=============================================================================
// CPeer
//=============================================================================
class CPeer
{
private:
	DECLARE_CORE_API;

	CSocket*		m_pSocket;
	CPacket			m_pktSend;
	CPacket			m_pktRecv;

	EPeerState		m_eState;
	string			m_sAddress;
	ushort			m_unPort;
	uint			m_uiID;
	string			m_sKey;

	uint			m_uiExpireTime;

	uint			m_uiCurrentChannelInfo;
	CPeer();

	void	Send();
	bool	Receive();

	bool	ConnectingFrame();
	bool	AuthorizingFrame();
	bool	ChannelInfoFrame();
	bool	ConnectedFrame();

public:
	~CPeer();
	CPeer(CCore *pCore);

	inline void				SetSocket(CSocket *pSocket)	{ m_pSocket = pSocket; }

	inline CPacket&			GetSendPacket()						{ return m_pktSend; }

	inline void				SetAddress(const string &sAddress)	{ m_sAddress = sAddress; }
	inline const string&	GetAddress() const					{ return m_sAddress; }
	
	inline void				SetPort(ushort unPort)				{ m_unPort = unPort; }
	inline ushort			GetPort() const						{ return m_unPort; }

	inline void				SetKey(const string &sKey)			{ m_sKey = sKey; }
	inline const string&	GetKey() const						{ return m_sKey; }

	inline void				SetID(uint uiID)					{ m_uiID = uiID; }
	inline uint				GetID() const						{ return m_uiID; }

	inline void				SetState(EPeerState eState)			{ m_eState = eState; }
	inline EPeerState		GetState() const					{ return m_eState; }

	inline void				SetExpireTime(uint uiTime)			{ m_uiExpireTime = uiTime; }

	bool					Frame();
	void					Disconnect();

	void					SendWelcome(uint uiLocalID);
	void					SendPeerInfo(CPeer *pPeer);
};
//=============================================================================

#endif //__C_PEER_H__

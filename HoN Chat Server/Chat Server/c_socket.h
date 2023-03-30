// (C)2005 S2 Games
// c_socket.h
//
//=============================================================================
#ifndef __C_SOCKET_H__
#define __C_SOCKET_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_packet.h"
#include "c_netdriver.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
const int MAX_ADDRESS_LENGTH(64);
const int MAX_PACKETS_RESENT_PER_FRAME(1);
const int PACKET_TIMEOUT(500);

const int K2_SOCKET_EMPTY(0);
const int K2_SOCKET_CLOSED(-1);
const int K2_SOCKET_ERROR(-2);
//=============================================================================

//=============================================================================
// CSocket
//=============================================================================
class CSocket
{
private:
	CNetDriver*		m_pNetDriver;

	bool			m_bInitialized;
	ESocketType		m_eType;
	uint			m_uiSocket;

	string			m_sRecvAddress;
	uint			m_uiRecvAddress;
	word			m_wRecvPort;

	string			m_sSendAddress;
	word			m_wSendPort;

	void*			m_pSendAddr;

	bool			m_bIsLocalConnection;
	bool			m_bIsLANConnection;


	inline bool	RequiresConnection() const		{ return m_eType == K2_SOCKET_TCP; }

	inline int	ReceivePacketTCP(CPacket &pkt)	{ pkt.Clear(); if (!m_bInitialized) return 0; return m_pNetDriver->ReceivePacket(m_uiSocket, pkt); }
	inline int	ReceivePacketUDP(CPacket &pkt)	{ pkt.Clear(); if (!m_bInitialized) return 0; return m_pNetDriver->ReceivePacket(m_uiSocket, pkt, m_sRecvAddress, m_uiRecvAddress, m_wRecvPort); }

public:
	~CSocket();
	CSocket();

	bool			Init(CNetDriver *pNetDriver, ESocketType eType, word wPort = 0, bool bBlocking = false);
	bool			Init(const CSocket *pSocket, uint uiNewSocket);

	bool			OpenListenPort(int iMaxConnectionsWaiting = 0);
	CSocket*		AcceptConnection(CMemManager *pMemManager);
	bool			SetSendAddr(const string &addr, word wPort = 0);

	inline const string&	GetRecvAddress() const				{ return m_sRecvAddress; }
	inline uint				GetRecvAddressBin() const			{ return m_uiRecvAddress; }
	inline word				GetRecvPort() const					{ return m_wRecvPort; }

	inline const string&	GetSendAddress() const				{ return m_sSendAddress; }
	inline word				GetSendPort() const					{ return m_wSendPort; }

	inline int				ReceivePacket(CPacket &pkt)			{ return (m_eType == K2_SOCKET_TCP) ? ReceivePacketTCP(pkt) : ReceivePacketUDP(pkt); }
	inline bool				SendPacket(CPacket &pkt)			{ return m_bInitialized ? (m_pNetDriver->SendPacket(m_eType, m_uiSocket, m_pSendAddr, pkt) == pkt.GetLength()) : false; }

	inline bool				Close()								{ if (!m_bInitialized) return true; m_bInitialized = false; return m_pNetDriver->CloseConnection(m_uiSocket); }

	inline void				AllowBroadcast(bool bValue)			{ if (m_bInitialized) m_pNetDriver->AllowBroadcast(m_uiSocket, bValue); }
	inline wstring			GetBroadcastAddress()				{ return m_bInitialized ? m_pNetDriver->GetBroadcastAddress(m_uiSocket) : WSNULL; }

	inline bool				IsInitialized() const				{ return m_bInitialized; }
	inline bool				IsLocalConnection() const			{ return m_bIsLocalConnection; }
	inline bool				IsLANConnection() const				{ return m_bIsLANConnection; }

	inline bool				IsConnected(int iSecondsToWait = 0)	{ return m_bInitialized ? m_pNetDriver->IsConnected(m_uiSocket, iSecondsToWait) : false; }
	inline bool				DataWaiting(uint uiWaitTime = 0)	{ return m_bInitialized ? m_pNetDriver->DataWaiting(m_uiSocket, uiWaitTime) : false; }
	inline bool				HasError(uint uiWaitTime = 0)		{ return m_bInitialized ? m_pNetDriver->HasError(m_uiSocket, uiWaitTime) : false; }

	inline void				SetSendBuffer(uint uiSendBuffer)	{ if (m_bInitialized) m_pNetDriver->SetSendBuffer(m_uiSocket, uiSendBuffer); }
	inline void				SetRecvBuffer(uint uiRecvBuffer)	{ if (m_bInitialized) m_pNetDriver->SetRecvBuffer(m_uiSocket, uiRecvBuffer); }
};
//=============================================================================

#endif //__C_SOCKET_H__

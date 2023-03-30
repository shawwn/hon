// (C)2009 S2 Games
// c_pendingconnection.h
//
//=============================================================================
#ifndef __C_PENDINGCONNECTION_H__
#define __C_PENDINGCONNECTION_H__

//=============================================================================
// Declarations
//=============================================================================
class CSocket;
class CPacket;
class CHTTPRequest;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
enum EPendingConnectionState
{
	PENDING_HANDSHAKE,
	PENDING_CLIENT_AUTH,
	PENDING_SERVER_AUTH
};
//=============================================================================

//=============================================================================
// CPendingConnection
//=============================================================================
class CPendingConnection
{
private:
	DECLARE_CORE_API;

	CSocket*				m_pSocket;
	CHTTPRequest*			m_pRequest;
	EPendingConnectionState	m_eState;
	uint					m_uiTimeStamp;

	string					m_sCookie;
	string					m_sToken;
	uint					m_uiAccountID;

	CPendingConnection();

	bool	HandshakeFrame();
	bool	ClientAuthFrame();
	bool	ServerAuthFrame();
	void	ProcessClientAuthResponse();
	void	ProcessServerAuthResponse();

public:
	~CPendingConnection();
	CPendingConnection(CSocket *pSocket, CCore *pCore);

	bool	Frame();
	
	bool	RequestClientAuth(CPacket &pkt);
	bool	RequestServerAuth(CPacket &pkt);
};
//=============================================================================

#endif //__C_PENDINGCONNECTION_H__

// (C)2004 S2 Games
// c_netdriver.h
//
//=============================================================================
#ifndef __C_NETDRIVER_H__
#define __C_NETDRIVER_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_core.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CPacket;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
typedef struct in_addr		inetAddr_t;
typedef struct sockaddr_in	netAddr_t;
typedef struct sockaddr		sockAddr_t;

enum ESocketType
{
	K2_SOCKET_TCP,
	K2_SOCKET_UDP,

	K2_SOCKET_INVALID
};
//=============================================================================

//=============================================================================
// CNetDriver
//=============================================================================
class CNetDriver
{
private:
	DECLARE_CORE_API;

	bool	NewSocket(uint &uiSocket, ESocketType eType, bool bBlocking);
	void	AddHeader(char fullPacket[], byte flags, uint seq);

public:
	~CNetDriver()	{}
	CNetDriver();

	bool	Initialize(CCore *pCore);
	void	Shutdown();

	bool	OpenPort(uint &uiSocket, word &wRequestedPort, ESocketType eType, bool bBlocking);
	bool	CloseConnection(uint uiSocket);
	int		ReceivePacket(uint uiSocket, CPacket &recv, string &sAddrName, uint &uiAddr, word &wPort);
	int		ReceivePacket(uint uiSocket, CPacket &recv);
	size_t	SendPacket(ESocketType eType, uint uiSocket, const void *pVoidAddr, const CPacket &packet);
	bool	SetSendAddress(string &sAddress, word &wPort, void *&pVoidNetAddr, bool &bIsLocalConnection, bool &bIsLANConnection);
	void	Flush(uint uiSocket);
	bool	Connect(uint uiSocket, void *&pVoidNetAddr);
	bool	IsConnected(uint uiSocket, uint uiMSecToWait);
	bool	DataWaiting(uint uiSocket, uint uiWaitTime = 0);
	bool	HasError(uint uiSocket, uint uiMSecToWait = 0);
	bool	AcceptConnection(uint uiFromSocket, uint &uiNewSocket, string &sAddrName, uint &uiAddr, word &wPort);

	bool	StartListening(uint &uiSocket, int iNumConnWaiting = 0);

	void	AllowBroadcast(uint uiSocket, bool bValue);
	wstring	GetBroadcastAddress(uint uiSocket);

	void	SetSendBuffer(uint uiSocket, uint uiSendBuffer);
	void	SetRecvBuffer(uint uiSocket, uint uiRecvBuffer);
};
//=============================================================================

#endif //__C_NETDRIVER_H__

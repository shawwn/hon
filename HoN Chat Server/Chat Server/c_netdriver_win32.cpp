// (C)2004 S2 Games
// c_netdriver_win32.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "chatserver_common.h"

#include <winsock2.h>
#include <ws2tcpip.h>

#include "c_netdriver.h"
#include "c_packet.h"
#include "c_system.h"
#include "c_console.h"
#include "c_socket.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
struct SWinsockServer
{
	WSADATA	wsaData;
	uint	nonblock;
}
g_WinsockServer;
//=============================================================================

/*====================
  CNetDriver::CNetDriver
  ====================*/
CNetDriver::CNetDriver() :
NULL_CORE_API
{
}


/*====================
  CNetDriver::Initialize
  ====================*/
bool	CNetDriver::Initialize(CCore *pCore)
{
	INIT_CORE_API(pCore);

	m_pConsole->Admin() << L"Network Initialization" << newl;

	int iResult(WSAStartup(0x202, &g_WinsockServer.wsaData));
	if (iResult != 0)
	{
		m_pConsole->Admin() << L"WSAStartup() failed: " << m_pSystem->GetErrorString(iResult) << newl;
		return false;
	}

	g_WinsockServer.nonblock = 1;

	char szAddr[256];
	memset(szAddr, 0, 256);
	if (gethostname(szAddr, 255) == SOCKET_ERROR)
	{
		m_pConsole->Admin() << L"gethostname() failed: " << m_pSystem->GetErrorString(WSAGetLastError()) << newl;
		return false;
	}

	m_pConsole->Admin() << L"Host is: " << szAddr << newl;

	addrinfo *pInfo(NULL);
	getaddrinfo(szAddr, NULL, NULL, &pInfo);
	int i(0);
	while (pInfo != NULL)
	{
		sockaddr_in *pAddr((sockaddr_in*)pInfo->ai_addr);
		m_pConsole->Admin() << _T("NIC #") << i++ << _T(": ") << inet_ntoa(pAddr->sin_addr) << newl;
		pInfo = pInfo->ai_next;
	}

	return true;
}


/*====================
  CNetDriver::Shutdown
  ====================*/
void	CNetDriver::Shutdown()
{
	m_pConsole->Std() << _T("Network Shutdown") << newl;
	WSACleanup();
}


/*====================
  CNetDriver::CloseConnection
  ====================*/
bool	CNetDriver::CloseConnection(uint uiSocket)
{
	return closesocket(uiSocket) != SOCKET_ERROR;
}


/*====================
  CNetDriver::NewSocket
  ====================*/
bool	CNetDriver::NewSocket(uint &uiSocket, ESocketType eType, bool bBlock)
{
	SOCKET sock;

	try
	{
		if (uiSocket != INVALID_SOCKET)
			CloseConnection(uiSocket);

		// Make sure we use the correct type for our protocol
		switch (eType)
		{
		case K2_SOCKET_UDP:
			sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
			break;

		case K2_SOCKET_TCP:
			sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			break;

		default:
			EX_ERROR(_T("Invalid socket type"));
			break;
		}

		uiSocket = uint(sock);

		if (sock == INVALID_SOCKET)
			EX_ERROR(_T("socket() failure: ") + m_pSystem->GetErrorString(WSAGetLastError()));

		uint uiBlock(!bBlock);
		if (ioctlsocket(uiSocket, FIONBIO, (ULONG*)&uiBlock) == SOCKET_ERROR)
			EX_ERROR(_T("ioctlsocket() failure: ") + WSAGetLastError());

		return true;
	}
	catch (CException &ex)
	{
		CloseConnection(uiSocket);
		ex.Process(_T("CNetDriver::NewSocket() - "), NO_THROW);
		return false;
	}
}


/*====================
  CNetDriver::OpenPort
  ====================*/
bool	CNetDriver::OpenPort(uint &uiSocket, word &wRequestedPort, ESocketType eType, bool bBlocking)
{
	try
	{
		sockaddr_in addr;
		memset(&addr, 0, sizeof(sockaddr_in));
		addr.sin_family = AF_INET;

		addr.sin_addr.s_addr = INADDR_ANY;
		addr.sin_port = htons(wRequestedPort);

		if (!NewSocket(uiSocket, eType, bBlocking))
			EX_ERROR(_T("Socket creation failed"));

		if (bind(uiSocket, (sockaddr*)&(addr), sizeof(sockaddr_in)) == SOCKET_ERROR)
			EX_ERROR(_T("bind() failed: ") + m_pSystem->GetErrorString(WSAGetLastError()));

		socklen_t namelen = sizeof(sockaddr_in);
		getsockname(uiSocket, (sockaddr*)(&addr), &namelen);
		wRequestedPort = ntohs(addr.sin_port);

		return true;
	}
	catch (CException &ex)
	{
		CloseConnection(uiSocket);
		ex.Process(_T("CNetDriver::OpenPort() - "), NO_THROW);
		return false;
	}
}


/*====================
  CNetDriver::ReceivePacket
  ====================*/
int		CNetDriver::ReceivePacket(uint uiSocket, CPacket &pkt, string &sAddrName, uint &uiAddr, word &wPort)
{
	PROFILE("CNetDriver::ReceivePacket");

	pkt.Clear();

	sockaddr_in from;
	socklen_t fromlen(sizeof(sockaddr));
	int iBytesRead(recvfrom(uiSocket, pkt.LockBuffer(), MAX_PACKET_SIZE, 0, (sockaddr*)&from, &fromlen));

	if (iBytesRead == SOCKET_ERROR)
	{
		pkt.SetLength(0);
		if (WSAGetLastError() != WSAEWOULDBLOCK)
			return -1;

		return 0;
	}

	if (iBytesRead > 0)
	{
		uiAddr = from.sin_addr.S_un.S_addr;
		sAddrName = inet_ntoa(from.sin_addr);
		wPort = ntohs(from.sin_port);
	}

	pkt.SetLength(iBytesRead);
	return iBytesRead;
}

int		CNetDriver::ReceivePacket(uint uiSocket, CPacket &pkt)
{
	PROFILE("CNetDriver::ReceivePacket");

	pkt.Clear();

	int iBytesRead(recv(uiSocket, pkt.LockBuffer(), MAX_PACKET_SIZE, 0));

	if (iBytesRead == SOCKET_ERROR)
	{
		pkt.SetLength(0);
		int iError(WSAGetLastError());
		if (iError == WSAEWOULDBLOCK)
			return K2_SOCKET_EMPTY;

		return K2_SOCKET_ERROR;
	}

	pkt.SetLength(iBytesRead);
	return (iBytesRead == 0) ? K2_SOCKET_CLOSED : iBytesRead;
}


/*====================
  CNetDriver::GetBroadcastAddress
  ====================*/
wstring	CNetDriver::GetBroadcastAddress(uint uiSocket)
{
	sockaddr_in netAddr;
	memset(&netAddr, 0, sizeof(sockaddr_in));

	DWORD dwBytesWritten;
	WSAIoctl(uiSocket, SIO_GET_BROADCAST_ADDRESS, NULL, NULL, &netAddr, sizeof(sockaddr_in), &dwBytesWritten, NULL, NULL);

	return StringToTString(inet_ntoa(netAddr.sin_addr));
}


/*====================
  CNetDriver::SendPacket
  ====================*/
size_t	CNetDriver::SendPacket(ESocketType eType, uint uiSocket, const void *pVoidAddr, const CPacket &packet)
{
	PROFILE("CSocket::SendPacket")

	if (packet.GetUnreadLength() == 0)
		return 0;

	const sockaddr *pSockAddr(static_cast<const sockaddr*>(pVoidAddr));

	// Write packet size so the packet can be reconstructed from the stream
	if (eType == K2_SOCKET_TCP)
	{
		char cBuffer[MAX_PACKET_SIZE + sizeof(ushort)];

		ushort unLength(htons(ushort(MIN<uint>(packet.GetUnreadLength(), USHRT_MAX))));

		*(ushort *)cBuffer = unLength;
		
		memcpy(cBuffer + 2, packet.GetBuffer(), packet.GetUnreadLength());

		int ret(sendto(uiSocket, cBuffer, packet.GetUnreadLength() + sizeof(ushort), 0, pSockAddr, sizeof(sockaddr)));

		if (ret == SOCKET_ERROR)
			return 0;

		return ret;
	}
	else
	{
		int ret(sendto(uiSocket, packet.GetBuffer(), packet.GetUnreadLength(), 0, pSockAddr, sizeof(sockaddr)));
		if (ret == SOCKET_ERROR)
			return 0;

		return ret;
	}
	
	return 0;
}


/*====================
  CNetDriver::SetSendAddress
  ====================*/
bool	CNetDriver::SetSendAddress(string &sAddress, word &wPort, void *&pVoidNetAddr, bool &bIsLocalConnection, bool &bIsLANConnection)
{
	try
	{
		// Clear the old NetAddr struct if it exists and allocate a new one
		sockaddr_in *pNetAddr(static_cast<sockaddr_in*>(pVoidNetAddr));
		if (pNetAddr == NULL)
		{
			pNetAddr = NEW(m_pMemManager) sockaddr_in;
			pVoidNetAddr = pNetAddr;
		}
		memset(pNetAddr, 0, sizeof(sockaddr_in));

		if (sAddress.empty())
			EX_ERROR(_T("No address specified"));

		// Extract port and base ip address from sAddr
		size_t zColon(sAddress.find_first_of(':'));
		if (zColon != string::npos)
		{
			uint uiPort(AtoI(sAddress.substr(zColon + 1)));
			if (uiPort > USHRT_MAX)
				EX_ERROR(_T("Invalid port"));
			wPort = ushort(uiPort);
			sAddress = sAddress.substr(0, zColon);
		}

		if (wPort == 0)
			EX_ERROR(_T("No port specified"));

		if (IsIPAddress(sAddress))
		{
			uint uint_addr;
			uint_addr = inet_addr(sAddress.c_str());
			memcpy(&pNetAddr->sin_addr, &uint_addr, 4);
			pNetAddr->sin_family = AF_INET;
		}
		else
		{
			struct hostent *pHP(gethostbyname(sAddress.c_str()));
			if (pHP == NULL)
				EX_ERROR(_T("Failed to resolve address:") + m_pSystem->GetErrorString(WSAGetLastError()));

			memcpy(&pNetAddr->sin_addr, pHP->h_addr, pHP->h_length);
			pNetAddr->sin_family = pHP->h_addrtype;
		}

		sAddress = inet_ntoa(pNetAddr->sin_addr);
		pNetAddr->sin_port = htons(wPort);

		svector vIPBytes(TokenizeString(sAddress, '.'));

		if (sAddress == "127.0.0.1" || sAddress == "localhost")
			bIsLocalConnection = true;
		else
			bIsLocalConnection = false;

		if ((vIPBytes[0] == "192" && vIPBytes[1] == "168") ||
			(vIPBytes[0] == "172" && AtoI(vIPBytes[1]) >= 16 && AtoI(vIPBytes[1]) <= 30) ||
			(vIPBytes[0] == "10"))
			bIsLANConnection = true;
		else
			bIsLANConnection = false;

		return true;
	}
	catch (CException &ex)
	{
		ex.Process(_T("CNetConnection::SetSendAddr() - "), NO_THROW);
		return false;
	}
}


/*====================
  CNetDriver::IsConnected
  ====================*/
bool	CNetDriver::IsConnected(uint uiSocket, uint uiMSecToWait)
{
	PROFILE("CNetDriver::IsConnected")

	if (uiSocket == INVALID_SOCKET)
		return false;

	uint uiStartTime(m_pCore->GetMilliseconds());

	do
	{
		fd_set fdSocketSet;
		timeval timeWait;

		FD_ZERO(&fdSocketSet);
		FD_SET(uiSocket, &fdSocketSet);

		timeWait.tv_sec = 0;
		timeWait.tv_usec = 0;

		if (select(NULL, NULL, &fdSocketSet, NULL, &timeWait) > 0)
			return true;

		if (uiMSecToWait == 0)
			return false;

		m_pSystem->Sleep(1);
	}
	while(m_pCore->GetMilliseconds() - uiStartTime < uiMSecToWait);

	return false;
}


/*====================
  CNetDriver::HasError
  ====================*/
bool	CNetDriver::HasError(uint uiSocket, uint uiMSecToWait)
{
	if (uiSocket == INVALID_SOCKET)
		return true;

	uint uiStartTime(m_pCore->GetMilliseconds());

	do
	{
		fd_set fdSocketSet;
		timeval timeWait;

		FD_ZERO(&fdSocketSet);
		FD_SET(uiSocket, &fdSocketSet);

		timeWait.tv_sec = 0;
		timeWait.tv_usec = 0;

		if (select(NULL, NULL, NULL, &fdSocketSet, &timeWait) > 0)
			return true;

		if (uiMSecToWait == 0)
			return false;

		m_pSystem->Sleep(1);
	}
	while (m_pCore->GetMilliseconds() - uiStartTime < uiMSecToWait);

	return false;
}


/*====================
  CNetDriver::DataWaiting
  ====================*/
bool	CNetDriver::DataWaiting(uint uiSocket, uint uiWaitTime)
{
	PROFILE("CNetDriver::DataWaiting")

	if (uiSocket == INVALID_SOCKET)
		return false;

	uint uiStartTime(m_pCore->GetMilliseconds());

	do
	{
		fd_set fdSocketSet;
		timeval timeWait;

		FD_ZERO(&fdSocketSet);
		FD_SET(uiSocket, &fdSocketSet);

		timeWait.tv_sec = 0;
		timeWait.tv_usec = 0;

		if (select(NULL, &fdSocketSet, NULL, NULL, &timeWait) > 0)
			return true;

		if(uiWaitTime == 0)
			return false;

		m_pSystem->Sleep(1);
	}
	while(m_pCore->GetMilliseconds() - uiStartTime < uiWaitTime);

	return false;
}


/*====================
  CNetDriver::StartListening
  ====================*/
bool	CNetDriver::StartListening(uint &uiSocket, int iNumConnWaiting)
{
	if (iNumConnWaiting == 0)
		iNumConnWaiting = SOMAXCONN;

	return (listen(uiSocket, iNumConnWaiting) == 0 ? true : false);
}


/*====================
  CNetDriver::AcceptConnection
  ====================*/
bool	CNetDriver::AcceptConnection(uint uiFromSocket, uint &uiNewSocket, string &sAddrName, uint &uiAddr, word &wPort)
{
	PROFILE("CNetDriver::AcceptConnection")

	sockaddr_in from;
	socklen_t fromlen(sizeof(sockaddr));

	uiNewSocket = accept(uiFromSocket, (sockaddr*)&from, &fromlen);
	if (uiNewSocket == INVALID_SOCKET)
		return false;

	uiAddr = from.sin_addr.S_un.S_addr;
	sAddrName = inet_ntoa(from.sin_addr);
	wPort = ntohs(from.sin_port);
	return true;
}


/*====================
  CNetDriver::Connect
  ====================*/
bool	CNetDriver::Connect(uint uiSocket, void *&pVoidNetAddr)
{
	if (pVoidNetAddr == NULL)
		return false;

	if (connect(uiSocket, (sockaddr*)pVoidNetAddr, sizeof(sockaddr)) == SOCKET_ERROR)
	{
		int iError(WSAGetLastError());
		if (iError != WSAEWOULDBLOCK)
		{
			m_pConsole->Std() << L"Could not initialize connection: " << m_pSystem->GetErrorString(iError) << newl;
			return false;
		}
	}

	return true;
}


/*====================
  CNetDriver::Flush
  ====================*/
void	CNetDriver::Flush(uint uiSocket)
{
	char cBuffer[1];
	int iBytesRead(1);
	while (iBytesRead != 0)
		iBytesRead = recv(uiSocket, cBuffer, 1, 0);
}


/*====================
  CNetDriver::AllowBroadcast
  ====================*/
void	CNetDriver::AllowBroadcast(uint uiSocket, bool bValue)
{
	setsockopt(uiSocket, SOL_SOCKET, SO_BROADCAST, (char*)&bValue, sizeof(bool));
}


/*====================
  CNetDriver::SetSendBuffer
  ====================*/
void	CNetDriver::SetSendBuffer(uint uiSocket, uint uiSendBuffer)
{
	setsockopt(uiSocket, SOL_SOCKET, SO_SNDBUF, (char *)&uiSendBuffer, sizeof(uiSendBuffer));
}


/*====================
  CNetDriver::SetRecvBuffer
  ====================*/
void	CNetDriver::SetRecvBuffer(uint uiSocket, uint uiRecvBuffer)
{
	setsockopt(uiSocket, SOL_SOCKET, SO_RCVBUF, (char *)&uiRecvBuffer, sizeof(uiRecvBuffer));
}

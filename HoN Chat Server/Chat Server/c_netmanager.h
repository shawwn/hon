// (C)2008 S2 Games
// c_netmanager.h
//
//=============================================================================
#ifndef __C_NETMANAGER_H__
#define __C_NETMANAGER_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_core.h"

#include "c_netdriver.h"
#include "c_socket.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CPHPData;
class CPendingConnection;
//=============================================================================

//=============================================================================
// CNetManager
//=============================================================================
class CNetManager
{
private:
	DECLARE_STL_LIST_TYPES(PendingConnection, CPendingConnection*);

	DECLARE_CORE_API;
	
	CHeap*					m_pHeap;
	CHeap*					m_pSTLHeap;

	CNetDriver				m_NetDriver;
	CSocket					m_sockListen;
	string					m_sPublicAddress;
	ushort					m_unListenPort;
	ushort					m_unRequestedListenPort;
	PendingConnectionList*	m_pPendingConnections;

	bool					m_bListening;

	uint					m_uiReinitTime;
	uint					m_uiConnectionCount;
	uint					m_uiSuccessfulConnectionCount;

	bool	OpenListenSocket(ushort unPort = 0);
	void	Reinitialize();
	void	AcceptConnection();

public:
	~CNetManager();
	CNetManager();

	inline const string&	GetPublicAddress() const		{ return m_sPublicAddress; }
	inline ushort			GetListenPort() const			{ return m_unListenPort; }

	inline void				IncrementSuccessfulConnectionCount()	{ ++m_uiSuccessfulConnectionCount; }
	inline void				DecrementConnectionCount()				{ --m_uiConnectionCount; }

	bool					Initialize(CCore *pCore);
	void					Frame();
	CSocket*				Connect(const string &sAddress, ushort unPort, ESocketType eType);
	void					Disconnect();	
	inline void				StartListening(ushort unPort)	{ OpenListenSocket(unPort); }

	void					FlushPendingConnections();

	void					PrintUsage();
};
//=============================================================================
#endif	//__C_NETMANAGER_H__
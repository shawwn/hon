// (C)2010 S2 Games
// c_gameservermanager.h
//
//=============================================================================
#ifndef __C_GAMESERVERMANAGER_H__
#define __C_GAMESERVERMANAGER_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_core.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CGameServer;
class CSocket;
//=============================================================================

//=============================================================================
// CGameServerManager
//=============================================================================
class CGameServerManager
{
private:
	DECLARE_STL_VECTOR_TYPES(Server, CGameServer*);
	DECLARE_STL_VECTOR_TYPES(ServerID, uint);
	DECLARE_STL_LIST_TYPES(Server, CGameServer*);
	DECLARE_STL_MAP_TYPES(AddressToID, string, uint);
	DECLARE_STL_MAP_TYPES(LocationList, wstring, ServerList*);
	DECLARE_STL_MAP_TYPES(LocationCount, wstring, uint);

	DECLARE_CORE_API;

	CHeap*				m_pHeap;
	CHeap*				m_pSTLHeap;

	ServerVector*		m_pGameServers;
	ServerIDVector*		m_pAvailableIDs;
	AddressToIDMap*		m_pAddressToID;
	LocationListMap*	m_pLocationLists;
	LocationCountMap*	m_pServerPeakCounts;

	void	RemoveFromLocationList(CGameServer *pServer);

public:
	~CGameServerManager();
	CGameServerManager();

	bool				Initialize(CCore *pCore);
	void				Frame();

	CGameServer*		AddGameServer(uint uiServerID, CSocket *pSocket);
	void				RemoveGameServer(CGameServer *pServer);

	void				UpdateServerLocation(CGameServer *pServer);
	void				UpdateGameServerInfo(CGameServer *pServer);

	inline CGameServer*	GetGameServer(uint uiID)							{ return (uiID >= m_pGameServers->size()) ? NULL : m_pGameServers->at(uiID); }
	CGameServer*		GetServerFromLocation(const wstring &sLocation);
	CGameServer*		GetGameServerFromAddress(const string &sAddressPort);

	void				SendSubstitution();

	void				PrintUsage();
	void				PrintServerList();
	void				PrintGameInfo(const string &sAddressPort);	
};
//=============================================================================

#endif //__C_GAMESERVERMANAGER_H__

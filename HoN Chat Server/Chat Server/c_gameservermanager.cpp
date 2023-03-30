// (C)2010 S2 Games
// c_gameservermanager.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "chatserver_common.h"

#include "c_gameservermanager.h"
#include "c_gameserver.h"
#include "c_console.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
CVAR(uint,	gs_ConnectedTimeout,	30);
//=============================================================================

/*====================
  CGameServerManager::~CGameServerManager
  ====================*/
CGameServerManager::~CGameServerManager()
{
	for (ServerVector_it itGameServer(m_pGameServers->begin()), itEnd(m_pGameServers->end()); itGameServer != itEnd; ++itGameServer)
		delete *itGameServer;
	delete m_pGameServers;

	for (LocationListMap_it itLocation(m_pLocationLists->begin()), itEnd(m_pLocationLists->end()); itLocation != itEnd; ++itLocation)
		delete itLocation->second;
	delete m_pLocationLists;
	
	delete m_pAddressToID;

	delete m_pServerPeakCounts;

	delete m_pAvailableIDs;

	delete m_pSTLHeap;
	delete m_pHeap;
}


/*====================
  CGameServerManager::CGameServerManager
  ====================*/
CGameServerManager::CGameServerManager() :
NULL_CORE_API,
m_pHeap(NULL),
m_pSTLHeap(NULL),

m_pGameServers(NULL),
m_pAvailableIDs(NULL),
m_pAddressToID(NULL)
{
}


/*====================
  CGameServerManager::Initialize
  ====================*/
bool	CGameServerManager::Initialize(CCore *pCore)
{
	INIT_CORE_API(pCore);

	m_pHeap = m_pMemManager->AllocateHeap("gamesrvr");
	m_pSTLHeap = m_pMemManager->AllocateHeap("gamesrvr_stl");

	m_pGameServers = HEAP_NEW(m_pMemManager, m_pHeap) ServerVector(ServerVector_alloc(m_pMemManager, m_pSTLHeap));
	m_pAvailableIDs = HEAP_NEW(m_pMemManager, m_pHeap) ServerIDVector(ServerIDVector_alloc(m_pMemManager, m_pSTLHeap));
	m_pAddressToID = HEAP_NEW(m_pMemManager, m_pHeap) AddressToIDMap(std::less<string>(), AddressToIDMap_alloc(m_pMemManager, m_pSTLHeap));
	m_pLocationLists = HEAP_NEW(m_pMemManager, m_pHeap) LocationListMap(std::less<wstring>(), LocationListMap_alloc(m_pMemManager, m_pSTLHeap));
	m_pServerPeakCounts = HEAP_NEW(m_pMemManager, m_pHeap) LocationCountMap(std::less<wstring>(), LocationCountMap_alloc(m_pMemManager, m_pSTLHeap));

	return true;
}


/*====================
  CGameServerManager::Frame
  ====================*/
void	CGameServerManager::Frame()
{
	PROFILE("CGameServerManager::Frame");

	for (ServerVector_it itServer(m_pGameServers->begin()), itEnd(m_pGameServers->end()); itServer != itEnd; ++itServer)
	{
		// Skip blank entries
		CGameServer *pGameServer(*itServer);
		if (pGameServer == NULL)
			continue;

		// Remove servers that have timed out
		if (m_pCore->GetSeconds() - pGameServer->GetLastReceiveTime() > gs_ConnectedTimeout || !pGameServer->ReadSocket())
		{
			m_pAvailableIDs->push_back(pGameServer->GetID());
			delete pGameServer;
			*itServer = NULL;
			continue;
		}

		// Ping servers that are halfway to timing out
		if (m_pCore->GetSeconds() - pGameServer->GetLastReceiveTime() > gs_ConnectedTimeout / 2)
			pGameServer->SendPing();
	}
}


/*====================
  CGameServerManager::AddGameServer
  ====================*/
CGameServer*	CGameServerManager::AddGameServer(uint uiServerID, CSocket *pSocket)
{
	uint uiID(-1);
	if (m_pAvailableIDs->empty())
	{
		uiID = INT_SIZE(m_pGameServers->size());
		m_pGameServers->push_back(NULL);
	}
	else
	{
		uiID = m_pAvailableIDs->back();
		m_pAvailableIDs->pop_back();
	}

	CGameServer *pNewGameServer(HEAP_NEW(m_pMemManager, m_pHeap) CGameServer(m_pCore, pSocket, uiID));
	
	m_pGameServers->at(uiID) = pNewGameServer;

	return pNewGameServer;
}


/*====================
  CGameServerManager::RemoveFromLocationList
  ====================*/
void	CGameServerManager::RemoveFromLocationList(CGameServer *pServer)
{
	if (!pServer->HasLocationIterator())
		return;

	LocationListMap_it itList(pServer->GetLocationList());
	itList->second->erase(pServer->GetLocationIterator());
	if (itList->second->empty())
	{
		delete itList->second;
		m_pLocationLists->erase(itList);
	}

	pServer->InvalidateLocationIterator();
}


/*====================
  CGameServerManager::RemoveGameServer
  ====================*/
void	CGameServerManager::RemoveGameServer(CGameServer *pServer)
{
	RemoveFromLocationList(pServer);
	m_pGameServers->at(pServer->GetID()) = NULL;
	m_pAvailableIDs->push_back(pServer->GetID());
}


/*====================
  CGameServerManager::UpdateServerLocation
  ====================*/
void	CGameServerManager::UpdateServerLocation(CGameServer *pServer)
{
	RemoveFromLocationList(pServer);

	LocationListMap_it itLocation(m_pLocationLists->find(pServer->GetLocation()));
	if (itLocation == m_pLocationLists->end())
	{
		ServerList *pList(HEAP_NEW(m_pMemManager, m_pHeap) ServerList(ServerList_alloc(m_pMemManager, m_pSTLHeap)));
		itLocation = m_pLocationLists->insert(LocationListMap_pair(pServer->GetLocation(), pList)).first;
	}

	itLocation->second->push_front(pServer);
	ServerList_it itEntry(itLocation->second->begin());
	pServer->StoreLocationIterator(itLocation, itEntry);
	
	LocationCountMap_it itCount(m_pServerPeakCounts->find(pServer->GetLocation()));
	if (itCount == m_pServerPeakCounts->end())
		m_pServerPeakCounts->insert(LocationCountMap_pair(pServer->GetLocation(), uint(itLocation->second->size())));
	else
		itCount->second = MAX(itCount->second, uint(itLocation->second->size()));
}


/*====================
  CGameServerManager::UpdateGameServerInfo
  ====================*/
void	CGameServerManager::UpdateGameServerInfo(CGameServer *pServer)
{
	if (pServer == NULL)
		return;
	
	string sThisAddressPort(pServer->GetAddress() + ":" + XtoS(pServer->GetPort()));	
	
	// does the server exist in the list?
	AddressToIDMap_it itServer(m_pAddressToID->find(sThisAddressPort));
	if (itServer == m_pAddressToID->end())
	{
		// it doesn't, insert a new server
		m_pAddressToID->insert(AddressToIDMap_pair(sThisAddressPort, pServer->GetID()));
	}
	else
	{		
		// it does exist, update it with the new server info
		itServer->second = pServer->GetID();
	}
	
	//PrintGameInfo(sThisAddressPort);
}


/*====================
  CGameServerManager::GetServerFromLocation
  ====================*/
CGameServer*	CGameServerManager::GetServerFromLocation(const wstring &sLocation)
{
	LocationListMap_it itLocation(m_pLocationLists->find(sLocation));
	if (itLocation == m_pLocationLists->end())
		return NULL;

	for (ServerList_it itServer(itLocation->second->begin()), itEnd(itLocation->second->end()); itServer != itEnd; ++itServer)
	{
		if (!(*itServer)->HasMatch() && (*itServer)->GetStatus() == 1)
			return *itServer;
	}

	return NULL;
}


/*====================
  CGameServerManager::GetGameServerFromAddress
  ====================*/
CGameServer*	CGameServerManager::GetGameServerFromAddress(const string &sAddressPort)
{	
	AddressToIDMap_it itFind(m_pAddressToID->find(sAddressPort));

	if (itFind == m_pAddressToID->end())
		return NULL;

	return GetGameServer(itFind->second);
}


/*====================
  CGameServerManager::PrintUsage
  ====================*/
void	CGameServerManager::PrintUsage()
{
	uint uiActiveCount(0);
	for (ServerVector_it itServer(m_pGameServers->begin()), itEnd(m_pGameServers->end()); itServer != itEnd; ++itServer)
	{
		if (*itServer != NULL)
			++uiActiveCount;
	}

	m_pConsole->Admin()
		<< newl
		<< L"Game Servers     " << newl
		<< L"---------------- " << newl
		<< L"Active:          " << XtoA(uiActiveCount, FMT_DELIMIT, 7) << " / " << XtoA(INT_SIZE(m_pGameServers->size()), FMT_DELIMIT) << newl
		<< L"---------------- " << newl;
}


/*====================
  CGameServerManager::PrintServerList
  ====================*/
void	CGameServerManager::PrintServerList()
{
	for (LocationCountMap_it itLocation(m_pServerPeakCounts->begin()), itEnd(m_pServerPeakCounts->end()); itLocation != itEnd; ++itLocation)
	{
		// Print location and count
		LocationListMap_it itList(m_pLocationLists->find(itLocation->first));
		m_pConsole->Admin()
			<< L" [" << XtoW(itLocation->first, FMT_NONE, 3) << L"]          "
			<< (itList == m_pLocationLists->end() ? 0 : INT_SIZE(itList->second->size()))
			<< L" / " << itLocation->second << newl;

		if (itList == m_pLocationLists->end())
			continue;

		uint uiCount[6] = { 0, 0, 0, 0, 0, 0 };
		for (ServerList_it itServer(itList->second->begin()), itServerEnd(itList->second->end()); itServer != itServerEnd; ++itServer)
		{
			++uiCount[CLAMP(int((*itServer)->GetStatus()), 0, 5)];
		}
		m_pConsole->Admin()
			<< L"  Sleeping: " << uiCount[0] << newl
			<< L"  Idle:     " << uiCount[1] << newl
			<< L"  Active:   " << uiCount[2] << newl
			<< L"  Broken:   " << uiCount[3] << newl
			<< L"  Dead:     " << uiCount[4] << newl
			<< L"  Unknown:  " << uiCount[5] << newl;
	}
}


/*====================
  CGameServerManager::PrintGameInfo
  ====================*/
void	CGameServerManager::PrintGameInfo(const string &sAddressPort)
{
	CGameServer* pServer(GetGameServerFromAddress(sAddressPort));
	
	if (pServer == NULL)
	{
		m_pConsole->Std() << L"The requested server " << sAddressPort << L" was not found." << newl;
		return;
	}	

	m_pConsole->Std()
		<< L"Game Server Info" << newl
		<< L"----------------------" << newl
		<< L"Server ID: " << pServer->GetServerID() << newl
		<< L"Address: " << pServer->GetAddress() << L":" << pServer->GetPort() << newl
		<< L"Arranged Type: " << pServer->GetArrangedType() << newl
		<< L"Location: " << pServer->GetLocation() << newl
		<< L"Name: " << pServer->GetName() << newl
		<< L"Status: " << pServer->GetStatus() << newl
		<< L"Arranged Type: " << pServer->GetArrangedType() << newl
		<< L"Official: " << pServer->GetOfficial() << newl
		<< L"No Leaver: " << pServer->GetNoLeaver() << newl
		<< L"Private: " << pServer->GetPrivate() << newl
		<< L"Map Name: " << pServer->GetMapName() << newl
		<< L"Tier: " << pServer->GetTier() << newl
		<< L"Game Name: " << pServer->GetGameName() << newl
		<< L"Game Mode: " << pServer->GetGameModeName() << newl
		<< L"Team Size: " << pServer->GetTeamSize() << newl
		<< L"Easy Mode: " << pServer->GetEasyMode() << newl
		<< L"All Heroes: " << pServer->GetAllHeroes() << newl
		<< L"Force Random: " << pServer->GetForceRandom() << newl
		<< L"Auto Balanced: " << pServer->GetAutoBalanced() << newl
		<< L"Advanced Options: " << pServer->GetAdvancedOptions() << newl
		<< L"MinPSR: " << pServer->GetMinPSR() << newl
		<< L"MaxPSR: " << pServer->GetMaxPSR() << newl
		<< L"Dev Heroes: " << pServer->GetDevHeroes() << newl
		<< L"Hardcore: " << pServer->GetHardcore() << newl;
}

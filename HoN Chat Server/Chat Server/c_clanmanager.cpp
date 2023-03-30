// (C)2009 S2 Games
// c_clanmanager.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "chatserver_common.h"

#include "c_clanmanager.h"
#include "c_clan.h"
//=============================================================================

/*====================
  CClanManager::~CClanManager
  ====================*/
CClanManager::~CClanManager()
{
	for (ClanMap_it itClan(m_pClans->begin()), itEnd(m_pClans->end()); itClan != itEnd; ++itClan)
		delete itClan->second;
	delete m_pClans;

	delete m_pSTLHeap;
	delete m_pHeap;
}


/*====================
  CClanManager::CClanManager
  ====================*/
CClanManager::CClanManager() :
NULL_CORE_API,
m_pHeap(NULL),
m_pSTLHeap(NULL),
m_pClans(NULL)
{
}


/*====================
  CClanManager::Initialize
  ====================*/
bool	CClanManager::Initialize(CCore *pCore)
{
	INIT_CORE_API(pCore);

	m_pHeap = m_pMemManager->AllocateHeap("clan");
	m_pSTLHeap = m_pMemManager->AllocateHeap("clan_stl");

	m_pClans = HEAP_NEW(m_pMemManager, m_pHeap) ClanMap(std::less<uint>(), ClanMap_alloc(m_pMemManager, m_pSTLHeap));

	return true;
}


/*====================
  CClanManager::AddClan
  ====================*/
CClan*	CClanManager::AddClan(uint uiClanID)
{
	PROFILE("CClanManager::AddClan")

	CClan *pClan(GetClan(uiClanID));
	if (pClan != NULL)
		return pClan;
	
	pClan = HEAP_NEW(m_pMemManager, m_pHeap) CClan(m_pCore, m_pHeap);
	if (pClan == NULL)
		return NULL;

	m_pClans->insert(ClanMap_pair(uiClanID, pClan));
	pClan->SetClanID(uiClanID);

	return pClan;
}

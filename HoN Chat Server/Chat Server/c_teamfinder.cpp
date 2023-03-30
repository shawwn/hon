// (C)2009 S2 Games
// c_Team.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "chatserver_common.h"

#include "c_teamfinder.h"
#include "c_clientmanager.h"
#include "c_console.h"
#include "c_matchmaker.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================

//=============================================================================

/*====================
  CTeamFinder::~CTeamFinder
  ====================*/
CTeamFinder::~CTeamFinder()
{
	for (int ii(0); ii < 5; ii++)
	{
		for(int i(0); i < 4; i++)
			delete m_aPoolList[ii][i];
	}

	for (GroupPoolList_it it(m_lPotentialGroups->begin()); it != m_lPotentialGroups->end(); ++it)
		delete (*it);
	delete m_lPotentialGroups;

	delete m_vVacancies;

	for (GroupVector_it it(m_vMasterList->begin()); it != m_vMasterList->end(); ++it)
		delete (*it);
	delete m_vMasterList;
}


/*====================
  CTeamFinder::CTeamFinder
  ====================*/
CTeamFinder::CTeamFinder() :
NULL_CORE_API,
m_pHeap(NULL),
m_pSTLHeap(NULL)
{
}

/*====================
  CTeamFinder::Initialize
  ====================*/
void	CTeamFinder::Initialize(CCore *pCore)
{
	INIT_CORE_API(pCore);

	m_pHeap = m_pMemManager->AllocateHeap("teamfind");
	m_pSTLHeap = m_pMemManager->AllocateHeap("teamfind_stl");

	for (int ii(0); ii < 5; ii++)
	{
		for(int i(0); i < 4; i++)
			m_aPoolList[ii][i] = HEAP_NEW(m_pMemManager, m_pHeap) GroupPoolList(GroupPoolList_alloc(m_pMemManager, m_pSTLHeap));
	}
	
	m_lPotentialGroups = HEAP_NEW(m_pMemManager, m_pHeap) GroupPoolList(GroupPoolList_alloc(m_pMemManager, m_pSTLHeap));
	m_vVacancies = HEAP_NEW(m_pMemManager, m_pHeap) VacancyVector(VacancyVector_alloc(m_pMemManager, m_pSTLHeap));
	m_vMasterList = HEAP_NEW(m_pMemManager, m_pHeap) GroupVector(GroupVector_alloc(m_pMemManager, m_pSTLHeap));

}

/*====================
  CTeamFinder::Shutdown
  ====================*/
void CTeamFinder::Shutdown()
{
	delete m_pHeap;
	delete m_pSTLHeap;
}

/*====================
  CTeamFinder::Frame
  ====================*/
void CTeamFinder::Frame()
{

}

/*====================
  CTeamFinder::PrintUsage
  ====================*/
void CTeamFinder::PrintUsage()
{

}

/*====================
  CTeamFinder::AddGroup
  ====================*/
void CTeamFinder::AddGroup(CGroup *pGroup)
{
	if (pGroup == NULL)
		return;

	if (pGroup->GetGroupSize() == pGroup->GetBracket() + 1)
	{
		CTeam *pNewTeam = new CTeam(m_pCore, pGroup, m_pHeap);
		SendToMatchmaking(pNewTeam);
		return;
	}

	FindPotentialGroups(pGroup);
	if (m_lPotentialGroups->size() < 1)
	{
		AddGroupToWaitPool(pGroup);
	}
	else
	{
		int iRandom = rand() % m_lPotentialGroups->size();
		GroupPoolList_it it(m_lPotentialGroups->begin());
		for (int i(0); i < iRandom; i++)
			++it;

		GroupNodeList *pGroupNode = (*it);
		CTeam *pNewFullTeam = new CTeam(m_pCore, pGroup, m_pHeap);
		for (GroupNodeList_it it(pGroupNode->begin()); it != pGroupNode->end(); ++it)
		{
			pNewFullTeam->AddGroup(GetGroupFromID(*it));
			RemoveGroupFromPool(GetGroupFromID(*it));
		}
		SendToMatchmaking(pNewFullTeam);
	}

}

/*====================
  CTeamFinder::FindPotentialGroups
  ====================*/
void CTeamFinder::FindPotentialGroups(CGroup *pGroup)
{
	int iSize = pGroup->GetGroupSize();
	int iBracket = pGroup->GetBracket();
	GroupPoolList* lGroupList = m_aPoolList[iBracket][4 - iSize];
	m_lPotentialGroups->clear();

	for (GroupPoolList_it it(lGroupList->begin()); it != lGroupList->end(); ++it)
	{
		bool bAdd(true);
		for (GroupNodeList_it itt((*it)->begin()); itt != (*it)->end(); ++itt)
		{
			if (!GetGroupFromID(*itt)->IsCompatible(pGroup))
				bAdd = false;
		}
		if (bAdd)
			m_lPotentialGroups->push_back(*it);
	}
}

/*====================
  CTeamFinder::SendToMatchmaking
  ====================*/
void CTeamFinder::SendToMatchmaking(CTeam *cNewTeam)
{
	m_pMatchMaker->AddTeam(cNewTeam);
}

/*====================
  CTeamFinder::AddGroupToWaitPool
  ====================*/
void CTeamFinder::AddGroupToWaitPool(CGroup *pGroup)
{
	pGroup->SendStatusChange(MATCHMAKER_STATUS_IN_TEAMFINDER);

	int iSize = pGroup->GetGroupSize();
	int iBracket = pGroup->GetBracket();
	//Add group to master list.
	if (m_vVacancies->size() > 0)
	{
		pGroup->SetGroupID(m_vVacancies->back());
		m_vMasterList->at(m_vVacancies->back()) = pGroup;
		m_vVacancies->pop_back();
	}
	else
	{
		m_vMasterList->push_back(pGroup);
		pGroup->SetGroupID(m_vMasterList->size() - 1); 
	}

	GroupNodeList* gNewNode = HEAP_NEW(m_pMemManager, m_pHeap) GroupNodeList(GroupNodeList_alloc(m_pMemManager, m_pSTLHeap));
	gNewNode->push_back(pGroup->GetGroupID());
	m_aPoolList[iBracket][iSize - 1]->push_back(gNewNode);

	for (int i(iBracket - 2); i >= 0; i--)
	{
		if (i + iSize < iBracket)
			AddGroupToList(m_aPoolList[iBracket][i], m_aPoolList[iBracket][i + iSize], pGroup);
	}

}

/*====================
  CTeamFinder::AddGroupToList
  ====================*/
void CTeamFinder::AddGroupToList(GroupPoolList* lGroupListScan, GroupPoolList* lGroupListAdd, CGroup *pGroup)
{
	for (GroupPoolList_it it(lGroupListScan->begin()); it != lGroupListScan->end(); ++it)
	{
		bool bAdd(true);
		for (GroupNodeList_it itt((*it)->begin()); itt != (*it)->end(); ++itt)
		{
			if (!GetGroupFromID(*itt)->IsCompatible(pGroup))
			{
				bAdd = false;
				break;
			}
		}
		if (bAdd)
		{
			GroupNodeList* gNewNode = HEAP_NEW(m_pMemManager, m_pHeap) GroupNodeList(GroupNodeList_alloc(m_pMemManager, m_pSTLHeap));
			for (GroupNodeList_it itt((*it)->begin()); itt != (*it)->end(); ++itt)
				gNewNode->push_back(*itt);
			gNewNode->push_back(pGroup->GetGroupID());
			lGroupListAdd->push_back(gNewNode);
		}
	}
}

/*====================
  CTeamFinder::RemoveGroupFromPool
  ====================*/
void CTeamFinder::RemoveGroupFromPool(CGroup *pGroup)
{
	int iBracket = pGroup->GetBracket();
	for (int i(pGroup->GetGroupSize() - 1); i < 4; i++)
		RemoveGroupFromList(m_aPoolList[iBracket][i], pGroup);
}

/*====================
  CTeamFinder::RemoveGroupFromList
  ====================*/
void CTeamFinder::RemoveGroupFromList(GroupPoolList* lGroupList, CGroup *pGroup)
{
	for (GroupPoolList_it it(lGroupList->begin()); it != lGroupList->end(); ++it)
	{
		for (GroupNodeList_it itt((*it)->begin()); itt != (*it)->end(); ++itt)
		{
			if ((*itt) == pGroup->GetGroupID())
			{
				it = lGroupList->erase(it);
				break;
			}
		}
		if (it == lGroupList->end())
			break;
	}
}

/*====================
  CTeamFinder::AddVirtualGroup
  ====================*/
CGroup* CTeamFinder::AddVirtualGroup()
{
	//have each group join one region
	uint uiVirtualGroupRegion = 0;
//	uint uiRandomRegion = rand() % NUM_REGIONS;
	uiVirtualGroupRegion |= BIT(REGION_USW);
//	int iRand = rand() % 4;
	int iRand = 4;
	CGroup * cVirtualGroup = new CGroup(m_pCore, m_pClientManager->AddVirtualClient(), uiVirtualGroupRegion);
	int i = 0;
	while (i < iRand)
	{
		cVirtualGroup->AddClient(m_pClientManager->AddVirtualClient());
		i++;
	}

	return cVirtualGroup;
}

/*====================
  CTeamFinder::ClientHasDropped
  ====================*/
void CTeamFinder::ClientHasDropped(uint uiClientID)
{
	uint uiID = GetClientsGroupID(uiClientID);
	CGroup * pGroup = GetGroupFromID(uiID);
	
	RemoveGroupFromPool(pGroup);
	//Send notification to client's group members.
	pGroup->NotifyGroupClientDropped(m_pClientManager->GetClientFromAccountID(uiClientID));

	//Delete group from master list.
	delete m_vMasterList->at(uiID);
	m_vVacancies->push_back(uiID);
}

/*====================
  CTeamFinder::GetClientsGroupID
  ====================*/
uint CTeamFinder::GetClientsGroupID(uint uiClientID)
{
	for (GroupVector_it it(m_vMasterList->begin()); it != m_vMasterList->end(); ++it)
	{
		if ((*it)->HasClient(uiClientID))
			return (*it)->GetGroupID();
	}
	return NULL;
}

/*====================
  CTeamFinder::AddVirtualTeam
  ====================*/
CTeam* CTeamFinder::AddVirtualTeam()
{
	CTeam * pTeam = new CTeam(m_pCore, AddVirtualGroup(), m_pHeap);
	pTeam->SetVirtual(true);
	return pTeam;
}

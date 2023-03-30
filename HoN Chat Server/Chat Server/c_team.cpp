// (C)2009 S2 Games
// c_Team.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "chatserver_common.h"

#include "c_team.h"
#include "c_teamfinder.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
EXTERN_CVAR(uint, team_MaxSpread);
//=============================================================================

/*====================
  CTeam::~CTeam
  ====================*/
CTeam::~CTeam()
{
	for (TeamGroupList_it it(m_lTeamGroups->begin()); it != m_lTeamGroups->end(); ++it)
		delete (*it);
	delete m_lTeamGroups;
}


/*====================
  CTeam::CTeam
  ====================*/
CTeam::CTeam(CCore *pCore, CGroup *pGroup, CHeap *pHeap) :
NULL_CORE_API,
m_uiTeamID(0),
m_uiServerWait(0),
m_bVirtual(false),
m_bHasServer(false)
{
	INIT_CORE_API(pCore);
	m_lTeamGroups = HEAP_NEW(m_pMemManager, pHeap) TeamGroupList(TeamGroupList_alloc(m_pMemManager, pHeap));
	m_lTeamGroups->push_back(pGroup);
	m_uiRegionFlags = pGroup->GetRegions();
	CalculateRating();
}

/*====================
  CTeam::CalculateRating
  ====================*/
void CTeam::CalculateRating()
{
	int iTeamRating(0);
	for (TeamGroupList_it it(m_lTeamGroups->begin()); it != m_lTeamGroups->end(); ++it)
		iTeamRating += (*it)->GetRating();

	iTeamRating /= m_lTeamGroups->size();
}


/*====================
  CTeam::AddGroup
  ====================*/
void CTeam::AddGroup(CGroup *pGroup)
{
	m_lTeamGroups->push_back(pGroup);
	for (TeamGroupList_it it(m_lTeamGroups->begin()); it != m_lTeamGroups->end(); ++it)
		m_uiRegionFlags &= (*it)->GetRegions();
	CalculateRating();
}


/*====================
  CTeam::HasClient
  ====================*/
bool CTeam::HasClient(CClient *pClient)
{
	for (TeamGroupList_it it(m_lTeamGroups->begin()); it != m_lTeamGroups->end(); ++it)
	{
		if ((*it)->HasClient(pClient->GetAccountID()))
			return true;
	}
	return false;
}


/*====================
  CTeam::DropClient
  ====================*/
void CTeam::DropClient(CClient *pClient)
{
	TeamGroupList_it it(m_lTeamGroups->begin());
	while (it != m_lTeamGroups->end())
	{
		if ((*it)->HasClient(pClient->GetAccountID()))
		{
			delete (*it);
			m_lTeamGroups->erase(it);
			break;
		}

		++it;
	}
}


/*====================
  CTeam::IsCompatible
  ====================*/
bool CTeam::IsCompatible(CTeam *pTeam)
{
	if (pTeam->GetRating() > m_fRating + team_MaxSpread || pTeam->GetRating() < m_fRating - team_MaxSpread)
		return false;
	
	for (uint uiRegion(0); uiRegion < NUM_REGIONS; ++uiRegion)
	{
		if ((BIT(uiRegion) & m_uiRegionFlags) != 0)
			return true;
	}

	return false;
}


/*====================
  CTeam::GetTeamSize
  ====================*/
int CTeam::GetTeamSize()
{
	int iSize(0);
	for (TeamGroupList_it it(m_lTeamGroups->begin()); it != m_lTeamGroups->end(); ++it)
		iSize += (*it)->GetGroupSize();
	return iSize;
}


/*====================
  CTeam::GetClient
  ====================*/
CClient* CTeam::GetClient(int iClient)
{
	int i(0);
	for (TeamGroupList_it it(m_lTeamGroups->begin()); i <= iClient && it != m_lTeamGroups->end(); ++it)
	{
		if (i + (*it)->GetGroupSize() <= iClient)
		{
			i += (*it)->GetGroupSize();
		}
		else
		{
			while (i < iClient)
				i++;
			return (*it)->GetClient(i);
		}
	}
	return NULL;
}


/*====================
  CTeam::NotifyWaitingForServer
  ====================*/
void CTeam::NotifyWaitingForServer()
{
	for (TeamGroupList_it it(m_lTeamGroups->begin()); it != m_lTeamGroups->end(); ++it)
	{
		for (int i(0); i < (*it)->GetGroupSize(); i++)
			(*it)->GetClient(i)->SendMatchMakerUpdate(MATCHMAKER_STATUS_WAITING_FOR_SERVER);
	}
}

/*====================
  CTeam::IsOnline
  ====================*/
bool CTeam::IsOnline()
{
	for (TeamGroupList_it it(m_lTeamGroups->begin()); it != m_lTeamGroups->end(); ++it)
	{
		if (!(*it)->IsOnline())
			return false;
	}
	return true;
}


/*====================
  CTeam::DisbandToTeamFinder
  ====================*/
void CTeam::DisbandToTeamFinder()
{
	for (TeamGroupList_it it(m_lTeamGroups->begin()); it != m_lTeamGroups->end(); ++it)
	{
		if ((*it)->IsOnline())
		{
			(*it)->SendStatusChange(MATCHMAKER_STATUS_TEAMMATE_DROPPED);
			m_pTeamFinder->AddGroup((*it));
		}
		else
		{
			(*it)->SendStatusChange(MATCHMAKER_STATUS_GROUPMATE_DROPPED);
			delete (*it);
		}
	}

	m_lTeamGroups->clear();
}

/*====================
  CTeam::NotifyStatus
  ====================*/
void CTeam::NotifyStatus(byte yStatus, uint uiParamA, uint uiParamB)
{
	for (TeamGroupList_it it(m_lTeamGroups->begin()); it != m_lTeamGroups->end(); ++it)
		(*it)->SendStatusChange(yStatus, uiParamA, uiParamB);
}


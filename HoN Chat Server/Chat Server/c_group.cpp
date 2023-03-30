// (C)2009 S2 Games
// c_group.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "chatserver_common.h"

#include "c_group.h"
#include "c_clientmanager.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
CVAR(float,		GroupWeightMult_2,		1.0f);
CVAR(float,		GroupWeightMult_3,		1.0f);
CVAR(float,		GroupWeightMult_4,		1.0f);
CVAR(float,		GroupWeightMult_5,		1.0f);

CVAR(float,     GroupWeightInverse_2,	0.0f);
CVAR(float,     GroupWeightInverse_3,	0.0f);
CVAR(float,     GroupWeightInverse_4,	0.0f);
CVAR(float,     GroupWeightInverse_5,	0.0f);

CVAR(int,		GroupWeight_2,		10);
CVAR(int,		GroupWeight_3,		20);
CVAR(int,		GroupWeight_4,		30);
CVAR(int,		GroupWeight_5,		40);

CVAR(int,		MaxGroupSpread,		100);
CVAR(int,		MinGroupSpread,		100);

//=============================================================================

/*====================
  CGroup::~CGroup
  ====================*/
CGroup::~CGroup()
{
	for (ClientVector_it it(m_vGroupMembers.begin()); it != m_vGroupMembers.end(); ++it)
	{
		if ((*it) != NULL)
			(*it)->SetGroup(NULL);
	}
}


/*====================
  CGroup::CGroup
  ====================*/
CGroup::CGroup(CCore *pCore, CClient *pClient, uint uiRegionFlags) :
NULL_CORE_API,
m_uiRegionFlags(uiRegionFlags),
m_iBracket(1), //this should be changed so it is set by the client.
m_vGroupMembers(ClientVector_alloc(pCore->GetMemManager(), pCore->GetClientManager()->GetSTLHeap()))
{
	INIT_CORE_API(pCore);
	m_vGroupMembers.push_back(pClient);
	if (pClient != NULL)
		pClient->SetGroup(this);

	CalculateRating();
}


/*====================
  CGroup::CalculateRating
  ====================*/
void	CGroup::CalculateRating()
{
	float fGroupRating(0);
	for (ClientVector_it it(m_vGroupMembers.begin()); it != m_vGroupMembers.end(); ++it)
		fGroupRating += (*it)->GetRating(MATCH_POOL_SOLO);

	fGroupRating /= m_vGroupMembers.size();
	m_fRating = fGroupRating;
}


/*====================
  CGroup::GetClient
  ====================*/
CClient* CGroup::GetClient(uint iClientNumber)
{
	if (iClientNumber < m_vGroupMembers.size())
		return m_vGroupMembers[iClientNumber];
	return NULL;
}


/*====================
  CGroup::GetClient
  ====================*/
CClient* CGroup::GetClient(const wstring &sName)
{
	for (ClientVector_it it(m_vGroupMembers.begin()); it != m_vGroupMembers.end(); ++it)
	{
		if((*it)->GetName() == sName)
			return *it;
	}
	return NULL;
}


/*====================
  CGroup::IsCompatible
  ====================*/
bool CGroup::IsCompatible(CGroup *otherGroup)
{
	if (otherGroup->GetRating() > (m_fRating - MinGroupSpread) && otherGroup->GetRating() < (m_fRating + MaxGroupSpread))
	{
		for (uint uiRegion(0); uiRegion < NUM_REGIONS; ++uiRegion)
		{
			if ((BIT(uiRegion) & m_uiRegionFlags) != 0)
				return true;
		}
	}	
	return false;
}


/*====================
  CGroup::HasClient
  ====================*/
bool CGroup::HasClient(uint ClientID)
{
	for (ClientVector_it it(m_vGroupMembers.begin()); it != m_vGroupMembers.end(); ++it)
	{
		if ((*it)->GetAccountID() == ClientID)
			return true;
	}
	return false;
}


/*====================
  CGroup::RemoveClient
  ====================*/
void CGroup::RemoveClient(CClient *pClient)
{
	for (ClientVector_it it(m_vGroupMembers.begin()); it != m_vGroupMembers.end(); ++it)
	{
		if (*it == pClient)
		{
			m_vGroupMembers.erase(it);
			CalculateRating();
			break;
		}
	}
}


/*====================
  CGroup::NotifyGroupClientDropped
  ====================*/
void CGroup::NotifyGroupClientDropped(CClient * pClient)
{
	
	for (int i(0); i < m_vGroupMembers.size(); i++)
	{
			if (m_vGroupMembers[i] == pClient)
			{
				m_vGroupMembers[i] = NULL;
				break;
			}
	}
}


/*====================
  CGroup::IsOnline
  ====================*/
bool CGroup::IsOnline()
{
	for (ClientVector_it it(m_vGroupMembers.begin()); it != m_vGroupMembers.end(); ++it)
	{
		if ((*it) == NULL)
			return false;

		if (!(*it)->IsConnected())
			return false;
	}
	return true;
}

/*====================
  CGroup::AddClient
  ====================*/
void CGroup::AddClient(CClient *pClient)
{
	if (pClient == NULL)
		return;

	m_vGroupMembers.push_back(pClient);
	CalculateRating();

	pClient->SetGroup(this);
}


/*====================
  CGroup::SendStatusChange
  ====================*/
void CGroup::SendStatusChange(byte yStatus, uint uiParamA, uint uiParamB)
{
	for (ClientVector_it it(m_vGroupMembers.begin()); it != m_vGroupMembers.end(); ++it)
	{
		if ((*it) == NULL || !(*it)->IsConnected())
			continue;
		
		(*it)->SendMatchMakerUpdate(yStatus, uiParamA, uiParamB);
	}
}



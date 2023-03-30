// (C)2009 S2 Games
// c_TeamFinder.h
//
//=============================================================================
#ifndef __C_TEAMFINDER_H
#define __C_TEAMFINDER_H

//=============================================================================
// Headers
//=============================================================================
#include "c_client.h"
#include "c_group.h"
#include "c_team.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
DECLARE_STL_VECTOR_TYPES(Group, CGroup*)
DECLARE_STL_VECTOR_TYPES(Vacancy, int)

DECLARE_STL_LIST_TYPES(GroupNode, uint)
DECLARE_STL_LIST_TYPES(GroupPool, GroupNodeList*)
//=============================================================================

//=============================================================================
// CTeamFinder
//=============================================================================
class CTeamFinder
{
private:
	DECLARE_CORE_API;

	CHeap* m_pHeap;
	CHeap* m_pSTLHeap;

	GroupVector* m_vMasterList;
	VacancyVector* m_vVacancies;

	GroupPoolList* m_aPoolList[5][4];
	GroupPoolList* m_lPotentialGroups;

	void				AddGroupToWaitPool(CGroup *pGroup);
	void				FindPotentialGroups(CGroup *pGroup);
	void				SendToMatchmaking(CTeam *cNewTeam);
	void				AddGroupToList(GroupPoolList* lGroupListScan, GroupPoolList* lGroupListAdd, CGroup *pGroup);
	void				RemoveGroupFromPool(CGroup *pGroup);
	void				RemoveGroupFromList(GroupPoolList* lGroupList, CGroup *pGroup);

public:
	CTeamFinder();
	~CTeamFinder();

	CGroup*				AddVirtualGroup();
	CTeam*				AddVirtualTeam();
	CGroup*				GetGroupFromID(uint uiID)		{ return m_vMasterList->at(uiID); }

	void				AddGroup(CGroup *pGroup);
	void				ClientHasDropped(uint uiClientID);
	uint				GetClientsGroupID(uint uiClientID);

	void				Initialize(CCore *pCore);
	void				Frame();
	void				Shutdown();
	void				PrintUsage();

	
};
//=============================================================================
#endif	//__C_TEAMFINDER_H
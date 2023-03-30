// (C)2009 S2 Games
// c_team.h
//
//=============================================================================
#ifndef __C_TEAM_H__
#define __C_TEAM_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_client.h"
#include "c_group.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
DECLARE_STL_LIST_TYPES(TeamGroup, CGroup*)
//=============================================================================

//=============================================================================
// CTeam
//=============================================================================
class CTeam
{
private:
	TeamGroupList *m_lTeamGroups;
	DECLARE_CORE_API;
	float	m_fRating;
	uint	m_uiRegionFlags;
	uint	m_uiTeamID;
	uint	m_uiServerID;
	bool    m_bHasServer;
	bool	m_bVirtual;
	uint	m_uiServerWait;

	void	CalculateRating();

public:
	CTeam(CCore *pCore, CGroup *pGroup, CHeap *pHeap);
	~CTeam();

	uint	GetID()								{ return m_uiTeamID; }
	void	SetID(uint uiID)					{ m_uiTeamID = uiID; }
	
	uint	GetServerWait()						{ return m_uiServerWait; }
	void	SetServerWait(uint uiServerWait)	{ m_uiServerWait = uiServerWait; }

	uint	IsVirtual()							{ return m_bVirtual; }
	void	SetVirtual(bool bVirtual)			{ m_bVirtual = bVirtual; }

	uint	GetServer()							{ return m_uiServerID; }
	void	SetServer(uint uiID)				{ m_uiServerID = uiID; m_bHasServer = true; }

	void	RemoveServer()						{ m_bHasServer = false; }
	bool	HasServer()							{ return m_bHasServer; }

	int		GetRating()							{ return m_fRating; }
	uint	GetRegions()						{ return m_uiRegionFlags; }
	TeamGroupList*	GetGroups()					{ return m_lTeamGroups; }

	void		NotifyWaitingForServer();
	void		DisbandToTeamFinder();

	int			GetTeamSize();
	CClient*	GetClient(int iClient);
	bool		IsCompatible(CTeam *pTeam);
	bool		IsOnline();
	bool		HasClient(CClient *pClient);
	void		DropClient(CClient *pClient);
	void		AddGroup(CGroup *pGroup);
	void		NotifyStatus(byte yStatus, uint uiParamA = 0, uint uiParamB = 0);

};
//=============================================================================
#endif	//__C_TEAM_H__
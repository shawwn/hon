// (C)2009 S2 Games
// c_matchmaker.h
//
//=============================================================================
#ifndef __C_MATCHMAKER_H__
#define __C_MATCHMAKER_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_core.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CHTTPRequest;
class CMatch;
class CGameServer;

EXTERN_CVAR(uint, matchmaker_provisionalCount);
EXTERN_CVAR(uint, matchmaker_createMatchTimeout);
//=============================================================================

//=============================================================================
// CMatchMaker
//=============================================================================
class CMatchMaker
{
private:
	DECLARE_STL_LIST_TYPES(Match, CMatch*);
	DECLARE_STL_VECTOR_TYPES(Match, CMatch*);
	DECLARE_STL_VECTOR_TYPES(Client, CClient*);
	DECLARE_STL_VECTOR_TYPES(ui, uint);

	DECLARE_CORE_API;

	CHeap*				m_pHeap;
	CHeap*				m_pSTLHeap;

	uint				m_uiMatchCounter;

	MatchVector*		m_apMatches[NUM_REGIONS];
	uiVector*			m_apFreeMatchIndices[NUM_REGIONS];
	MatchList*			m_pPendingMatches;
	MatchVector*		m_pPotentialMatches;

	// Stats
	uint				m_uiLastPeriod;
	uint				m_uiTotalJoins;
	uint				m_aTotalJoins[NUM_REGIONS];
	uint				m_aRecentJoins[NUM_REGIONS][12];
	uint				m_aTotalCancels[NUM_REGIONS];
	uint				m_uiMatchesRequested;
	uint				m_uiMatchesCreated;
	uint				m_uiMatchesFailed;
	uint				m_uiTotalWaitTime;

	CMatch*		SpawnNewMatch(uint uiRegion);

	inline uint	GetCurrentPeriod() const				{ return (m_pCore->GetSeconds() / 300) % 12; }
	inline void	IncrementJoinCounters(uint uiRegion)	{ ++m_uiTotalJoins; ++m_aTotalJoins[uiRegion]; ++m_aRecentJoins[uiRegion][GetCurrentPeriod()]; } 

public:
	~CMatchMaker();
	CMatchMaker();

	uint		GetRegionFromName(const wstring &sName);
	uint		GetRegionFlagFromName(const wstring &sName);

	inline void	IncrementCancelCounter(uint uiRegion)	{ ++m_aTotalCancels[uiRegion]; }
	inline void	IncrementFailCounter()					{ ++m_uiMatchesFailed; }
	inline void	IncrementCreatedCounter()				{ ++m_uiMatchesCreated; }

	void		Initialize(CCore *pCore);
	void		Frame();
	void		Shutdown();

	void		AddClient(CClient *pClient, uint uiRegionFlags);
	void		CreateMatch(CMatch *pMatch);
	CClient*	RequestSubstitute(CMatch *pMatch, uint uiOldAccountID);

	void		PrintUsage();
	void		PrintRegionDetails(const wstring &sRegion);
};
//=============================================================================

#endif //__C_MATCHMAKER_H__

// (C)2008 S2 Games
// c_gamemanager.h
//
//=============================================================================
#ifndef __C_GAMEMANAGER_H__
#define __C_GAMEMANAGER_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_core.h"

#include "c_httpmanager.h"
#include "c_tournmatch.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
enum EGameManagerHTTPRequest
{
	GAME_MANAGER_GET_TOURNAMENT_GAMES,
	GAME_MANAGER_GET_LEAGUE_GAMES,
	GAME_MANAGER_REFRESH_SERVER_LIST
};
//=============================================================================

//=============================================================================
// CGameManager
//=============================================================================
class CGameManager
{
private:
	typedef CSTLAllocator<CTournMatch*>			MatchList_alloc;
	typedef list<CTournMatch*, MatchList_alloc>	MatchList;
	typedef MatchList::iterator					MatchList_it;
	typedef MatchList::const_iterator			MatchList_cit;
	
	DECLARE_CORE_API;

	CHeap*				m_pHeap;
	CHeap*				m_pSTLHeap;

	bool				m_bPoll;
	wstring				m_sCookie;
	
	HTTPRequestList*	m_pHTTPRequests;
		
	MatchList*			m_pMatches;

public:
	~CGameManager();
	CGameManager();

	void			Initialize(CCore *pCore);
	void			Frame();
	void			ProcessIncomingPackets();
	void			ProcessHTTPRequests();

	void			ProcessGetTournamentGamesResponse(const wstring &sResponse);
	void			ProcessRefreshServerListResponse(const wstring &sResponse);
	
	void			RequestServerList();
	void			RequestTournamentGames();
	void			TournMatchIsReady(uint uiMatchID, uint uiChallenge);

};
//=============================================================================
#endif	//__C_GAMEMANAGER_H__
// (C)2009 S2 Games
// c_gamemanager.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "chatserver_common.h"

#include "c_gamemanager.h"
#include "c_httprequest.h"
#include "c_phpdata.h"
#include "c_console.h"
#include "c_matchmaker.h"
//=============================================================================

/*====================
  CGameManager::~CGameManager
  ====================*/
CGameManager::~CGameManager()
{
	delete m_pHTTPRequests;
	delete m_pMatches;

	delete m_pSTLHeap;
	delete m_pHeap;
}


/*====================
  CGameManager::CGameManager
  ====================*/
CGameManager::CGameManager() :
NULL_CORE_API,
m_pHeap(NULL),
m_pSTLHeap(NULL),
m_pMatches(NULL),
m_pHTTPRequests(NULL)
{
}


/*====================
  CClientManager::Initialize
  ====================*/
void	CGameManager::Initialize(CCore *pCore)
{
	INIT_CORE_API(pCore);

	m_pHeap = m_pMemManager->AllocateHeap("game");
	m_pSTLHeap = m_pMemManager->AllocateHeap("game_stl");

	m_pMatches = HEAP_NEW(m_pMemManager, m_pHeap) MatchList(MatchList_alloc(m_pMemManager, m_pSTLHeap));
	m_pHTTPRequests = HEAP_NEW(m_pMemManager, m_pHeap) HTTPRequestList(HTTPRequestList_alloc(m_pMemManager, m_pSTLHeap));

	m_bPoll = true;	
}


/*====================
  CGameManager::Frame
  ====================*/
void	CGameManager::Frame()
{
	PROFILE("CGameManager::Frame")

	CDate date(true);

	ProcessIncomingPackets();

	// poll at 2 minute intervals for testing purposes, increase to 5 min in production
	if ((date.GetMinute() % 2) == 0)
	{
		if (m_bPoll)
		{
			m_pConsole->Admin() << L"Polling for tourn games that need to be spawned at " << date.GetDateString() << L" " << date.GetTimeString() << L"..." << newl;	
			
			RequestServerList();			

			m_bPoll = false;			
		}
	}
	else
	{
		if (m_bPoll == false)
			m_bPoll = true;
	}	
}


/*====================
  CGameManager::ProcessIncomingPackets
  ====================*/
void	CGameManager::ProcessIncomingPackets()
{
	PROFILE("CGameManager::ProcessIncomingPackets")

	ProcessHTTPRequests();	
}


/*====================
  CGameManager::ProcessHTTPRequests
  ====================*/
void	CGameManager::ProcessHTTPRequests()
{
	PROFILE("CGameManager::ProcessHTTPRequests")

	if (m_pHTTPRequests->empty())
		return;

	for (HTTPRequestList_it itRequest(m_pHTTPRequests->begin()), itEnd(m_pHTTPRequests->end()); itRequest != itEnd; )
	{
		switch ((*itRequest)->GetStatus())
		{
		case HTTP_REQUEST_SENDING:
			++itRequest;
			break;

		case HTTP_REQUEST_ERROR:
			m_pHTTPManager->ReleaseRequest(*itRequest);
			STL_ERASE(*m_pHTTPRequests, itRequest);
			if (m_pHTTPRequests->empty())
				return;
			break;

		case HTTP_REQUEST_SUCCESS:
			switch ((*itRequest)->GetType())
			{
			case GAME_MANAGER_GET_TOURNAMENT_GAMES:				
				ProcessGetTournamentGamesResponse((*itRequest)->GetResponse());
				break;

			case GAME_MANAGER_GET_LEAGUE_GAMES:
				break;
				
			case GAME_MANAGER_REFRESH_SERVER_LIST:
				ProcessRefreshServerListResponse((*itRequest)->GetResponse());
				break;				
			}
			
			m_pHTTPManager->ReleaseRequest(*itRequest);
			STL_ERASE(*m_pHTTPRequests, itRequest);
			if (m_pHTTPRequests->empty())
				return;
			break;
		}
	}
}


/*====================
  CGameManager::ProcessUserInfoInGameResponse
  ====================*/
void	CGameManager::ProcessGetTournamentGamesResponse(const wstring &sResponse)
{

/*			// left in for reference
			m_pConsole->Admin() << newl
				<< L"Need to spawn tournament game for tourn_id " << pTourn->GetString(L"tourn_id") << L"..." << newl
				<< L"Tournament Name: " << pTourn->GetString(L"tourn_name") << newl
				<< L"Tournament Match: " << pTourn->GetString(L"matchname") << newl
				<< L"Match Date: " << pTourn->GetString(L"matchdate") << L"  Match Time: " << pTourn->GetString(L"matchtime") << newl
				<< L"Game Mode: " << pTourn->GetString(L"gamemode") << L"  All Heroes: " << pTourn->GetString(L"allheroes") << newl
				<< L"Easy Mode: " << pTourn->GetString(L"easymode") << L"  Random Hero: " << pTourn->GetString(L"randomhero") << newl
				<< L"Advanced Options: " << pTourn->GetString(L"advancedoptions") << L"                  " << newl
				<< L"No Hero Repick: " << pTourn->GetString(L"noherorepick") << L"  No Hero Swap: " << pTourn->GetString(L"noheroswap") << L"  Allow Veto: " << pTourn->GetString(L"allowveto") << newl
				<< L"No Agi Heroes: " << pTourn->GetString(L"noagiheroes") << L"  No Int Heroes: " << pTourn->GetString(L"nointheroes") << L"  No Str Heroes: " << pTourn->GetString(L"nostrheroes") << newl
				<< L"Drop Items: " << pTourn->GetString(L"dropitems") << L"  No Power-Ups: " << pTourn->GetString(L"nopowerups") << L"  No Respawn Timer: " << pTourn->GetString(L"norespawntimer") << newl
				<< L"Reverse Hero Select: " << pTourn->GetString(L"reverseheroselect") << L"  Duplicate Heroes: " << pTourn->GetString(L"dupliateheroes") << L"  Alt Hero Picking: " << pTourn->GetString(L"altheropicking") << newl
				<< L"------------------------------------------" << newl
				<< L"Team #1                   Team #2   " << newl
				<< L"------------------------------------------" << newl
				<< sTeam1Id1 << L"                         " << sTeam2Id1 << newl
				<< sTeam1Id2 << L"                         " << sTeam2Id2 << newl
				<< sTeam1Id3 << L"                         " << sTeam2Id3 << newl
				<< sTeam1Id4 << L"                         " << sTeam2Id4 << newl
				<< sTeam1Id5 << L"                         " << sTeam2Id5 << newl
				<< L"------------------------------------------" << newl
				<< L"Spectators:" << newl
				<< sSpectatorId1 <<    L"                         " << sSpectatorId2 << newl
				<< L"------------------------------------------" << newl
				<< L"Referees:" << newl
				<< SRefereeId1 <<     L"                         " << SRefereeId2 << newl
				<< L"------------------------------------------" << newl
				<< L"Team 1 Banned Heroes:" << newl
				<< L"------------------------------------------" << newl;

*/

	m_pConsole->Admin() << _T("Received tourn game response....") << newl;

	CPHPData phpResponse(sResponse);
	const CPHPData *pError(phpResponse.GetVar(L"error"));
	if (pError != NULL)
		return;

	uint uiTournListArrayNum(0);
	uint uiNewTournMatchCount(0);
	
	const CPHPData *phpTournList(phpResponse.GetVar(_T("tourn_game_list")));
	const CPHPData *phpTournInfo(phpTournList->GetVar(uiTournListArrayNum));

	if (phpTournInfo == NULL)
	{
		m_pConsole->Admin() << _T("RequestTournamentData: No information for given tournament.") << newl;
		return;
	}		

	while (phpTournInfo != NULL)
	{	
		bool bFoundMatch(false);
		uint uiTimeStamp = AtoI(phpTournInfo->GetString(L"tourn_nextmatchunixtime"));

		if (uiTimeStamp > 900)
		{
			// Matches that have already started or are more than 15 minutes away are irrelevant to us for now
			phpTournInfo = phpTournList->GetVar(++uiTournListArrayNum);
			continue;
		}

		//uint uiTournID = AtoI(phpTournInfo->GetString(L"tourn_id"));
		uint uiTournMatchID = AtoI(phpTournInfo->GetString(L"tourn_match_id"));		
				
		uint uiTeam1PlayerListSize(INT_SIZE(phpTournInfo->GetVar(_T("team1_account_list"))->GetSize()));
		uint uiTeam2PlayerListSize(INT_SIZE(phpTournInfo->GetVar(_T("team2_account_list"))->GetSize()));
		uint uiRefereePlayerListSize(INT_SIZE(phpTournInfo->GetVar(_T("referees_account_list"))->GetSize()));
		uint uiSpectatorPlayerListSize(INT_SIZE(phpTournInfo->GetVar(_T("spectators_account_list"))->GetSize()));

		MatchList_it itMatch(m_pMatches->begin()), itEnd(m_pMatches->end());
		while (itMatch != itEnd)
		{
			CTournMatch *pMatch(*itMatch);

			if (pMatch->GetID() == uiTournMatchID)
			{
				bFoundMatch = true;
				break;
			}
			++itMatch;
		}
		
		if (!bFoundMatch)
		{
			wstring sMatchName = phpTournInfo->GetString(L"tourn_nextmatchname");
			wstring sMatchSettings = L"map:" + phpTournInfo->GetString(L"map") + L" mode:" + phpTournInfo->GetString(L"gamemode") + L" teamsize:" + XtoW(uiTeam1PlayerListSize) + L" spectators:" + XtoW(uiSpectatorPlayerListSize) + L" referees:" + XtoW(uiRefereePlayerListSize);
			wstring sTournName = phpTournInfo->GetString(L"tourn_name");
			
			uint uiTournID = AtoI(phpTournInfo->GetString(_T("tourn_id")));
			
			// Create a new tourn match entry
			CTournMatch *pTournMatch(HEAP_NEW(m_pMemManager, m_pHeap) CTournMatch(m_pCore, m_pHeap, uiTournMatchID, uiTournID, WideToSingle(sTournName), uiTeam1PlayerListSize, uiTeam2PlayerListSize, uiRefereePlayerListSize, uiSpectatorPlayerListSize));
			if (pTournMatch == NULL)
				return;							
			//pTournMatch->SetupMatch(uiTournMatchID, sMatchName, sMatchSettings, uiTimeStamp);
				
				
			const CPHPData *phpTournInfoTeam1AccountList(phpTournInfo->GetVar(_T("team1_account_list")));
			const CPHPData *phpTournInfoTeam1PlayerList(phpTournInfo->GetVar(_T("team1_name_list")));			
			
			if (phpTournInfoTeam1AccountList != NULL && phpTournInfoTeam1PlayerList != NULL)
			{				
				uint uiTournInfoPlayerListArrayNum(0);
				const CPHPData *phpTournInfoTeam1Account(phpTournInfoTeam1AccountList->GetVar(uiTournInfoPlayerListArrayNum));
				const CPHPData *phpTournInfoTeam1Player(phpTournInfoTeam1PlayerList->GetVar(uiTournInfoPlayerListArrayNum));
				while (phpTournInfoTeam1Account != NULL && phpTournInfoTeam1Player != NULL)
				{			
					pTournMatch->AddTournClient(AtoI(phpTournInfoTeam1Account->GetString()), TEAM_1);	
									
					phpTournInfoTeam1Account = phpTournInfoTeam1AccountList->GetVar(++uiTournInfoPlayerListArrayNum);
					phpTournInfoTeam1Player = phpTournInfoTeam1PlayerList->GetVar(uiTournInfoPlayerListArrayNum);
				}								
			}				
			
			const CPHPData *phpTournInfoTeam2AccountList(phpTournInfo->GetVar(_T("team2_account_list")));
			const CPHPData *phpTournInfoTeam2PlayerList(phpTournInfo->GetVar(_T("team2_name_list")));
			
			if (phpTournInfoTeam2AccountList != NULL && phpTournInfoTeam2PlayerList != NULL)
			{				
				uint uiTournInfoPlayerListArrayNum(0);
				const CPHPData *phpTournInfoTeam2Account(phpTournInfoTeam2AccountList->GetVar(uiTournInfoPlayerListArrayNum));
				const CPHPData *phpTournInfoTeam2Player(phpTournInfoTeam2PlayerList->GetVar(uiTournInfoPlayerListArrayNum));
				while (phpTournInfoTeam2Account != NULL && phpTournInfoTeam2Player != NULL)
				{
					pTournMatch->AddTournClient(AtoI(phpTournInfoTeam2Account->GetString()), TEAM_2);	
					
					phpTournInfoTeam2Account = phpTournInfoTeam2AccountList->GetVar(++uiTournInfoPlayerListArrayNum);
					phpTournInfoTeam2Player = phpTournInfoTeam2PlayerList->GetVar(uiTournInfoPlayerListArrayNum);
				}								
			}	
			
			const CPHPData *phpTournInfoRefereeAccountList(phpTournInfo->GetVar(_T("referees_account_list")));
			const CPHPData *phpTournInfoRefereePlayerList(phpTournInfo->GetVar(_T("referees_name_list")));
			
			if (phpTournInfoRefereeAccountList != NULL && phpTournInfoRefereePlayerList != NULL)
			{				
				uint uiTournInfoPlayerListArrayNum(0);
				const CPHPData *phpTournInfoRefereeAccount(phpTournInfoRefereeAccountList->GetVar(uiTournInfoPlayerListArrayNum));
				const CPHPData *phpTournInfoRefereePlayer(phpTournInfoRefereePlayerList->GetVar(uiTournInfoPlayerListArrayNum));
				while (phpTournInfoRefereeAccount != NULL && phpTournInfoRefereePlayer != NULL)
				{
					pTournMatch->AddTournClient(AtoI(phpTournInfoRefereeAccount->GetString()), TEAM_REFEREE);			
					
					phpTournInfoRefereeAccount = phpTournInfoRefereeAccountList->GetVar(++uiTournInfoPlayerListArrayNum);
					phpTournInfoRefereePlayer = phpTournInfoRefereePlayerList->GetVar(uiTournInfoPlayerListArrayNum);
				}								
			}		
			
			const CPHPData *phpTournInfoSpectatorAccountList(phpTournInfo->GetVar(_T("spectators_account_list")));
			const CPHPData *phpTournInfoSpectatorPlayerList(phpTournInfo->GetVar(_T("spectators_name_list")));
			
			if (phpTournInfoSpectatorAccountList != NULL && phpTournInfoSpectatorPlayerList != NULL)
			{				
				uint uiTournInfoPlayerListArrayNum(0);
				const CPHPData *phpTournInfoSpectatorAccount(phpTournInfoSpectatorAccountList->GetVar(uiTournInfoPlayerListArrayNum));
				const CPHPData *phpTournInfoSpectatorPlayer(phpTournInfoSpectatorPlayerList->GetVar(uiTournInfoPlayerListArrayNum));
				while (phpTournInfoSpectatorAccount != NULL && phpTournInfoSpectatorPlayer != NULL)
				{
					pTournMatch->AddTournClient(AtoI(phpTournInfoSpectatorAccount->GetString()), TEAM_SPECTATOR);							
					
					phpTournInfoSpectatorAccount = phpTournInfoSpectatorAccountList->GetVar(++uiTournInfoPlayerListArrayNum);
					phpTournInfoSpectatorPlayer = phpTournInfoSpectatorPlayerList->GetVar(uiTournInfoPlayerListArrayNum);
				}								
			}
			

			// Add new match to list and try to create the tourn game, if no matching tourn_id challenge is received in 
			// TournMatchIsReady(), the next time the poller runs (5 min) it will re-attempt to create the match	

			/*
			if (!m_pServers->empty())
			{
				SServerEntry &server(m_pServers->front());							
				pTournMatch->Create(server.m_sAddress, server.m_unPort);
				m_pServers->pop_front();

				m_pConsole->Admin() << _T("Adding match #") << uiTournMatchID << _T(" to the match list...") << newl;
				uiNewTournMatchCount++;
				m_pMatches->push_back(pTournMatch);	
			}
			else
			{
				SAFE_DELETE(pTournMatch);		
				m_pConsole->Admin() << _T("Failed to add match #") << uiTournMatchID << _T(" to the match list, no available servers.") << newl;
			}
			/**/
		}
	
		phpTournInfo = phpTournList->GetVar(++uiTournListArrayNum);
	}
	
	m_pConsole->Admin() << L"Received " << uiTournListArrayNum << L" match(es), added " << uiNewTournMatchCount << L" match(es) to the match list currently containing " << INT_SIZE(m_pMatches->size()) << " match(es)..." << newl;
}


/*====================
  CGameManager::ProcessUserInfoInGameResponse
  ====================*/
void	CGameManager::ProcessRefreshServerListResponse(const wstring &sResponse)
{
	m_pConsole->Admin() << L"Received response for fresh tourn server list..." << newl;

	/*
	m_pServers->clear();	
	
	CPHPData phpResponse(sResponse);
	const CPHPData *pError(phpResponse.GetVar(L"error"));
	if (pError != NULL)
		return;
		
	// Cache the server list
	const CPHPData *pServerList(phpResponse.GetVar(L"server_list"));
	if (pServerList != NULL)
	{
		for (uint uiIndex(0); uiIndex < pServerList->GetSize(); ++uiIndex)
		{
			const CPHPData *pServer(pServerList->GetVar(uiIndex));
			if (pServer == NULL)
				continue;

			m_pServers->push_back(SServerEntry(WideToSingle(pServer->GetString(L"ip")), pServer->GetInteger(L"port")));
			m_pConsole->Admin() << (uiIndex + 1) << L") " << m_pServers->back().m_sAddress << L":" << m_pServers->back().m_unPort << newl;
		}
	}
	
	RequestTournamentGames();
	/**/
}


/*====================
  CGameManager::RequestServerList
  ====================*/
void	CGameManager::RequestServerList()
{
	CHTTPRequest *pRequest(m_pHTTPManager->SpawnRequest());
	if (pRequest == NULL)
		return;

	m_pConsole->Admin() << L"Requesting fresh tourn server list..." << newl;

	pRequest->Reset();
	pRequest->SetType(GAME_MANAGER_REFRESH_SERVER_LIST);
	pRequest->SetTargetURL("sl1.hon.s2games.com/client_requester_jt.php");
	pRequest->AddVariable(L"f", L"open_server_list_test_jt");
	pRequest->AddVariable(L"cookie", m_sCookie);
	pRequest->AddVariable(L"gametype", L"90");
	pRequest->AddVariable(L"region", L"TST");
	//pRequest->AddVariable(L"limit", L"1");	
	pRequest->SendPostRequest();
	m_pHTTPRequests->push_back(pRequest);	
}


/*====================
  CGameManager::RequestTournamentGames
  ====================*/
void	CGameManager::RequestTournamentGames()
{
		CHTTPRequest *pRequest(m_pHTTPManager->SpawnRequest());
		if (pRequest == NULL)
			return;

		m_pConsole->Admin() << L"Requesting tournament game list..." << newl;

		pRequest->Reset();
		pRequest->SetType(GAME_MANAGER_GET_TOURNAMENT_GAMES);
		pRequest->SetTargetURL("tournaments.heroesofnewerth.com/tourn_requester.php");
		pRequest->AddVariable(_T("f"), _T("get_tournament_game_list"));
		pRequest->SendPostRequest();
		m_pHTTPRequests->push_back(pRequest);
}


/*====================
  CGameManager::TournMatchIsReady
  ====================*/
void	CGameManager::TournMatchIsReady(uint uiTournMatchID, uint uiChallenge)
{
	CTournMatch *pTournMatch = NULL;
	bool bFoundMatch(false);

	MatchList_it itMatch(m_pMatches->begin()), itEnd(m_pMatches->end());
	while (itMatch != itEnd)
	{
		pTournMatch = (*itMatch);

		if ((pTournMatch->GetID() == uiTournMatchID) && (pTournMatch->GetChallenge() == uiChallenge))
		{
			bFoundMatch = true;
			break;
		}
		++itMatch;
	}
	
	if (!bFoundMatch) 
		return;

	// need to send a http request to master server indicating the actual matchid and that the tournament game has started
	// so the next time gamemanager polls it sees the game was made and permanently removes it from the list of upcoming tourn games
	pTournMatch->Announce();
	
	m_pConsole->Admin() << L"Announcing tourn match #" << uiTournMatchID << newl;

	m_pMatches->erase(itMatch);

	SAFE_DELETE(pTournMatch);
}

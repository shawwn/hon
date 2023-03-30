// (C)2009 S2 Games
// c_matchmaker.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "chatserver_common.h"

#include "c_matchmaker.h"
#include "c_match.h"
#include "c_client.h"
#include "c_httprequest.h"
#include "c_phpdata.h"
#include "c_console.h"
#include "c_netmanager.h"
#include "c_gameservermanager.h"
#include "c_gameserver.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
CVAR(uint,		matchmaker_teamSize,					5);
CVAR(bool,		matchmaker_disabled,					true);
CVAR(uint,		matchmaker_minPotentialMatches,			2);
CVAR(uint,		matchmaker_provisionalCount,			5);
CVAR(wstring,	matchmaker_allowedRegions,				L"USW USE EU");

const wstring g_sRegionNames[] =
{
	L"TST",
	L"USW",
	L"USE",
	L"EU",
	L"SG",
	L"MY",
	L"VN",
	L"PH",
	WSNULL
};
//=============================================================================

/*====================
  CMatchMaker::~CMatchMaker
  ====================*/
CMatchMaker::~CMatchMaker()
{
	for (uint uiRegion(0); uiRegion < NUM_REGIONS; ++uiRegion)
	{
		MatchVector *pMatches(m_apMatches[uiRegion]);
		for (MatchVector_it itMatch(pMatches->begin()), itEnd(pMatches->end()); itMatch != itEnd; ++itMatch)
			delete *itMatch;

		delete pMatches;
		delete m_apFreeMatchIndices[uiRegion];
	}

	for (MatchList_it itMatch(m_pPendingMatches->begin()), itEnd(m_pPendingMatches->end()); itMatch != itEnd; ++itMatch)
		delete *itMatch;
	delete m_pPendingMatches;

	delete m_pPotentialMatches;

	delete m_pSTLHeap;
	delete m_pHeap;
}


/*====================
  CMatchMaker::CMatchMaker
  ====================*/
CMatchMaker::CMatchMaker() :
NULL_CORE_API,
m_pHeap(NULL),
m_pSTLHeap(NULL),

m_uiMatchCounter(0),

m_pPendingMatches(NULL),
m_pPotentialMatches(NULL),

m_uiLastPeriod(0),
m_uiTotalJoins(0),
m_uiMatchesRequested(0),
m_uiMatchesCreated(0),
m_uiMatchesFailed(0),
m_uiTotalWaitTime(0)
{
	memset(m_aTotalJoins, 0, sizeof(uint) * NUM_REGIONS);
	memset(m_aRecentJoins, 0, sizeof(uint) * NUM_REGIONS * 12);
	memset(m_aTotalCancels, 0, sizeof(uint) * NUM_REGIONS);
}


/*====================
  CMatchMaker::GetRegionFromName
  ====================*/
uint	CMatchMaker::GetRegionFromName(const wstring &sName)
{
	if (sName == L"TST")
		return REGION_TST;
	else if (sName == L"USW")
		return REGION_USW;
	else if (sName == L"USE")
		return REGION_USE;
	else if (sName == L"EU")
		return REGION_EU;
	else if (sName == L"SG")
		return REGION_SG;
	else if (sName == L"MY")
		return REGION_MY;
	else if (sName == L"VN")
		return REGION_VN;
	else if (sName == L"PH")
		return REGION_PH;
	else
		return -1;
}


/*====================
  CMatchMaker::GetRegionFlagFromName
  ====================*/
uint	CMatchMaker::GetRegionFlagFromName(const wstring &sName)
{
	if (sName == L"TST")
		return BIT(REGION_TST);
	else if (sName == L"USW")
		return BIT(REGION_USW);
	else if (sName == L"USE")
		return BIT(REGION_USE);
	else if (sName == L"EU")
		return BIT(REGION_EU);
	else if (sName == L"SG")
		return BIT(REGION_SG);
	else if (sName == L"MY")
		return BIT(REGION_MY);
	else if (sName == L"VN")
		return BIT(REGION_VN);
	else if (sName == L"PH")
		return BIT(REGION_PH);
	else
		return 0;
}


/*====================
  CMatchMaker::Initialize
  ====================*/
void	CMatchMaker::Initialize(CCore *pCore)
{
	INIT_CORE_API(pCore);

	m_pHeap = m_pMemManager->AllocateHeap("matcher");
	m_pSTLHeap = m_pMemManager->AllocateHeap("matcher_stl");

	for (uint uiRegion(0); uiRegion < NUM_REGIONS; ++uiRegion)
	{
		m_apMatches[uiRegion] = HEAP_NEW(m_pMemManager, m_pHeap) MatchVector(MatchVector_alloc(m_pMemManager, m_pSTLHeap));
		m_apFreeMatchIndices[uiRegion] = HEAP_NEW(m_pMemManager, m_pHeap) uiVector(uiVector_alloc(m_pMemManager, m_pSTLHeap));

		while (m_apMatches[uiRegion]->size() < matchmaker_minPotentialMatches)
			SpawnNewMatch(uiRegion);
	}

	m_pPendingMatches = HEAP_NEW(m_pMemManager, m_pHeap) MatchList(MatchList_alloc(m_pMemManager, m_pSTLHeap));
	m_pPotentialMatches = HEAP_NEW(m_pMemManager, m_pHeap) MatchVector(MatchVector_alloc(m_pMemManager, m_pSTLHeap));

	m_pConsole->MatchMaker() << L"MatchMaker initialized" << newl;
}


/*====================
  GetRegionName
  ====================*/
const wstring&	GetRegionName(uint uiRegion)
{
	return g_sRegionNames[CLAMP(uiRegion, 0u, uint(NUM_REGIONS))];
}


/*====================
  CMatchMaker::Frame
  ====================*/
void	CMatchMaker::Frame()
{
	PROFILE("CMatchMaker::Frame");

	uint uiCurrentPeriod(GetCurrentPeriod());
	if (m_uiLastPeriod != uiCurrentPeriod)
	{
		for (uint uiRegion(0); uiRegion < NUM_REGIONS; ++uiRegion)
			m_aRecentJoins[uiRegion][uiCurrentPeriod] = 0;

		m_uiLastPeriod = uiCurrentPeriod;
	}

	// Send out requests to start any matches that are ready
	for (MatchList_it itMatch(m_pPendingMatches->begin()), itEnd(m_pPendingMatches->end()); itMatch != itEnd; )
	{
		CMatch *pMatch(*itMatch);

		// Request an available server
		CGameServer *pServer(m_pGameServerManager->GetServerFromLocation(pMatch->GetRegionName()));
		if (pServer != NULL)
		{
			// Create the new match, the CMatch* becomes the responsibility of the CServer it is assigned to
			m_pConsole->MatchMaker()
				<< L"[" << m_pCore->GetTimeString() << L"]"
				<< L" Starting match #" << pMatch->GetID()
				<< L" (" << pMatch->GetRegionName() << L":" << pMatch->GetRegionalIndex() << L")"
				<< L" on server: " << pServer->GetName()
				<< newl;

			m_uiTotalWaitTime += pMatch->GetWaitTime();
			++m_uiMatchesRequested;

			pServer->CreateMatch(pMatch);
			itMatch = m_pPendingMatches->erase(itMatch);
			continue;
		}

		++itMatch;
	}
}


/*====================
  CMatchMaker::Shutdown
  ====================*/
void	CMatchMaker::Shutdown()
{
}


/*====================
  CMatchMaker::SpawnNewMatch
  ====================*/
CMatch*		CMatchMaker::SpawnNewMatch(uint uiRegion)
{
	// Allocate new match
	CMatch *pNewMatch(HEAP_NEW(m_pMemManager, m_pHeap) CMatch(m_pCore, m_pHeap));
	
	// Find an available slot in the vector
	uint uiRegionalIndex(0);
	if (m_apFreeMatchIndices[uiRegion]->empty())
	{
		uiRegionalIndex = INT_SIZE(m_apMatches[uiRegion]->size());
		m_apMatches[uiRegion]->push_back(pNewMatch);
	}
	else
	{
		uiRegionalIndex = m_apFreeMatchIndices[uiRegion]->back();
		m_apFreeMatchIndices[uiRegion]->pop_back();
		m_apMatches[uiRegion]->at(uiRegionalIndex) = pNewMatch;
	}

	// Setup initial match settigns
	pNewMatch->SetMatchID(m_uiMatchCounter);
	pNewMatch->SetMatchSize(matchmaker_teamSize * 2);
	pNewMatch->SetRegion(GetRegionName(uiRegion), uiRegion);
	pNewMatch->SetRegionalIndex(uiRegionalIndex);

	++m_uiMatchCounter;

	m_pConsole->MatchMaker()
		<< L"[" << m_pCore->GetTimeString() << L"]"
		<< L" Spawned new match #" << pNewMatch->GetID()
		<< L"(" << pNewMatch->GetRegionName() << L":" << pNewMatch->GetRegionalIndex() << L")"
		<< newl;

	return pNewMatch;
}


/*====================
  CMatchMaker::AddClient
  ====================*/
void	CMatchMaker::AddClient(CClient *pClient, uint uiRegionFlags)
{
	if (pClient == NULL || matchmaker_disabled || uiRegionFlags == 0)
		return;

	if (pClient->IsLeaver() && !matchmaker_allowLeavers)
		return;

	m_pConsole->MatchMaker()
		<< L"[" << m_pCore->GetTimeString() << L"]"
		<< L" Adding player to MatchMaker pool: " << pClient->GetName()
		<< L" " << pClient->GetRating(MATCH_POOL_SOLO)
		<< L" " << pClient->GetMatchCount(MATCH_POOL_SOLO)
		<< newl;

	// Check for matches needing a substitute

	// Search for all potential matches this player can join
	m_pPotentialMatches->clear();

	for (uint uiRegion(0); uiRegion < NUM_REGIONS; ++uiRegion)
	{
		if ((BIT(uiRegion) & uiRegionFlags) == 0)
			continue;

		for (MatchVector_it itMatch(m_apMatches[uiRegion]->begin()), itEnd(m_apMatches[uiRegion]->end()); itMatch != itEnd; ++itMatch)
		{
			CMatch *pMatch(*itMatch);
			if (pMatch == NULL)
				continue;

			if (pMatch->CanJoin(pClient))
				m_pPotentialMatches->push_back(pMatch);
		}
	}

	// If the player can't join any matches, create a new one
	if (m_pPotentialMatches->empty())
	{
		// Randomly choose from acceptable regions
		uint uiRegion(0);
		while ((BIT(uiRegion) & uiRegionFlags) == 0) uiRegion = rand() % NUM_REGIONS;
		CMatch *pNewMatch(SpawnNewMatch(uiRegion));
		pNewMatch->AddClient(pClient);

		IncrementJoinCounters(uiRegion);
		return;
	}

	// Randomly join one of the valid matches
	CMatch *pMatch(m_pPotentialMatches->at(rand() % m_pPotentialMatches->size()));
	pMatch->AddClient(pClient);

	IncrementJoinCounters(pMatch->GetRegion());
	
	// Start the match if it is ready
	if (pMatch->IsReady())
		CreateMatch(pMatch);
}


/*====================
  CMatchMaker::CreateMatch
  ====================*/
void	CMatchMaker::CreateMatch(CMatch *pMatch)
{
	m_pConsole->MatchMaker()
		<< L"[" << m_pCore->GetTimeString() << L"]"
		<< L" Preparing match #" << pMatch->GetID()
		<< L"(" << pMatch->GetRegionName() << L":" << pMatch->GetRegionalIndex() << L")"
		<< newl;

	// Add to pending match list
	m_pPendingMatches->push_back(pMatch);

	// Remove from region list
	uint uiRegion(pMatch->GetRegion());
	m_apMatches[uiRegion]->at(pMatch->GetRegionalIndex()) = NULL;
	m_apFreeMatchIndices[uiRegion]->push_back(pMatch->GetRegionalIndex());

	// Make sure there is a minimum number of potential matches
	while (m_apMatches[uiRegion]->size() - m_apFreeMatchIndices[uiRegion]->size() < matchmaker_minPotentialMatches)
		SpawnNewMatch(uiRegion);
}


/*====================
  CMatchMaker::RequestSubstitute
  ====================*/
CClient*	CMatchMaker::RequestSubstitute(CMatch *pMatch, uint uiOldAccountID)
{
	m_pConsole->MatchMaker()
		<< L"[" << m_pCore->GetTimeString() << L"]"
		<< L" Substitution request for match #" << pMatch->GetID()
		<< L"(" << pMatch->GetRegionName() << L":" << pMatch->GetRegionalIndex() << L")"
		<< L" player #" << uiOldAccountID
		<< newl;

	ClientVector vSubstitutes(ClientVector_alloc(m_pMemManager, m_pSTLHeap));

	// Search players sitting in the pool for a suitable candidate to grab
	MatchVector *pMatches(m_apMatches[pMatch->GetRegion()]);
	for (MatchVector_it itMatch(pMatches->begin()), itEnd(pMatches->end()); itMatch != itEnd; ++itMatch)
	{
		CMatch *pSearchMatch(*itMatch);
		if (pSearchMatch == NULL || pSearchMatch == pMatch)
			continue;

		pSearchMatch->GetSubstitute(pMatch, uiOldAccountID, vSubstitutes);
	}

	// If no suitable player was found, store the request
	if (vSubstitutes.empty())
	{
		return NULL;
	}

	// Randomly select one of the viable clients
	std::random_shuffle(vSubstitutes.begin(), vSubstitutes.end());
	return vSubstitutes.back();
}


/*====================
  CMatchMaker::PrintUsage
  ====================*/
void	CMatchMaker::PrintUsage()
{
	m_pConsole->Admin()
		<< newl
		<< L"MatchMaker         " << newl
		<< L"----------         " << newl
		<< L"Pending Matches:   " << INT_SIZE(m_pPendingMatches->size()) << newl
		<< L"Matches Requested: " << m_uiMatchesRequested << newl
		<< L"Matches Created:   " << m_uiMatchesCreated << L" (" << INT_ROUND(100.0f * (m_uiMatchesRequested > 0 ? (1.0f - (m_uiMatchesCreated / float(m_uiMatchesRequested))) : 0.0f)) << L"%)" << newl
		<< L"Matches Failed     " << m_uiMatchesFailed << L" (" << INT_ROUND(100.0f * (m_uiMatchesCreated > 0 ? (1.0f - (m_uiMatchesFailed / float(m_uiMatchesCreated))) : 0.0f)) << L"%)" << L" (" << INT_ROUND(100.0f * (m_uiMatchesRequested > 0 ? (1.0f - (m_uiMatchesFailed / float(m_uiMatchesRequested))) : 0.0f)) << L"%)" << newl
		<< L"Average wait:      " << (m_uiMatchesCreated > 0 ? m_uiTotalWaitTime / float(m_uiMatchesCreated) : 0) << " sec" << newl;

	uint uiNumPeriods(MIN((m_pCore->GetSeconds() / 300) + 1, 12u));

	for (uint uiRegion(0); uiRegion < NUM_REGIONS; ++uiRegion)
	{
		uint uiRecentTotal(0);
		for (uint uiPeriod(0); uiPeriod < uiNumPeriods; ++uiPeriod)
			uiRecentTotal += m_aRecentJoins[uiRegion][uiPeriod];

		m_pConsole->Admin()
			<< XtoW(g_sRegionNames[uiRegion] + L":", FMT_ALIGNLEFT, 4)
			<< XtoW(XtoW(INT_SIZE(m_apMatches[uiRegion]->size() - m_apFreeMatchIndices[uiRegion]->size())) + L"/" + XtoW(INT_SIZE(m_apMatches[uiRegion]->size())), FMT_NONE, 7)
			<< L" " << XtoW(m_aTotalJoins[uiRegion], FMT_DELIMIT, 7) << " joins"
			<< L" (" << XtoA(m_aTotalJoins[uiRegion] / (m_pCore->GetSeconds() / 60.0f), FMT_NONE, 5, 1) << L" per min)"
			<< L" " << XtoW(m_aTotalCancels[uiRegion], FMT_DELIMIT, 7) << " cancels"
			<< L" (" << XtoA(m_aTotalJoins[uiRegion] > 0 ? m_aTotalCancels[uiRegion] / float(m_aTotalJoins[uiRegion]) * 100.0f : 0.0f, FMT_NONE, 3, 0) << L"%)"
			<< L" (" << XtoA(m_uiTotalJoins > 0 ? m_aTotalJoins[uiRegion] / float(m_uiTotalJoins) * 100.0f : 0.0f, FMT_NONE, 3, 0) << L"%)"
			<< L" " << uiRecentTotal << L" this hour"
			<< newl;
	}
}


/*====================
  CMatchMaker::PrintRegionDetails
  ====================*/
void	CMatchMaker::PrintRegionDetails(const wstring &sRegion)
{
	uint uiRegion(GetRegionFromName(sRegion));
	if (uiRegion >= NUM_REGIONS)
		return;

	m_pConsole->Admin() << L"[" << sRegion << L"]" << newl;

	MatchVector *pMatches(m_apMatches[uiRegion]);
	for (MatchVector_it itMatch(pMatches->begin()), itEnd(pMatches->end()); itMatch != itEnd; ++itMatch)
	{
		CMatch *pMatch(*itMatch);
		if (pMatch == NULL)
			continue;

		if (pMatch->IsEmpty())
		{
			m_pConsole->Admin()
			<< L" #" << pMatch->GetID() << newl
			<< L" *empty*" << newl
			<< newl;
			continue;
		}

		m_pConsole->Admin()
			<< L" #" << pMatch->GetID() << newl
			<< L" Players: " << pMatch->GetPlayerCount() << "/" << pMatch->GetMatchSize() << newl
			<< L" Wait time: " << m_pCore->GetSeconds() - pMatch->GetTimeStamp() << newl
			<< L" Mean Rating: " << INT_ROUND(pMatch->GetMeanRating()) << newl
			<< L" Allowed range: " << INT_ROUND(pMatch->GetMeanRating() - pMatch->GetMaxSpread()) << L" - " << INT_ROUND(pMatch->GetMeanRating() + pMatch->GetMaxSpread()) << newl
			<< L" Ready: " << pMatch->IsReady() << newl
			<< L" Provisional: " << pMatch->IsProvisional() << (pMatch->IsStrictProvisional() ? L" [strict]" : L" [open]") << newl
			<< L" Leavers: " << pMatch->GetAllowLeavers() << newl
			<< newl;
	}
}

// (C)2009 S2 Games
// c_match.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "chatserver_common.h"

#include "c_match.h"
#include "c_matchmaker.h"
#include "c_httprequest.h"
#include "c_phpdata.h"
#include "c_netmanager.h"
#include "c_console.h"
#include "c_client.h"
#include "c_gameserver.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
CVAR(wstring,	matchmaker_matchSettings,			L"map:caldavar mode:bd teamsize:5");
CVAR(float,		matchmaker_teamRankWeighting,		6.5f);
CVAR(float,		matchmaker_logisticPredictionScale,	80.0f);
CVAR(float,		matchmaker_spreadBase,				10.0f);
CVAR(float,		matchmaker_spreadScale,				0.3333f);
CVAR(float,		matchmaker_spreadMax,				150.0f);
CVAR(float,		matchmaker_spreadPower,				1.5f);
CVAR(bool,		matchmaker_allowLeavers,			false);
CVAR(uint,		matchmaker_openMatchTime,			180);
//=============================================================================

/*====================
  CMatch::~CMatch
  ====================*/
CMatch::~CMatch()
{
	for (ClientDeque_it itClient(m_pClients->begin()), itEnd(m_pClients->end()); itClient != itEnd; ++itClient)
		(*itClient)->LeaveMatch(this);

	delete m_pClients;
	delete m_apTeams[0];
	delete m_apTeams[1];
}


/*====================
  CMatch::CMatch
  ====================*/
CMatch::CMatch(CCore *pCore, CHeap *pMatchMakerHeap) :
NULL_CORE_API,
m_pHeap(pMatchMakerHeap),

m_uiID(0),
m_uiChallenge(rand()),
m_uiRegion(0),
m_uiRegionalIndex(0),
m_bReady(false),
m_bArrangedMatch(true),
m_bProvisional(false),
m_bLeavers(false),
m_uiTimeStamp(INVALID_TIME),
m_uiMatchSize(0),
m_fMeanRating(0.0f)
{
	INIT_CORE_API(pCore);

	m_pClients = HEAP_NEW(m_pMemManager, m_pHeap) ClientDeque(ClientDeque_alloc(m_pMemManager, m_pHeap));
	m_apTeams[0] = HEAP_NEW(m_pMemManager, m_pHeap) ClientVector(ClientVector_alloc(m_pMemManager, m_pHeap));
	m_apTeams[1] = HEAP_NEW(m_pMemManager, m_pHeap) ClientVector(ClientVector_alloc(m_pMemManager, m_pHeap));
}


/*====================
  CMatch::CanJoin
  ====================*/
bool	CMatch::CanJoin(CClient *pClient) const
{
	if (pClient == NULL)
		return false;

	// Check for an empty match, these are up for grabs
	if (m_pClients->empty())
		return true;

	// Check for a full match
	if (m_pClients->size() >= m_uiMatchSize)
		return false;

	// Check leaver status
	if (pClient->IsLeaver() && !matchmaker_allowLeavers)
		return false;
	if (pClient->IsLeaver() != m_bLeavers)
		return false;

	// Check that the rating is within range
	// Acceptable range of rating grows on an exponential curve as the match ages
	uint uiSeconds(m_pCore->GetSeconds() - m_uiTimeStamp);
	float fMaxSpread(GetMaxSpread());
	if (fabs(pClient->GetRating(MATCH_POOL_SOLO) - m_fMeanRating) > fMaxSpread)
		return false;

	// Check provisional status
	if (pClient->GetMatchCount(MATCH_POOL_SOLO) < matchmaker_provisionalCount)
	{
		// Provisional clients can only ever join a provisional match
		return m_bProvisional;
	}
	else
	{
		// Non provisional clients can join provisional matches that have been waiting for too long
		if (uiSeconds >= matchmaker_openMatchTime)
			return true;

		return !m_bProvisional;
	}

	return true;
}


/*====================
  CMatch::CalculateMeanRating
  ====================*/
void	CMatch::CalculateMeanRating()
{
	m_fMeanRating = 0.0f;
	if (m_pClients->empty())
		return;

	for (ClientDeque_it itClient(m_pClients->begin()), itEnd(m_pClients->end()); itClient != itEnd; ++itClient)
		m_fMeanRating += (*itClient)->GetRating(MATCH_POOL_SOLO);

	m_fMeanRating /= m_pClients->size();
}


/*====================
  CMatch::GetMatchPrediction
  ====================*/
float	CMatch::GetMatchPrediction(int iTeam) const
{
	ClientVector *pTeam(m_apTeams[iTeam]);
	ClientVector *pOtherTeam(m_apTeams[iTeam ^ 1]);

	// Calculate team ratings
	float fRating(0.0f);
	for (ClientVector_cit itClient(pTeam->begin()), itEnd(pTeam->end()); itClient != itEnd; ++itClient)
	{
		if (*itClient == NULL)
			continue;

		fRating += pow((*itClient)->GetRating(MATCH_POOL_SOLO), matchmaker_teamRankWeighting);
	}
	float fTeamRating(INT_ROUND(pow(fRating, 1.0f / matchmaker_teamRankWeighting)));

	fRating = 0.0f;
	for (ClientVector_it itClient(pOtherTeam->begin()), itEnd(pOtherTeam->end()); itClient != itEnd; ++itClient)
	{
		if (*itClient == NULL)
			continue;

		fRating += pow((*itClient)->GetRating(MATCH_POOL_SOLO), matchmaker_teamRankWeighting);
	}
	float fOtherTeamRating(INT_ROUND(pow(fRating, 1.0f / matchmaker_teamRankWeighting)));

	// Get the current prediction
	return 1.0f / (1.0f + pow(M_E, -(fTeamRating - fOtherTeamRating) / matchmaker_logisticPredictionScale));
}


/*====================
  CMatch::CompareClientRating
  ====================*/
bool	CMatch::CompareClientRating(CClient *pClientA, CClient *pClientB)
{
	if (pClientA == NULL)
		return false;

	if (pClientB == NULL)
		return true;

	if (pClientA->GetRating(MATCH_POOL_SOLO) > pClientB->GetRating(MATCH_POOL_SOLO))
		return true;

	return false;
}


/*====================
  CMatch::AssignTeams
  ====================*/
void	CMatch::AssignTeams()
{
	// Clear existing teams
	m_apTeams[0]->clear();
	m_apTeams[1]->clear();

	// Sort the players by rank
	std::sort(m_pClients->begin(), m_pClients->end(), CompareClientRating);

	int iCurrentTeam(rand() & 1);
	ClientVector *pTeam(m_apTeams[iCurrentTeam]);
	ClientVector *pOtherTeam(m_apTeams[iCurrentTeam ^ 1]);
	float fTeamRating(0.0f);
	float fOtherTeamRating(0.0f);

	// Start assigning (first pass)
	for (ClientDeque_it itClient(m_pClients->begin()), itClientEnd(m_pClients->end()); itClient != itClientEnd; ++itClient)
	{
		// Put client on a team
		pTeam->push_back(*itClient);

		// Recalculate team rating
		float fRating(0.0f);
		for (ClientVector_it itTeam(pTeam->begin()), itTeamEnd(pTeam->end()); itTeam != itTeamEnd; ++itTeam)
			fRating += pow((*itTeam)->GetRating(MATCH_POOL_SOLO), matchmaker_teamRankWeighting);
		fTeamRating = pow(fRating, 1.0f / matchmaker_teamRankWeighting);
		
		if (pTeam->size() >= m_uiMatchSize / 2 || (fTeamRating > fOtherTeamRating && pOtherTeam->size() < m_uiMatchSize / 2))
		{
			SWAP(pTeam, pOtherTeam);
			SWAP(fTeamRating, fOtherTeamRating);
		}
	}

	// Make a pass at each player and look for improvements
	for (uint uiTeamA(0); uiTeamA < pTeam->size(); ++uiTeamA)
	{
		for (uint uiTeamB(0); uiTeamB < pOtherTeam->size(); ++uiTeamB)
		{
			// Get the current prediction
			float fOldProbability(GetMatchPrediction(iCurrentTeam));
			
			// Try swapping players
			SWAP(pTeam->at(uiTeamA), pOtherTeam->at(uiTeamB));
			
			// Calculate new prediction
			float fNewProbability(GetMatchPrediction(iCurrentTeam));
			
			// Swap back if this didn't improve the balance
			if (fabs(fOldProbability - 0.5f) < fabs(fNewProbability - 0.5f))
				SWAP(pTeam->at(uiTeamA), pOtherTeam->at(uiTeamB));
		}
	}

	// Sort teams so that captains are the highest rated player
	std::sort(pTeam->begin(), pTeam->end(), CompareClientRating);
	std::sort(pOtherTeam->begin(), pOtherTeam->end(), CompareClientRating);
}


/*====================
  CMatch::Start
  ====================*/
void	CMatch::Start()
{
	AssignTeams();
	m_uiTimeStamp = m_pCore->GetSeconds();

	// Update clients that the match has been created
	for (int iTeam(0); iTeam < 2; ++iTeam)
	{
		for (uint ui(0); ui < m_apTeams[iTeam]->size(); ++ui)
		{
			if (m_apTeams[iTeam]->at(ui) != NULL)
				m_apTeams[iTeam]->at(ui)->SendMatchMakerUpdate(MATCHMAKER_STATUS_WAITING_FOR_SERVER);
		}
	}
}


/*====================
  CMatch::AnnounceMatchReady
  ====================*/
void	CMatch::AnnounceMatchReady(const string &sAddress, ushort unPort)
{
	m_pMatchMaker->IncrementCreatedCounter();

	CBufferFixed<24> buffer;
	buffer << CHAT_CMD_AUTO_MATCH_CONNECT << m_uiID << StringToUTF8(sAddress) << byte(0) << unPort;

	for (int iTeam(0); iTeam < 2; ++iTeam)
	{
		for (uint ui(0); ui < m_apTeams[iTeam]->size(); ++ui)
		{
			if (m_apTeams[iTeam]->at(ui) != NULL)
				m_apTeams[iTeam]->at(ui)->Send(buffer);
		}
	}
}


/*====================
  CMatch::GetSubstitute
  ====================*/
void	CMatch::GetSubstitute(CMatch *pOtherMatch, uint uiOldAccountID, ClientVector &vSubstitutes)
{
	for (ClientDeque_it itClient(m_pClients->begin()), itEnd(m_pClients->end()); itClient != itEnd; ++itClient)
	{
		CClient *pSubstitute(*itClient);
		if (pOtherMatch->CanJoin(pSubstitute))
			vSubstitutes.push_back(pSubstitute);
	}
}


/*====================
  CMatch::InvalidateClient
  ====================*/
void	CMatch::InvalidateClient(CClient *pClient, const wstring &sReason)
{	
	ClientDeque_it itClient(std::find(m_pClients->begin(), m_pClients->end(), pClient));
	if (itClient != m_pClients->end())
	{
		m_pConsole->MatchMaker()
			<< L"[" << m_pCore->GetTimeString() << L"]"
			<< L" Removing client "
			<< pClient->GetName() << L" #" << pClient->GetAccountID()
			<< L" from match #" << GetID()
			<< L" (" << GetRegionName() << L":" << GetRegionalIndex() << L")"
			<< L" - " << sReason
			<< newl;

		m_pClients->erase(itClient);
	}

	for (int iTeam(0); iTeam < 2; ++iTeam)
	{
		for (uint ui(0); ui < m_apTeams[iTeam]->size(); ++ui)
		{
			if (m_apTeams[iTeam]->at(ui) == pClient)
				m_apTeams[iTeam]->at(ui) = NULL;
		}
	}

	CalculateMeanRating();
}


/*====================
  CMatch::AddClient
  ====================*/
void	CMatch::AddClient(CClient *pClient)
{
	m_pConsole->MatchMaker()
		<< L"[" << m_pCore->GetTimeString() << L"]"
		<< L" Adding client "
		<< pClient->GetName() << L" #" << pClient->GetAccountID()
		<< L" to match #" << GetID()
		<< L" (" << GetRegionName() << L":" << GetRegionalIndex() << L")"
		<< newl;
	
	// Reset values when first client joins
	if (m_pClients->empty())
	{
		m_bLeavers = pClient->IsLeaver();
		m_bProvisional = pClient->GetMatchCount(MATCH_POOL_SOLO) < matchmaker_provisionalCount;
		m_uiTimeStamp = m_pCore->GetSeconds();
	}

	m_pClients->push_back(pClient);

	pClient->JoinMatch(this);

	// Update clients with current player counts
	if (!m_bReady)
	{
		for (ClientDeque_it itClient(m_pClients->begin()), itEnd(m_pClients->end()); itClient != itEnd; ++itClient)
			(*itClient)->SendMatchMakerUpdate(MATCHMAKER_STATUS_IN_POOL, INT_SIZE(m_pClients->size()), m_uiMatchSize);
	}

	CalculateMeanRating();

	// Flag the match as ready if this client filled it up
	if (m_pClients->size() == m_uiMatchSize && !m_bReady)
	{
		m_bReady = true;
		for (ClientDeque_it itClient(m_pClients->begin()), itEnd(m_pClients->end()); itClient != itEnd; ++itClient)
			(*itClient)->SendMatchMakerUpdate(MATCHMAKER_STATUS_LOOKING_FOR_SERVER, m_uiID);
	}
}


/*====================
  CMatch::WriteMatchInfo
  ====================*/
void	CMatch::WriteMatchInfo(CPacket &pkt)
{
	pkt
		<< NET_CHAT_GS_CREATE_MATCH			// Command
		<< m_uiID							// ID
		<< m_uiChallenge					// Challenge
		<< (L"Automatch #" + XtoW(m_uiID))	// Name
		<< matchmaker_matchSettings;		// Settings

	// Placeholder for player count
	uint *pCount(reinterpret_cast<uint*>(pkt.LockBuffer(sizeof(uint))));
	*pCount = 0;

	// Build team lists
	for (int iTeam(0); iTeam < 2; ++iTeam)
	{
		for (uint uiSlot(0); uiSlot < m_apTeams[iTeam]->size(); ++uiSlot)
		{
			if (m_apTeams[iTeam]->at(uiSlot) == NULL)
				continue;

			pkt << m_apTeams[iTeam]->at(uiSlot)->GetAccountID() << iTeam + 1 << uiSlot;
			++(*pCount);
		}
	}
}

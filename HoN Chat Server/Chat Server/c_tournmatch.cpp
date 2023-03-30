// (C)2009 S2 Games
// c_tournmatch.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "chatserver_common.h"

#include "c_tournmatch.h"
#include "c_httprequest.h"
#include "c_phpdata.h"
#include "c_console.h"
#include "c_netmanager.h"
#include "c_clientmanager.h"
//=============================================================================

/*====================
  CTournMatch::CTournMatch
  ====================*/
CTournMatch::CTournMatch(CCore *pCore, CHeap *pHeap, uint uiTournMatchID, uint uiTournID, const string &sTournName, uint uiTeam1Size, uint uiTeam2Size, uint uiRefereeSize, uint uiSpectatorSize) :
CMatch(pCore, pHeap),
m_bTournMatch(true),
m_vTeam1Clients(uiTeam1Size, NULL),
m_vTeam2Clients(uiTeam2Size, NULL),
m_vReferees(uiRefereeSize, NULL),
m_vSpectators(uiSpectatorSize, NULL),
m_uiTournID(uiTournID),
m_sTournName(sTournName)
{
}


/*====================
  CTournMatch::AddTournClient
  ====================*/
void	CTournMatch::AddTournClient(uint AccountID, int uiTeam)
{	
	uint uiSlot = 0;
			
	if (uiTeam == TEAM_1)
	{
		for (uiSlot = 0; uiSlot < m_vTeam1Clients.size(); uiSlot++)
		{
			if (m_vTeam1Clients[uiSlot] != NULL)
				continue;

			m_vTeam1Clients[uiSlot] = AccountID;
			return;
		}	
	}
	
	if (uiTeam == TEAM_2)
	{
		for (uiSlot = 0; uiSlot < m_vTeam2Clients.size(); uiSlot++)
		{
			if (m_vTeam2Clients[uiSlot] != NULL)
				continue;

			m_vTeam2Clients[uiSlot] = AccountID;
			return;
		}	
	}
	
	if (uiTeam == TEAM_REFEREE)
	{
		for (uiSlot = 0; uiSlot < m_vReferees.size(); uiSlot++)
		{
			if (m_vReferees[uiSlot] != NULL)
				continue;

			m_vReferees[uiSlot] = AccountID;
			return;
		}	
	}
	
	if (uiTeam == TEAM_SPECTATOR)
	{
		for (uiSlot = 0; uiSlot < m_vSpectators.size(); uiSlot++)
		{
			if (m_vSpectators[uiSlot] != NULL)
				continue;

			m_vSpectators[uiSlot] = AccountID;
			return;
		}	
	}

}


/*====================
  CTournMatch::Create
  ====================*/
void	CTournMatch::Create(const string &sAddress, const ushort unPort)
{
	m_pConsole->Admin() << L"CTournMatch Requesting tourn match creation #" << m_uiID << L" " << sAddress << L":" << unPort << newl;
	
	SetTimeStamp(m_pCore->GetSeconds());
	//SetMatchAddress(sAddress);
	//SetMatchPort(unPort);

	CPacket pkt;

	// Header (used by the game, ignored in this case but still needs to be there)
	pkt << ushort(0) << byte(0);
	
	// Request
	pkt
		//<< NETCMD_CREATE_TOURN_MATCH			// Command
		<< m_uiID								// ID
		<< m_uiChallenge						// Challenge
		<< L"JKNGKFVNEOSNUENSVUVJ"				// Cookie
		//<< m_sMatchName							// Name
		//<< m_sMatchSettings						// Settings
		<< m_iMatchStartTime;					// Time to match start
	
	pkt << byte(m_vTeam1Clients.size());
	for (uint i(0); i < m_vTeam1Clients.size(); ++i)
		pkt << m_vTeam1Clients[i];

	pkt << byte(m_vTeam2Clients.size());
	for (uint i(0); i < m_vTeam1Clients.size(); ++i)
		pkt << m_vTeam2Clients[i];
		
	pkt << byte(m_vReferees.size());
	for (uint i(0); i < m_vReferees.size(); ++i)
		pkt << m_vReferees[i];
	
	pkt << byte(m_vSpectators.size());
	for (uint i(0); i < m_vSpectators.size(); ++i)
		pkt << m_vSpectators[i];

	//m_pNetManager->SendGameServerRequest(m_sAddress, m_unPort, pkt);
	m_pConsole->Std() << L"Requesting match creation #" << m_uiID << L" " << sAddress << L":" << unPort << newl;	
}


/*====================
  CTournMatch::Announce
  ====================*/
void	CTournMatch::Announce()
{
	CClient *pClient(NULL);

	CBufferDynamic buffer;

	//buffer << CHAT_CMD_TOURN_MATCH_READY << GetMatchAddress() << byte(0) << GetMatchPort() << m_uiTournID << m_sTournName << byte(0);
	
	for (uint i(0); i < m_vTeam1Clients.size(); ++i)
	{
		pClient = m_pClientManager->GetClientFromAccountID(m_vTeam1Clients[i]);
		
		if (pClient == NULL)
			continue;

		pClient->Send(buffer);		
	}

	for (uint i(0); i < m_vTeam1Clients.size(); ++i)
	{
		pClient = m_pClientManager->GetClientFromAccountID(m_vTeam2Clients[i]);
		
		if (pClient == NULL)
			continue;

		pClient->Send(buffer);		
	}

	for (uint i(0); i < m_vReferees.size(); ++i)
	{
		pClient = m_pClientManager->GetClientFromAccountID(m_vReferees[i]);
		
		if (pClient == NULL)
			continue;

		pClient->Send(buffer);			
	}

	for (uint i(0); i < m_vSpectators.size(); ++i)
	{	
		pClient = m_pClientManager->GetClientFromAccountID(m_vSpectators[i]);
		
		if (pClient == NULL)
			continue;

		pClient->Send(buffer);		
	}
}

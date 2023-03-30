// (C)2009 S2 Games
// c_core.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "chatserver_common.h"

#include "c_core.h"
#include "c_console.h"
#include "c_netmanager.h"
#include "c_matchmaker.h"
#include "c_peermanager.h"
#include "c_clanmanager.h"
//#include "c_gamemanager.h"
#include "c_clientmanager.h"
#include "c_gameservermanager.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
CVAR(uint, core_FrameDelay, 0);
//=============================================================================

/*====================
  CCore::CCore
  ====================*/
CCore::CCore(CMemManager *pMemManager) :
m_pSystem(NULL),
m_pMemManager(pMemManager),
m_pConsole(NULL),
m_pProfiler(NULL),
m_pHTTPManager(NULL),
m_pNetManager(NULL),
m_pPeerManager(NULL),
//m_pGameManager(NULL),
m_pChannelManager(NULL),
m_pClanManager(NULL),
m_pClientManager(NULL),
m_pMatchMaker(NULL),
m_uiMilliseconds(0),
m_uiSeconds(0),
m_bDateChanged(false),
m_uiNextTimeUpdate(INVALID_TIME),
m_bExitRequested(false),
m_iExitCode(0)
{
}


/*====================
  CCore::UpdateDateTimeStrings
  ====================*/
void	CCore::UpdateDateTimeStrings()
{
	PROFILE("CCore::UpdateDateTimeStrings")

	CDate date(true);

	m_bDateChanged = date.GetDay() != m_datePrev.GetDay();

	m_sDateString = date.GetDateString(DATE_SHORT_YEAR | DATE_YEAR_LAST | DATE_MONTH_FIRST);
	m_sTimeString = date.GetTimeString(TIME_NO_SECONDS);
	m_sDateTimeString = m_sDateString + L" " + m_sTimeString;

	m_datePrev = date;

	m_uiNextTimeUpdate = m_pSystem->GetMilliseconds() + 60000;
}


/*====================
  CCore::Initialize
  ====================*/
bool	CCore::Initialize()
{
	// Initialize MemManager first so it can allocate the other members
	if (m_pMemManager == NULL)
		return false;

	// MemManager can safely access functions from Core right now, but nothing else. It will store
	// the pointer to Core right now and retrieve the rest of the API after it has been allocated
	if (!m_pMemManager->Initialize(this))
		return false;

	m_pHeap = m_pMemManager->AllocateHeap("core");

	// Allocate API components
	ALLOCATE_CORE_MEMBER(System);
	ALLOCATE_CORE_MEMBER(Console);
	ALLOCATE_CORE_MEMBER(Profiler);
	ALLOCATE_CORE_MEMBER(HTTPManager);
	ALLOCATE_CORE_MEMBER(NetManager);
	ALLOCATE_CORE_MEMBER(PeerManager);
	ALLOCATE_CORE_MEMBER(ChannelManager);
	ALLOCATE_CORE_MEMBER(ClanManager);
	ALLOCATE_CORE_MEMBER(ClientManager);
	ALLOCATE_CORE_MEMBER(MatchMaker);
	ALLOCATE_CORE_MEMBER(GameServerManager);

	// Now that all the components are allocated, make sure that MemManager has all of its pointers
	m_pMemManager->SynchronizeCoreAPI();

	// Process command line
	m_pSystem->ProcessCommandLine(this);

	m_pSystem->Initialize(this);
	
	m_dateStart = CDate(true);
	UpdateDateTimeStrings();

	if (!m_pConsole->Initialize(this))
		return false;

	m_pConsole->Admin() << m_dateStart.GetDateString() << L" " << m_dateStart.GetTimeString() << newl;
	m_pConsole->Admin() << L"Using profile: " << m_pSystem->GetProfile() << newl;

	m_pProfiler->Initialize(this);
	
	if (!m_pHTTPManager->Initialize(this))
		return false;
	
	if (!m_pNetManager->Initialize(this))
		return false;
	
	if (!m_pPeerManager->Initialize(this))
		return false;

	m_pMatchMaker->Initialize(this);
	
	if (!m_pChannelManager->Initialize(this))
		return false;

	m_pClanManager->Initialize(this);
	m_pClientManager->Initialize(this);
	m_pGameServerManager->Initialize(this);

	m_pConsole->ExecuteFile(L"autoexec.cfg");
	return true;
}


/*====================
  CCore::Shutdown
  ====================*/
void	CCore::Shutdown()
{
	m_pMatchMaker->Shutdown();
	m_pHTTPManager->Shutdown();
	m_pConsole->Shutdown();

	delete m_pGameServerManager;
	delete m_pClientManager;
	delete m_pMatchMaker;
	delete m_pClanManager;
	delete m_pChannelManager;
	delete m_pPeerManager;
	delete m_pNetManager;
	delete m_pHTTPManager;
	delete m_pProfiler;
	delete m_pConsole;
	delete m_pSystem;

	delete m_pHeap;
}


/*====================
  CCore::Execute
  ====================*/
int	CCore::Execute()
{
	while (!m_bExitRequested)
	{
		m_pProfiler->StartFrame();

		m_uiMilliseconds =m_pSystem->GetMilliseconds();
		m_uiSeconds = m_pSystem->GetSeconds();
		
		if (m_uiMilliseconds >= m_uiNextTimeUpdate)
		{
			UpdateDateTimeStrings();
			if (m_bDateChanged)
				m_pConsole->StartLogFile();
		}

		m_pHTTPManager->Frame();
		m_pNetManager->Frame();
		m_pPeerManager->Frame();
		m_pMatchMaker->Frame();
#if 0
		m_pGameManager->Frame();
#endif

		m_pClientManager->Frame();
		m_pGameServerManager->Frame();
		m_pChannelManager->Frame();
		m_pConsole->Frame();

		m_pProfiler->EndFrame();

		m_pSystem->Sleep(MAX(core_FrameDelay.Get(), 1u));
	}

	Shutdown();

	return m_iExitCode;
}


/*====================
  CCore::RequestExit
  ====================*/
void	CCore::RequestExit(int iExitCode)
{
	m_iExitCode = iExitCode;
	m_bExitRequested = true;
}


/*====================
  CCore::LoadState
  ====================*/
void	CCore::LoadState()
{
	CFileHandle hLoadFile(L"chatserver.save", FILE_READ | FILE_TEXT);
	m_pChannelManager->LoadState(hLoadFile);
}


/*====================
  CCore::SaveState
  ====================*/
void	CCore::SaveState()
{
	CFileHandle hSaveFile(L"chatserver.save", FILE_WRITE | FILE_TEXT);
	m_pChannelManager->SaveState(hSaveFile);
}


/*====================
  CCore::PrintUsage
  ====================*/
void	CCore::PrintUsage()
{
	CDate date(true);

	m_pConsole->Admin()
		<< L"Core" << newl
		<< L"----" << newl
		<< L"Current time:  " << date.GetString() << newl
		<< L"Running since: " << m_dateStart.GetString() << newl
		<< L"Up time:       ";
	
	uint uiUptime(m_pSystem->GetSeconds());
	bool bShow(false);

	uint uiDays(uiUptime / SEC_PER_DAY);
	if (uiDays > 0)
	{
		m_pConsole->Admin() << uiDays << L"d ";
		bShow = true;
	}

	uint uiHours(SecToHr(uiUptime % SEC_PER_DAY));
	if (uiHours > 0 || bShow)
	{
		m_pConsole->Admin() << uiHours << L"h ";
		bShow = true;
	}

	uint uiMinutes((uiUptime % SEC_PER_HR) / SEC_PER_MIN);
	if (uiMinutes > 0 || bShow)
		m_pConsole->Admin() << uiMinutes << L"m ";

	m_pConsole->Admin() << uiUptime % SEC_PER_MIN << L"s" << newl;

	m_pHTTPManager->PrintUsage();
	m_pNetManager->PrintUsage();
	m_pClientManager->PrintUsage();
	m_pChannelManager->PrintUsage();
	m_pMatchMaker->PrintUsage();
	m_pGameServerManager->PrintUsage();
	//m_pPeerManager->PrintUsage();
}

// (C)2007 S2 Games
// c_serverstatstracker.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_server_common.h"

#include "c_serverstatstracker.h"

#include "../k2/c_uitrigger.h"
#include "../k2/c_filedisk.h"
#include "../k2/c_filehttp.h"

#include "../hon_shared/c_replaymanager.h"
#include "../hon_shared/c_player.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
SINGLETON_INIT(CServerStatsTracker)

CVAR_BOOLF(		svr_sendStats,					true,	CVAR_SERVERINFO | CVAR_SAVECONFIG);
CVAR_STRING(	svr_statsSubmitURL,				"/irc_updater/irc_stats.php");
//=============================================================================

/*====================
  CServerStatsTracker::~CServerStatsTracker
  ====================*/
CServerStatsTracker::~CServerStatsTracker()
{
	SAFE_DELETE(m_pServerDatabase);
	SAFE_DELETE(m_pStatsDatabase);
	SAFE_DELETE(m_pStatsPoster);
}


/*====================
  CServerStatsTracker::CServerStatsTracker
  ====================*/
CServerStatsTracker::CServerStatsTracker() :
m_iServerID(-1),
m_iMatchID(-1),
m_sSalt(_T("")),
m_uiMatchLength(0),
m_uiLastUpdate(0),
m_bMatchInProgress(false)
{
	m_pServerDatabase = K2_NEW(global,   CHTTPPostRequest);
	m_pServerDatabase->SetHost(_T("masterserver.savage2.s2games.com"));
	m_pServerDatabase->SetTargetURI(_T("/irc_updater/svr_request.php"));

	m_pStatsDatabase = K2_NEW(global,   CHTTPPostRequest);
	m_pStatsDatabase->SetHost(_T("masterserver.savage2.s2games.com"));
	m_pStatsDatabase->SetTargetURI(_T("/irc_updater/irc_requester.php"));

	m_pStatsPoster = K2_NEW(global,   CHTTPPostRequest);
	m_pStatsPoster->SetHost(_T("masterserver.savage2.s2games.com"));
	m_pStatsPoster->SetTargetURI(svr_statsSubmitURL);
}


/*====================
  CServerStatsTracker::StartMatch
  ====================*/
void	CServerStatsTracker::StartMatch(const tstring &sLogin, const tstring &sPass, bool bClanMatch, const tstring &sMap, int iPort, PlayerMap &mapClients)
{
	int iNumPlayers(0);

	m_iMatchID = -1;

	if (sLogin.empty() || sPass.empty() || !Game.IsServer() || !svr_sendStats)
	{
		m_iServerID = -1;
		return;
	}

	m_pServerDatabase->AddVariable(_T("f"), _T("auth"));
	m_pServerDatabase->AddVariable(_T("login"), sLogin);
	m_pServerDatabase->AddVariable(_T("pass"), sPass);
	m_pServerDatabase->AddVariable(_T("type"), bClanMatch ? _T("clan") : _T("reg"));
	m_pServerDatabase->AddVariable(_T("port"), XtoA(iPort));
	m_pServerDatabase->AddVariable(_T("map"), sMap);

	for (PlayerMap_it it(mapClients.begin()); it != mapClients.end(); ++it)
	{
		
		m_pServerDatabase->AddVariable(_T("account_id[") + XtoA(iNumPlayers) + _T("]"), XtoA(it->second->GetAccountID()));
		++iNumPlayers;
	}

	m_pServerDatabase->SendRequest(_T("StartMatch"));

	m_bMatchInProgress = true;
	m_uiLastUpdate = K2System.Milliseconds();
}


/*====================
  CServerStatsTracker::WriteStatsToFile
  ====================*/
void	CServerStatsTracker::WriteStatsToFile(PlayerMap &mapClients, const tstring &sMapName)
{
	tstring sFilePath;
	if (m_iMatchID != -1)
		sFilePath = _T("~/matches/match_") + XtoA(m_iMatchID) + _T(".txt");
	else
		sFilePath = FileManager.GetNextFileIncrement(6, _T("~/matches/unsent_"), _T("txt"));

	CFile *pFile(FileManager.GetFile(sFilePath, FILE_WRITE | FILE_TEXT));
	if (pFile == NULL)
		return;

	uint uiHours(m_uiMatchLength / MS_PER_HR);
	uint uiMinutes((m_uiMatchLength % MS_PER_HR) / MS_PER_MIN);
	uint uiSeconds((m_uiMatchLength % MS_PER_MIN) / MS_PER_SEC);
	tstring sTimePlayed(XtoA(uiHours, FMT_PADZERO, 2) + _T(":") + XtoA(uiMinutes, FMT_PADZERO, 2) + _T(":") + XtoA(uiSeconds, FMT_PADZERO, 2));

	pFile->WriteString(_T("Match ID: ") + XtoA(m_iMatchID) + newl);
	pFile->WriteString(_T("Match Length: ") + sTimePlayed + newl + newl);
	pFile->WriteString(_T("Server Version: ") + K2System.GetVersionString() + newl + newl);
	pFile->WriteString(_T("Map: ") + sMapName + newl);
	pFile->WriteString(_T("Total Users: ") + XtoA(INT_SIZE(mapClients.size())) + newl);
	pFile->WriteString(_T("Winning Team: ") + XtoA(Game.GetWinningTeam()) + newl + newl + newl);

#if 0
	for (PlayerMap_it it(mapClients.begin()); it != mapClients.end(); ++it)
	{
		CPlayer *pClient(it->second);

		pFile->WriteString(_T("============================================\n"));
		pFile->WriteString(_T("Name: ") + pClient->GetName() + newl);
		pFile->WriteString(_T("Account ID: ") + XtoA(pClient->GetAccountID()) + newl + newl);

		uiHours = pClient->GetPlayTime() / MS_PER_HR;
		uiMinutes = (pClient->GetPlayTime() % MS_PER_HR) / MS_PER_MIN;
		uiSeconds = (pClient->GetPlayTime() % MS_PER_MIN) / MS_PER_SEC;
		sTimePlayed = XtoA(uiHours, FMT_PADZERO, 2) + _T(":") + XtoA(uiMinutes, FMT_PADZERO, 2) + _T(":") + XtoA(uiSeconds, FMT_PADZERO, 2);
		pFile->WriteString(_T("Time Played: ") + sTimePlayed + newl + newl);

		pFile->WriteString(_T("Clan: ") + pClient->GetClanName() + newl);
		pFile->WriteString(_T("Clan Rank: ") + pClient->GetClanRank() + newl + newl);

		pFile->WriteString(_T("Team: ") + XtoA(pClient->GetTeam()) + newl);

		pFile->WriteString(_T("Unspent Gold: ") + XtoA(pClient->GetGold()) + newl);

		//pFile->WriteString(_T("Experience: ") + XtoA(pClient->GetExperience()) + newl);
		//pFile->WriteString(_T("Level: ") + XtoA(pClient->GetLevel()) + newl);

		pFile->WriteString(_T("Kills: ") + XtoA(pClient->GetMatchStatTotalInt(PLAYER_MATCH_KILLS)) + newl);
		pFile->WriteString(_T("Deaths: ") + XtoA(pClient->GetMatchStatTotalInt(PLAYER_MATCH_DEATHS)) + newl);
		pFile->WriteString(_T("Assists: ") + XtoA(pClient->GetMatchStatTotalInt(PLAYER_MATCH_ASSISTS)) + newl);
		pFile->WriteString(_T("Player Damage: ") + XtoA(pClient->GetMatchStatTotalFloat(PLAYER_MATCH_PLAYER_DAMAGE)) + newl);
		pFile->WriteString(_T("Buildings Razed: ") + XtoA(pClient->GetMatchStatTotalInt(PLAYER_MATCH_RAZED)) + newl);
		pFile->WriteString(_T("Building Damage: ") + XtoA(pClient->GetMatchStatTotalFloat(PLAYER_MATCH_BUILDING_DAMAGE)) + newl);
		pFile->WriteString(_T("Souls Spent: ") + XtoA(pClient->GetMatchStatTotalFloat(PLAYER_MATCH_SOULS_SPENT)) + newl);
		pFile->WriteString(_T("Resurrects: ") + XtoA(pClient->GetMatchStatTotalInt(PLAYER_MATCH_RESURRECTS)) + newl);
		pFile->WriteString(_T("HP Healed: ") + XtoA(pClient->GetMatchStatTotalFloat(PLAYER_MATCH_HEALED)) + newl);
		pFile->WriteString(_T("Gold Earned: ") + XtoA(pClient->GetMatchStatTotalInt(PLAYER_MATCH_GOLD_EARNED)) + newl);
		pFile->WriteString(_T("NPCs Killed: ") + XtoA(pClient->GetMatchStatTotalInt(PLAYER_MATCH_NPC_KILLS)) + newl + newl + newl);
	}
#endif

	pFile->Close();
}


/*====================
  CServerStatsTracker::SubmitStats
  ====================*/
void	CServerStatsTracker::SubmitStats(PlayerMap &mapClients, const tstring &sMapName)
{
#if 0
	WriteStatsToFile(mapClients, sMapName);

	if (m_iMatchID == -1 || m_iServerID == -1 || !svr_sendStats || !Game.IsServer())
		return;

	if (!ReplayManager.GetReplayFilename().empty())
	{
		tstring sNewName(Filename_GetPath(ReplayManager.GetReplayFilename()) + XtoA(GetMatchID()));
		tstring sExtension(Filename_GetExtension(ReplayManager.GetReplayFilename()));
		if (!sExtension.empty())
			sNewName += _T(".") + sExtension;

		FileManager.Rename(ReplayManager.GetReplayFilename(), sNewName);
	}

	uint uiHours(m_uiMatchLength / MS_PER_HR);
	uint uiMinutes((m_uiMatchLength % MS_PER_HR) / MS_PER_MIN);
	uint uiSeconds((m_uiMatchLength % MS_PER_MIN) / MS_PER_SEC);
	tstring sTimePlayed(XtoA(uiHours, FMT_PADZERO, 2) + _T(":") + XtoA(uiMinutes, FMT_PADZERO, 2) + _T(":") + XtoA(uiSeconds, FMT_PADZERO, 2));

	// Get commander/team info
	CTeamInfo *pTeam;
	tstring sRaceString[2];
	int iCommAcctID[2];
	tstring sCommStats[2];
	bool bCommExists[2];

	for (int i(1); i <= 2; i++)
	{
		pTeam = Game.GetTeam(i);

		bCommExists[i - 1] = false;
		iCommAcctID[i - 1] = 0;
		sRaceString[i - 1] = _T("H");
	}

	// Calculate team with highest SF
	int iHighSF[2] = { 0, 0 };
	int iTeamPlayers[2] = { 0, 0 };
	int iHighTeam(0);

	for (PlayerMap_it it(mapClients.begin()); it != mapClients.end(); ++it)
	{
		if (it->second->GetTeam() > 0 && (GameServer.GetLastGameLength() - it->second->GetPlayTime() <= 30000) && it->second->GetAccountID() != -1)
		{
			iHighSF[it->second->GetTeam() - 1] += it->second->GetPersistantStat(PLAYER_PERSISTANT_SKILLFACTOR);
			iTeamPlayers[it->second->GetTeam() - 1]++;
		}
	}

	for (int i(1); i <= 2; ++i)
	{
		if (iTeamPlayers[i - 1] > 0)
			iHighSF[i - 1] = iHighSF[i - 1] / iTeamPlayers[i - 1];
		else
			iHighSF[i - 1] = 0;

		if (iHighTeam == 0 || iHighSF[i - 1] > iHighSF[iHighTeam - 1])
			iHighTeam = i;
	}

	tstring sFinalString;
	int iNumPlayers(0);
	for (PlayerMap_it it(mapClients.begin()); it != mapClients.end(); ++it)
	{
		// User must be logged in
		if (it->second->GetAccountID() != -1 && (GameServer.GetLastGameLength() - it->second->GetPlayTime() <= 30000))
		{
			sFinalString += _T("i:") + XtoA(iNumPlayers) + _T(";") + GenerateStatsString(it->second, iHighTeam);
			++iNumPlayers;
		}
	}

	sFinalString = _T("a:") + XtoA(iNumPlayers) + _T(":{") + sFinalString + _T("}");

	Console.ServerGame << _T("Sending game statistics...") << newl;

	tstring sStatString;
	sStatString += _T("f=") + URLEncode(_T("end_game"));
	sStatString += _T("&match_id=") + URLEncode(XtoA(m_iMatchID));
	sStatString += _T("&svr_id=") + URLEncode(XtoA(m_iServerID));
	sStatString += _T("&version=") + URLEncode(K2System.GetVersionString());
	sStatString += _T("&salt=") + URLEncode(m_sSalt);
	sStatString += _T("&time=") + URLEncode(sTimePlayed);
	sStatString += _T("&map=") + URLEncode(sMapName);
	sStatString += _T("&winner=") + URLEncode(XtoA(Game.GetWinningTeam()));
	sStatString += _T("&player_stats=") + URLEncode(sFinalString);

	tstring sCommString;
	int iNumComm(0);

	if (bCommExists[0])
		iNumComm++;

	if (bCommExists[1])
		iNumComm++;
	
	sCommString = _T("a:") + XtoA(iNumComm) + _T(":{");

	iNumComm = 0;
	
	if (bCommExists[0])
	{
		sCommString += _T("i:") + XtoA(iNumComm) + _T(";") + sCommStats[0];
		iNumComm++;
	}
	
	if (bCommExists[1])
		sCommString += _T("i:") + XtoA(iNumComm) + _T(";") + sCommStats[1];

	sCommString += _T("}");

	sStatString += _T("&commander_stats=") + URLEncode(sCommString);
	sStatString += _T("&team=") + URLEncode(
		_T("a:2:{i:1;a:4:{s:6:\"avg_sf\";i:") + XtoA(iHighSF[0]) + _T(";") + // Team 1
		_T("s:4:\"race\";s:1:\"") + sRaceString[0] + _T("\";") +
		_T("s:9:\"clan_name\";s:3:\"N/A\";") +
		_T("s:9:\"commander\";i:") + XtoA(iCommAcctID[0]) + _T(";") +
		_T("}i:2;a:4:{s:6:\"avg_sf\";i:") + XtoA(iHighSF[1]) + _T(";") + // Team 2
		_T("s:4:\"race\";s:1:\"") + sRaceString[1] + _T("\";") +
		_T("s:9:\"clan_name\";s:3:\"N/A\";") +
		_T("s:9:\"commander\";i:") + XtoA(iCommAcctID[1]) + _T(";") +
		_T("}}"));

	tstring sStatFileName(_T("~/replays/") + XtoA(GetMatchID()) + _T(".stats"));
	tstring sTempStatFileName(sStatFileName + _T(".temp"));
	CFileHandle fileStatsRequest(sTempStatFileName, FILE_WRITE | FILE_BINARY);
	fileStatsRequest.Write(svr_statsSubmitURL.GetString().c_str(), svr_statsSubmitURL.GetString().length());
	fileStatsRequest.Write("\n", 1);
	fileStatsRequest.Write(sStatString.c_str(), sStatString.size());
	fileStatsRequest.Close();
	FileManager.Rename(sTempStatFileName, sStatFileName);

	m_iServerID = -1;
#endif
}


/*====================
  CServerStatsTracker::RetrieveStats
  ====================*/
void	CServerStatsTracker::RetrieveStats(int iAccountID)
{
	if (iAccountID > 0)
	{
		m_pStatsDatabase->AddVariable(_T("f"), _T("get_all_stats"));
		m_pStatsDatabase->AddVariable(_T("account_id[0]"), XtoA(iAccountID));

		m_pStatsDatabase->SendRequest();
	}
}


/*====================
  CServerStatsTracker::SubmitAllData
  ====================*/
void	CServerStatsTracker::SubmitAllData()
{
	bool bContinue(true);
	while (bContinue)
	{
		bContinue = false;

		if (m_pStatsPoster != NULL && m_pStatsPoster->GetStatus() == HTTP_POST_SENDING)
		{
			bContinue = true;
			m_pStatsPoster->Frame();
		}

		if (m_pServerDatabase != NULL && m_pServerDatabase->GetStatus() == HTTP_POST_SENDING)
		{
			bContinue = true;
			m_pServerDatabase->Frame();
		}

		if (m_pStatsDatabase != NULL && m_pStatsDatabase->GetStatus() == HTTP_POST_SENDING)
		{
			bContinue = true;
			m_pStatsDatabase->Frame();
		}

		K2System.Sleep(5);
	}
}


/*====================
  CServerStatsTracker::Frame
  ====================*/
void	CServerStatsTracker::Frame()
{
// FIXME: DISABLED
#if 0
	PROFILE("CServerStatsTracker::Frame");

	CDBResponse *pResponse(NULL);

	if (m_bMatchInProgress)
	{
		m_uiMatchLength += (K2System.Milliseconds() - m_uiLastUpdate);
		m_uiLastUpdate = K2System.Milliseconds();
	}

	if (m_pStatsPoster != NULL)
		m_pStatsPoster->Frame();

	//Authentication, match starts and match ends
	if (m_pServerDatabase != NULL)
		pResponse = m_pServerDatabase->Frame();

	if (pResponse != NULL)
	{
		if (pResponse->GetResponseName() =="StartMatch")
		{
			if (pResponse->GetVarString("Error").empty() && pResponse->GetVarArray("error") == NULL && !pResponse->GetVarString("match_id").empty())
			{
				m_sSalt = StringToTString(pResponse->GetVarString("salt"));
				m_iMatchID = AtoI(pResponse->GetVarString("match_id"));
				m_iServerID = AtoI(pResponse->GetVarString("svr_id"));

				Console.ServerGame << _T("Authenticated server successfully, stats will be recorded this game. Match ID: ") << m_iMatchID << _T(", Server ID: ") << m_iServerID << newl;
			}
			else if (!pResponse->GetVarString("Error").empty())
			{
				Console.Err << _T("CServerStatsTracker::Frame() - Error authenticating, stats will not be saved: ") << pResponse->GetVarString("Error") << newl;
				m_iMatchID = -1;
				m_iServerID = -1;
			}
			else if (pResponse->GetVarArray("error") != NULL)
			{
				Console.Err << _T("CServerStatsTracker::Frame() - Error authenticating, stats will not be saved: ") << pResponse->GetVarArray("error")->GetVarString("") << newl;
				m_iMatchID = -1;
				m_iServerID = -1;
			}
			else
			{
				Console.Err << _T("CServerStatsTracker::Frame() - Error authenticating, stats will not be saved.") << newl;
				m_iMatchID = -1;
				m_iServerID = -1;
			}
		}
	}

	//Player stats
	pResponse = NULL;

	if (m_pStatsDatabase != NULL)
		pResponse = m_pStatsDatabase->Frame();

	if (pResponse != NULL)
	{
		int iClientNum(GameServer.GetClientNumFromAccountID(AtoI(pResponse->GetResponseName())));

		if (iClientNum != -1)
		{
			CPlayer *pClient(GameServer.GetPlayer(iClientNum));

			if (pClient != NULL && pResponse->GetVarArray("all_stats") != NULL && pResponse->GetVarArray("all_stats")->GetArrayNum(0) != NULL)
			{
				pClient->SetClanName(StringToTString(pResponse->GetVarArray("all_stats")->GetArrayNum(0)->GetVarString("clan_name")));
				pClient->SetClanRank(StringToTString(pResponse->GetVarArray("all_stats")->GetArrayNum(0)->GetVarString("member_rank")));

				#define GET_STAT(stat) AtoI(pResponse->GetVarArray("all_stats")->GetArrayNum(0)->GetVarString(stat))

				pClient->SetPersistantStat(PLAYER_PERSISTANT_RANK, GET_STAT("overall_r"));

				pClient->SetPersistantStat(PLAYER_PERSISTANT_LEVEL, GET_STAT("level"));
				pClient->SetPersistantStat(PLAYER_PERSISTANT_DISCONNECTS, GET_STAT("ties"));
				pClient->SetPersistantStat(PLAYER_PERSISTANT_ASSISTS, GET_STAT("assists"));
				pClient->SetPersistantStat(PLAYER_PERSISTANT_SOULS, GET_STAT("souls"));
				pClient->SetPersistantStat(PLAYER_PERSISTANT_RAZED, GET_STAT("razed"));
				pClient->SetPersistantStat(PLAYER_PERSISTANT_PLAYERDAMAGE, GET_STAT("pdmg"));
				pClient->SetPersistantStat(PLAYER_PERSISTANT_BUILDINGDAMAGE, GET_STAT("bdmg"));
				pClient->SetPersistantStat(PLAYER_PERSISTANT_NPCKILLS, GET_STAT("npc"));
				pClient->SetPersistantStat(PLAYER_PERSISTANT_HPHEALED, GET_STAT("hp_healed"));
				pClient->SetPersistantStat(PLAYER_PERSISTANT_RESURRECTS, GET_STAT("res"));
				pClient->SetPersistantStat(PLAYER_PERSISTANT_KARMA, GET_STAT("karma"));
				pClient->SetPersistantStat(PLAYER_PERSISTANT_GOLDEARNED, GET_STAT("gold"));
				pClient->SetPersistantStat(PLAYER_PERSISTANT_SKILLFACTOR, GET_STAT("sf"));
				pClient->SetPersistantStat(PLAYER_PERSISTANT_LOYALTY, GET_STAT("lf"));

				uint uiExp(GET_STAT("exp"));
				uint uiTimePlayed(GET_STAT("secs"));
				uint uiKills(GET_STAT("kills"));
				uint uiDeaths(GET_STAT("deaths"));
				uint uiWins(GET_STAT("wins"));
				uint uiLosses(GET_STAT("losses"));

				uint uiHoursPlayed(uiTimePlayed / MS_PER_HR);
				uint uiMinutesPlayed((uiTimePlayed % MS_PER_HR) / MS_PER_MIN);
				uint uiSecondsPlayed((uiTimePlayed % MS_PER_MIN) / MS_PER_SEC);

				pClient->SetPersistantStat(PLAYER_PERSISTANT_HOURSPLAYED, uiHoursPlayed);
				pClient->SetPersistantStat(PLAYER_PERSISTANT_MINUTESPLAYED, uiMinutesPlayed);
				pClient->SetPersistantStat(PLAYER_PERSISTANT_SECONDSPLAYED, uiSecondsPlayed);

				pClient->SetPersistantStat(PLAYER_PERSISTANT_EXPERIENCE, uiExp);
				pClient->SetPersistantStat(PLAYER_PERSISTANT_WINS, uiWins);
				pClient->SetPersistantStat(PLAYER_PERSISTANT_LOSSES, uiLosses);

				pClient->SetPersistantStat(PLAYER_PERSISTANT_KILLS, uiKills);
				pClient->SetPersistantStat(PLAYER_PERSISTANT_DEATHS, uiDeaths);

				if (uiKills > uiDeaths && uiDeaths != 0)
				{
					pClient->SetPersistantStat(PLAYER_PERSISTANT_KILLSRATIO, uiKills / uiDeaths);
					pClient->SetPersistantStat(PLAYER_PERSISTANT_DEATHSRATIO, 1);
				}
				else if (uiDeaths > uiKills && uiKills != 0)
				{
					pClient->SetPersistantStat(PLAYER_PERSISTANT_KILLSRATIO, 1);
					pClient->SetPersistantStat(PLAYER_PERSISTANT_DEATHSRATIO, uiDeaths / uiKills);
				}
				else
				{
					pClient->SetPersistantStat(PLAYER_PERSISTANT_KILLSRATIO, 1);
					pClient->SetPersistantStat(PLAYER_PERSISTANT_DEATHSRATIO, 1);
				}

				#undef GET_STAT

				Console << _T("Recieved persistant stats for client ") << iClientNum << _T(" (Account ID: ") << pResponse->GetResponseName() << _T(").") << newl;
			}
			else
				Console << _T("Could not process persistant stats for client ") << iClientNum << _T(" (Account ID: ") << pResponse->GetResponseName() << _T(").") << newl;
		}
		else
			Console << _T("Could not process persistant stats for client ") << iClientNum << _T(" (Account ID: ") << pResponse->GetResponseName() << _T(").") << newl;
	}
#endif
}


/*====================
  CServerStatsTracker::GenerateStatsString
  ====================*/
tstring	CServerStatsTracker::GenerateStatsString(CPlayer *pClient, int iTeamHighSF)
{
	bool bStillConnected(true);
	if (pClient->GetTeam() == 0 || (pClient->IsDisconnected() && pClient->GetDisconnectTime() < Game.GetPhaseStartTime()))
		bStillConnected = false;

	// Send 20 stats
	tstring sStats(_T("a:20:{"));

	// Serialize the stats string
#if 0
	sStats += _T("s:10:\"account_id\";i:") + XtoA(pClient->GetAccountID()) + _T(";");
	sStats += _T("s:9:\"clan_name\";s:") + XtoA(INT_SIZE(pClient->GetClanName().length())) + _T(":\"") + pClient->GetClanName() + _T("\";");
	sStats += _T("s:4:\"team\";i:") + XtoA(pClient->GetLastTeam()) + _T(";");
	sStats += _T("s:5:\"karma\";i:") + XtoA(pClient->GetKarma()) + _T(";");
	sStats += _T("s:3:\"exp\";i:") + XtoA(iExp) + _T(";");
	sStats += _T("s:5:\"kills\";i:") + XtoA(pClient->GetMatchStatTotalInt(PLAYER_MATCH_KILLS)) + _T(";");
	sStats += _T("s:6:\"deaths\";i:") + XtoA(pClient->GetMatchStatTotalInt(PLAYER_MATCH_DEATHS)) + _T(";");
	sStats += _T("s:7:\"assists\";i:") + XtoA(pClient->GetMatchStatTotalInt(PLAYER_MATCH_ASSISTS)) + _T(";");
	sStats += _T("s:5:\"souls\";i:") + XtoA(pClient->GetMatchStatTotalInt(PLAYER_MATCH_SOULS_SPENT)) + _T(";");
	sStats += _T("s:5:\"razed\";i:") + XtoA(pClient->GetMatchStatTotalInt(PLAYER_MATCH_RAZED)) + _T(";");
	sStats += _T("s:4:\"pdmg\";i:") + XtoA(pClient->GetMatchStatTotalFloat(PLAYER_MATCH_PLAYER_DAMAGE), 0, 0, 0) + _T(";");
	sStats += _T("s:4:\"bdmg\";i:") + XtoA(pClient->GetMatchStatTotalFloat(PLAYER_MATCH_BUILDING_DAMAGE), 0, 0, 0) + _T(";");
	sStats += _T("s:3:\"npc\";i:") + XtoA(pClient->GetMatchStatTotalInt(PLAYER_MATCH_NPC_KILLS)) + _T(";");
	sStats += _T("s:9:\"hp_healed\";i:") + XtoA(pClient->GetMatchStatTotalFloat(PLAYER_MATCH_HEALED), 0, 0, 0) + _T(";");
	sStats += _T("s:3:\"res\";i:") + XtoA(pClient->GetMatchStatTotalInt(PLAYER_MATCH_RESURRECTS)) + _T(";");
	sStats += _T("s:4:\"gold\";i:") + XtoA(pClient->GetMatchStatTotalInt(PLAYER_MATCH_GOLD_EARNED)) + _T(";");
	sStats += _T("s:4:\"secs\";i:") + XtoA(pClient->GetPlayTime() / MS_PER_SEC) + _T(";");
	sStats += _T("s:10:\"end_status\";i:") + XtoA(bStillConnected, true) + _T(";");
	sStats += _T("s:2:\"sf\";i:") + XtoA(pClient->GetPersistantStat(PLAYER_PERSISTANT_SKILLFACTOR)) + _T(";");
//	sStats += _T("s:6:\"d_secs\";i:") + XtoA(pClient->GetDiscTime()) + _T(";");
#endif

	sStats += _T("}");

	return sStats;
}


/*====================
  CServerStatsTracker::EndMatch
  ====================*/
void	CServerStatsTracker::EndMatch()
{
	m_bMatchInProgress = false;
	m_iServerID = -1;
	m_sSalt = _T("");
	m_uiMatchLength = 0;

	m_setRatingSubmitted.clear();

	// Clear all "start game" requests
	m_pServerDatabase->ResetRequest();
}


/*====================
  CServerStatsTracker::SubmitRating
  ====================*/
void	CServerStatsTracker::SubmitRating(int iAccountID, int iRating, const tstring &sComment)
{
	if (m_iMatchID == -1)
		return;

	if (iRating < 1 || iRating > 5)
		return;

	if (iAccountID <= 0)
		return;

	if (sComment.empty())
		return;

	if (m_setRatingSubmitted.find(iAccountID) != m_setRatingSubmitted.end())
		return;

	m_setRatingSubmitted.insert(iAccountID);

	m_pStatsDatabase->AddVariable(_T("f"), _T("match_replays"));
	m_pStatsDatabase->AddVariable(_T("match_id"), XtoA(m_iMatchID));
	m_pStatsDatabase->AddVariable(_T("account_id"), XtoA(iAccountID));
	m_pStatsDatabase->AddVariable(_T("comment"), sComment);
	m_pStatsDatabase->AddVariable(_T("rating"), XtoA(iRating));

	m_pStatsDatabase->SendRequest(false);
}

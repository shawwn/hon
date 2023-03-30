// (C)2009 S2 Games
// console_commands.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "chatserver_common.h"

#include "c_console.h"
#include "c_channel.h"
#include "c_netmanager.h"
#include "c_peermanager.h"
#include "c_matchmaker.h"
#include "c_clientmanager.h"
#include "c_httpmanager.h"
#include "c_httprequest.h"
#include "c_gameservermanager.h"
//=============================================================================

/*--------------------
  Quit
  --------------------*/
CMD(Quit, 0)
{
	pCore->RequestExit();
}


/*--------------------
  StartProfiling
  --------------------*/
#ifndef NO_PROFILE
CMD(StartProfiling, 0)
{
	pCore->GetProfiler()->RequestStartProfiling();
}
#endif // NO_PROFILE


/*--------------------
  StopProfiling
  --------------------*/
#ifndef NO_PROFILE
CMD(StopProfiling, 0)
{
	pCore->GetProfiler()->RequestStopProfiling();
}
#endif // NO_PROFILE


/*--------------------
  Snapshot
  --------------------*/
#ifndef NO_PROFILE
CMD(Snapshot, 0)
{
	pCore->GetProfiler()->RequestStartProfiling();
	pCore->GetProfiler()->RequestStopProfiling();
}
#endif // NO_PROFILE


/*--------------------
  Broadcast <message>
  --------------------*/
CMD(Broadcast, 1)
{
	wstring sText(ConcatinateArgs(vArgList));
	CBufferDynamic buffer;
	buffer << CHAT_CMD_MESSAGE_ALL << WStringToUTF8(L"Server Message") << byte(0) << WStringToUTF8(sText) << byte(0);
	
	pCore->GetClientManager()->Broadcast(buffer);
}


/*--------------------
  PrintUsage
  --------------------*/
CMD(PrintUsage, 0)
{
	pCore->PrintUsage();
}


CMD(PrintServerList, 0)
{
	pCore->GetGameServerManager()->PrintServerList();
}


/*--------------------
  CmdList
  --------------------*/
CMD(CmdList, 0)
{
	pCore->GetConsole()->PrintCommandList();
}


/*--------------------
  VarList
  --------------------*/
CMD(VarList, 0)
{
	pCore->GetConsole()->PrintVariableList();
}


/*--------------------
  Connect <address> <port> <key>
  --------------------*/
CMD(Connect, 3)
{
	pCore->GetPeerManager()->Connect(WideToSingle(vArgList[0]), AtoN(vArgList[1]), WideToSingle(vArgList[2]));
}


/*--------------------
  Listen [port]
  --------------------*/
CMD(Listen, 0)
{
	if (!vArgList.empty())
		pCore->GetNetManager()->StartListening(AtoN(vArgList[0]));
	else
		pCore->GetNetManager()->StartListening(0);
}


/*--------------------
  Disconnect
  --------------------*/
CMD(Disconnect, 0)
{
	pCore->GetNetManager()->Disconnect();
	pCore->GetPeerManager()->Disconnect();
}


/*--------------------
  ClientInfo <name|account id>
  --------------------*/
CMD(ClientInfo, 1)
{
	CClientManager *pClientManager(pCore->GetClientManager());

	CClient *pClient(pClientManager->GetClientFromName(vArgList[0]));
	if (pClient == NULL)
		pClient = pClientManager->GetClientFromAccountID(AtoI(vArgList[0]));
	if (pClient == NULL)
	{
		pCore->GetConsole()->Admin() << L"Client not found." << newl;
		return;
	}

	pClient->PrintInfo();
}


/*--------------------
  PurgeChannel <name|id>
  --------------------*/
CMD(PurgeChannel, 1)
{
	CChannel *pChannel(pCore->GetChannelManager()->GetChannel(vArgList[0]));
	if (pChannel == NULL)
		pChannel = pCore->GetChannelManager()->GetChannel(AtoI(vArgList[0]));
	if (pChannel == NULL)
	{
		pCore->GetConsole()->Admin() << L"Channel not found." << newl;
		return;
	}

	pChannel->Purge();
}


/*--------------------
  ChannelInfo <name|id>
  --------------------*/
CMD(ChannelInfo, 1)
{
	CChannel *pChannel(pCore->GetChannelManager()->GetChannel(vArgList[0]));
	if (pChannel == NULL)
		pChannel = pCore->GetChannelManager()->GetChannel(AtoI(vArgList[0]));
	if (pChannel == NULL)
	{
		pCore->GetConsole()->Admin() << L"Channel not found." << newl;
		return;
	}

	pChannel->PrintInfo();
}


/*--------------------
  Kick <name|account id>
  --------------------*/
CMD(Kick, 1)
{
	CClientManager *pClientManager(pCore->GetClientManager());

	CClient *pClient(pClientManager->GetClientFromName(vArgList[0]));
	if (pClient == NULL)
		pClient = pClientManager->GetClientFromAccountID(AtoI(vArgList[0]));
	if (pClient == NULL)
	{
		pCore->GetConsole()->Admin() << L"Client not found." << newl;
		return;
	}

	pClientManager->RemoveClient(pClient);
}


/*--------------------
  Crash
  --------------------*/
CMD(Crash, 0)
{
	int *p(NULL);
	*p = 0;
}


/*--------------------
  AddVirtualClient
  --------------------*/
CMD(AddVirtualClient, 0)
{
	CClientManager *pClientManager(pCore->GetClientManager());

	uint uiCount(1);
	if (!vArgList.empty())
		uiCount = AtoI(vArgList[0]);

	for (uint uiIndex(0); uiIndex < uiCount; ++uiIndex)
		pClientManager->AddVirtualClient();
}


/*--------------------
  PutClientInQueue
  --------------------*/
CMD(PutClientInQueue, 1)
{
	CClientManager *pClientManager(pCore->GetClientManager());

	CClient *pClient(pClientManager->GetClientFromName(vArgList[0]));
	if (pClient == NULL)
		pClient = pClientManager->GetClientFromAccountID(AtoI(vArgList[0]));
	if (pClient == NULL)
	{
		pCore->GetConsole()->Admin() << L"Client not found." << newl;
		return;
	}

	pCore->GetMatchMaker()->AddClient(pClient, BIT(REGION_TST));
}


/*--------------------
  FillQueue
  --------------------*/
CMD(FillQueue, 1)
{
	CClientManager *pClientManager(pCore->GetClientManager());

	uint uiCount(1);
	if (!vArgList.empty())
		uiCount = AtoI(vArgList[0]);

	for (uint uiIndex(0); uiIndex < uiCount; ++uiIndex)
	{
		CClient *pClient(pClientManager->AddVirtualClient());
		if (pClient == NULL)
			break;

		pCore->GetMatchMaker()->AddClient(pClient, BIT(REGION_USW));
	}
}


/*--------------------
  SaveState
  --------------------*/
CMD(SaveState, 0)
{
	pCore->SaveState();
}


/*--------------------
  LoadState
  --------------------*/
CMD(LoadState, 0)
{
	pCore->LoadState();
}


/*--------------------
  Tweet
  --------------------*/
CMD(Tweet, 1)
{
	CHTTPRequest *pRequest(pCore->GetHTTPManager()->SpawnRequest());
	if (pRequest == NULL)
		return;

	pRequest->SetTargetURL("twitter.com/statuses/update.json");
	pRequest->SetUserName(L"HoNChat");
	pRequest->SetPassword(L"8bD2BFnev287Ok5cZrE8");
	pRequest->AddVariable(L"status", ConcatinateArgs(vArgList));
	pRequest->SendPostRequest();
	pRequest->Wait();

	pCore->GetConsole()->Admin() << pRequest->GetResponse() << newl;
}


/*--------------------
  RebootCURL
  --------------------*/
CMD(RebootCURL, 0)
{
	pCore->GetHTTPManager()->Reboot();
}


/*--------------------
  GenerateChannels
  --------------------*/
CMD(GenerateChannels, 1)
{
	uint uiCount(AtoI(vArgList[0]));
	for (uint uiIndex(0); uiIndex < uiCount; ++uiIndex)
		pCore->GetChannelManager()->CreatePublicChannel(L"Test Channel #" + XtoW(uiIndex));
}


/*====================
  RandomString
  ====================*/
static
tstring		RandomString(uint uiLength)
{
	tstring sRet;

	for (uint ui(0); ui < uiLength; ++ui)
		sRet += char(M_Random(int('A'), int('Z')));

	return sRet;
}


/*--------------------
  SpamChannels
  --------------------*/
CMD(SpamChannels, 1)
{
	uint uiCount(AtoI(vArgList[0]));
	for (uint uiIndex(0); uiIndex < uiCount; ++uiIndex)
		pCore->GetChannelManager()->CreatePublicChannel(RandomString(M_Random(3, 12)));
}


/*--------------------
  PrintGameInfo
  --------------------*/
CMD(PrintGameInfo, 1)
{
	pCore->GetGameServerManager()->PrintGameInfo(WideToSingle(vArgList[0]));
}


/*--------------------
  PrintRegionDetails
  --------------------*/
CMD(PrintRegionDetails, 1)
{
	pCore->GetMatchMaker()->PrintRegionDetails(vArgList[0]);
}


/*--------------------
  PrintPlayerCounts
  --------------------*/
CMD(PrintPlayerCounts, 0)
{
	pCore->GetClientManager()->PrintPlayerCounts();
}


/*--------------------
  PrintPlayerCountTotals
  --------------------*/
CMD(PrintPlayerCountTotals, 0)
{
	pCore->GetClientManager()->PrintPlayerCountTotals();
}

// (C)2008 S2 Games
// c_channelmanager.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "chatserver_common.h"

#include "c_channelmanager.h"
#include "c_channel.h"
#include "c_console.h"
#include "c_client.h"
//=============================================================================

/*====================
  CChannelManager::~CChannelManager
  ====================*/
CChannelManager::~CChannelManager()
{
	for (ChannelVector_cit itChannel(m_pChannels->begin()), itEnd(m_pChannels->end()); itChannel != itEnd; ++itChannel)
		delete *itChannel;
	delete m_pChannels;

	delete m_pGeneralChannels;
	delete m_pPublicChannels;
	delete m_pChannelNames;
	delete m_pClanChannels;
	delete m_pMatchChannels;

	delete m_pProcessChannels;

	delete m_pSTLHeap;
	delete m_pHeap;
}


/*====================
  CChannelManager::AllocateChannel
  ====================*/
CChannel*	CChannelManager::AllocateChannel()
{
	PROFILE("CChannelManager::AllocateChannel");

	CChannel *pChannel(HEAP_NEW(m_pMemManager, m_pHeap) CChannel(m_pCore, m_pHeap));
	if (pChannel == NULL)
		return NULL;

	uint uiChannelID(INVALID_CHANNEL);
	if (m_vAvailableIDs.empty())
	{
		uiChannelID = INT_SIZE(m_pChannels->size());
		m_pChannels->push_back(pChannel);
	}
	else
	{
		uiChannelID = m_vAvailableIDs.back();
		m_vAvailableIDs.pop_back();
		m_pChannels->at(uiChannelID) = pChannel;
	}

	pChannel->SetChannelID(uiChannelID);
	return pChannel;
}


/*====================
  CChannelManager::RemoveChannel
  ====================*/
void	CChannelManager::RemoveChannel(CChannel *pChannel)
{
	PROFILE("CChannelManager::RemoveChannel");

	if (pChannel == NULL)
		return;

	uint uiChannelID(pChannel->GetChannelID());
	m_vAvailableIDs.push_back(uiChannelID);
	
	m_pChannels->at(uiChannelID) = NULL;
	m_pChannelNames->erase(pChannel->GetNameLower());
	
	if (pChannel->HasAllFlags(CHANNEL_FLAG_CLAN))
		m_pClanChannels->erase(pChannel->GetClanID());

	if (pChannel->HasAllFlags(CHANNEL_FLAG_GENERAL_USE))
	{
		m_vAvailableGeneralChannels.push_back(pChannel->GetGeneralIndex());
		m_pGeneralChannels->at(pChannel->GetGeneralIndex()) = NULL;
	}

	if (pChannel->GetMatchID() != -1)
		m_pMatchChannels->erase(pChannel->GetMatchID());

	if (pChannel->GetPublicID() != -1)
	{
		m_vAvailablePublicIDs.push_back(pChannel->GetPublicID());
		m_pPublicChannels->at(pChannel->GetPublicID()) = NULL;
	}

	SAFE_DELETE(pChannel);
}


/*====================
  CChannelManager::CChannelManager
  ====================*/
CChannelManager::CChannelManager() :
NULL_CORE_API,
m_pHeap(NULL),
m_pSTLHeap(NULL),

m_pChannels(NULL),
m_pGeneralChannels(NULL),
m_pPublicChannels(NULL),
m_pChannelNames(NULL),
m_pClanChannels(NULL),
m_pMatchChannels(NULL),
m_pProcessChannels(NULL)
{
}


/*====================
  CChannelManager::Initialize
  ====================*/
bool	CChannelManager::Initialize(CCore *pCore)
{
	INIT_CORE_API(pCore);

	m_pHeap = m_pMemManager->AllocateHeap("channel");
	m_pSTLHeap = m_pMemManager->AllocateHeap("channel_stl");

	m_pChannels = HEAP_NEW(m_pMemManager, m_pHeap) ChannelVector(ChannelVector_alloc(m_pMemManager, m_pSTLHeap));
	m_pGeneralChannels = HEAP_NEW(m_pMemManager, m_pHeap) ChannelVector(ChannelVector_alloc(m_pMemManager, m_pSTLHeap));
	m_pPublicChannels = HEAP_NEW(m_pMemManager, m_pHeap) ChannelVector(ChannelVector_alloc(m_pMemManager, m_pSTLHeap));
	m_pChannelNames = HEAP_NEW(m_pMemManager, m_pHeap) ChannelNameMap(std::less<wstring>(), ChannelNameMap_alloc(m_pMemManager, m_pSTLHeap));
	m_pClanChannels = HEAP_NEW(m_pMemManager, m_pHeap) ClanChannelMap(std::less<uint>(), ClanChannelMap_alloc(m_pMemManager, m_pSTLHeap));
	m_pMatchChannels = HEAP_NEW(m_pMemManager, m_pHeap) MatchChannelMap(std::less<uint>(), MatchChannelMap_alloc(m_pMemManager, m_pSTLHeap));
	m_pProcessChannels = HEAP_NEW(m_pMemManager, m_pHeap) ChannelList(ChannelList_alloc(m_pMemManager, m_pSTLHeap));

	CreateGeneralChannel();

	return true;
}


/*====================
  CChannelManager::Frame
  ====================*/
void	CChannelManager::Frame()
{
	PROFILE("CChannelManager::Frame");

	for (ChannelList_it itChannel(m_pProcessChannels->begin()), itChannelEnd(m_pProcessChannels->end()); itChannel != itChannelEnd; )
	{
		switch ((*itChannel)->Frame())
		{
		case CHANNEL_ACTIVE:
			++itChannel;
			break;

		case CHANNEL_IDLE:
			STL_ERASE(*m_pProcessChannels, itChannel);
			break;

		case CHANNEL_DEAD:
			RemoveChannel(*itChannel);
			STL_ERASE(*m_pProcessChannels, itChannel);
			break;
		}
	}
}


/*====================
  CChannelManager::GetDefaultChannel
  ====================*/
CChannel*	CChannelManager::GetDefaultChannel(const CClient *pClient)
{
	PROFILE("CChannelManager::GetDefaultChannel");

	// Put them in their clan channel if they are in a clan
	if (pClient->HasClan())
	{
		CChannel *pChannel(GetClanChannel(pClient->GetClanID()));
		if (pChannel == NULL)
			pChannel = CreateClanChannel(pClient->GetClan());

		if (pChannel != NULL)
			return pChannel;
	}

	// Check for already in a general channel
	CChannel *pChannel(pClient->GetGeneralChannel());
	if (pChannel != NULL)
		return pChannel;

	return GetGeneralChannel();
}


/*====================
  CChannelManager::CreatePublicChannel
  ====================*/
CChannel*	CChannelManager::CreatePublicChannel(const wstring &sName, uint uiFlags, bool bForce, const wstring &sTopic)
{
	PROFILE("CChannelManager::CreatePublicChannel");

	// Sanitize name
	wstring sCleanName(StripExtraneousSpaces(sName));

	size_t zStart(0);
	size_t zEnd(0);
	while (zEnd != wstring::npos)
	{
		zStart = sCleanName.find_first_of(L"'<>\t\n\r\"\\", zStart);
		if (zStart == wstring::npos)
			break;

		zEnd = sCleanName.find_first_not_of(L"'<>\t\n\r\"\\", zStart);
		sCleanName.erase(zStart, zEnd - zStart);
	}

	if (sCleanName.size() > 20)
		sCleanName.erase(20);

	if (sCleanName.empty() || IsAllWhiteSpace(sCleanName))
		return NULL;

	wstring sLowerName(LowerString(sCleanName));

	// Check for reserved names
	if (!bForce &&
		(sLowerName.substr(0, 2) == L"s2" ||
		sLowerName.substr(0, 4) == L"clan" ||
		sLowerName.substr(0, 5) == L"match"))
		return NULL;

	CChannel *pChannel(GetChannel(sLowerName));
	if (pChannel != NULL)
	{
		pChannel->AddFlags(uiFlags);
		return pChannel;
	}

	pChannel = AllocateChannel();
	if (pChannel == NULL)
		return NULL;

	pChannel->SetFlags(uiFlags);
	pChannel->SetName(sCleanName);
	pChannel->SetTopic(sTopic);

	m_pChannelNames->insert(ChannelNameMap_pair(sLowerName, pChannel));

	// Store in public channel vector
	uint uiIndex(0);
	if (m_vAvailablePublicIDs.empty())
	{
		uiIndex = INT_SIZE(m_pPublicChannels->size());
		m_pPublicChannels->push_back(pChannel);
	}
	else
	{
		uiIndex = m_vAvailablePublicIDs.back();
		m_vAvailablePublicIDs.pop_back();
		m_pPublicChannels->at(uiIndex) = pChannel;
	}

	pChannel->SetPublicID(uiIndex);
	return pChannel;
}


/*====================
  CChannelManager::CreateGeneralChannel
  ====================*/
CChannel*	CChannelManager::CreateGeneralChannel()
{
	PROFILE("CChannelManager::CreateGeneralChannel");

	CChannel *pChannel(AllocateChannel());
	if (pChannel == NULL)
		return NULL;

	uint uiIndex(0);
	if (m_vAvailableGeneralChannels.empty())
	{
		uiIndex = INT_SIZE(m_pGeneralChannels->size());
		m_pGeneralChannels->push_back(pChannel);
	}
	else
	{
		uiIndex = m_vAvailableGeneralChannels.back();
		m_vAvailableGeneralChannels.pop_back();
		m_pGeneralChannels->at(uiIndex) = pChannel;
	}
	
	pChannel->SetFlags(CHANNEL_FLAG_GENERAL_USE | CHANNEL_FLAG_RESERVED);
	pChannel->SetTopic(L"Welcome to Heroes of Newerth!");
	if (uiIndex == 0)
	{
		pChannel->SetName(L"HoN");
		pChannel->AddFlags(CHANNEL_FLAG_PERMANENT);
	}
	else
	{
		pChannel->SetName(L"HoN " + XtoW(uiIndex + 1));
	}

	pChannel->SetGeneralIndex(uiIndex);
	m_pChannelNames->insert(ChannelNameMap_pair(pChannel->GetNameLower(), pChannel));
	return pChannel;
}


/*====================
  CChannelManager::CreateClanChannel
  ====================*/
CChannel*	CChannelManager::CreateClanChannel(CClan *pClan)
{
	PROFILE("CChannelManager::CreateClanChannel");

	if (pClan == NULL)
		return NULL;

	ClanChannelMap_it itChannel(m_pClanChannels->find(pClan->GetClanID()));
	if (itChannel != m_pClanChannels->end())
		return itChannel->second;

	CChannel *pChannel(AllocateChannel());
	if (pChannel == NULL)
		return NULL;

	pChannel->SetFlags(CHANNEL_FLAG_CLAN | CHANNEL_FLAG_PERMANENT | CHANNEL_FLAG_RESERVED);
	pChannel->SetClanID(pClan->GetClanID());
	pChannel->SetName(L"Clan " + pClan->GetName());
	pChannel->SetTopic(L"Welcome to the " + pClan->GetName() + L" clan channel!");

	m_pChannelNames->insert(ChannelNameMap_pair(pChannel->GetNameLower(), pChannel));
	m_pClanChannels->insert(ClanChannelMap_pair(pClan->GetClanID(), pChannel));
	return pChannel;
}


/*====================
  CChannelManager::CreateMatchChannel
  ====================*/
CChannel*	CChannelManager::CreateMatchChannel(uint uiMatchID)
{
	PROFILE("CChannelManager::CreateMatchChannel");

	if (uiMatchID == -1)
		return NULL;

	MatchChannelMap_it itChannel(m_pMatchChannels->find(uiMatchID));
	if (itChannel != m_pMatchChannels->end())
		return itChannel->second;

	CChannel *pChannel(AllocateChannel());
	if (pChannel == NULL)
		return NULL;

	pChannel->SetFlags(CHANNEL_FLAG_UNJOINABLE | CHANNEL_FLAG_SERVER | CHANNEL_FLAG_RESERVED);
	pChannel->SetName(L"Match " + XtoW(uiMatchID));
	pChannel->SetTopic(L"Post-Game Chat Channel");
	pChannel->SetMatchID(uiMatchID);

	m_pChannelNames->insert(ChannelNameMap_pair(pChannel->GetNameLower(), pChannel));
	m_pMatchChannels->insert(MatchChannelMap_pair(uiMatchID, pChannel));
	return pChannel;
}


/*====================
  CChannelManager::GetGeneralChannel
  ====================*/
CChannel*	CChannelManager::GetGeneralChannel()
{
	PROFILE("CChannelManager::GetGeneralChannel");

	for (ChannelVector_it itChannel(m_pGeneralChannels->begin()), itEnd(m_pGeneralChannels->end()); itChannel != itEnd; ++itChannel)
	{
		CChannel *pChannel(*itChannel);
		if (pChannel == NULL)
			return CreateGeneralChannel();

		if (pChannel->GetClientCount() < 50)
			return pChannel;
	}

	return CreateGeneralChannel();
}


/*====================
  CChannelManager::GetClanChannel
  ====================*/
CChannel*	CChannelManager::GetClanChannel(uint uiClanID)
{
	PROFILE("CChannelManager::GetClanChannel");

	if (uiClanID == INVALID_CLAN_ID)
		return NULL;

	ClanChannelMap_it itChannel(m_pClanChannels->find(uiClanID));
	if (itChannel == m_pClanChannels->end())
		return NULL;

	return itChannel->second;
}


/*====================
  CChannelManager::GetChannel
  ====================*/
CChannel*	CChannelManager::GetChannel(const wstring &sName)
{
	PROFILE("CChannelManager::GetChannel");

	ChannelNameMap_it itChannel(m_pChannelNames->find(LowerString(sName)));
	if (itChannel == m_pChannelNames->end())
		return NULL;

	return itChannel->second;
}


/*====================
  CChannelManager::SendChannelListing
  ====================*/
void	CChannelManager::SendChannelListing(CClient *pClient)
{
	PROFILE("CChannelManager::SendChannelListing");

	if (pClient == NULL)
		return;

	static CBufferDynamic buffer(4096);
	buffer.Clear();

	uint uiCount(0);
	uint uiIndex(pClient->GetChannelListIndex());
	while (uiIndex < m_pChannels->size() && uiCount < 140)
	{
		CChannel *pChannel(m_pChannels->at(uiIndex));
		++uiIndex;

		if (pChannel == NULL)
			continue;

		if (!pChannel->CanSee(pClient))
			continue;

		++uiCount;
		buffer << NET_CHAT_CL_CHANNEL_INFO << pChannel->GetChannelID() << pChannel->GetNameUTF8() << byte(0) << pChannel->GetClientCount();
	}

	if (uiIndex < m_pChannels->size())
		pClient->SetChannelListIndex(uiIndex);
	else
		pClient->SetChannelListIndex(INVALID_CHANNEL);

	buffer << NET_CHAT_CL_CHANNEL_LIST_SYN;
	pClient->Send(buffer);
}


/*====================
  CChannelManager::SendChannelSublisting
  ====================*/
void	CChannelManager::SendChannelSublisting(CClient *pClient)
{
	PROFILE("CChannelManager::SendChannelListing");

	if (pClient == NULL)
		return;

	static CBufferDynamic buffer(4096);
	buffer.Clear();

	uint uiCount(0);
	uint uiIndex(pClient->GetChannelListIndex());
	byte ySequence(pClient->GetChannelListSequence());
	wstring sHead(pClient->GetChannelListHead());
	while (uiIndex < m_pChannels->size() && uiCount < 140)
	{
		CChannel *pChannel(m_pChannels->at(uiIndex));
		++uiIndex;

		if (pChannel == NULL)
			continue;

		if (!pChannel->CanSee(pClient))
			continue;

		if (pChannel->GetNameLower().compare(0, sHead.length(), sHead) != 0)
			continue;

		++uiCount;
		buffer << NET_CHAT_CL_CHANNEL_INFO_SUB << ySequence << pChannel->GetChannelID() << pChannel->GetNameUTF8() << byte(0) << pChannel->GetClientCount();
	}

	if (uiIndex < m_pChannels->size())
	{
		pClient->SetChannelListIndex(uiIndex);
		buffer << NET_CHAT_CL_CHANNEL_SUBLIST_SYN << ySequence;
	}
	else
	{
		pClient->SetChannelListIndex(INVALID_CHANNEL);
		buffer << NET_CHAT_CL_CHANNEL_SUBLIST_END << ySequence;
	}
	
	pClient->Send(buffer);
}


/*====================
  CChannelManager::AddNextChannelInfo
  ====================*/
uint	CChannelManager::AddNextChannelInfo(uint uiChannelID, CPacket &pkt)
{
	for (uint uiNextChannelID(uiChannelID); uiNextChannelID < m_pChannels->size(); ++uiNextChannelID)
	{
		CChannel *pChannel(m_pChannels->at(uiNextChannelID));
		while (pChannel == NULL && uiNextChannelID < m_pChannels->size())
		{
			pChannel = m_pChannels->at(uiNextChannelID);
			++uiNextChannelID;
		}

		if (pChannel == NULL)
			return INVALID_CHANNEL;

		if (!pChannel->GetInfoForPeer(pkt))
			return uiNextChannelID;
	}

	return INVALID_CHANNEL;
}


/*====================
  CChannelManager::UpdateChannel
  ====================*/
void	CChannelManager::UpdateChannel(CPacket &pkt)
{
	uint uiChannelID(pkt.ReadInt());
	
	if (uiChannelID >= m_pChannels->size())
		m_pChannels->resize(uiChannelID + 1, NULL);

	if (m_pChannels->at(uiChannelID) == NULL)
		m_pChannels->at(uiChannelID) = HEAP_NEW(m_pMemManager, m_pHeap) CChannel(m_pCore, m_pHeap);

	if (m_pChannels->at(uiChannelID) == NULL)
		return;

	m_pChannels->at(uiChannelID)->ReadUpdate(pkt);
}


/*====================
  CChannelManager::LoadState
  ====================*/
void	CChannelManager::LoadState(CFileHandle &hLoadFile)
{
	while (!hLoadFile.IsEOF())
	{
		wsvector vChannelInfo(TokenizeString(hLoadFile.ReadLineW(), L';', L'\\'));
		if (vChannelInfo.size() < 10)
			continue;

		wsvector vFlags(TokenizeString(vChannelInfo[0], L','));
		uint uiFlags(0);
		for (uint uiIndex(0); uiIndex < vFlags.size(); ++uiIndex)
		{
			if (vFlags[uiIndex] == L"PERMANENT")
				uiFlags |= CHANNEL_FLAG_PERMANENT;
			if (vFlags[uiIndex] == L"SERVER")
				uiFlags |= CHANNEL_FLAG_SERVER;
			if (vFlags[uiIndex] == L"HIDDEN")
				uiFlags |= CHANNEL_FLAG_HIDDEN;
			if (vFlags[uiIndex] == L"RESERVED")
				uiFlags |= CHANNEL_FLAG_RESERVED;
			if (vFlags[uiIndex] == L"GENERAL")
				uiFlags |= CHANNEL_FLAG_GENERAL_USE;
			if (vFlags[uiIndex] == L"UNJOINABLE")
				uiFlags |= CHANNEL_FLAG_UNJOINABLE;
			if (vFlags[uiIndex] == L"AUTH_REQUIRED")
				uiFlags |= CHANNEL_FLAG_AUTH_REQUIRED;
			if (vFlags[uiIndex] == L"CLAN")
				uiFlags |= CHANNEL_FLAG_CLAN;
		}

		uint uiClanID(AtoI(vChannelInfo[1]));
		uint uiMatchID(AtoI(vChannelInfo[2]));
		wstring sName(UnescapeCharacters(vChannelInfo[3]));
		wstring sTopic(UnescapeCharacters(vChannelInfo[4]));
		wstring sPassword(UnescapeCharacters(vChannelInfo[4]));

		CChannel *pChannel(NULL);
		if (uiClanID != INVALID_CLAN_ID && (uiFlags & CHANNEL_FLAG_CLAN))
			pChannel = CreateClanChannel(NULL);
		else if ((uiFlags & (CHANNEL_FLAG_CLAN | CHANNEL_FLAG_GENERAL_USE)) == 0)
			pChannel = CreatePublicChannel(sName, uiFlags, true, sTopic);

		if (pChannel == NULL)
			continue;

		pChannel->SetFlags(uiFlags);
		if (!sPassword.empty())
			pChannel->SetPassword(NULL, sPassword);
	}
}


/*====================
  CChannelManager::SaveState
  ====================*/
void	CChannelManager::SaveState(CFileHandle &hSaveFile) const
{
	for (ChannelVector_cit itChannel(m_pChannels->begin()), itEnd(m_pChannels->end()); itChannel != itEnd; ++itChannel)
	{
		if (*itChannel == NULL)
			continue;

		(*itChannel)->SaveState(hSaveFile);
	}
}


/*====================
  CChannelManager::PrintUsage
  ====================*/
void	CChannelManager::PrintUsage() const
{
	uint uiActiveCount(0);
	uint uiGeneralCount(0);
	uint uiPublicCount(0);

	for (ChannelVector_cit itChannel(m_pChannels->begin()), itEnd(m_pChannels->end()); itChannel != itEnd; ++itChannel)
	{
		CChannel *pChannel(*itChannel);
		if (pChannel == NULL)
			continue;

		++uiActiveCount;

		if (pChannel->HasAllFlags(CHANNEL_FLAG_GENERAL_USE))
			++uiGeneralCount;

		if (pChannel->GetPublicID() != -1)
			++uiPublicCount;
	}

	m_pConsole->Admin()
		<< newl
		<< L"Channels    " << newl
		<< L"------------" << newl
		<< L"Total:      " << XtoW(uiActiveCount, FMT_DELIMIT, 5) << L" / " << XtoW(INT_SIZE(m_pChannels->size()), FMT_DELIMIT) << newl
		<< L"Public:     " << XtoW(uiPublicCount, FMT_DELIMIT, 5) << L" / " << XtoW(INT_SIZE(m_pPublicChannels->size()), FMT_DELIMIT) << newl
		<< L"General:    " << XtoW(uiGeneralCount, FMT_DELIMIT, 5) << L" / " << XtoW(INT_SIZE(m_pGeneralChannels->size()), FMT_DELIMIT) << newl
		<< L"Clan:       " << XtoW(INT_SIZE(m_pClanChannels->size()), FMT_DELIMIT, 5) << newl
		<< L"Match:      " << XtoW(INT_SIZE(m_pMatchChannels->size()), FMT_DELIMIT, 5) << newl;
}

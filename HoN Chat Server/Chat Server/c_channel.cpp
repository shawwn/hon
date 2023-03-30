// (C)2009 S2 Games
// c_channel.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "chatserver_common.h"

#include "c_channel.h"
#include "c_console.h"
#include "c_clientmanager.h"
//=============================================================================

/*====================
  CChannel::~CChannel
  ====================*/
CChannel::~CChannel()
{
	delete m_pClients;
}


/*====================
  CChannel::CChannel
  ====================*/
CChannel::CChannel(CCore *pCore, CHeap *pHeap) :
NULL_CORE_API,
m_pHeap(pHeap),

m_pClients(NULL),
m_eStatus(CHANNEL_IDLE),
m_uiChannelID(INVALID_CHANNEL),
m_uiClanID(INVALID_CLAN_ID),
m_uiGeneralIndex(-1),
m_uiMatchID(-1),
m_uiPublicID(-1),
m_uiFlags(0)
{
	INIT_CORE_API(pCore);

	m_pClients = HEAP_NEW(m_pMemManager, m_pHeap) ClientSet(std::less<ClientSet::key_type>(), ClientSet_alloc(m_pMemManager, m_pHeap));
}


/*====================
  CChannel::SetTopic
  ====================*/
void	CChannel::SetTopic(const wstring &sTopic)
{
	PROFILE("CChannel::SetTopic")

	// limit the topic length to avoid some channel name + topic UI problems
	string sTopicUTF8(WStringToUTF8(sTopic.substr(0, CHAT_CHANNEL_TOPIC_MAX_LENGTH)));

	if (m_sTopicUTF8 == sTopicUTF8)
		return;

	m_sTopicUTF8 = sTopicUTF8;
	
	m_bufferSend.Clear();
	m_bufferSend << CHAT_CMD_CHANNEL_TOPIC << m_uiChannelID << m_sTopicUTF8 << byte(0);
	Broadcast(m_bufferSend);
}


/*====================
  CChannel::Frame
  ====================*/
EChannelStatus	CChannel::Frame()
{
	PROFILE("CChannel::Frame")

	// Check for silence expiring
	map<uint, uint>::iterator itUser(m_mapSilenced.begin());
	while (itUser != m_mapSilenced.end())
	{
		if (itUser->second > m_pCore->GetMilliseconds())
		{
			++itUser;
			continue;
		}

		// User is now unsilenced
		CClient *pTarget(m_pClientManager->GetClientFromAccountID(itUser->first));

		STL_ERASE(m_mapSilenced, itUser);

		if (pTarget == NULL)
			continue;

		m_bufferSend.Clear();
		m_bufferSend << CHAT_CMD_CHANNEL_SILENCE_LIFTED << m_sNameUTF8 << byte(0);
		pTarget->Send(m_bufferSend);
	}

	if (m_mapSilenced.empty())
		m_eStatus = CHANNEL_IDLE;

	if (m_pClients->empty() && !HasAllFlags(CHANNEL_FLAG_PERMANENT))
		m_eStatus = CHANNEL_DEAD;

	return m_eStatus;
}


/*====================
  CChannel::Broadcast
  ====================*/
void	CChannel::Broadcast(IBuffer &buffer, CClient *pIgnoreClient)
{
	PROFILE("CChannel::Broadcast")

	for (ClientSet_it itClient(m_pClients->begin()), itEnd(m_pClients->end()); itClient != itEnd; ++itClient)
	{
		if (*itClient != pIgnoreClient)
			(*itClient)->Send(buffer);
	}
}


/*====================
  CChannel::BroadcastChannelUpdate
  ====================*/
void	CChannel::BroadcastChannelUpdate()
{
	PROFILE("CChannel::BroadcastChannelUpdate")

	m_bufferSend.Clear();
	m_bufferSend
		<< CHAT_CMD_CHANNEL_UPDATE
		<< m_uiChannelID
		<< m_sNameUTF8 << byte(0)
		<< byte(m_uiFlags & 0xff)
		<< m_sTopicUTF8 << byte(0)
		<< INT_SIZE(m_mapAdmins.size());

	for (AdminMap_it it(m_mapAdmins.begin()), itEnd(m_mapAdmins.end()); it != itEnd; ++it)
		m_bufferSend << it->first << byte(it->second);

	Broadcast(m_bufferSend);
}


/*====================
  CChannel::SetPassword
  ====================*/
void	CChannel::SetPassword(CClient *pClient, const wstring &sPassword)
{
	PROFILE("CChannel::SetPassword")

	if (!IsAdmin(pClient, CHAT_CLIENT_ADMIN_LEADER))
		return;

	m_sPassword = sPassword;

	m_bufferSend.Clear();
	m_bufferSend << CHAT_CMD_CHANNEL_SET_PASSWORD << m_uiChannelID << pClient->GetNameUTF8() << byte(0);
	Broadcast(m_bufferSend);
}


/*====================
  CChannel::CanJoin
  ====================*/
bool	CChannel::CanJoin(CClient *pClient, bool bForce, const wstring &sPassword)
{
	PROFILE("CChannel::CanJoin")

	if (pClient == NULL || HasClient(pClient))
		return false;

	if (bForce)
		return true;

	if (HasAnyFlags(CHANNEL_FLAG_UNJOINABLE))
		return false;

	if (IsAdmin(pClient))
		return true;

	if (HasAnyFlags(CHANNEL_FLAG_AUTH_REQUIRED) && m_setAuthed.find(pClient->GetNameLower()) == m_setAuthed.end())
		return false;

	if (!m_sPassword.empty() && CompareNoCase(m_sPassword, sPassword) != 0)
		return false;

	return true;
}


/*====================
  CChannel::CanSee
  ====================*/
bool	CChannel::CanSee(CClient *pClient)
{
	PROFILE("CChannel::CanSee")

	if (HasAnyFlags(CHANNEL_FLAG_HIDDEN | CHANNEL_FLAG_UNJOINABLE))
		return false;

	if (IsAdmin(pClient))
		return true;

	if (HasAllFlags(CHANNEL_FLAG_AUTH_REQUIRED) && m_setAuthed.find(pClient->GetNameLower()) == m_setAuthed.end())
		return false;

	// FIXME: Passworded channels should be visible, but marked and prompt for password on join
	if (HasPassword())
		return false;

	return true;
}


/*====================
  CChannel::GetAdminLevel
  ====================*/
EAdminLevel	CChannel::GetAdminLevel(CClient *pClient)
{
	PROFILE("CChannel::GetAdminLevel")

	if (pClient == NULL)
		return CHAT_CLIENT_ADMIN_NONE;
	else if (pClient->HasAllFlags(CLIENT_IS_STAFF))
		return CHAT_CLIENT_ADMIN_STAFF;
	else if (HasAnyFlags(CHANNEL_FLAG_GENERAL_USE | CHANNEL_FLAG_SERVER))
		return CHAT_CLIENT_ADMIN_NONE;

	EAdminLevel eRank(CHAT_CLIENT_ADMIN_NONE);
	if (HasAllFlags(CHANNEL_FLAG_CLAN) && m_uiClanID == pClient->GetClanID())
	{
		if (pClient->HasAllFlags(CLIENT_IS_CLAN_LEADER))
			eRank = CHAT_CLIENT_ADMIN_ADMINISTRATOR;
		else if (pClient->HasAllFlags(CLIENT_IS_OFFICER))
			eRank = CHAT_CLIENT_ADMIN_LEADER;
	}

	AdminMap_it it(m_mapAdmins.find(pClient->GetAccountID()));
	if (it != m_mapAdmins.end())
		return MAX(eRank, it->second);

	return eRank;
}


/*====================
  CChannel::IsBanned
  ====================*/
bool	CChannel::IsBanned(CClient *pClient)
{
	PROFILE("CChannel::IsBanned")

	if (GetAdminLevel(pClient) == CHAT_CLIENT_ADMIN_STAFF)
		return false;

	if (m_setBanned.find(pClient->GetAccountID()) == m_setBanned.end())
		return false;
	
	return true;
}


/*====================
  CChannel::IsSilenced
  ====================*/
bool	CChannel::IsSilenced(CClient *pClient)
{
	PROFILE("CChannel::IsSilenced")

	if (m_mapSilenced.find(pClient->GetAccountID()) != m_mapSilenced.end() && GetAdminLevel(pClient) < CHAT_CLIENT_ADMIN_STAFF)
		return true;

	return false;
}


/*====================
  CChannel::AddClient
  ====================*/
bool	CChannel::AddClient(CClient *pClient, bool bForce, const wstring &sPassword)
{
	PROFILE("CChannel::AddClient")

	if (!CanJoin(pClient, bForce, sPassword))
		return false;

	// Reject banned clients
	if (IsBanned(pClient))
	{
		m_bufferSend.Clear();
		m_bufferSend << CHAT_CMD_CHANNEL_IS_BANNED << m_sNameUTF8 << byte(0);
		pClient->Send(m_bufferSend);
		return false;
	}

	// Check if the client should be made an admin
	EAdminLevel eRank(GetAdminLevel(pClient));
	if (IsFree())
		eRank = MAX(eRank, CHAT_CLIENT_ADMIN_ADMINISTRATOR);
	if (eRank > CHAT_CLIENT_ADMIN_NONE)
	{
		m_mapAdmins[pClient->GetAccountID()] = eRank;
		BroadcastChannelUpdate();
	}

	// Update clients already in the channel on the new user
	m_bufferSend.Clear();
	m_bufferSend << CHAT_CMD_JOINED_CHANNEL << pClient->GetNameUTF8() << byte(0) << pClient->GetAccountID() << m_uiChannelID << pClient->GetStatusByte() << pClient->GetFlags();
	Broadcast(m_bufferSend);

	// Update the new user on clients already in the channel
	m_bufferSend.Clear();
	m_bufferSend
		<< CHAT_CMD_CHANGED_CHANNEL
		<< m_sNameUTF8 << byte(0)
		<< m_uiChannelID
		<< byte(m_uiFlags & 0xff)
		<< m_sTopicUTF8 << byte(0)
		<< INT_SIZE(m_mapAdmins.size());

	for (AdminMap_it itAdmin(m_mapAdmins.begin()), itEnd(m_mapAdmins.end()); itAdmin != itEnd; ++itAdmin)
		m_bufferSend << itAdmin->first << byte(itAdmin->second);

	m_bufferSend << INT_SIZE(m_pClients->size());

	for (ClientSet_it it(m_pClients->begin()), itEnd(m_pClients->end()); it != itEnd; ++it)
	{
		CClient *pTarget(*it);
		m_bufferSend << pTarget->GetNameUTF8() << byte(0) << pTarget->GetAccountID() << pTarget->GetStatusByte() << pTarget->GetFlags();
	}

	pClient->Send(m_bufferSend);

	m_pClients->insert(pClient);
	pClient->AddChannel(m_uiChannelID);
	return true;
}


/*====================
  CChannel::RemoveClient
  ====================*/
void	CChannel::RemoveClient(CClient *pClient)
{
	PROFILE("CChannel::RemoveClient")

	if (pClient == NULL || m_pClients->find(pClient) == m_pClients->end())
		return;

	// Update clients already in the channel on the user
	m_bufferSend.Clear();
	m_bufferSend << CHAT_CMD_LEFT_CHANNEL << pClient->GetAccountID() << m_uiChannelID;
	Broadcast(m_bufferSend);

	// Remove the user from the channel
	m_pClients->erase(pClient);
	pClient->RemoveChannel(m_uiChannelID);

	// Channel needs to check for deletion
	SetActive();
}


/*====================
  CChannel::EnableAuth
  ====================*/
void	CChannel::EnableAuth(CClient *pClient)
{
	PROFILE("CChannel::EnableAuth")

	if (!IsAdmin(pClient, CHAT_CLIENT_ADMIN_LEADER))
		return;

	AddFlags(CHANNEL_FLAG_AUTH_REQUIRED);

	m_bufferSend.Clear();
	m_bufferSend << CHAT_CMD_CHANNEL_SET_AUTH << m_uiChannelID;
	Broadcast(m_bufferSend);
}


/*====================
  CChannel::DisableAuth
  ====================*/
void	CChannel::DisableAuth(CClient *pClient)
{
	PROFILE("CChannel::DisableAuth")

	if (!IsAdmin(pClient, CHAT_CLIENT_ADMIN_LEADER))
		return;

	RemoveFlags(CHANNEL_FLAG_AUTH_REQUIRED);

	m_bufferSend.Clear();
	m_bufferSend << CHAT_CMD_CHANNEL_REMOVE_AUTH << m_uiChannelID;
	Broadcast(m_bufferSend);
}


/*====================
  CChannel::SendAuthList
  ====================*/
void	CChannel::SendAuthList(CClient *pClient)
{
	PROFILE("CChannel::SendAuthList")

	if (!IsAdmin(pClient, CHAT_CLIENT_ADMIN_LEADER))
		return;

	m_bufferSend.Clear();
	m_bufferSend << CHAT_CMD_CHANNEL_LIST_AUTH << m_uiChannelID << INT_SIZE(m_setAuthed.size());

	for (sset_it it(m_setAuthed.begin()), itEnd(m_setAuthed.end()); it != itEnd; ++it)
	{
		CClient *pClient(m_pClientManager->GetClientFromName(*it));

		if (pClient != NULL)
			m_bufferSend << pClient->GetNameUTF8() << byte(0);
		else
			m_bufferSend << TStringToUTF8(*it) << byte(0);
	}

	pClient->Send(m_bufferSend);
}


/*====================
  CChannel::AddAuth
  ====================*/
void	CChannel::AddAuth(CClient *pClient, const wstring &sName)
{
	PROFILE("CChannel::AddAuth")

	if (!IsAdmin(pClient, CHAT_CLIENT_ADMIN_LEADER))
		return;

	size_t zPos(sName.find(L"]"));
	wstring sLowerName(LowerString(zPos == wstring::npos ? sName : sName.substr(zPos + 1)));
	if (m_setAuthed.find(sLowerName) == m_setAuthed.end())
	{
		m_setAuthed.insert(sLowerName);

		m_bufferSend.Clear();
		m_bufferSend << CHAT_CMD_CHANNEL_ADD_AUTH_USER << m_uiChannelID << WStringToUTF8(sName) << byte(0);
		pClient->Send(m_bufferSend);
		return;
	}

	m_bufferSend.Clear();
	m_bufferSend << CHAT_CMD_CHANNEL_ADD_AUTH_FAIL << m_uiChannelID << WStringToUTF8(sName) << byte(0);
	pClient->Send(m_bufferSend);
}


/*====================
  CChannel::RemoveAuth
  ====================*/
void	CChannel::RemoveAuth(CClient *pClient, const wstring &sName)
{
	PROFILE("CChannel::RemoveAuth")

	if (!IsAdmin(pClient, CHAT_CLIENT_ADMIN_LEADER))
		return;

	wstring sLowerName(LowerString(sName));
	if (m_setAuthed.find(sLowerName) == m_setAuthed.end())
	{
		m_bufferSend.Clear();
		m_bufferSend << CHAT_CMD_CHANNEL_REM_AUTH_FAIL << m_uiChannelID << WStringToUTF8(sName) << byte(0);
		pClient->Send(m_bufferSend);
		return;
	}

	m_setAuthed.erase(sLowerName);

	m_bufferSend.Clear();
	m_bufferSend << CHAT_CMD_CHANNEL_REM_AUTH_USER << m_uiChannelID << WStringToUTF8(sName) << byte(0);
	pClient->Send(m_bufferSend);
}


/*====================
  CChannel::Demote
  ====================*/
void	CChannel::Demote(CClient *pClient, CClient *pTarget)
{
	PROFILE("CChannel::Demote")

	EAdminLevel eSource(GetAdminLevel(pClient));
	EAdminLevel eTarget(GetAdminLevel(pTarget));

	if (eSource == CHAT_CLIENT_ADMIN_NONE || eTarget == CHAT_CLIENT_ADMIN_NONE)
		return;

	if (eSource <= eTarget)
		return;

	switch (eTarget)
	{
	case CHAT_CLIENT_ADMIN_NONE:			break;
	case CHAT_CLIENT_ADMIN_OFFICER:			m_mapAdmins.erase(pTarget->GetAccountID()); break;
	case CHAT_CLIENT_ADMIN_LEADER:			m_mapAdmins[pTarget->GetAccountID()] = CHAT_CLIENT_ADMIN_OFFICER; break;
	case CHAT_CLIENT_ADMIN_ADMINISTRATOR:	m_mapAdmins[pTarget->GetAccountID()] = CHAT_CLIENT_ADMIN_LEADER; break;
	case CHAT_CLIENT_ADMIN_STAFF:			break;
	}

	m_bufferSend.Clear();
	m_bufferSend << CHAT_CMD_CHANNEL_DEMOTE << m_uiChannelID << pTarget->GetAccountID() << pClient->GetAccountID();
	Broadcast(m_bufferSend);
}


/*====================
  CChannel::Promote
  ====================*/
void	CChannel::Promote(CClient *pClient, CClient *pTarget)
{
	PROFILE("CChannel::Promote")

	if (pTarget == NULL)
		return;

	EAdminLevel eSource(GetAdminLevel(pClient));
	EAdminLevel eTarget(GetAdminLevel(pTarget));

	if (eSource <= eTarget + 1)
		return;

	switch (eTarget)
	{
	case CHAT_CLIENT_ADMIN_NONE:			m_mapAdmins[pTarget->GetAccountID()] = CHAT_CLIENT_ADMIN_OFFICER; break;
	case CHAT_CLIENT_ADMIN_OFFICER:			m_mapAdmins[pTarget->GetAccountID()] = CHAT_CLIENT_ADMIN_LEADER; break;
	case CHAT_CLIENT_ADMIN_LEADER:			m_mapAdmins[pTarget->GetAccountID()] = CHAT_CLIENT_ADMIN_ADMINISTRATOR; break;
	case CHAT_CLIENT_ADMIN_ADMINISTRATOR:	break;
	case CHAT_CLIENT_ADMIN_STAFF:			break;
	}

	m_bufferSend.Clear();
	m_bufferSend << CHAT_CMD_CHANNEL_PROMOTE << m_uiChannelID << pTarget->GetAccountID() << pClient->GetAccountID();
	Broadcast(m_bufferSend);
}


/*====================
  CChannel::Silence
  ====================*/
void	CChannel::Silence(CClient *pClient, CClient *pTarget, uint uiDuration)
{
	PROFILE("CChannel::SilenceUser")

	if (pTarget == NULL)
		return;

	EAdminLevel eSource(GetAdminLevel(pClient));
	EAdminLevel eTarget(GetAdminLevel(pTarget));

	if (eSource <= eTarget)
		return;

	const uint uiExpireTime(m_pCore->GetMilliseconds() + uiDuration);
	
	m_mapSilenced[pTarget->GetAccountID()] = uiExpireTime;

	m_bufferSend.Clear();
	m_bufferSend
		<< CHAT_CMD_CHANNEL_SILENCE_PLACED
		<< m_sNameUTF8 << byte(0)
		<< pClient->GetNameUTF8() << byte(0)
		<< pTarget->GetNameUTF8() << byte(0)
		<< uiDuration;
	Broadcast(m_bufferSend);

	if (!HasClient(pTarget))
		pTarget->Send(m_bufferSend);

	// Channel needs to check this silence expiring
	SetActive();
}


/*====================
  CChannel::Kick
  ====================*/
void	CChannel::Kick(CClient *pClient, CClient *pTarget)
{
	PROFILE("CChannel::Kick")

	if (pTarget == NULL)
		return;

	EAdminLevel eSource(GetAdminLevel(pClient));
	EAdminLevel eTarget(GetAdminLevel(pTarget));

	if (eSource <= eTarget)
		return;

	m_bufferSend.Clear();
	m_bufferSend << CHAT_CMD_CHANNEL_KICK << m_uiChannelID << pClient->GetAccountID() << pTarget->GetAccountID();
	Broadcast(m_bufferSend);

	RemoveClient(pTarget);
}


/*====================
  CChannel::Ban
  ====================*/
void	CChannel::Ban(CClient *pClient, CClient *pTarget)
{
	PROFILE("CChannel::Ban")

	if (pTarget == NULL)
		return;

	EAdminLevel eSource(GetAdminLevel(pClient));
	EAdminLevel eTarget(GetAdminLevel(pTarget));

	if (eSource <= eTarget)
		return;

	if (m_setBanned.find(pTarget->GetAccountID()) != m_setBanned.end())
		return;

	m_setBanned.insert(pTarget->GetAccountID());
	RemoveClient(pTarget);

	m_bufferSend.Clear();
	m_bufferSend << CHAT_CMD_CHANNEL_BAN << m_uiChannelID << pClient->GetAccountID() << pTarget->GetNameUTF8() << byte(0);
	Broadcast(m_bufferSend);

	pTarget->Send(m_bufferSend);
}


/*====================
  CChannel::LiftBan
  ====================*/
void	CChannel::LiftBan(CClient *pClient, CClient *pTarget)
{
	PROFILE("CChannel::LiftBan")

	if (!IsAdmin(pClient, CHAT_CLIENT_ADMIN_OFFICER) || pTarget == NULL)
		return;

	if (m_setBanned.find(pTarget->GetAccountID()) == m_setBanned.end())
		return;

	m_setBanned.erase(pTarget->GetAccountID());

	m_bufferSend.Clear();
	m_bufferSend << CHAT_CMD_CHANNEL_UNBAN << m_uiChannelID << pClient->GetAccountID() << pTarget->GetNameUTF8() << byte(0);
	Broadcast(m_bufferSend);

	if (!HasClient(pTarget))
		pTarget->Send(m_bufferSend);
}


/*====================
  CChannel::GetInfoForPeer
  ====================*/
bool	CChannel::GetInfoForPeer(CPacket &pkt)
{
	PROFILE("CChannel::GetInfoForPeer")

	size_t zSize(
		sizeof(byte) +									// Command
		sizeof(uint) +									// Channel ID
		sizeof(uint) +									// Clan ID
		sizeof(uint) +									// General Index
		sizeof(uint) +									// Match ID
		sizeof(uint) +									// Public ID
		sizeof(uint) +									// Flags
		m_sNameUTF8.size() + 1 +						// Name
		m_sTopicUTF8.size() + 1 +						// Topic
		m_sPassword.size() + 1 +						// Password
		m_pClients->size() * sizeof(uint) + 2 +		// Client list
		m_mapAdmins.size() * (sizeof(uint) + 2) + 2 +	// Admins
		m_setBanned.size() * sizeof(uint) + 2 +			// Bans
		0//m_setAuthed.size() * sizeof(uint) +  2		// Authed
		);

	if (zSize > pkt.GetRemainingSpace())
		return false;

	pkt << PEER_CMD_CHANNEL_INFO
		<< m_uiChannelID
		<< m_uiClanID
		<< m_uiGeneralIndex
		<< m_uiMatchID
		<< m_uiPublicID
		<< m_uiFlags
		<< m_sNameUTF8
		<< m_sTopicUTF8
		<< m_sPassword;

	pkt << ushort(m_pClients->size());
	for (ClientSet_it itClient(m_pClients->begin()), itEnd(m_pClients->end()); itClient != itEnd; ++itClient)
		pkt << (*itClient)->GetClientID();

	pkt << ushort(m_mapAdmins.size());
	for (AdminMap_it itAdmin(m_mapAdmins.begin()), itEnd(m_mapAdmins.end()); itAdmin != itEnd; ++itAdmin)
		pkt << itAdmin->first << uint(itAdmin->second);

	pkt << ushort(m_setBanned.size());
	for (uiset_it itBan(m_setBanned.begin()), itEnd(m_setBanned.end()); itBan != itEnd; ++itBan)
		pkt << *itBan;

	return true;
}


/*====================
  CChannel::ReadUpdate
  ====================*/
void	CChannel::ReadUpdate(CPacket &pkt)
{
	m_uiClanID = pkt.ReadInt();
	m_uiGeneralIndex = pkt.ReadInt();
	m_uiMatchID = pkt.ReadInt();
	m_uiPublicID = pkt.ReadInt();
	m_uiFlags = pkt.ReadInt();
	m_sNameUTF8 = pkt.ReadUTF8String();
	m_sNameLower = LowerString(UTF8ToWString(m_sNameUTF8));
	m_sTopicUTF8 = pkt.ReadUTF8String();
	m_sPassword = pkt.ReadWString();
}


/*====================
  CChannel::SaveState
  ====================*/
void	CChannel::SaveState(CFileHandle &hFile) const
{
	if (HasAllFlags(CHANNEL_FLAG_PERMANENT))
		hFile << L"PERMANENT,";
	if (HasAllFlags(CHANNEL_FLAG_SERVER))
		hFile << L"SERVER,";
	if (HasAllFlags(CHANNEL_FLAG_HIDDEN))
		hFile << L"HIDDEN,";
	if (HasAllFlags(CHANNEL_FLAG_RESERVED))
		hFile << L"RESERVED,";
	if (HasAllFlags(CHANNEL_FLAG_GENERAL_USE))
		hFile << L"GENERAL,";
	if (HasAllFlags(CHANNEL_FLAG_UNJOINABLE))
		hFile << L"UNJOINABLE,";
	if (HasAllFlags(CHANNEL_FLAG_AUTH_REQUIRED))
		hFile << L"AUTH_REQUIRED,";
	if (HasAllFlags(CHANNEL_FLAG_CLAN))
		hFile << L"CLAN,";
	hFile << L";";

	hFile
		<< m_uiClanID << L";"
		<< int(m_uiMatchID) << L";"
		<< EscapeCharacters(UTF8ToWString(m_sNameUTF8), L",;\\") << L";"
		<< EscapeCharacters(UTF8ToWString(m_sTopicUTF8), L",;\\") << L";"
		<< m_sPassword << L";";

	// Admins
	for (AdminMap_cit itAdmin(m_mapAdmins.begin()), itEnd(m_mapAdmins.end()); itAdmin != itEnd; ++itAdmin)
		hFile << itAdmin->first << L":" << itAdmin->second << L",";
	hFile << L";";

	// Bans
	for (uiset_cit itBan(m_setBanned.begin()), itEnd(m_setBanned.end()); itBan != itEnd; ++itBan)
		hFile << *itBan << L",";
	hFile << L";";

	// Auth list
	for (set<wstring>::const_iterator itAuth(m_setAuthed.begin()), itEnd(m_setAuthed.end()); itAuth != itEnd; ++itAuth)
		hFile << *itAuth << L",";
	hFile << L";";

	// Silence
	for (map<uint, uint>::const_iterator itSilence(m_mapSilenced.begin()), itEnd(m_mapSilenced.end()); itSilence != itEnd; ++itSilence)
		hFile << itSilence->first << L":" << itSilence->second << L",";
	hFile << L";";

	hFile << newl;
}


/*====================
  CChannel::PrintInfo
  ====================*/
void	CChannel::PrintInfo() const
{
	m_pConsole->Admin()
		<< L"Channel #" << m_uiChannelID << newl
		<< L"---------" << newl
		<< L"Name:    " << UTF8ToWString(m_sNameUTF8) << newl;

	m_pConsole->Admin() << L"Flags:   ";
	if (m_uiFlags & CHANNEL_FLAG_PERMANENT) m_pConsole->Admin() << L"[PERMANENT] ";
	if (m_uiFlags & CHANNEL_FLAG_SERVER) m_pConsole->Admin() << L"[MATCH] ";
	if (m_uiFlags & CHANNEL_FLAG_HIDDEN) m_pConsole->Admin() << L"[HIDDEN] ";
	if (m_uiFlags & CHANNEL_FLAG_RESERVED) m_pConsole->Admin() << L"[RESERVED] ";
	if (m_uiFlags & CHANNEL_FLAG_GENERAL_USE) m_pConsole->Admin() << L"[GENERAL] ";
	if (m_uiFlags & CHANNEL_FLAG_UNJOINABLE) m_pConsole->Admin() << L"[NO JOIN] ";
	if (m_uiFlags & CHANNEL_FLAG_AUTH_REQUIRED) m_pConsole->Admin() << L"[AUTH] ";
	if (m_uiFlags & CHANNEL_FLAG_CLAN) m_pConsole->Admin() << L"[CLAN] ";
	m_pConsole->Admin() << newl;

	if (m_pClients->empty())
		return;

	m_pConsole->Admin()
		<< newl
		<< L"Clients" << newl
		<< L"---------" << newl;
	for (ClientSet_cit itClient(m_pClients->begin()), itEnd(m_pClients->end()); itClient != itEnd; ++itClient)
	{
		CClient *pClient(*itClient);
		if (pClient == NULL)
			continue;

		m_pConsole->Admin() << L"[" << pClient->GetClientID() << L"] " << pClient->GetName() << newl;
	}
}


/*====================
  CChannel::Purge
  ====================*/
void	CChannel::Purge()
{
	RemoveFlags(CHANNEL_FLAG_AUTH_REQUIRED);

	m_mapAdmins.clear();
	m_setBanned.clear();
	m_setAuthed.clear();
	m_mapSilenced.clear();
	m_sPassword.clear();
	BroadcastChannelUpdate();
}

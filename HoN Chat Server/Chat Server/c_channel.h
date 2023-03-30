// (C)2009 S2 Games
// c_channel.h
//
//=============================================================================
#ifndef __C_CHANNEL_H__
#define __C_CHANNEL_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_core.h"

#include "c_channelmanager.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CClient;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
const uint CHANNEL_FLAG_PERMANENT		(BIT(0));
const uint CHANNEL_FLAG_SERVER			(BIT(1));
const uint CHANNEL_FLAG_HIDDEN			(BIT(2));
const uint CHANNEL_FLAG_RESERVED		(BIT(3));
const uint CHANNEL_FLAG_GENERAL_USE		(BIT(4));
const uint CHANNEL_FLAG_UNJOINABLE		(BIT(5));
const uint CHANNEL_FLAG_AUTH_REQUIRED	(BIT(6));
const uint CHANNEL_FLAG_CLAN			(BIT(7));
// Flags beyond this point are untransmitted until the client gets updated!

enum EAdminLevel
{
	CHAT_CLIENT_ADMIN_NONE,
	CHAT_CLIENT_ADMIN_OFFICER,
	CHAT_CLIENT_ADMIN_LEADER,
	CHAT_CLIENT_ADMIN_ADMINISTRATOR,
	CHAT_CLIENT_ADMIN_STAFF,

	NUM_ADMIN_LEVELS
};

enum EChannelStatus
{
	CHANNEL_ACTIVE,	// Keep in process list
	CHANNEL_IDLE,	// Remove from process list
	CHANNEL_DEAD,	// Delete

	NUM_CHANNEL_STATUSES
};

const uint MAX_USERS_PER_CHANNEL(50);

const uint INVALID_CHANNEL(-1);
//=============================================================================

//=============================================================================
// CChannel
//=============================================================================
class CChannel
{
private:
	DECLARE_STL_SET_TYPES(Client, CClient*)

	DECLARE_CORE_API;

	typedef	map<uint, EAdminLevel>		AdminMap;
	typedef pair<uint, EAdminLevel>		AdminPair;
	typedef	AdminMap::iterator			AdminMap_it;
	typedef	AdminMap::const_iterator	AdminMap_cit;

	CHeap*			m_pHeap;

	EChannelStatus	m_eStatus;

	CBufferDynamic	m_bufferSend;

	uint			m_uiChannelID;
	uint			m_uiClanID;
	uint			m_uiGeneralIndex;
	uint			m_uiMatchID;
	uint			m_uiPublicID;

	uint			m_uiFlags;
	
	wstring			m_sNameLower;
	string			m_sNameUTF8;
	string			m_sTopicUTF8;

	ClientSet*		m_pClients;
	AdminMap		m_mapAdmins;
	uiset			m_setBanned;
	set<wstring>	m_setAuthed;
	map<uint, uint>	m_mapSilenced;
	wstring			m_sPassword;

public:
	~CChannel();
	CChannel(CCore *pCore, CHeap *pHeap);

	void			SetChannelID(uint uiID)					{ m_uiChannelID = uiID; }
	uint			GetChannelID() const					{ return m_uiChannelID; }

	void			SetClanID(uint uiID)					{ m_uiClanID = uiID; }
	uint			GetClanID() const						{ return m_uiClanID; }

	void			SetGeneralIndex(uint uiIndex)			{ m_uiGeneralIndex = uiIndex; }
	uint			GetGeneralIndex() const					{ return m_uiGeneralIndex; }

	void			SetMatchID(uint uiID)					{ m_uiMatchID = uiID; }
	uint			GetMatchID() const						{ return m_uiMatchID; }

	void			SetPublicID(uint uiID)					{ m_uiPublicID = uiID; }
	uint			GetPublicID() const						{ return m_uiPublicID; }

	void			SetName(const wstring &sName)			{ m_sNameLower = LowerString(sName); m_sNameUTF8 = WStringToUTF8(sName); }
	const wstring&	GetNameLower() const					{ return m_sNameLower; }
	const string&	GetNameUTF8() const						{ return m_sNameUTF8; }

	void			SetTopic(const wstring &sTopic);
	const string&	GetTopicUTF8() const					{ return m_sTopicUTF8; }

	void			SetPassword(CClient *pClient, const wstring &sPassword);
	const wstring&	GetPassword() const						{ return m_sPassword; }
	inline bool		HasPassword() const						{ return !m_sPassword.empty(); }

	inline void		SetFlags(uint uiFlags)					{ m_uiFlags = uiFlags; }
	inline void		AddFlags(uint uiFlags)					{ m_uiFlags |= uiFlags; }
	inline bool		HasAllFlags(uint uiFlags) const			{ return (m_uiFlags & uiFlags) == uiFlags; }
	inline bool		HasAnyFlags(uint uiFlags) const			{ return (m_uiFlags & uiFlags) != 0; }
	inline uint		GetFlags() const						{ return m_uiFlags; }
	inline void		RemoveFlags(uint uiFlags)				{ m_uiFlags &= ~uiFlags; }
	inline void		ClearFlags()							{ m_uiFlags = 0; }

	bool			IsFree() const							{ return m_pClients->empty() && m_mapAdmins.empty() && !HasAnyFlags(CHANNEL_FLAG_RESERVED); }

	ushort			GetClientCount() const					{ return ushort(m_pClients->size()); }

	void			AddClientsToSet(ClientSet &setClients)	{ setClients.insert(m_pClients->begin(), m_pClients->end()); }

	bool			HasClient(CClient *pClient) const		{ return (pClient == NULL) ? false : m_pClients->find(pClient) != m_pClients->end(); }
	bool			IsEmpty() const							{ return m_pClients->empty(); }
	
	bool			CanJoin(CClient *pClient, bool bForce, const wstring &sPassword);
	bool			CanSee(CClient *pClient);

	EAdminLevel		GetAdminLevel(CClient *pClient);
	inline bool		IsAdmin(CClient *pClient, EAdminLevel eMinLevel = CHAT_CLIENT_ADMIN_OFFICER)	{ return GetAdminLevel(pClient) >= eMinLevel; }
	bool			IsBanned(CClient *pClient);
	bool			IsSilenced(CClient *pClient);

	inline void		SetActive()								{ if (m_eStatus != CHANNEL_ACTIVE) m_pChannelManager->AddChannelToProcessList(this); m_eStatus = CHANNEL_ACTIVE; }
	EChannelStatus	Frame();

	void			Broadcast(IBuffer &buffer, CClient *pIgnoreClient = NULL);

	void			BroadcastChannelUpdate();

	bool			AddClient(CClient *pClient, bool bForce = false, const wstring &sPassword = WSNULL);
	void			RemoveClient(CClient *pClient);

	void			EnableAuth(CClient *pClient);
	void			DisableAuth(CClient *pClient);
	void			SendAuthList(CClient *pClient);
	void			AddAuth(CClient *pClient, const wstring &sName);
	void			RemoveAuth(CClient *pClient, const wstring &sName);

	void			Demote(CClient *pClient, CClient *pTarget);
	void			Promote(CClient *pClient, CClient *pTarget);

	void			Silence(CClient *pClient, CClient *pTarget, uint uiDuration);

	void			Kick(CClient *pClient, CClient *pTarget);
	void			Ban(CClient *pClient, CClient *pTarget);
	void			LiftBan(CClient *pClient, CClient *pTarget);

	bool			GetInfoForPeer(CPacket &pkt);
	void			ReadUpdate(CPacket &pkt);

	void			SaveState(CFileHandle &hFile) const;
	void			PrintInfo() const;
	void			Purge();
};
//=============================================================================

#endif //__C_CHANNEL_H__

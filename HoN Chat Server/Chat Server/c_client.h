// (C)2009 S2 Games
// c_client.h
//
//=============================================================================
#ifndef __C_CLIENT_H__
#define __C_CLIENT_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_core.h"

#include "c_socket.h"
#include "c_clan.h"
#include "c_httpmanager.h"
#include "c_channelmanager.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CClientManager;
class CClient;
class CMatch;
class CChannel;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
const uint USER_COUNT_UPDATE_DELAY(30000);

enum EClientStatus
{
	CLIENT_STATUS_DISCONNECTED,
	_DEPRECATED_CLIENT_STATUS_CONNECT_REQUESTED,
	_DEPRECATED_CLIENT_STATUS_WAITING_FOR_AUTH,
	CLIENT_STATUS_CONNECTED,
	CLIENT_STATUS_JOINING_GAME,
	CLIENT_STATUS_IN_GAME,

	NUM_CLIENT_STATUSES
};

const int CLIENT_IS_OFFICER			(BIT(0));
const int CLIENT_IS_CLAN_LEADER		(BIT(1));
const int CLIENT_LOOKING_FOR_CLAN	(BIT(2));


const int CLIENT_IS_STAFF			(BIT(5));
const int CLIENT_IS_PREMIUM			(BIT(6));
const int CLIENT_VIRTUAL			(BIT(7));

enum EClientHTTPRequest
{
	CLIENT_REQUEST_INVALID,

	CLIENT_REQUEST_NOTIFICATION,
	CLIENT_REQUEST_CLAN_ADD,
	CLIENT_REQUEST_CLAN_CREATE,
	CLIENT_REQUEST_USER_INFO_IN_GAME,
	CLIENT_REQUEST_UPDATE_RATING,
	CLIENT_REQUEST_JOIN_MATCHMAKING_POOL,
	CLIENT_REQUEST_BUDDY_ADD,
	CLIENT_REQUEST_BUDDY_APPROVE,
	CLIENT_REQUEST_BUDDY_REMOVE,
};

enum EChatModeType
{
	CHAT_MODE_AVAILABLE,
	CHAT_MODE_AFK,
	CHAT_MODE_DND,
	CHAT_MODE_INVISIBLE,
};

struct SClanCreate
{
	uint		uiRecvTime;
	bool		bAccepted[4];
	uint		uiTarget[4];
	uint		uiFounderAccountID;
	wstring		sClan;
	wstring		sTag;
};

enum EAccountType
{
	ACCOUNT_DISABLED,
	ACCOUNT_DEMO,
	ACCOUNT_SERVER,
	ACCOUNT_REGULAR,
	ACCOUNT_PREMIUM,
	ACCOUNT_STAFF,
	ACCOUNT_GM,

	NUM_ACCOUNT_TYPES
};
//=============================================================================

//=============================================================================
// CClient
//=============================================================================
class CClient
{
private:
	DECLARE_STL_SET_TYPES(Client, CClient*)
	DECLARE_STL_MULTIMAP_TYPES(Match, int, CClient*)
	typedef pair<MatchMultiMap*, MatchMultiMap_it>	MatchSeat;
	DECLARE_STL_VECTOR_TYPES(MatchSeat, MatchSeat)

	DECLARE_CORE_API;

	CHeap*			m_pHeap;

	uint			m_uiSessionStart;

	EClientStatus	m_eStatus;

	CSocket*		m_pSocket;
	uint			m_uiLastRecvTime;
	bool			m_bSentPing;
	uint			m_uiRequestCount;
	uint			m_uiLastUserCountUpdate;
	uint			m_uiChannelListIndex;
	bool			m_bChannelListSub;
	bool			m_bChannelListWait;
	byte			m_yChannelListSequence;
	wstring			m_sChannelListHead;
	CBufferDynamic	m_bufferSend;

	EAccountType	m_eAccountType;
	byte			m_yFlags;
	bool			m_bIsPaidAccount;

	uint			m_uiPeerID;
	uint			m_uiAccountID;
	string			m_sCookie;
	string			m_sToken;
	uint			m_uiClientID;
	CClan*			m_pClan;

	uint			m_uiChatModeType;
	wstring			m_sChatModeTypeReason;	

	float			m_aRating[NUM_MATCH_POOLS];
	uint			m_aMatchCount[NUM_MATCH_POOLS];
	bool			m_aInitialRatingSet[NUM_MATCH_POOLS];
	int				m_iMatches;
	int				m_iDisconnects;

	wstring			m_sName;		// Name as it appears in account info
	wstring			m_sNameLower;	// Name in all lower case
	string			m_sNameUTF8;	// Name including clan tag prefix in UTF8 format for network transmission

	uint			m_uiMatchID;
	string			m_sServerAddressPort;
	wstring			m_sGameName;
	string			m_sGameNameUTF8;

	uiset			m_setChannels;
	uiset			m_setBuddies;

	uint			m_uiRegionFlags;
	CMatch*			m_pPendingMatch;

	HTTPRequestList	m_lHTTPRequests;
	CClient();

	bool			HandleInviteToServer(CClient *pTarget);
	CHTTPRequest*	SpawnMatchMakerInfoRequest();

public:
	~CClient();
	CClient(CCore *pCore, CHeap *pHeap, CSocket *pSocket);

	inline void				SetSessionStart(uint uiSeconds)			{ m_uiSessionStart = uiSeconds; }
	inline uint				GetSessionStart() const					{ return m_uiSessionStart; }

	inline void				SetStatus(EClientStatus eStatus)		{ m_eStatus = eStatus; }
	inline EClientStatus	GetStatus() const						{ return m_eStatus; }
	inline byte				GetStatusByte() const					{ return byte(m_eStatus); }

	inline void				AddBuddy(uint uiAccountID)				{ m_setBuddies.insert(uiAccountID); }
	
	inline void				SetCookie(const string &sCookie)		{ m_sCookie = sCookie; }
	inline const string&	GetCookie() const						{ return m_sCookie; }
	inline void				SetToken(const string &sToken)			{ m_sToken = sToken; }
	inline const string&	GetToken() const						{ return m_sToken; }
	inline void				SetClientID(uint uiID)					{ m_uiClientID = uiID; }
	inline uint				GetClientID() const						{ return m_uiClientID; }

	inline void				SetAccountType(EAccountType eType)		{ m_eAccountType = eType; }
	inline EAccountType		GetAccountType() const					{ return m_eAccountType; }

	inline void				SetAccountID(uint uiID)					{ m_uiAccountID = uiID; }
	inline uint				GetAccountID() const					{ return m_uiAccountID; }

	inline void				ClearFlags()							{ m_yFlags = 0; }
	inline void				SetFlags(byte yFlags)					{ m_yFlags = yFlags; }
	inline void				AddFlags(byte yFlags)					{ m_yFlags |= yFlags; }
	inline void				RemoveFlags(byte yFlags)				{ m_yFlags &= ~yFlags; }
	inline bool				HasAllFlags(byte yFlags) const			{ return (m_yFlags & yFlags) == yFlags; }
	inline bool				HasAnyFlags(byte yFlags) const			{ return (m_yFlags & yFlags) != 0; }
	inline byte				GetFlags() const						{ return m_yFlags; }

	inline void				SetPaidAccount(bool b)					{ m_bIsPaidAccount = b; }

	inline float			GetRating(EMatchPool ePool) const			{ return m_aRating[ePool]; }
	inline void				SetRating(EMatchPool ePool, float fRating)	{ m_aRating[ePool] = fRating; }

	inline uint				GetMatchCount(EMatchPool ePool) const			{ return m_aMatchCount[ePool]; }
	inline void				SetMatchCount(EMatchPool ePool, uint uiCount)	{ m_aMatchCount[ePool] = uiCount; }

	inline void				SetTotalMatches(int iCount)				{ m_iMatches = iCount; }

	inline void				SetDisconnectCount(int iCount)			{ m_iDisconnects = iCount; }
	inline int				GetDisconnectCount() const				{ return m_iDisconnects; }

	inline uint				GetChannelListIndex() const				{ return m_uiChannelListIndex; }
	inline void				SetChannelListIndex(uint uiIndex)		{ m_uiChannelListIndex = uiIndex; }

	bool					IsLeaver() const;

	inline byte				GetChannelListSequence() const			{ return m_yChannelListSequence; }
	inline const wstring&	GetChannelListHead() const				{ return m_sChannelListHead; }

	// Name
	void					SetName(const wstring &sName);
	inline const wstring&	GetName() const							{ return m_sName; }
	inline const string&	GetNameUTF8() const						{ return m_sNameUTF8; }
	inline const wstring&	GetNameLower() const					{ return m_sNameLower; }

	void					PrintInfo() const;

	// Network
	inline void				ResetRequestCounter()					{ m_uiRequestCount = 0; }
	bool					IncrementRequestCounter();
	inline void				DecrementRequestCounter()				{ if (m_uiRequestCount > 0) --m_uiRequestCount; }
	inline bool				IsConnected() const						{ return m_pSocket == NULL ? false : m_pSocket->IsConnected(); }
	void					Disconnect();
	
	void					VerifyNotification(uint uiKey);
	void					ProcessNotification(const wstring &sResult);
	void					ProcessHTTPRequests();
	bool					Frame();
	bool					CheckTimeout();
	inline void				KeepAlive()								{ m_uiLastRecvTime = m_pCore->GetSeconds(); }
	bool					ConnectingFrame();
	void					Send(const IBuffer &buffer);

	void					SendPing();
	void					SendNotification(const wstring &sMessage, byte yType);
	void					SendUserCount(uint uiUserCount);

	inline void				NotifyClan(byte yType, const wstring &sMessage)	{ if (m_pClan != NULL) m_pClan->Notify(yType, sMessage); }

	// Clan
	void					SetClan(CClan *pClan);
	inline CClan*			GetClan() const							{ return m_pClan; }
	inline bool				HasClan() const							{ return m_pClan != NULL; }
	inline uint				GetClanID() const						{ return m_pClan == NULL ? INVALID_CLAN_ID : m_pClan->GetClanID(); }

	inline const wstring&	GetClanName() const						{ return m_pClan == NULL ? WSNULL : m_pClan->GetName(); }
	inline const string&	GetClanNameUTF8() const					{ return m_pClan == NULL ? SNULL : m_pClan->GetNameUTF8(); }
	
	uint					GetChatModeType()						{ return m_uiChatModeType; }
	wstring					GetChatModeTypeReason()					{ return m_sChatModeTypeReason; }
	void					SetChatModeType(uint uiChatModeType)	{ m_uiChatModeType = uiChatModeType; }

	// Current game
	inline void				ClearCurrentMatch()						{ m_uiMatchID = -1; m_sServerAddressPort.clear(); m_sGameName.clear(); }
	inline void				SetMatchID(uint uiMatchID)				{ m_uiMatchID = uiMatchID; }
	inline uint				GetMatchID() const						{ return m_uiMatchID; }
	inline void				SetServerAddressPort(const wstring& sAddressPort)	{ m_sServerAddressPort = WStringToUTF8(sAddressPort); }
	inline const string&	GetServerAddressUTF8() const			{ return m_sServerAddressPort; }
	inline void				SetServerName(const wstring& sGameName)	{ m_sGameName = sGameName; m_sGameNameUTF8 = WStringToUTF8(sGameName); }
	inline const wstring&	GetServerName() const					{ return m_sGameName; }
	inline const string&	GetServerNameUTF8() const				{ return m_sGameNameUTF8; }

	void					SetOnline();
	void					SetOffline();

	void					InvalidateAllRequests()					{ m_lHTTPRequests.clear(); }

	// Match making
	void					JoinMatch(CMatch *pMatch);
	inline void				LeaveMatch()							{ m_pPendingMatch = NULL; }
	inline void				LeaveMatch(CMatch *pMatch)				{ if (m_pPendingMatch == pMatch) m_pPendingMatch = NULL; }
	void					SendMatchMakerUpdate(byte yStatus, uint uiParamA = 0, uint uiParamB = 0);

	// Channels
	inline void				AddChannel(uint uiChannelID)			{ m_setChannels.insert(uiChannelID); }
	inline void				RemoveChannel(uint uiChannelID)			{ m_setChannels.erase(uiChannelID); }
	inline uint				GetNumChannels() const					{ return INT_SIZE(m_setChannels.size()); }
	CChannel*				GetGeneralChannel() const;
	void					LeaveAllChannels(uint uiFlags = 0);
	void					AddChannelsToSet(uiset &setChannels, CClient *pViewer = NULL) const;
	void					AddChannelMatesToSet(ClientSet &setPeers) const;

	void					UpdateStatus();
	void					SendStatusUpdates();

	void					SendToPeers(const IBuffer &buffer);

	bool					UserCountUpdateNeeded();

	bool					HandleChannelMessage(CPacket &pkt);
	bool					HandleWhisper(CPacket &pkt);
	bool					HandleIM(CPacket &pkt);
	bool					HandleRequestBuddyAdd(CPacket &pkt);
	bool					HandleRequestBuddyAddApprove(CPacket &pkt);
	bool					HandleBuddyRemoveNotification(CPacket &pkt);
	bool					HandleJoiningGame(CPacket &pkt);
	bool					HandleJoinedGame(CPacket &pkt);
	bool					HandleLeftGame();
	bool					HandleClanWhisper(CPacket &pkt);
	bool					HandleClanPromoteNotification(CPacket &pkt);
	bool					HandleClanDemoteNotification(CPacket &pkt);
	bool					HandleClanRemoveNotification(CPacket &pkt);
	bool					HandleLookingForClan();
	bool					HandleNotLookingForClan();
	bool					HandleJoinChannel(CPacket &pkt);
	bool					HandleWhisperBuddies(CPacket &pkt);
	bool					HandleLeaveChannel(CPacket &pkt);
	bool					HandleInviteIDToServer(CPacket &pkt);
	bool					HandleInviteNameToServer(CPacket &pkt);
	bool					HandleInviteRejected(CPacket &pkt);
	bool					HandleUserInfo(CPacket &pkt);
	bool					HandleGetUserStatus(CPacket &pkt);
	bool					HandleChannelTopic(CPacket &pkt);
	bool					HandleChannelKick(CPacket &pkt);
	bool					HandleChannelBan(CPacket &pkt);
	bool					HandleChannelUnban(CPacket &pkt);
	bool					HandleChannelSilenceUser(CPacket &pkt);
	bool					HandleChannelPromote(CPacket &pkt);
	bool					HandleChannelDemote(CPacket &pkt);
	bool					HandleMessageAll(CPacket &pkt);
	bool					HandleChannelSetAuth(CPacket &pkt);
	bool					HandleChannelRemoveAuth(CPacket &pkt);
	bool					HandleChannelAddAuthUser(CPacket &pkt);
	bool					HandleChannelRemoveAuthUser(CPacket &pkt);
	bool					HandleChannelListAuth(CPacket &pkt);
	bool					HandleChannelSetPassword(CPacket &pkt);
	bool					HandleChannelJoinPassword(CPacket &pkt);
	bool					HandleRequestAutoMatch(CPacket &pktRecv);
	bool					HandleCancelAutoMatch(CPacket &pktRecv);
	bool					HandleChatRoll(CPacket &pktRecv);
	bool					HandleChatEmote(CPacket &pktRecv);
	bool					HandleSetChatModeType(CPacket &pktRecv);
	bool					HandleRequestMatchmakingInfo(CPacket &pktRecv);
	bool					HandleChannelSublist(CPacket &pkt);
	bool					HandleChannelSublistAck(CPacket &pkt);
	bool					ProcessRequestAdminKick(CPacket &pkt);

	void					ReceiveWhisper(const string &sSenderNameUTF8, const string &sMessageUTF8);

	void					SendClanInvite(CClient *pInviter);
	void					ProcessClanInviteResponse(const wstring &sResponse);
	void					CreateClan(SClanCreate &clan);
	void					ProcessCreateClanResponse(const wstring &sResponse);
	void					ProcessUserInfoInGameResponse(const wstring &sResponse);
	void					ProcessUpdateRatingResponse(const wstring &sResponse, bool bJoinMatchMaker);
	void					ProcessRequestBuddyAddResponse(const wstring &sResponse);
	void					ProcessRequestBuddyApproveResponse(const wstring &sResponse);
};
//=============================================================================

#endif //__C_CLIENT_H__

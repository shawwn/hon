// (C)2008 S2 Games
// c_clientmanager.h
//
//=============================================================================
#ifndef __C_CLIENTMANAGER_H__
#define __C_CLIENTMANAGER_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_core.h"

#include "c_client.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CSocket;
class CPacket;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
const uint NUM_REQUIRED_CLAN_MEMBERS(4);

struct SClanInvite
{
	uint		uiRecvTime;
	uint		uiTargetID;
	uint		uiOriginID;
	uint		uiClanID;
	wstring		sClan;
	wstring		sTag;
	bool		bCreateClan;
};

enum EClanRank
{
	CLAN_RANK_NONE,
	CLAN_RANK_MEMBER,
	CLAN_RANK_OFFICER,
	CLAN_RANK_LEADER
};

struct SPlayerCount
{
	uint uiPaid;
	uint uiOther;
};
//=============================================================================

//=============================================================================
// CClientManager
//=============================================================================
class CClientManager
{
private:
	DECLARE_STL_MAP_TYPES(AccountClient, uint, uint);
	DECLARE_STL_MAP_TYPES(NameClient, wstring, uint);
	DECLARE_STL_VECTOR_TYPES(Client, CClient*);
	DECLARE_STL_MAP_TYPES(ClanInvite, uint, SClanInvite);
	DECLARE_STL_MAP_TYPES(ClanCreate, uint, SClanCreate);
	DECLARE_STL_MAP_TYPES(PlayerCount, uint, SPlayerCount);
	DECLARE_STL_LIST_TYPES(HTTPRequest, CHTTPRequest*)

	DECLARE_CORE_API;

	CHeap*				m_pHeap;
	CHeap*				m_pSTLHeap;

	CHTTPRequest*		m_pHeartbeat;

	ClientVector*		m_pActiveClients;
	uivector			m_vAvailableActiveIDs;

	AccountClientMap*	m_pAccountToClientID;
	NameClientMap*		m_pNameToClientID;

	uint				m_uiNextMessageTime;
	uint				m_uiNextHeartbeat;

	uint				m_uiTotalUniqueClients;
	uint				m_uiTotalOnline;
	uint				m_uiTotalInGame;

	LONGLONG			m_llTotalSessions;
	LONGLONG			m_llTotalSessionLength;
	LONGLONG			m_llTotalLongSessions;
	LONGLONG			m_llTotalLongSessionLength;

	uint				m_uiVirtualClientCount;

	ClanInviteMap*		m_pClanInvites;
	ClanCreateMap*		m_pClanCreates;

	PlayerCountMap*		m_pPlayerCounts;
	HTTPRequestList*	m_pHTTPRequests;

	void	SendHeartbeat();

public:
	~CClientManager();
	CClientManager();

	inline void		IncrementInGameCount()							{ ++m_uiTotalInGame; }
	inline void		DecrementInGameCount()							{ --m_uiTotalInGame; }

	inline void		IncrementOnlineCount()							{ ++m_uiTotalOnline; }
	inline void		DecrementOnlineCount()							{ --m_uiTotalOnline; }
	
	inline CHeap*	GetSTLHeap() const								{ return m_pSTLHeap; }

	void			Initialize(CCore *pCore);
	void			Frame();

	CClient*		AddClient(uint uiAccountID, const wstring &sName, CSocket *pSocket);
	void			RemoveClient(CClient *pClient);

	bool			ProcessData(CClient *pClient);

	inline CClient*	GetActiveClient(uint uiClientID)				{ if (uiClientID >= m_pActiveClients->size()) return false; return m_pActiveClients->at(uiClientID); }
	CClient*		GetClientFromAccountID(uint uiAccountID);
	CClient*		GetClientFromName(const wstring &sName);

	void			AddClanMemberFailed(CClient *pClient);
	void			AddClanMemberSuccess(CClient *pClient);

	void			CreateClanSuccess(uint uiAccountID, uint uiClanID);
	void			CreateClanFailed(uint uiAccountID);
	void			CreateClanFailedClanName(uint uiAccountID);
	void			CreateClanFailedTag(uint uiAccountID);
	
	void			RequestBuddyAddSuccess(const uint uiAccountID, const wstring sAccountNickName, const uint uiAccountNotifyID, const wstring sAccountNotification, const uint uiBuddyAccountID, const wstring sBuddyNickName, const uint uiBuddyNotifyID, const wstring sBuddyNotification);
	void			RequestBuddyAddFail(const uint uiType, const uint uiAccountID, const uint uiAccountNotifyID, const wstring sBuddyNickName);
	void			RequestBuddyApproveSuccess(const uint uiAccountID, const wstring sAccountNickName, const uint uiAccountNotifyID, const wstring sAccountNotification, const uint uiBuddyAccountID, const wstring sBuddyNickName, const uint uiBuddyNotifyID, const wstring sBuddyNotification);
	void			RequestBuddyApproveFail(const uint uiAccountID, const wstring sAccountNickName, const uint uiAccountNotifyID, const uint uiBuddyAccountID, const wstring sBuddyNickName, const uint uiBuddyNotifyID);

	void			Broadcast(const IBuffer &buffer, byte yFlags = 0);

	CClient*		AddVirtualClient();

	void			SpawnNewHeartbeatRequest();
	void			InvalidateAllClientRequests();

	void			SendConnectionCountsUpdate(uint uiAddress, SPlayerCount &count);
	void			IncrementConnectionCounts(uint uiAddr, bool bIsPaid);
	void			DecrementConnectionCounts(uint uiAddr, bool bIsPaid);

	void			PrintUsage();
	void			PrintPlayerCounts();
	void			PrintPlayerCountTotals();

	// Recieved data related functions
	bool			HandleClanInvite(CClient *pClient, CPacket &pkt);
	bool			HandleClanInviteRejected(CClient *pClient, CPacket &pkt);
	bool			HandleClanInviteAccepted(CClient *pClient, CPacket &pkt);
	bool			HandleCreateClan(CClient *pClient, CPacket &pkt);
};
//=============================================================================
#endif	//__C_CLIENTMANAGER_H__
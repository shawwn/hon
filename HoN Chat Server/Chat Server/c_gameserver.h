// (C)2010 S2 Games
// c_gameserver.h
//
//=============================================================================
#ifndef __C_GAMESERVER_H__
#define __C_GAMESERVER_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_packet.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CSocket;
class CMatch;
//=============================================================================

//=============================================================================
// CGameServer
//=============================================================================
class CGameServer
{
private:
	DECLARE_STL_LIST_TYPES(Server, CGameServer*);
	DECLARE_STL_MAP_TYPES(LocationList, wstring, ServerList*);

	DECLARE_CORE_API;

	uint					m_uiID;

	CSocket*				m_pSocket;
	CPacket					m_pktSend;
	uint					m_uiLastReceiveTime;
	bool					m_bSentPing;

	uint					m_uiServerID;
	wstring					m_sLocation;
	wstring					m_sName;
	string					m_sAddress;
	ushort					m_unPort;
	byte					m_yStatus;
	byte					m_yArrangedType;
	byte					m_yOfficial;
	byte					m_yNoLeaver;
	byte					m_yPrivate;	
	byte					m_yTier;
	wstring					m_sMapName;
	wstring					m_sGameName;
	wstring					m_sGameModeName;	
	byte					m_yTeamSize;
	byte					m_yEasyMode;
	byte					m_yAllHeroes;
	byte					m_yForceRandom;
	byte					m_yAutoBalanced;
	byte					m_yAdvancedOptions;
	ushort					m_unMinPSR;
	ushort					m_unMaxPSR;
	byte					m_yDevHeroes;
	byte					m_yHardcore;


	CMatch*					m_pMatch;

	bool					m_bHasLocationIterator;
	LocationListMap_it		m_itLocationList;
	ServerList_it			m_itLocation;

	CGameServer();

	bool	ProcessStatusUpdate(CPacket &pkt);
	bool	ProcessAnnounceMatch(CPacket &pkt);
	bool	ProcessRemindPlayer(CPacket &pkt);
	bool	ProcessReplacePlayer(CPacket &pkt);

public:
	~CGameServer();
	CGameServer(CCore *pCore, CSocket *pSocket, uint uiID);

	inline CMatch*				GetMatch() const						{ return m_pMatch; }

	inline uint					GetLastReceiveTime() const				{ return m_uiLastReceiveTime; }

	inline uint					GetID() const							{ return m_uiID; }
	inline uint					GetServerID() const						{ return m_uiServerID; }
	inline const wstring&		GetLocation() const						{ return m_sLocation; }
	inline string&				GetAddress()							{ return m_sAddress; }
	inline ushort				GetPort() const							{ return m_unPort; }
	inline const wstring&		GetName() const							{ return m_sName; }
	inline byte					GetStatus() const						{ return m_yStatus; }
	inline byte					GetArrangedType() const					{ return m_yArrangedType; }
	inline bool					HasMatch() const						{ return m_pMatch != NULL; }
	inline const wstring&		GetGameName() const						{ return m_sGameName; }
	
	
	inline byte					GetOfficial() const						{ return m_yOfficial; }
	inline byte					GetNoLeaver() const						{ return m_yNoLeaver; }
	inline byte					GetPrivate() const						{ return m_yPrivate; }
	inline byte					GetTier() const							{ return m_yTier; }
	inline const wstring&		GetMapName() const						{ return m_sMapName; }
	inline const wstring&		GetGameModeName() const					{ return m_sGameModeName; }
	inline byte					GetTeamSize() const						{ return m_yTeamSize; }
	inline byte					GetEasyMode() const						{ return m_yEasyMode; }
	inline byte					GetAllHeroes() const					{ return m_yAllHeroes; }
	inline byte					GetForceRandom() const					{ return m_yForceRandom; }
	inline byte					GetAutoBalanced() const					{ return m_yAutoBalanced; }
	inline byte					GetAdvancedOptions() const				{ return m_yAdvancedOptions; }
	inline ushort				GetMinPSR() const						{ return m_unMinPSR; }
	inline ushort				GetMaxPSR() const						{ return m_unMaxPSR; }
	inline byte					GetDevHeroes() const					{ return m_yDevHeroes; }
	inline byte					GetHardcore() const						{ return m_yHardcore; }
	

	inline bool							HasLocationIterator() const		{ return m_bHasLocationIterator; }
	inline const LocationListMap_it&	GetLocationList() const			{ return m_itLocationList; }
	inline const ServerList_it&			GetLocationIterator() const		{ return m_itLocation; }
	inline void							InvalidateLocationIterator()	{ m_bHasLocationIterator = false; }
	
	inline void	StoreLocationIterator(LocationListMap_it &itList, ServerList_it &itEntry)
	{
		m_itLocationList = itList;
		m_itLocation = itEntry;
		m_bHasLocationIterator = true;
	}

	bool	ReadSocket();
	void	Disconnect();
	void	SetOnline();

	void	SendPing();
	void	SendSubstitution(uint uiOldClientID, uint uiNewClientID);
	void	CreateMatch(CMatch *pMatch);
};
//=============================================================================

#endif //__C_GAMESERVER_H__
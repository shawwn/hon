// (C)2008 S2 Games
// c_channelmanager.h
//
//=============================================================================
#ifndef __C_CHANNELMANAGER_H__
#define __C_CHANNELMANAGER_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_core.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CClientManager;
class CClient;
class CClan;
class CChannel;
class CPacket;
//=============================================================================

//=============================================================================
// CChannelManager
//=============================================================================
class CChannelManager
{
private:
	DECLARE_STL_VECTOR_TYPES(Channel, CChannel*);
	DECLARE_STL_MAP_TYPES(ChannelName, wstring, CChannel*);
	DECLARE_STL_MAP_TYPES(ClanChannel, uint, CChannel*);
	DECLARE_STL_MAP_TYPES(MatchChannel, uint, CChannel*);
	DECLARE_STL_LIST_TYPES(Channel, CChannel*);

	DECLARE_CORE_API;

	CHeap*				m_pHeap;
	CHeap*				m_pSTLHeap;

	ChannelVector*		m_pChannels;
	uivector			m_vAvailableIDs;
	ChannelNameMap*		m_pChannelNames;
	
	ChannelVector*		m_pGeneralChannels;
	uivector			m_vAvailableGeneralChannels;
	ClanChannelMap*		m_pClanChannels;
	MatchChannelMap*	m_pMatchChannels;
	ChannelVector*		m_pPublicChannels;
	uivector			m_vAvailablePublicIDs;

	ChannelList*		m_pProcessChannels;

	CChannel*	AllocateChannel();
	void		RemoveChannel(CChannel *pChannel);

public:
	~CChannelManager();
	CChannelManager();

	bool		Initialize(CCore *pCore);
	void		Frame();

	CChannel*	GetDefaultChannel(const CClient *pClient);

	CChannel*	CreatePublicChannel(const wstring &sName, uint uiFlags = 0, bool bForce = false, const wstring &sTopic = WSNULL);
	CChannel*	CreateGeneralChannel();
	CChannel*	CreateClanChannel(CClan *pClan);
	CChannel*	CreateMatchChannel(uint uiMatchID);

	CChannel*	GetGeneralChannel();
	CChannel*	GetClanChannel(uint uiClanID);
	CChannel*	GetMatchChannel(uint uiMatchID);
	CChannel*	GetChannel(uint uiChannelID)				{ if (uiChannelID >= m_pChannels->size()) return NULL; return m_pChannels->at(uiChannelID); }
	CChannel*	GetChannel(const wstring &sChannel);

	void		SendChannelListing(CClient *pClient);
	void		SendChannelSublisting(CClient *pClient);

	inline void	AddChannelToProcessList(CChannel *pChannel)	{ m_pProcessChannels->push_back(pChannel); }

	uint		AddNextChannelInfo(uint uiChannelID, CPacket &pkt);
	void		UpdateChannel(CPacket &pkt);

	void		LoadState(CFileHandle &hLoadFile);
	void		SaveState(CFileHandle &hSaveFile) const;

	void		PrintUsage() const;
};
//=============================================================================
#endif	//__C_CHANNELMANAGER_H__
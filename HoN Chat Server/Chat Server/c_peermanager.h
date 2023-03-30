// (C)2009 S2 Games
// c_peermanager.h
//
//=============================================================================
#ifndef __C_PEERMANAGER_H__
#define __C_PEERMANAGER_H__

//=============================================================================
// Includes
//=============================================================================
#include "c_core.h"

#include "c_peer.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CSocket;
class IUpdate;
//=============================================================================

//=============================================================================
// CPeerManager
//=============================================================================
class CPeerManager
{
private:
	DECLARE_STL_MAP_TYPES(Peer, uint, CPeer*);
	DECLARE_STL_LIST_TYPES(Peer, CPeer*);

	DECLARE_CORE_API;

	CHeap*			m_pHeap;
	CHeap*			m_pSTLHeap;

	string			m_sKey;
	uint			m_uiID;
	uint			m_uiNextID;

	PeerList*		m_pPendingPeers;
	PeerMap*		m_pPeers;

public:
	~CPeerManager();
	CPeerManager();

	inline uint				GetID() const		{ return m_uiID; }
	inline const string&	GetKey() const		{ return m_sKey; }

	bool		Initialize(CCore *pCore);
	void		Frame();

	void		AddPeer(CSocket *pSocket, CPacket &pkt);
	void		Connect(const string &sAddress, ushort unPort, const string &sKey);
	void		Connect(CPeer *pPeer);

	void		Disconnect();

	void		Connected(uint uiLocalID, CPeer *pNewPeer);

	void		SendPeerList(CPeer *pTargetPeer);
	void		BroadcastPeerData(CPeer *pPeer);

	CPeer*		GetPeer(uint uiID);
	void		UpdatePeer(CPacket &pkt);

	void		RelayWhisper(const string &sSenderUTF8, const wstring &sTarget, const string &sMessageUTF8);

	void		PrintUsage() const;
};
//=============================================================================

#endif //__C_PEERMANAGER_H__

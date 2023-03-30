// (C)2009 S2 Games
// c_clan.h
//
//=============================================================================
#ifndef __C_CLAN_H__
#define __C_CLAN_H__

//=============================================================================
// Declarations
//=============================================================================
class CClient;
//=============================================================================

//=============================================================================
// CClan
//=============================================================================
class CClan
{
private:
	DECLARE_STL_SET_TYPES(Client, CClient*)

	DECLARE_CORE_API;

	CHeap*					m_pHeap;

	uint					m_uiClanID;

	wstring					m_sName;
	string					m_sNameUTF8;
	wstring					m_sTag;
	string					m_sTagUTF8;


	mutable CBufferDynamic	m_bufferSend;

public:
	~CClan();
	CClan(CCore *pCore, CHeap *pHeap);

	ClientSet*				m_pClients;

	void			SetClanID(uint uiID)					{ m_uiClanID = uiID; }
	uint			GetClanID() const						{ return m_uiClanID; }

	void			SetName(const wstring &sName)			{ m_sName = sName; m_sNameUTF8 = WStringToUTF8(sName); }
	const wstring&	GetName() const							{ return m_sName; }
	const string&	GetNameUTF8() const						{ return m_sNameUTF8; }

	void			SetTag(const wstring &sTag)				{ m_sTag = sTag; m_sTagUTF8 = WStringToUTF8(sTag); }
	const wstring&	GetTag() const							{ return m_sTag; }
	const string&	GetTagUTF8() const						{ return m_sTagUTF8; }

	void			AddClient(CClient *pClient)				{ m_pClients->insert(pClient); }
	void			RemoveClient(CClient *pClient)			{ m_pClients->erase(pClient); }
	void			AddClientsToSet(ClientSet &setClients)	{ setClients.insert(m_pClients->begin(), m_pClients->end()); }
	
	void			Notify(byte yType, const wstring &sMessage) const;
	void			Broadcast(const IBuffer &buffer, CClient *pIgnore = NULL) const;
};
//=============================================================================

#endif //__C_CLAN_H__

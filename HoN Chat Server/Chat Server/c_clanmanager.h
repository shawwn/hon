// (C)2009 S2 Games
// c_clanmanager.h
//
//=============================================================================
#ifndef __C_CLANMANAGER_H__
#define __C_CLANMANAGER_H__

//=============================================================================
// Declarations
//=============================================================================
class CClan;
//=============================================================================

//=============================================================================
// CClanManager
//=============================================================================
class CClanManager
{
private:
	DECLARE_STL_MAP_TYPES(Clan, uint, CClan*);

	DECLARE_CORE_API;

	CHeap*		m_pHeap;
	CHeap*		m_pSTLHeap;

	ClanMap*	m_pClans;

public:
	~CClanManager();
	CClanManager();

	bool	Initialize(CCore *pCore);

	CClan*	GetClan(uint uiClanID)	{ ClanMap_it it(m_pClans->find(uiClanID)); if (it == m_pClans->end()) return NULL; return it->second; }
	CClan*	AddClan(uint uiClanID);
};
//=============================================================================


#endif //__C_CLANMANAGER_H__
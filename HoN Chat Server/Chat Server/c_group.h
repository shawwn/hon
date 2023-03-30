// (C)2009 S2 Games
// c_group.h
//
//=============================================================================
#ifndef __C_GROUP_H__
#define __C_GROUP_H__

//=============================================================================
// Defines
//=============================================================================
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CClient;
//=============================================================================

//=============================================================================
// Cgroup
//=============================================================================
class CGroup
{
private:
	DECLARE_STL_VECTOR_TYPES(Client, CClient*);
	
	DECLARE_CORE_API;

	ClientVector	m_vGroupMembers;
	float			m_fRating;
	uint			m_uiRegionFlags;
	uint			m_uiGroupID;
	int				m_iBracket;

	void			CalculateRating();

public:
	CGroup(CCore *pCore, CClient *pClient, uint uiRegionFlags);
	~CGroup();

	int				GetGroupSize()					{ return INT_SIZE(m_vGroupMembers.size()); }
	float			GetRating()						{ return m_fRating; }
	uint			GetRegions()					{ return m_uiRegionFlags; }
	
	void			SetBracket(int iBracket)		{ m_iBracket = CLAMP(iBracket, 0, 4); }
	int				GetBracket()					{ return m_iBracket; }

	void			SetGroupID(uint uiGroupID)		{ m_uiGroupID = uiGroupID; }
	uint			GetGroupID()					{ return m_uiGroupID; }

	void			NotifyGroupClientDropped(CClient * pClient);
	bool			IsCompatible(CGroup *otherGroup);
	bool			IsOnline();
	bool			HasClient(uint ClientID);
	void			AddClient(CClient *pClient);
	void			RemoveClient(CClient *pClient);
	void			SendStatusChange(byte yStatus, uint uiParamA = 0, uint uiParamB = 0);
	CClient*		GetClient(uint iClientNumber);
	CClient*		GetClient(const wstring &sName);
};
//=============================================================================
#endif	//__C_GROUP_H__
// (C)2009 S2 Games
// c_match.h
//
//=============================================================================
#ifndef __C_MATCH_H__
#define __C_MATCH_H__

//=============================================================================
// Declarations
//=============================================================================
class CGameServer;
class CPacket;

EXTERN_CVAR(uint, matchmaker_openMatchTime);
EXTERN_CVAR(float, matchmaker_spreadBase);
EXTERN_CVAR(float, matchmaker_spreadScale);
EXTERN_CVAR(float, matchmaker_spreadPower);
EXTERN_CVAR(float, matchmaker_spreadMax);
EXTERN_CVAR(bool, matchmaker_allowLeavers);
//=============================================================================

//=============================================================================
// CMatch
//=============================================================================
class CMatch
{
private:
	DECLARE_STL_DEQUE_TYPES(Client, CClient*);
	DECLARE_STL_VECTOR_TYPES(Client, CClient*);
	DECLARE_STL_LIST_TYPES(Match, CMatch*);

	CHeap*			m_pHeap;

	bool			m_bReady;

	bool			m_bArrangedMatch;
	bool			m_bProvisional;
	bool			m_bLeavers;
	
	ClientDeque*	m_pClients;
	ClientVector*	m_apTeams[2];
		
	uint			m_uiMatchSize;

	CMatch();

protected:
	DECLARE_CORE_API;

	uint			m_uiID;
	uint			m_uiChallenge;
	uint			m_uiTimeStamp;

	int				m_iMatchStartTime;
	
	wstring			m_sRegion;
	uint			m_uiRegion;
	uint			m_uiRegionalIndex;

	float			m_fMeanRating;

	static bool	CompareClientRating(CClient *pClientA, CClient *pClientB);
	void		AssignTeams();
	void		CalculateMeanRating();
	float		GetMatchPrediction(int iTeam) const;

public:
	CMatch(CCore *pCore, CHeap *pMatchMakerHeap);
	~CMatch();
	
	void			SetMatchID(uint uiID)					{ m_uiID = uiID; }
	uint			GetID() const							{ return m_uiID; }
	
	bool			GetAllowLeavers() const					{ return m_bLeavers; }
	bool			IsProvisional() const					{ return m_bProvisional; }
	bool			IsStrictProvisional() const				{ return m_bProvisional && ((m_pCore->GetSeconds() - m_uiTimeStamp) < matchmaker_openMatchTime); }

	void			SetRegion(const wstring &sRegion, uint uiRegion)	{ m_sRegion = sRegion; m_uiRegion = uiRegion; }
	const wstring&	GetRegionName() const								{ return m_sRegion; }
	uint			GetRegion() const									{ return m_uiRegion; }
	void			SetRegionalIndex(uint uiID)							{ m_uiRegionalIndex = uiID; }
	uint			GetRegionalIndex() const							{ return m_uiRegionalIndex; }

	uint			GetTimeStamp() const					{ return m_uiTimeStamp; }
	void			SetTimeStamp(uint uiTimeStamp) 			{ m_uiTimeStamp = uiTimeStamp; }
	uint			GetWaitTime() const						{ return m_pCore->GetSeconds() - m_uiTimeStamp; }
	
	float			GetMeanRating() const					{ return m_fMeanRating; }
	float			GetMaxSpread() const					{ return MIN(matchmaker_spreadBase + pow(GetWaitTime() * matchmaker_spreadScale, matchmaker_spreadPower), matchmaker_spreadMax.Get()); }

	void			SetMatchSize(uint uiSize)				{ m_uiMatchSize = uiSize; }
	uint			GetMatchSize() const					{ return m_uiMatchSize; }
	uint			GetPlayerCount() const					{ return INT_SIZE(m_pClients->size()); }

	uint			GetChallenge() const					{ return m_uiChallenge; }
	bool			Validate(uint uiID, uint uiChallenge)	{ return m_uiID == uiID && m_uiChallenge == uiChallenge; }

	bool			CanJoin(CClient *pClient) const;
	bool			IsEmpty() const							{ return !m_bReady && m_pClients->empty(); }
	bool			IsReady() const							{ return m_bReady; }

	void			Start();
	void			AnnounceMatchReady(const string &sAddress, ushort unPort);

	void			GetSubstitute(CMatch *pOtherMatch, uint uiOldAccountID, ClientVector &vSubstitutes);

	void			InvalidateClient(CClient *pClient, const wstring &sReason);
	void			AddClient(CClient *pClient);
	void			WriteMatchInfo(CPacket &pkt);
};
//=============================================================================
#endif	//__C_MATCH_H__
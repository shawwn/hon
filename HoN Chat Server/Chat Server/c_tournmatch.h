// (C)2009 S2 Games
// c_tournmatch.h
//
//=============================================================================
#ifndef __C_TOURNMATCH_H__
#define __C_TOURNMATCH_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_match.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================

typedef vector<uint>					AccountVector;
typedef AccountVector::iterator			AccountVector_it;
typedef AccountVector::const_iterator	AccountVector_cit;

// taken from i_gameentity.h
const uint TEAM_2			(2);
const uint TEAM_1			(1);
const uint TEAM_SPECTATOR	(0);
const uint TEAM_REFEREE		(-3);

//=============================================================================

//=============================================================================
// CTournMatch
//=============================================================================
class CTournMatch : public CMatch
{
private:
	bool			m_bTournMatch;	
	
	AccountVector	m_vTeam1Clients;
	AccountVector	m_vTeam2Clients;
	AccountVector	m_vReferees;
	AccountVector	m_vSpectators;	
	
	uint			m_uiTournID;
	string			m_sTournName;
	
	CTournMatch();

public:
	~CTournMatch()	{}
	CTournMatch(CCore *pCore, CHeap *pHeap, uint uiTournMatchID, uint uiTournID, const string &sTournName, uint uiTeam1Size, uint uiTeam2Size, uint uiRefereeSize, uint uiSpectatorSize);

	void		AddTournClient(uint AccountID, int uiTeam);	
	void		Create(const string &sAddress, ushort unPort);
	
	void		Announce();
};
//=============================================================================
#endif	//__C_TOURNMATCH_H__
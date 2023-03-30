// (C)2009 S2 Games
// c_core.h
//
//=============================================================================
#ifndef __C_CORE_H__
#define __C_CORE_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_date.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CMemManager;
class CSystem;
class CConsole;
class CProfiler;
class CHTTPManager;
class CNetManager;
class CPeerManager;
//class CGameManager;
class CChannelManager;
class CClanManager;
class CClientManager;
class CMatchMaker;
class CGameServerManager;
class CHeap;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
#define DECLARE_CORE_MEMBER(name) \
private: \
	C##name*	m_p##name; \
public: \
	C##name*	Get##name()	{ return m_p##name; }

#define ALLOCATE_CORE_MEMBER(name) \
m_p##name = HEAP_NEW(m_pMemManager, m_pHeap) C##name;

#define _DECLARE_CORE_API(name) C##name*	m_p##name;
#define DECLARE_CORE_API \
_DECLARE_CORE_API(Core) \
_DECLARE_CORE_API(System) \
_DECLARE_CORE_API(MemManager) \
_DECLARE_CORE_API(Console) \
_DECLARE_CORE_API(Profiler) \
_DECLARE_CORE_API(HTTPManager) \
_DECLARE_CORE_API(NetManager) \
_DECLARE_CORE_API(PeerManager) \
_DECLARE_CORE_API(ChannelManager) \
_DECLARE_CORE_API(ClanManager) \
_DECLARE_CORE_API(ClientManager) \
_DECLARE_CORE_API(MatchMaker) \
_DECLARE_CORE_API(GameServerManager)

#define _NULL_CORE_API(name)	m_p##name(NULL)
#define NULL_CORE_API \
_NULL_CORE_API(Core), \
_NULL_CORE_API(System), \
_NULL_CORE_API(MemManager), \
_NULL_CORE_API(Console), \
_NULL_CORE_API(Profiler), \
_NULL_CORE_API(HTTPManager), \
_NULL_CORE_API(NetManager), \
_NULL_CORE_API(PeerManager), \
_NULL_CORE_API(ChannelManager), \
_NULL_CORE_API(ClanManager), \
_NULL_CORE_API(ClientManager), \
_NULL_CORE_API(MatchMaker), \
_NULL_CORE_API(GameServerManager)

#define _INIT_CORE_API(name)	m_p##name = m_pCore->Get##name(); assert(m_p##name != NULL);
#define INIT_CORE_API(core) \
m_pCore = core; assert(m_pCore != NULL); \
_INIT_CORE_API(System); \
_INIT_CORE_API(MemManager); \
_INIT_CORE_API(Console); \
_INIT_CORE_API(Profiler); \
_INIT_CORE_API(HTTPManager); \
_INIT_CORE_API(NetManager); \
_INIT_CORE_API(PeerManager); \
_INIT_CORE_API(ChannelManager); \
_INIT_CORE_API(ClanManager); \
_INIT_CORE_API(ClientManager); \
_INIT_CORE_API(MatchMaker); \
_INIT_CORE_API(GameServerManager);
//=============================================================================

//=============================================================================
// CCore
//=============================================================================
class CCore
{
private:
	CHeap*	m_pHeap;

	DECLARE_CORE_MEMBER(System);
	DECLARE_CORE_MEMBER(MemManager);
	DECLARE_CORE_MEMBER(Console);
	DECLARE_CORE_MEMBER(Profiler);
	DECLARE_CORE_MEMBER(HTTPManager);
	DECLARE_CORE_MEMBER(NetManager);
	DECLARE_CORE_MEMBER(PeerManager);
	DECLARE_CORE_MEMBER(ChannelManager);
	DECLARE_CORE_MEMBER(ClanManager);
	DECLARE_CORE_MEMBER(ClientManager);
	DECLARE_CORE_MEMBER(MatchMaker);
	DECLARE_CORE_MEMBER(GameServerManager);

	CDate			m_dateStart;
	uint			m_uiMilliseconds;
	uint			m_uiSeconds;
	CDate			m_datePrev;
	bool			m_bDateChanged;
	uint			m_uiNextTimeUpdate;
	wstring			m_sDateString;
	wstring			m_sTimeString;
	wstring			m_sDateTimeString;

	bool			m_bExitRequested;
	int				m_iExitCode;

	void	UpdateDateTimeStrings();

	CCore();

public:
	~CCore()	{}
	CCore(CMemManager *pMemManager);

	bool					Initialize();
	int						Execute();
	void					Shutdown();
	void					RequestExit(int iExitCode = EXIT_OK);

	inline uint				GetMilliseconds() const						{ return m_uiMilliseconds; }
	inline uint				GetSeconds() const							{ return m_uiSeconds; }
	inline const wstring&	GetDateString() const						{ return m_sDateString; }
	inline const wstring&	GetTimeString() const						{ return m_sTimeString; }
	inline const wstring&	GetDateTimeString() const					{ return m_sDateTimeString; }

	void					LoadState();
	void					SaveState();
	void					PrintUsage();
};
//=============================================================================
#endif //__C_CORE_H__

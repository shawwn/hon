// (C)2009 S2 Games
// chatserver_types.h
//
//=============================================================================
#ifndef __CHATSERVER_TYPES_H__
#define __CHATSERVER_TYPES_H__

//=============================================================================
// Declarations
//=============================================================================
class CClient;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
// Exit Codes
const uint EXIT_OK			(0);
const uint EXIT_INIT_FAILED	(1);
const uint EXIT_ERROR		(2);

// Keyboard
enum EKeyboardCode
{
	KEY_BACKSPACE,
	KEY_TAB,
	KEY_ENTER,
	KEY_ESCAPE,
	KEY_PGUP,
	KEY_PGDN,
	KEY_END,
	KEY_HOME,
	KEY_LEFT,
	KEY_UP,
	KEY_RIGHT,
	KEY_DOWN,
	KEY_INSERT,
	KEY_DELETE,

	KEY_INVALID
};

const uint KEYBOARD_CTRL		(BIT(0));
const uint KEYBOARD_CTRL_LEFT	(BIT(1) | KEYBOARD_CTRL);
const uint KEYBOARD_CTRL_RIGHT	(BIT(2) | KEYBOARD_CTRL);
const uint KEYBOARD_ALT			(BIT(3));
const uint KEYBOARD_ALT_LEFT	(BIT(4) | KEYBOARD_ALT);
const uint KEYBOARD_ALT_RIGHT	(BIT(5) | KEYBOARD_ALT);
const uint KEYBOARD_SHIFT		(BIT(6));
const uint KEYBOARD_CHARACTER	(BIT(7));

// STL
#define DECLARE_STL_MAP_TYPES(name, key, val) \
typedef pair<key, val>									name##Map_pair; \
typedef CSTLAllocator<name##Map_pair>					name##Map_alloc; \
typedef map<key, val, std::less<key>, name##Map_alloc>	name##Map; \
typedef name##Map::iterator								name##Map_it; \
typedef name##Map::const_iterator						name##Map_cit; \
typedef name##Map::reverse_iterator						name##Map_rit;

#define DECLARE_STL_MULTIMAP_TYPES(name, key, val) \
typedef pair<key, val>													name##MultiMap_pair; \
typedef CSTLAllocator<name##MultiMap_pair>								name##MultiMap_alloc; \
typedef std::multimap<key, val, std::less<key>, name##MultiMap_alloc>	name##MultiMap; \
typedef name##MultiMap::iterator										name##MultiMap_it; \
typedef name##MultiMap::const_iterator									name##MultiMap_cit; \
typedef name##MultiMap::reverse_iterator								name##MultiMap_rit;

#define DECLARE_STL_VECTOR_TYPES(name, val) \
typedef CSTLAllocator<val>				name##Vector_alloc; \
typedef vector<val, name##Vector_alloc>	name##Vector; \
typedef name##Vector::iterator			name##Vector_it; \
typedef name##Vector::const_iterator	name##Vector_cit; \
typedef name##Vector::reverse_iterator	name##Vector_rit;

#define DECLARE_STL_LIST_TYPES(name, val) \
typedef CSTLAllocator<val>				name##List_alloc; \
typedef list<val, name##List_alloc>		name##List; \
typedef name##List::iterator			name##List_it; \
typedef name##List::const_iterator		name##List_cit; \
typedef name##List::reverse_iterator	name##List_rit;

#define DECLARE_STL_DEQUE_TYPES(name, val) \
typedef CSTLAllocator<val>				name##Deque_alloc; \
typedef deque<val, name##Deque_alloc>	name##Deque; \
typedef name##Deque::iterator			name##Deque_it; \
typedef name##Deque::const_iterator		name##Deque_cit; \
typedef name##Deque::reverse_iterator	name##Deque_rit;

#define DECLARE_STL_SET_TYPES(name, val) \
typedef CSTLAllocator<val>							name##Set_alloc; \
typedef set<val, std::less<val>, name##Set_alloc>	name##Set; \
typedef name##Set::iterator							name##Set_it; \
typedef name##Set::const_iterator					name##Set_cit; \
typedef name##Set::reverse_iterator					name##Set_rit;

// Matchmaking
enum EMatchPool
{
	MATCH_POOL_SOLO,
	MATCH_POOL_TEAM,

	NUM_MATCH_POOLS
};

enum ERegions
{
	REGION_TST,
	REGION_USW,
	REGION_USE,
	REGION_EU,
	REGION_SG,
	REGION_MY,
	REGION_VN,
	REGION_PH,

	NUM_REGIONS
};

const uint INVALID_CLIENT_ID(-1);
const uint INVALID_ACCOUNT_ID(-1);
const uint INVALID_CLAN_ID(0);

//#define MASTER_SERVER_ADDRESS	"masterserver.garena.s2games.com"
#define MASTER_SERVER_ADDRESS	"masterserver.hon.s2games.com"

#define CLIENT_REQUEST_SCRIPT	"/client_requester.php"
#define SERVER_REQUEST_SCRIPT	"/server_requester.php"

const string LOCALHOST = "127.0.0.1";
//=============================================================================

#endif //__CHATSERVER_TYPES_H__

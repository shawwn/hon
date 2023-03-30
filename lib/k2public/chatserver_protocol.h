// (C)2010 S2 Games
// chatserver_protocol.h
//
//=============================================================================
#ifndef __CHATSERVER_PROTOCOL_H__
#define __CHATSERVER_PROTOCOL_H__

//=============================================================================
// Definitions
//=============================================================================
const uint CHAT_PROTOCOL_VERSION(9);

const ushort CHAT_CMD_CHANNEL_MSG				(0x03);				// Used when a user messages a channel
const ushort CHAT_CMD_CHANGED_CHANNEL			(0x04);				// Used when we change channels
const ushort CHAT_CMD_JOINED_CHANNEL			(0x05);				// Used when a new user joins our channel
const ushort CHAT_CMD_LEFT_CHANNEL				(0x06);				// Used when a user leaves our channel
const ushort CHAT_CMD_DISCONNECTED				(0x07);				// Used when we get disconnected
const ushort CHAT_CMD_WHISPER					(0x08);				// Used when one user whispers another
const ushort CHAT_CMD_WHISPER_FAILED			(0x09);				// Used when the whisper target could not be found
const ushort CHAT_CMD_WEB_NOTIFICATION			(0x0A);				// Recieved from database when web-related event occurs (depreciated/unused)
const ushort CHAT_CMD_INITIAL_STATUS			(0x0B);				// Sent on connect to update buddy and clan connection status for new client
const ushort CHAT_CMD_UPDATE_STATUS				(0x0C);				// Sent on connect to update buddy and clan connection status for old clients
const ushort CHAT_CMD_REQUEST_BUDDY_ADD			(0x0D);				// Sent from client to chat server to request a buddy add
const ushort CHAT_CMD_REQUEST_BUDDY_REMOVE		(0x0E);				// Sent from client to chat server to request a buddy be removed
const ushort CHAT_CMD_JOINING_GAME				(0x0F);				// Sent when a user starts joining a game
const ushort CHAT_CMD_JOINED_GAME				(0x10);				// Sent when a user finishes joining a game
const ushort CHAT_CMD_LEFT_GAME					(0x11);				// Sent when a user leaves a game
const ushort CHAT_CMD_NOTIFICATION				(0x12);				// Sent when the server has a notification for a client (depreciated/unused)
const ushort CHAT_CMD_CLAN_WHISPER				(0x13);				// Sent when whispering an entire clan
const ushort CHAT_CMD_CLAN_WHISPER_FAILED		(0x14);				// Sent when a whisper to a clan fails
const ushort CHAT_CMD_CLAN_PROMOTE_NOTIFY		(0x15);				// Sent with notification keys for the server to verify on clan promotion
const ushort CHAT_CMD_CLAN_DEMOTE_NOTIFY		(0x16);				// Sent with notification keys for the server to verify on clan demotion
const ushort CHAT_CMD_CLAN_REMOVE_NOTIFY		(0x17);				// Sent with notification keys for the server to verify on clan removal
const ushort CHAT_CMD_LOOKING_FOR_CLAN			(0x18);				// Notification stating user is now "looking for clan"
const ushort CHAT_CMD_NOT_LOOKING_FOR_CLAN		(0x19);				// Notification stating user is no longer "looking for clan"
const ushort CHAT_CMD_MULT_LOOKING_FOR_CLAN		(0x1A);				// Notification stating multiple users are "looking for clan" (sent on connect)
const ushort CHAT_CMD_FLOODING					(0x1B);				// Warning to user that their message wasn't sent due to flood control
const ushort CHAT_CMD_IM						(0x1C);				// Used when a user recieves/sends an IM through the CC panel
const ushort CHAT_CMD_IM_FAILED					(0x1D);				// Used when a user fails to send an IM
const ushort CHAT_CMD_JOIN_CHANNEL				(0x1E);				// Sent by user when joining a new channel
const ushort CHAT_CMD_WHISPER_BUDDIES			(0x20);				// Sending whisper to all buddies
const ushort CHAT_CMD_MAX_CHANNELS				(0x21);				// Error sent when user has joined max. # of channels
const ushort CHAT_CMD_LEAVE_CHANNEL				(0x22);				// Sent by user when leaving a channel
const ushort CHAT_CMD_INVITE_USER_ID			(0x23);				// Sent by game server to invite a user to a game by account ID
const ushort CHAT_CMD_INVITE_USER_NAME			(0x24);				// Sent by game server to invite a user to a game by account name
const ushort CHAT_CMD_INVITED_TO_SERVER			(0x25);				// Sent by chat server to notify a user of a pending server invite
const ushort CHAT_CMD_INVITE_FAILED_USER		(0x26);				// Notifies a user that their invite request failed because the target was not found
const ushort CHAT_CMD_INVITE_FAILED_GAME		(0x27);				// Notifies a user that their invite request failed because they are not in a game
const ushort CHAT_CMD_INVITE_REJECTED			(0x28);				// Indicates that a recieved invite was rejected
const ushort CHAT_CMD_USER_INFO					(0x2A);				// Returns information on a user
const ushort CHAT_CMD_USER_INFO_NO_EXIST		(0x2B);				// The requested user does not exist
const ushort CHAT_CMD_USER_INFO_OFFLINE			(0x2C);				// Returns information on an offline user
const ushort CHAT_CMD_USER_INFO_ONLINE			(0x2D);				// Returns information on an online user
const ushort CHAT_CMD_USER_INFO_IN_GAME			(0x2E);				// Returns information on a user in a game
const ushort CHAT_CMD_CHANNEL_UPDATE			(0x2F);				// Update channel information
const ushort CHAT_CMD_CHANNEL_TOPIC				(0x30);				// Set/get channel topic
const ushort CHAT_CMD_CHANNEL_KICK				(0x31);				// Kick user from channel
const ushort CHAT_CMD_CHANNEL_BAN				(0x32);				// Ban user from channel
const ushort CHAT_CMD_CHANNEL_UNBAN				(0x33);				// Unban user from channel
const ushort CHAT_CMD_CHANNEL_IS_BANNED			(0x34);				// User is banned from channel
const ushort CHAT_CMD_CHANNEL_SILENCED			(0x35);				// User is silenced in this channel
const ushort CHAT_CMD_CHANNEL_SILENCE_LIFTED	(0x36);				// User is no longer silenced in a channel
const ushort CHAT_CMD_CHANNEL_SILENCE_PLACED	(0x37);				// User is now silenced in a channel
const ushort CHAT_CMD_CHANNEL_SILENCE_USER		(0x38);				// Request to silence a user in a channel
const ushort CHAT_CMD_MESSAGE_ALL				(0x39);				// Administrator message to all users
const ushort CHAT_CMD_CHANNEL_PROMOTE			(0x3A);				// Request to promote a user in a channel
const ushort CHAT_CMD_CHANNEL_DEMOTE			(0x3B);				// Request to demote a user in a channel
const ushort CHAT_CMD_CHANNEL_SET_AUTH			(0x3E);				// User wants to enable authorization on a channel
const ushort CHAT_CMD_CHANNEL_REMOVE_AUTH		(0x3F);				// User wants to disable authorization on a channel
const ushort CHAT_CMD_CHANNEL_ADD_AUTH_USER		(0x40);				// User wants to add a user to the authorization list for a channel
const ushort CHAT_CMD_CHANNEL_REM_AUTH_USER		(0x41);				// User wants to remove a user from the authorization list for a channel
const ushort CHAT_CMD_CHANNEL_LIST_AUTH			(0x42);				// User wants to get the authorization list for a channel
const ushort CHAT_CMD_CHANNEL_SET_PASSWORD		(0x43);				// User wants to set the password for a channel
const ushort CHAT_CMD_CHANNEL_ADD_AUTH_FAIL		(0x44);				// Failed to add the user to the channel authorization list
const ushort CHAT_CMD_CHANNEL_REM_AUTH_FAIL		(0x45);				// Failed to remove the user from the channel authorization list
const ushort CHAT_CMD_JOIN_CHANNEL_PASSWORD		(0x46);				// Channel join with password
const ushort CHAT_CMD_CLAN_ADD_MEMBER			(0x47);				// Request to add a new clan member
const ushort CHAT_CMD_CLAN_ADD_REJECTED			(0x48);				// Request to add a member was rejected
const ushort CHAT_CMD_CLAN_ADD_FAIL_ONLINE		(0x49);				// Request to add a member failed, user was not online
const ushort CHAT_CMD_CLAN_ADD_FAIL_CLAN		(0x4A);				// Request to add a member failed, user is in a clan
const ushort CHAT_CMD_CLAN_ADD_FAIL_INVITED		(0x4B);				// Request to add a member failed, user has already been invited
const ushort CHAT_CMD_CLAN_ADD_FAIL_PERMS		(0x4C);				// Request to add a member failed, user does not have proper permissions
const ushort CHAT_CMD_CLAN_ADD_FAIL_UNKNOWN		(0x4D);				// Request to add a member failed
const ushort CHAT_CMD_NEW_CLAN_MEMBER			(0x4E);				// New user added to clan
const ushort CHAT_CMD_CLAN_ADD_ACCEPTED			(0x4F);				// Request to add a member was accepted
const ushort CHAT_CMD_CLAN_RANK_CHANGE			(0x50);				// Clan member's rank changed
const ushort CHAT_CMD_CLAN_CREATE_REQUEST		(0x51);				// Create clan request
const ushort CHAT_CMD_CLAN_CREATE_ACCEPT		(0x52);				// One of the founding members accepted the request
const ushort CHAT_CMD_CLAN_CREATE_REJECT		(0x53);				// One of the founding members rejected the request
const ushort CHAT_CMD_CLAN_CREATE_COMPLETE		(0x54);				// Clan creation completed successfully
const ushort CHAT_CMD_CLAN_CREATE_FAIL_CLAN		(0x55);				// Clan creation failed, one or more users are already in a clan
const ushort CHAT_CMD_CLAN_CREATE_FAIL_INVITE	(0x56);				// Clan creation failed, one or more users have an outstanding clan invitation
const ushort CHAT_CMD_CLAN_CREATE_FAIL_FIND		(0x57);				// Clan creation failed, one or more users could not be found
const ushort CHAT_CMD_CLAN_CREATE_FAIL_DUPE		(0x58);				// Clan creation failed, duplicate founding members
const ushort CHAT_CMD_CLAN_CREATE_FAIL_PARAM	(0x59);				// Clan creation failed, one or more parameters are invalid
const ushort CHAT_CMD_NAME_CHANGE				(0x5A);				// A user's name has changed
const ushort CHAT_CMD_CLAN_CREATE_FAIL_NAME		(0x5B);				// Clan creation failed, name invalid
const ushort CHAT_CMD_CLAN_CREATE_FAIL_TAG		(0x5C);				// Clan creation failed, tag invalid
const ushort CHAT_CMD_CLAN_CREATE_FAIL_UNKNOWN	(0x5D);				// Clan creation failed, unknown error

const ushort CHAT_CMD_AUTO_MATCH_WAITING		(0x60);				// Keep alive
const ushort CHAT_CMD_AUTO_MATCH_STATUS			(0x61);				// Reports to client progress towards forming a full match
const ushort CHAT_CMD_AUTO_MATCH_CONNECT		(0x62);				// The match is ready and the client should connect

const ushort CHAT_CMD_TOURN_MATCH_READY			(0x63);				// The tournament match has been created
const ushort CHAT_CMD_CHAT_ROLL					(0x64);				// The user just rolled
const ushort CHAT_CMD_CHAT_EMOTE				(0x65);				// The user just emoted
const ushort CHAT_CMD_SET_CHAT_MODE_TYPE		(0x66);				// Sets the chat mode type
const ushort CHAT_CMD_CHAT_MODE_AUTO_RESPONSE	(0x67);				// Used for sending an auto response message

const ushort CHAT_CMD_PLAYER_COUNT				(0x68);				// Reports user counts to players periodically
const ushort CHAT_CMD_SERVER_NOT_IDLE			(0x69);				// Server was not idle

const ushort PEER_CMD_HELLO						(0xa0);
const ushort PEER_CMD_GOODBYE					(0xa1);
const ushort PEER_CMD_WELCOME					(0xa2);
const ushort PEER_CMD_GO_AWAY					(0xa3);
const ushort PEER_CMD_REQUEST_PEER_INFO			(0xa4);
const ushort PEER_CMD_PEER_INFO					(0xa5);
const ushort PEER_CMD_END_PEER_INFO				(0xa6);
const ushort PEER_CMD_REQUEST_CHANNEL_INFO		(0xa7);
const ushort PEER_CMD_CHANNEL_INFO				(0xa8);
const ushort PEER_CMD_END_CHANNEL_INFO			(0xa9);

const ushort PEER_CMD_CHANNEL_MESSAGE			(0xb0);
const ushort PEER_CMD_WHISPER					(0xb1);

const ushort CHAT_CMD_REQUEST_BUDDY_ADD_RESPONSE		(0xb2);
const ushort CHAT_CMD_REQUEST_BUDDY_APPROVE				(0xb3);
const ushort CHAT_CMD_REQUEST_BUDDY_APPROVE_RESPONSE	(0xb4);

const ushort CHAT_CMD_REQUEST_GAME_INFO			(0xb5);

//
// General
//

// Bi-directional
const ushort NET_CHAT_PING						(0x2a00);
const ushort NET_CHAT_PONG						(0x2a01);

//
// Client
//

// Client -> Chat Server
const ushort NET_CHAT_CL_CONNECT				(0x0c00);			// Client requesting connection
const ushort NET_CHAT_CL_GET_CHANNEL_LIST		(0x0c01);			// Client requests a list of channels
const ushort NET_CHAT_CL_CHANNEL_LIST_ACK		(0x0c02);			// HACK: until TCP connections are handled properly
const ushort NET_CHAT_CL_GET_CHANNEL_SUBLIST	(0x0c03);			// Client requests a sub-list of channels (for auto-complete)
const ushort NET_CHAT_CL_CHANNEL_SUBLIST_ACK	(0x0c04);			// HACK: until TCP connections are handled properly
const ushort NET_CHAT_CL_GET_USER_STATUS		(0x0c05);			// Client requesting status of a specific user
const ushort NET_CHAT_CL_REQUEST_AUTO_MATCH		(0x0c06);			// Client would like to join the match making queue
const ushort NET_CHAT_CL_CANCEL_AUTO_MATCH		(0x0c07);			// Client would like to leave the match making queue
const ushort NET_CHAT_CL_ADMIN_KICK				(0x0c08);			// Admin request to disconnect target client from chat server
const ushort NET_CHAT_CL_REFRESH_UPGRADES		(0x0c09);			// Client is requesting an upgrade refresh for itself

// Bi-directional stuff related to TMM
const ushort NET_CHAT_CL_TMM_GROUP_CREATE					(0x0c0a);		// Client is requesting a new group be created
const ushort NET_CHAT_CL_TMM_GROUP_JOIN						(0x0c0b);		// Client is joining a group 
const ushort NET_CHAT_CL_TMM_GROUP_LEAVE					(0x0c0c);		// Client is leaving a group
const ushort NET_CHAT_CL_TMM_GROUP_INVITE					(0x0c0d);		// Client would like to invite someone to the group
const ushort NET_CHAT_CL_TMM_GROUP_INVITE_BROADCAST			(0x0c0e);		// Broadcast that a client would like to invite someone to the group
const ushort NET_CHAT_CL_TMM_GROUP_REJECT_INVITE			(0x0c0f);		// Client rejected invite
const ushort NET_CHAT_CL_TMM_GROUP_KICK						(0x0d00);		// The leader requested to kick a group member
const ushort NET_CHAT_CL_TMM_GROUP_JOIN_QUEUE				(0x0d01);		// The group leader wants to join the queue for a match
const ushort NET_CHAT_CL_TMM_GROUP_LEAVE_QUEUE				(0x0d02);		// The group leader wants to leave the join match queue
const ushort NET_CHAT_CL_TMM_GROUP_UPDATE					(0x0d03);		// Updates that occur whenever something in the group is updated
const ushort NET_CHAT_CL_TMM_GROUP_PLAYER_LOADING_STATUS	(0x0d04);		// Send updates on loading status
const ushort NET_CHAT_CL_TMM_GROUP_PLAYER_READY_STATUS		(0x0d05);		// Send updates on whether or not the player is ready
const ushort NET_CHAT_CL_TMM_GROUP_QUEUE_UPDATE				(0x0d06);		// Send information on the queue times to the group
const ushort NET_CHAT_CL_TMM_POPULARITY_UPDATE				(0x0d07);		// Send information on the popularities to all the groups
const ushort NET_CHAT_CL_TMM_GAME_OPTION_UPDATE				(0x0d08);		// Send group option updates to players when the group leader changes them
const ushort NET_CHAT_CL_TMM_MATCH_FOUND_UPDATE				(0x0d09);		// Send team a match info update when a match is found
const ushort NET_CHAT_CL_TMM_FAILED_TO_JOIN					(0x0e0a);		// Either TMM is disabled or they were not allowed to join due to being a leaver or being banned

// Chat Server -> Client
const ushort NET_CHAT_CL_ACCEPT					(0x1c00);			// Accept connection from client
const ushort NET_CHAT_CL_REJECT					(0x1c01);			// Refuse connection from client
const ushort NET_CHAT_CL_CHANNEL_INFO			(0x1c02);			// Basic information about a channel
const ushort NET_CHAT_CL_CHANNEL_LIST_SYN		(0x1c03);			// HACK: until TCP connections are handled properly
const ushort NET_CHAT_CL_CHANNEL_SUBLIST_START	(0x1c04);			// Start of a channel sub-list
const ushort NET_CHAT_CL_CHANNEL_INFO_SUB		(0x1c05);			// Basic information about a channel in a sublist
const ushort NET_CHAT_CL_CHANNEL_SUBLIST_SYN	(0x1c06);			// HACK: until TCP connections are handled properly
const ushort NET_CHAT_CL_CHANNEL_SUBLIST_END	(0x1c07);			// End of a channel sub-list
const ushort NET_CHAT_CL_USER_STATUS			(0x1c08);			// User status request reponse

// Client rejection reasons
const byte CHAT_CLIENT_REJECT_UNKNOWN			(0);
const byte CHAT_CLIENT_REJECT_BAD_VERSION		(1);
const byte CHAT_CLIENT_REJECT_AUTH_FAILED		(2);

// User status
const byte CHAT_CLIENT_USER_UNKNOWN				(0);
const byte CHAT_CLIENT_USER_DISCONNECTED		(1);
const byte CHAT_CLIENT_USER_CONNECTED			(2);
const byte CHAT_CLIENT_USER_JOINING_GAME		(3);
const byte CHAT_CLIENT_USER_IN_GAME				(4);

//
// Game Server
//

// Game Server -> Chat Server
const ushort NET_CHAT_GS_CONNECT				(0x0500);			// Game server requesting connection
const ushort NET_CHAT_GS_DISCONNECT				(0x0501);			// Game server disconnecting
const ushort NET_CHAT_GS_STATUS					(0x0502);			// Game server's current status
const ushort NET_CHAT_GS_ANNOUNCE_MATCH			(0x0503);			// An arrange match is ready for clients
const ushort NET_CHAT_GS_ABANDON_MATCH			(0x0504);			// An arranged match failed to start
const ushort NET_CHAT_GS_MATCH_STARTED			(0x0505);			// An arranged match has started successfully
const ushort NET_CHAT_GS_REMIND_PLAYER			(0x0506);			// An expected player has not yet connected to an arranged match
const ushort NET_CHAT_GS_REPLACE_PLAYER			(0x0507);			// Get a new player to replace a player that dropped out of an arranged match
const ushort NET_CHAT_GS_NOT_IDLE				(0x0508);			// Server was not idle
const ushort NET_CHAT_GS_MATCH_ABORTED			(0x0509);			// An arranged match failed to start

// Chat Server -> Game Server
const ushort NET_CHAT_GS_ACCEPT					(0x1500);			// Accept connection from game server 
const ushort NET_CHAT_GS_REJECT					(0x1501);			// Refuse connection from game server 
const ushort NET_CHAT_GS_CREATE_MATCH			(0x1502);			// Game server has been selected to host an arranged match
const ushort NET_CHAT_GS_ROSTER_SUBSTITUTE		(0x1503);			// Replace a player in an arranged match
const ushort NET_CHAT_GS_REMOTE_COMMAND			(0x1504);			// Execute a console command on this server

// Server rejection reasons
const byte SERVER_REJECT_UNKNOWN				(0);
const byte SERVER_REJECT_BAD_VERSION			(1);
const byte SERVER_REJECT_AUTH_FAILED			(2);

const ushort NET_CHAT_INVALID					(0xffff);


const byte CHAT_MESSAGE_MAX_LENGTH				(250);				// Max length of any channel chat message
const byte CHAT_CHANNEL_MAX_LENGTH				(30);				// Max length of channel names
const byte CHAT_CHANNEL_TOPIC_MAX_LENGTH		(140);				// Max length of channel topics


enum ETMMUpdateType
{
	TMM_CREATE_TEAM_GROUP = 1,
	TMM_GROUP_UPDATE,
	TMM_PLAYER_JOINED_GROUP,
	TMM_PLAYER_FINISHED_LOADING,
	TMM_PLAYER_LEFT_GROUP,
	TMM_PLAYER_KICKED_FROM_GROUP,
	TMM_GROUP_JOINED_QUEUE,
	TMM_GROUP_LEFT_QUEUE,
	TMM_INVITED_TO_GROUP,
	TMM_PLAYER_REJECTED_GROUP_INVITE,
	TMM_GROUP_QUEUE_UPDATE,
	TMM_GROUP_NO_MATCHES_FOUND,
	TMM_GROUP_NO_SERVERS_FOUND,
	TMM_POPULARITY_UPDATE,
	TMM_FOUND_MATCH_UPDATE,
	TMM_GROUP_FOUND_SERVER,
	TMM_MATCHMAKING_DISABLED,
	
	NUM_TMM_UPDATE_TYPES
};

enum EServerStatus
{
	SERVER_STATUS_SLEEPING = 0,
	SERVER_STATUS_IDLE,
	SERVER_STATUS_LOADING,
	SERVER_STATUS_ACTIVE,
	SERVER_STATUS_CRASHED,
	SERVER_STATUS_KILLED,

	SERVER_STATUS_UNKNOWN
};

enum EMatchAbortedReason
{
	MATCH_ABORTED_UNKNOWN = 0,

	MATCH_ABORT_CONNECT_TIMEOUT,
	MATCH_ABORT_START_TIMEOUT,
	MATCH_ABORT_PLAYER_LEFT
};
//=============================================================================

#endif //__CHATSERVER_PROTOCOL_H__

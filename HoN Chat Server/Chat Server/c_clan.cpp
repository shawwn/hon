// (C)2009 S2 Games
// c_clan.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "chatserver_common.h"

#include "c_clan.h"
#include "c_client.h"
//=============================================================================

/*====================
  CClan::~CClan
  ====================*/
CClan::~CClan()
{
	delete m_pClients;
}


/*====================
  CClan::CClan
  ====================*/
CClan::CClan(CCore *pCore, CHeap *pHeap) :
NULL_CORE_API,
m_pHeap(pHeap),

m_pClients(NULL),
m_uiClanID(INVALID_CLAN_ID)
{
	INIT_CORE_API(pCore);

	m_pClients = HEAP_NEW(m_pMemManager, m_pHeap) ClientSet(std::less<ClientSet::key_type>(), ClientSet_alloc(m_pMemManager, m_pHeap));
}


/*====================
  CClan::Broadcast
  ====================*/
void	CClan::Broadcast(const IBuffer &buffer, CClient *pIgnore) const
{
	PROFILE("CClan::Broadcast")

	for (ClientSet_cit itClient(m_pClients->begin()), itEnd(m_pClients->end()); itClient != itEnd; ++itClient)
	{
		CClient *pClient(*itClient);
		if (pClient == NULL || pClient == pIgnore || pClient->GetStatus() < CLIENT_STATUS_CONNECTED)
			continue;
			
		if (pClient->GetChatModeType() == CHAT_MODE_AFK)
		{
			// Even though this user is AFK, still pass the message to the receiver but don't notify the sender that they are AFK, too spammy
		}
		else if (pClient->GetChatModeType() == CHAT_MODE_DND)
		{
			// The user is in DND, do not pass pass the message to the receiver and don't notify the sender that they are DND either, too spammy
			continue;
		}			

		pClient->Send(buffer);
	}
}


/*====================
  CClan::Notify
  ====================*/
void	CClan::Notify(byte yType, const wstring &sMessage) const
{
	PROFILE("CClanManager::Notify")

	m_bufferSend.Clear();
	m_bufferSend << CHAT_CMD_NOTIFICATION << yType << TStringToUTF8(sMessage) << byte(0);
	Broadcast(m_bufferSend);
}

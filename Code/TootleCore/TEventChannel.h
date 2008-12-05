
#pragma once

#include "TRelay.h"
#include "TManager.h"


namespace TLMessaging
{
	class TEventChannel;
	class TEventChannelGroup;

	class TEventChannelManager;

	extern TPtr<TEventChannelManager> g_pEventChannelManager;
}

/*
	Event channel class - is a specialised relay (publisher and subscriber) that allows filtering of messages into seperate channels
*/
class TLMessaging::TEventChannel : public TRelay
{
public:
	TEventChannel(TRef refChannel) :
	  m_refChannelID(refChannel)
	{
	}

	inline	TRef				GetChannelID()	const	{ return m_refChannelID; }

protected:

	virtual void			ProcessMessage(TPtr<TLMessaging::TMessage>& pMessage)
	{
		// Is the message available for our channel?
		if(pMessage->HasChannelID(GetChannelID()))
		{
			// Yes - relay the message onot our subscribers
			PublishMessage(pMessage);
		}
	}

private:
	TRef						m_refChannelID;
};


class TLMessaging::TEventChannelGroup
{
public:
	TEventChannelGroup(TRef refPublisherID) :
	  m_refPublisherID(refPublisherID)
	  {
	  }

	inline TRef													GetPublisherID()		const	{ return m_refPublisherID; }

	// Registers an event channel with the group and returns a pointer tot he new event channel
	TPtr<TEventChannel>									RegisterEventChannel(TRef refChannelID)
	{
		s32 iIndex = m_EventChannels.Add(new TEventChannel(refChannelID));

		// Failed - return null
		if(iIndex == -1)
			return NULL;

		// Return the newly created element
		return m_EventChannels.ElementAt(iIndex);
	}

	// Unregisters an event channel from the group
	Bool															UnregisterEventChannel(TRef refChannelID)
	{
		s32 sIndex = FindEventChannelIndex(refChannelID);

		if(sIndex != -1)
			return m_EventChannels.RemoveAt(sIndex);

		return FALSE;
	}

	inline Bool													HasEventChannels()	const { return (m_EventChannels.GetSize() > 0); }

	TPtr<TLMessaging::TEventChannel>			FindEventChannel(TRef refChannelID);
	s32															FindEventChannelIndex(TRef refChannelID);

	inline Bool									operator==(TRefRef PublisherRef) const		{	return m_refPublisherID == PublisherRef;	}

private:
	TRef																m_refPublisherID;			// ID of the publisher that will publish events to these channels
	TPtrArray<TLMessaging::TEventChannel>		m_EventChannels;		// The filtered event channels
};

/*
	Event Channel Manager - keeps track of events channels
*/
class TLMessaging::TEventChannelManager : public TManager
{
public:

	TEventChannelManager(TRef refManagerID) :
	  TManager(refManagerID)
	  {
	  }

	// Channel management
	Bool				RegisterEventChannel(TPublisher* pPublisher, TRef refPublisherID, TRef refChannelID);
	Bool				UnregisterEventChannel(TPublisher* pPublisher, TRef refPublisherID, TRef refChannelID);

	// Subscription management
	Bool				SubscribeTo(TSubscriber* pSubscriber, TRef refPublisherID, TRef refChannelID);
	
	template <class T>
	Bool			SubscribeTo(TPtr<T> pSubscriber, TRef refPublisherID, TRef refChannelID)
	{
		TSubscriber* pSubscriberPtr = dynamic_cast<TSubscriber*>(pSubscriber.GetObject());
		return SubscribeTo(pSubscriberPtr, refPublisherID, refChannelID);
	}

	Bool				UnsubscribeFrom(TSubscriber* pSubscriber, TRef refPublisherID, TRef refChannelID);
	inline Bool			UnsubscribeFrom(TPtr<TSubscriber> pSubscriber, TRef refPublisherID, TRef refChannelID)
	{
		return UnsubscribeFrom(pSubscriber.GetObject(), refPublisherID, refChannelID);
	}

	// Message sending
	Bool				BroadcastMessage(TPtr<TMessage> pData, TRef refPublisherID, TRef refChannelID);

private:

	TPtr<TLMessaging::TEventChannelGroup>				FindEventChannelGroup(TRef refPublisherID);
	s32																		FindEventChannelGroupIndex(TRef refPublisherID);


	TPtr<TLMessaging::TEventChannel>						FindEventChannel(TRef refPublisherID, TRef refChannelID );
	s32																		FindEventChannelIndex(TRef refPublisherID, TRef refChannelID);

private:
	TPtrArray<TLMessaging::TEventChannelGroup>			m_EventChannelGroups;
};
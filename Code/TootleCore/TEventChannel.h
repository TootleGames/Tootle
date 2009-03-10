
#pragma once

#include "TRelay.h"
#include "TManager.h"


namespace TLMessaging
{
	class TEventChannel;
	class TEventChannelInterface;
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
	TEventChannel(TRefRef refChannel) :
		m_refChannelID(refChannel)
	{
	}

	FORCEINLINE TRefRef	GetChannelRef()	const					{	return m_refChannelID; }

	FORCEINLINE Bool	operator==(TRefRef ChannelRef) const	{	return GetChannelRef() == ChannelRef;	}

protected:

	virtual void		ProcessMessage(TLMessaging::TMessage& Message)
	{
		// Is the message available for our channel?
		if(Message.GetMessageRef() == GetChannelRef())
		{
			// Yes - relay the message onot our subscribers
			PublishMessage(Message);
		}
	}

private:
	TRef				m_refChannelID;
};


class TLMessaging::TEventChannelInterface
{
public:

	virtual ~TEventChannelInterface()
	{
	}

	FORCEINLINE TPtr<TEventChannel>&	RegisterEventChannel(TRefRef refChannelID);		// Registers an event channel with the group and returns a pointer tot he new event channel
	FORCEINLINE Bool					UnregisterEventChannel(TRefRef refChannelID);	// Unregisters an event channel from the group

	FORCEINLINE Bool					HasEventChannels() const						{	return (m_EventChannels.GetSize() > 0); }
	FORCEINLINE TPtr<TEventChannel>&	FindEventChannel(TRefRef refChannelID)			{	return m_EventChannels.FindPtr( refChannelID );	}
	FORCEINLINE s32						FindEventChannelIndex(TRefRef refChannelID)		{	return m_EventChannels.FindIndex( refChannelID );	}

private:
	TPtrArray<TEventChannel>	m_EventChannels;			// The filtered event channels
};


class TLMessaging::TEventChannelGroup : public TEventChannelInterface
{
public:
	TEventChannelGroup(TRefRef refPublisherID) :
		m_refPublisherID(refPublisherID)
	{
	}

	FORCEINLINE TRefRef					GetPublisherID() const						{ return m_refPublisherID; }

	FORCEINLINE Bool					operator==(const TEventChannelGroup& Group)		{	return GetPublisherID() == Group.GetPublisherID();	}
	FORCEINLINE Bool					operator==(TRefRef PublisherRef) const			{	return GetPublisherID() == PublisherRef;	}

private:
	TRef						m_refPublisherID;			// ID of the publisher that will publish events to these channels
};




/*
	Event Channel Manager - keeps track of events channels
*/
class TLMessaging::TEventChannelManager : public TManager
{
public:
	TEventChannelManager(TRefRef refManagerID) :
		TManager(refManagerID)
	{
	}

	// Channel management
	Bool							RegisterEventChannel(TPublisher* pPublisher, TRefRef refPublisherID, TRefRef refChannelID);
	Bool							UnregisterEventChannel(TPublisher* pPublisher, TRefRef refPublisherID, TRefRef refChannelID);

	// Subscription management
	Bool							SubscribeTo(TSubscriber* pSubscriber, TRefRef refPublisherID, TRefRef refChannelID);
	template <class T>
	FORCEINLINE Bool				SubscribeTo(TPtr<T>& pSubscriber, TRefRef refPublisherID, TRefRef refChannelID);

	Bool							UnsubscribeFrom(TSubscriber* pSubscriber, TRefRef refPublisherID, TRefRef refChannelID);
	FORCEINLINE Bool				UnsubscribeFrom(TPtr<TSubscriber>& pSubscriber, TRefRef refPublisherID, TRefRef refChannelID);

	// Message sending
	Bool							BroadcastMessage(TLMessaging::TMessage& pData, TRefRef refPublisherID, TRefRef refChannelID);

private:

	FORCEINLINE TPtr<TEventChannelGroup>&		FindEventChannelGroup(TRefRef refPublisherID)				{	return m_EventChannelGroups.FindPtr( refPublisherID );	}
	FORCEINLINE s32								FindEventChannelGroupIndex(TRefRef refPublisherID)			{	return m_EventChannelGroups.FindIndex( refPublisherID );	}

	TPtr<TEventChannel>&			FindEventChannel(TRefRef refPublisherID, TRefRef refChannelID );
	s32								FindEventChannelIndex(TRefRef refPublisherID, TRefRef refChannelID);

private:
	TPtrArray<TEventChannelGroup>	m_EventChannelGroups;
};




FORCEINLINE Bool TLMessaging::TEventChannelManager::UnsubscribeFrom(TPtr<TSubscriber>& pSubscriber,TRefRef refPublisherID,TRefRef refChannelID)
{
	return UnsubscribeFrom(pSubscriber.GetObject(), refPublisherID, refChannelID);
}


template <class T>
FORCEINLINE Bool TLMessaging::TEventChannelManager::SubscribeTo(TPtr<T>& pSubscriber, TRefRef refPublisherID, TRefRef refChannelID)
{
	TSubscriber* pSubscriberPtr = dynamic_cast<TSubscriber*>(pSubscriber.GetObject());
	return SubscribeTo(pSubscriberPtr, refPublisherID, refChannelID);
}

//------------------------------------------------
// Registers an event channel with the group and returns a pointer tot he new event channel
//------------------------------------------------
FORCEINLINE TPtr<TLMessaging::TEventChannel>& TLMessaging::TEventChannelInterface::RegisterEventChannel(TRefRef refChannelID)	
{
	TPtr<TLMessaging::TEventChannel> pChannel = new TEventChannel( refChannelID );
	return m_EventChannels.AddPtr( pChannel );
}


//------------------------------------------------
// Unregisters an event channel from the group
//------------------------------------------------
FORCEINLINE Bool TLMessaging::TEventChannelInterface::UnregisterEventChannel(TRefRef refChannelID)	
{
	TPtr<TLMessaging::TEventChannel> pChannel = new TEventChannel( refChannelID );
	return m_EventChannels.Remove( refChannelID );
}

/*
	TPublisher.h

	Author(s):		Duane Bradbury

	NOTE:	If you simply include the publisher .h file then you may get incomplete template compilation due to the publisher using the subscriber class.  
				I seperated the two class to make it easier to edit and include them but it might be easier to have them both in the same .h file as they are interdependant,
				as we can't include the subscriber .h file from here otherwise we get cyclical dependency

	Features:

	Changes:
*/


#pragma once

#include "TArray.h"
#include "TMessage.h"

namespace TLMessaging
{
	class TSubscriber;
	class TPublisher;
}


/*
	Publisher class - publishers data to all subscribers
*/
class TLMessaging::TPublisher
{
	friend class TLMessaging::TSubscriber;
public:
	TPublisher()				{}
	virtual ~TPublisher()		{	Shutdown();	}

	void				Shutdown()										{	RemoveAllSubscribers();	}

	Bool				Subscribe(TSubscriber* pSubscriber);
	Bool				Unsubscribe(TSubscriber* pSubscriber);

	FORCEINLINE Bool	HasSubscribers() const							{	return m_Subscribers.GetSize() != 0;	}
	FORCEINLINE Bool	HasSubscribers(TRefRef MessageRef) const		{	return HasSubscribers();	}	//	gr: for future enhancement, check if any subscribers are going to recieve this specific message...
	FORCEINLINE void	PublishMessage(TLMessaging::TMessage& Message);	//	send message to subscribers if we have any

protected:
	TArray<TSubscriber*>&	GetSubscribers()							{	return m_Subscribers;	}
	void				DoPublishMessage(TLMessaging::TMessage& Message,TSubscriber& Subscriber);

private:
	FORCEINLINE Bool	AddSubscriber(TSubscriber* pSubscriber)			{	return pSubscriber ? m_Subscribers.AddUnique(pSubscriber)!=-1 : FALSE;	}
	FORCEINLINE Bool	RemoveSubscriber(TSubscriber* pSubscriber)		{	return m_Subscribers.Remove(pSubscriber);	}

	void				RemoveAllSubscribers();
	void				DoPublishMessage(TLMessaging::TMessage& Message);

private:
	TArray<TLMessaging::TSubscriber*>		m_Subscribers;
};






FORCEINLINE void TLMessaging::TPublisher::PublishMessage(TLMessaging::TMessage& Message)
{
	//	Make sure there are subscribers to send to before sending message
	if ( HasSubscribers() )
	{
		DoPublishMessage(Message);
	}
}

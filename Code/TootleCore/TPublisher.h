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

#include "TPointerArray.h"
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
	virtual ~TPublisher()		{	Shutdown();	}	//	gr: moved this out of being protected so the arrays can delete them, not sure what this exposes as there were no comments as to why it was protected...
	
	Bool				Subscribe(TSubscriber* pSubscriber);
	Bool				Unsubscribe(TSubscriber* pSubscriber);

	FORCEINLINE Bool	HasSubscribers() const							{	return m_Subscribers.GetSize() != 0;	}
	FORCEINLINE Bool	HasSubscribers(TRefRef MessageRef) const		{	return HasSubscribers();	}	//	gr: for future enhancement, check if any subscribers are going to recieve this specific message...
	FORCEINLINE void	PublishMessage(TLMessaging::TMessage& Message);	//	send message to subscribers if we have any
	//FORCEINLINE void	PublishMessageReverse(TLMessaging::TMessage& Message);	//	send message to subscribers if we have any in reverse order

	FORCEINLINE void	SetPublishOrder(TLArray::TSortOrder::Type Order)	{	m_Subscribers.SetSortOrder(Order);	}

	virtual TRefRef		GetPublisherRef() const=0;						//	ref for this publisher (doesn't need to be unique, just an identifier)

protected:
	FORCEINLINE void			Shutdown()										{	RemoveAllSubscribers();	}
	
	TPointerArray<TLMessaging::TSubscriber,false>&	GetSubscribers()							{	return m_Subscribers;	}
	void				DoPublishMessage(TLMessaging::TMessage& Message,TSubscriber& Subscriber);

private:
	FORCEINLINE Bool	AddSubscriber(TSubscriber* pSubscriber)			{	return pSubscriber ? m_Subscribers.AddUnique(pSubscriber)!=-1 : FALSE;	}
	FORCEINLINE Bool	RemoveSubscriber(TSubscriber* pSubscriber)		{	return m_Subscribers.Remove(pSubscriber);	}

	void				RemoveAllSubscribers();
	void				DoPublishMessage(TLMessaging::TMessage& Message);

private:
	TPointerArray<TLMessaging::TSubscriber,false>		m_Subscribers;
};

//	explicitly set pointer as data type (need to find a way to do this generically!)
TLCore_DeclareIsDataType(TLMessaging::TPublisher*);






FORCEINLINE void TLMessaging::TPublisher::PublishMessage(TLMessaging::TMessage& Message)
{
	//	Make sure there are subscribers to send to before sending message
	if ( HasSubscribers() )
	{
		//	gr: set the publisher ref if it's not set
		if ( !Message.GetSenderRef().IsValid() )
			Message.SetSenderRef( this->GetPublisherRef() );

		//	publish
		DoPublishMessage(Message);
	}
}


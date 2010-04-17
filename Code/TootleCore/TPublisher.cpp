#include "TPublisher.h"
#include "TSubscriber.h"


//#define ENABLE_SUBSCRIBER_OUTPUT

Bool TLMessaging::TPublisher::Subscribe(TSubscriber* pSubscriber)
{
	if ( !AddSubscriber(pSubscriber) )
		return FALSE;
	
#ifdef ENABLE_SUBSCRIBER_OUTPUT
	TTempString Debug_String;
	TRef PublisherRef = this->GetPublisherRef();
	Debug_String << PublisherRef << " new subscriber " << pSubscriber->GetSubscriberRef();
	TLDebug_Print( Debug_String );
#endif
	
	return pSubscriber->AddPublisher(this);
}


Bool TLMessaging::TPublisher::Unsubscribe(TSubscriber* pSubscriber)	
{
	if ( !RemoveSubscriber(pSubscriber) )
		return FALSE;
	
#ifdef ENABLE_SUBSCRIBER_OUTPUT
	TTempString Debug_String;
	Debug_String << this->GetPublisherRef() << " removed subscriber " << pSubscriber->GetSubscriberRef();
	TLDebug_Print( Debug_String );
#endif
	
	return pSubscriber->RemovePublisher(this);
}


void TLMessaging::TPublisher::DoPublishMessage(TLMessaging::TMessage& Message,TSubscriber& Subscriber)
{
	Message.ResetReadPos();
	Subscriber.ProcessMessage( Message );
}


void TLMessaging::TPublisher::DoPublishMessage(TLMessaging::TMessage& Message)		
{
	for(u32 uIndex = 0; uIndex < m_Subscribers.GetSize(); uIndex++)
	{
		// Reset the read pos for each subscriber so they read from the start
		Message.ResetReadPos();

		TSubscriber* pSubscriber = m_Subscribers.ElementAt(uIndex);

		pSubscriber->ProcessMessage(Message);
	}
}


void TLMessaging::TPublisher::RemoveAllSubscribers()
{
	for(u32 uIndex = 0; uIndex < m_Subscribers.GetSize(); uIndex++)
	{
		TSubscriber* pSubscriber = m_Subscribers.ElementAt(uIndex);

		pSubscriber->RemovePublisher(this);
	}

	// Empty the list of subscribers
	m_Subscribers.Empty();
}


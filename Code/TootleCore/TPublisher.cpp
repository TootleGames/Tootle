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
	u32 SubscriberCount = m_Subscribers.GetSize();
	for(u32 uIndex = 0; uIndex <SubscriberCount; uIndex++)
	{
		// Reset the read pos for each subscriber so they read from the start
		Message.ResetReadPos();
		
#ifdef _DEBUG
		if ( m_Subscribers.GetSize() != SubscriberCount )
		{
			TDebugString Debug_String;
			Debug_String << "Number of subscribers in " << GetPublisherRef() << " has changed from " << SubscriberCount << " to " << m_Subscribers.GetSize() << " during publish";
			TLDebug_Print( Debug_String );
			break;
		}
#endif

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


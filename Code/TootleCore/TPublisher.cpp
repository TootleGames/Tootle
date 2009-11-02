#include "TPublisher.h"
#include "TSubscriber.h"




Bool TLMessaging::TPublisher::Subscribe(TSubscriber* pSubscriber)
{
	if ( !AddSubscriber(pSubscriber) )
		return FALSE;
	
	return pSubscriber->AddPublisher(this);
}


Bool TLMessaging::TPublisher::Unsubscribe(TSubscriber* pSubscriber)	
{
	if ( !RemoveSubscriber(pSubscriber) )
		return FALSE;
	
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


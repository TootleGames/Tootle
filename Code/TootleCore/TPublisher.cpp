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


Bool TLMessaging::TPublisher::Subscribe(TPtr<TSubscriber>& pSubscriber)
{	
	return Subscribe( pSubscriber.GetObject() );	
}


Bool TLMessaging::TPublisher::Unsubscribe(TPtr<TSubscriber>& pSubscriber)	
{	
	return Unsubscribe( pSubscriber.GetObject() );	
}


void TLMessaging::TPublisher::DoPublishMessage(TPtr<TMessage>& pMessage)		
{
	for(u32 uIndex = 0; uIndex < m_Subscribers.GetSize(); uIndex++)
	{
		// Reset the read pos for each subscriber so they read from the start
		pMessage->ResetReadPos();

		TSubscriber* pSubscriber = m_Subscribers.ElementAt(uIndex);

		pSubscriber->ProcessMessage(pMessage);
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


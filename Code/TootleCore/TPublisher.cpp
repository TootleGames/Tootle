
#include "TPublisher.h"
#include "TSubscriber.h"

using namespace TLMessaging;

Bool TPublisher::Subscribe(TPtr<TSubscriber>& pSubscriber)
{
	return Subscribe(pSubscriber.GetObject());
}

Bool	TPublisher::Subscribe(TSubscriber* pSubscriber)
{
	if(AddSubscriber(pSubscriber))
	{
		return pSubscriber->AddPublisher(this);
	}

	return FALSE;
}


Bool TPublisher::Unsubscribe(TPtr<TSubscriber>& pSubscriber)
{
	return Unsubscribe(pSubscriber.GetObject());
}

Bool	TPublisher::Unsubscribe(TSubscriber* pSubscriber)	
{
	if(RemoveSubscriber(pSubscriber))
	{
		return pSubscriber->RemovePublisher(this);
	}

	return FALSE;
}

void TPublisher::DoPublishMessage(TPtr<TMessage>& pMessage)		
{
	for(u32 uIndex = 0; uIndex < m_Subscribers.GetSize(); uIndex++)
	{
		// Reset the read pos for each subscriber so they read from the start
		pMessage->ResetReadPos();

		TSubscriber* pSubscriber = m_Subscribers.ElementAt(uIndex);

		pSubscriber->ProcessMessage(pMessage);
	}
}

void TPublisher::RemoveAllSubscribers()
{
	for(u32 uIndex = 0; uIndex < m_Subscribers.GetSize(); uIndex++)
	{
		TSubscriber* pSubscriber = m_Subscribers.ElementAt(uIndex);

		pSubscriber->RemovePublisher(this);
	}

	// Empty the list of subscribers
	m_Subscribers.Empty();
}



#pragma once

#include "TPublisher.h"

class TLMessaging::TSubscriber
{
	friend class TLMessaging::TPublisher;
public:
	TSubscriber()					{}
	virtual ~TSubscriber()			{	Shutdown();	}

	FORCEINLINE void	Shutdown()										{	UnsubscribeAll();	}

	FORCEINLINE Bool	SubscribeTo(TPublisher* pPublisher)				{	return pPublisher->Subscribe(this);	}
	FORCEINLINE Bool	SubscribeTo(TPtr<TPublisher> pPublisher)		{	return SubscribeTo( pPublisher.GetObject() );	}
	FORCEINLINE Bool	UnsubscribeFrom(TPublisher* pPublisher)			{	return pPublisher->Unsubscribe(this);	}
	FORCEINLINE Bool	UnsubscribeFrom(TPtr<TPublisher> pPublisher)	{	return UnsubscribeFrom( pPublisher.GetObject() );	}

protected:
	virtual void		ProcessMessage(TPtr<TLMessaging::TMessage>& pMessage) = 0;

private:
	FORCEINLINE Bool	AddPublisher(TPublisher* pPublisher)		{	return m_Publishers.Add(pPublisher) != -1;	}
	FORCEINLINE Bool	RemovePublisher(TPublisher* pPublisher)		{	return m_Publishers.Remove(pPublisher);	}
	FORCEINLINE void	UnsubscribeAll();							// Unsubscribe from all publishers

private:
	TArray<TLMessaging::TPublisher*>		m_Publishers;			// List of publishers
};




FORCEINLINE void TLMessaging::TSubscriber::UnsubscribeAll()
{
	// Unsubscribe from all publishers
	for(u32 uIndex = 0; uIndex < m_Publishers.GetSize(); uIndex++)
	{
		TPublisher* pPublisher = m_Publishers.ElementAt(uIndex);
		pPublisher->RemoveSubscriber(this);
	}

	// Empty the list
	m_Publishers.Empty();
}


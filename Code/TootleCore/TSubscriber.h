
#pragma once

#include "TPublisher.h"

class TLMessaging::TSubscriber
{
	friend class TLMessaging::TPublisher;
public:
	FORCEINLINE Bool	SubscribeTo(TPublisher* pPublisher)				{	return pPublisher->Subscribe(this);	}
	FORCEINLINE Bool	UnsubscribeFrom(TPublisher* pPublisher)			{	return pPublisher->Unsubscribe(this);	}

	virtual TRefRef		GetSubscriberRef() const=0;

protected:
	virtual ~TSubscriber()			{	Shutdown();	}
	
	FORCEINLINE void	Shutdown()										{	UnsubscribeAll();	}
	
	virtual void		ProcessMessage(TLMessaging::TMessage& Message) = 0;

private:
	FORCEINLINE Bool	AddPublisher(TPublisher* pPublisher)		{	return m_Publishers.AddUnique(pPublisher) != -1;	}
	FORCEINLINE Bool	RemovePublisher(TPublisher* pPublisher)		{	return m_Publishers.Remove(pPublisher);	}
	FORCEINLINE void	UnsubscribeAll();							// Unsubscribe from all publishers

private:
	TPointerArray<TLMessaging::TPublisher>		m_Publishers;			// List of publishers
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


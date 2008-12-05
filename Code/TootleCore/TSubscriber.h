
#pragma once

#include "TPublisher.h"

class TLMessaging::TSubscriber
{
	friend class TLMessaging::TPublisher;
public:
	TSubscriber()					{}

	virtual ~TSubscriber()		
	{
		UnsubscribeAll();
	}

	inline Bool SubscribeTo(TPublisher* pPublisher)
	{
		return pPublisher->Subscribe(this);
	}

	inline Bool UnsubscribeFrom(TPublisher* pPublisher)
	{
		return pPublisher->Unsubscribe(this);
	}


	inline Bool SubscribeTo(TPtr<TPublisher> pPublisher)
	{
		if ( !pPublisher )
		{
			TLDebug_Break("Attempted to subscribe to NULL publisher");
			return FALSE;
		}
		
		return pPublisher->Subscribe(this);
	}

	inline Bool UnsubscribeFrom(TPtr<TPublisher> pPublisher)
	{
		return pPublisher->Unsubscribe(this);
	}


protected:

	virtual void		ProcessMessage(TPtr<TLMessaging::TMessage>& pMessage) = 0;

private:

	inline Bool				AddPublisher(TPublisher* pPublisher)
	{
		m_Publishers.Add(pPublisher);
		return TRUE;
	}

	inline Bool				RemovePublisher(TPublisher* pPublisher)
	{
		return m_Publishers.Remove(pPublisher);
	}

	void						UnsubscribeAll()
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


private:
	TArray<TLMessaging::TPublisher*>		m_Publishers;		// List of publishers
};



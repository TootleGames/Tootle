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
	virtual ~TPublisher()	
	{
		RemoveAllSubscribers();
	}

	Bool				Subscribe(TSubscriber* pSubscriber);
	Bool				Unsubscribe(TSubscriber* pSubscriber);

	// TPtr wrapper routines
	Bool				Subscribe(TPtr<TSubscriber>& pSubscriber);
	Bool				Unsubscribe(TPtr<TSubscriber>& pSubscriber);

	FORCEINLINE Bool	HasSubscribers() const				{	return m_Subscribers.GetSize() != 0;	}

	inline void	PublishMessage(TPtr<TLMessaging::TMessage>& pMessage)
	{
		//	Make sure there are subscribers to send to
		if ( HasSubscribers() )
			DoPublishMessage(pMessage);
	}

private:
	inline Bool			AddSubscriber(TSubscriber* pSubscriber)	
	{
		m_Subscribers.Add(pSubscriber);

		return TRUE;
	}

	inline Bool			RemoveSubscriber(TSubscriber* pSubscriber)
	{
		 return m_Subscribers.Remove(pSubscriber);
	}

	void				RemoveAllSubscribers();

	void				DoPublishMessage(TPtr<TLMessaging::TMessage>& pMessage);

private:
	TArray<TLMessaging::TSubscriber*>		m_Subscribers;
};


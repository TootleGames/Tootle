/*
	TMessageQueue - interface for sending/receiving messages that do not need to be processed immediately

	NOTE:	This will generally be used as a shared resource possibly across multiple threads.  If we are using multithreading
				then we need to lock the resource when it is in use - no adding/removing messages when something (or itself) is alreayd doing
				similar procedures
*/

#pragma once

#include "TMessage.h"

namespace TLMessaging
{
	class TMessageQueue;
};


class TLMessaging::TMessageQueue
{
public:

	virtual ~TMessageQueue()
	{
		RemoveAllMessages();
	}

	// Message sending
	Bool			QueueMessage(TPtr<TMessage> pMessage)						
	{
		// NOTE: Will need to wait for a mutex if mutlithreading is used
		if(pMessage.IsValid())
			return (m_MessageQueue.Add(pMessage) != -1);

		return FALSE;
	}

	inline u32			NumberOfMessages()			const	{ return m_MessageQueue.GetSize(); }
	inline Bool			HasMessagesInQueue()		const	{ return (NumberOfMessages() > 0); }

protected:

	virtual void ProcessMessageFromQueue(TPtr<TLMessaging::TMessage>& pMessage)	= 0;		// Individual message processing - behaviour dependent on where it is used

	// inline wrapper for the main message queue process call
	inline void ProcessMessageQueue()
	{
		if(HasMessagesInQueue())
			DoProcessMessageQueue();
	}

private:

	// Main message queue processing
	void DoProcessMessageQueue()		
	{
		u32 uNumberOfMessages = NumberOfMessages();

		// NOTE: Will need to lock the message queue when we are updating it if using threads
		for(u32 uIndex = 0; uIndex < uNumberOfMessages; uIndex++)
		{
			TPtr<TLMessaging::TMessage>&	pMessage = m_MessageQueue.ElementAt(uIndex);

			if(pMessage.IsValid())
				ProcessMessageFromQueue(pMessage);
		}

		// Finished now remove the messages from the queue
		RemoveAllMessages();
	}

	// Remove all messages from the queue
	void			RemoveAllMessages()
	{
		for(u32 uIndex = 0; uIndex < m_MessageQueue.GetSize(); uIndex++)
			m_MessageQueue.ElementAt(uIndex) = NULL;

		m_MessageQueue.Empty();
	}


protected:
	TPtrArray<TLMessaging::TMessage>		m_MessageQueue;
};
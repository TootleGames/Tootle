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
	Bool			QueueMessage(TLMessaging::TMessage& Message)						
	{
		// NOTE: Will need to wait for a mutex if mutlithreading is used
		return (  m_MessageQueue.Add(Message) != -1 );
	}

	FORCEINLINE u32			NumberOfMessages()			const	{ return m_MessageQueue.GetSize(); }
	FORCEINLINE Bool			HasMessagesInQueue()		const	{ return (NumberOfMessages() > 0); }

protected:

	virtual void ProcessMessageFromQueue(TLMessaging::TMessage& Message)	= 0;		// Individual message processing - behaviour dependent on where it is used

	// FORCEINLINE wrapper for the main message queue process call
	FORCEINLINE void ProcessMessageQueue()
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
			TLMessaging::TMessage&	Message = m_MessageQueue.ElementAt(uIndex);

			ProcessMessageFromQueue(Message);
		}

		// Finished now remove the messages from the queue
		RemoveAllMessages();
	}

	// Remove all messages from the queue
	FORCEINLINE void		RemoveAllMessages()
	{
		m_MessageQueue.Empty(TRUE);
	}


protected:
	TArray<TLMessaging::TMessage>		m_MessageQueue;
};
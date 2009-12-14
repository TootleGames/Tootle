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

	FORCEINLINE Bool		QueueMessage(TLMessaging::TMessage& Message);		//	queue up message 

	FORCEINLINE u32			GetMessageQueueSize() const			{	return m_MessageQueue.GetSize();	}
	FORCEINLINE Bool		HasMessagesInQueue() const			{	return m_MessageQueue.GetSize() > 0;	}

protected:	
	virtual ~TMessageQueue()	{}

	virtual void			ProcessMessageFromQueue(TLMessaging::TMessage& Message)	= 0;		// Individual message processing - behaviour dependent on where it is used
	FORCEINLINE void		ProcessMessageQueue();				//	inline wrapper for the main message queue process call

private:
	FORCEINLINE void		DoProcessMessageQueue();			// Main message queue processing

protected:
	TArray<TLMessaging::TMessage>		m_MessageQueue;
};






//--------------------------------------------------------------
//	queue up message 
//--------------------------------------------------------------
FORCEINLINE Bool TLMessaging::TMessageQueue::QueueMessage(TLMessaging::TMessage& Message)						
{
	// NOTE: Will need to wait for a mutex if mutlithreading is used
	return (  m_MessageQueue.Add(Message) != -1 );
}

	

//--------------------------------------------------------------
//	inline wrapper for the main message queue process call
//--------------------------------------------------------------
FORCEINLINE void TLMessaging::TMessageQueue::ProcessMessageQueue()
{
	if ( HasMessagesInQueue() )
		DoProcessMessageQueue();
}



//--------------------------------------------------------------
// Main message queue processing
//--------------------------------------------------------------
FORCEINLINE void TLMessaging::TMessageQueue::DoProcessMessageQueue()		
{
	u32 uNumberOfMessages = GetMessageQueueSize();

	// NOTE: Will need to lock the message queue when we are updating it if using threads
	for(u32 uIndex=0;	uIndex<uNumberOfMessages;	uIndex++)
	{
		TLMessaging::TMessage&	Message = m_MessageQueue.ElementAt(uIndex);

		ProcessMessageFromQueue(Message);

		//	gr: warning, do not access Message here, there is a chance that another message was added to the queue
		//	during the process, so the reference(pointer) to Element(Index) coudl be invalid if the array was re-allocated
	}

	// Finished now remove the messages from the queue
	//	gr: dont remove new messages... only remove the ones we processed in case more messages were queued during this queue process
	m_MessageQueue.RemoveAt( 0, uNumberOfMessages );
}


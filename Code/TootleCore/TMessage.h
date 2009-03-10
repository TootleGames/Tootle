
#pragma once

#include "TBinaryTree.h"



namespace TLMessaging
{
	class TMessage;
};


/*
	Message - a generic message type that can have arbitrary data added to it
*/
class TLMessaging::TMessage : public TBinaryTree
{
public:
	TMessage()	: 
		TBinaryTree("None") 
	{}

	TMessage(TRefRef MessageRef) : 
		TBinaryTree(MessageRef)	
	{}
	
	TMessage(TRefRef MessageRef, TRefRef SenderRef) : 
		TBinaryTree(MessageRef),
		m_SenderRef(SenderRef)
	{	
	}

	FORCEINLINE TRefRef		GetMessageRef() const											{	return TBinaryTree::GetDataRef();	} // use binary tree's ref
	FORCEINLINE void		SetMessageRef(TRefRef MessageRef)								{	TBinaryTree::SetDataRef( MessageRef );	} // use binary tree's ref

	// Common message type data (header) which is optional
	FORCEINLINE void		SetSenderRef(TRefRef SenderRef)							{ m_SenderRef == SenderRef; }
	FORCEINLINE TRefRef		GetSenderRef()									const	{ return m_SenderRef; }									 	

	// Timestamp access
	TPtr<TBinaryTree>&		AddTimeStamp(TRefRef TimeStampRef);
	FORCEINLINE u32			GetTimeStamps(TRefRef TimeStampRef,TPtrArray<TBinaryTree>& TimeStamps)	{	return GetChildren( TimeStampRef, TimeStamps );	}	//	Returns all timestamps that have been added to the message

private:
	TRef					m_SenderRef;		// Sender ID
};


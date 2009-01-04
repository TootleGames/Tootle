
#pragma once

#include "TBinaryTree.h"



namespace TLMessaging
{
	static TRef g_ChannelIdRef = "ChannelID";

	class TMessage;
};


/*
	Message - a generic message type that can have arbitrary data added to it
*/
class TLMessaging::TMessage : public TBinaryTree
{
public:
	TMessage(TRefRef MessageRef) : TBinaryTree(MessageRef)									{  }
	explicit TMessage(TRefRef MessageRef, TRefRef SenderRef) : TBinaryTree(MessageRef)		{	SetSenderID(SenderRef);	}

	FORCEINLINE TRefRef		GetMessageRef() const											{	return TBinaryTree::GetDataRef();	} // use binary tree's ref
	FORCEINLINE void		SetMessageRef(TRefRef MessageRef)								{	TBinaryTree::SetDataRef( MessageRef );	} // use binary tree's ref

	// Common message type data (header) which is optional
	TPtr<TBinaryTree>&		SetSenderID(TRefRef SenderRef);
	TRef					GetSenderID();									 	

	TPtr<TBinaryTree>&		AddTimeStamp(TRefRef TimeStampRef);
	FORCEINLINE u32			GetTimeStamps(TRefRef TimeStampRef,TPtrArray<TBinaryTree>& TimeStamps)	{	return GetChildren( TimeStampRef, TimeStamps );	}	//	Returns all timestamps that have been added to the message

	TPtr<TBinaryTree>&		AddChannelID(TRefRef ChannelRef)						{	return AddChildAndData( TLMessaging::g_ChannelIdRef, ChannelRef);	}	//	Adds a new channel ID - more than one is acceptable
	void					GetChannelIDs(TArray<TRef>& ChannelRefs);
	Bool					HasChannelID(TRefRef ChannelRef);
};


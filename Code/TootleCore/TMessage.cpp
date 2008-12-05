
#include "TMessage.h"
#include "TLTime.h"

using namespace TLMessaging;

namespace TLMessaging
{
	static TRef g_SenderIDRef = "SenderID";
}

/*
	Returns the sender ID of the message
*/
TRef TMessage::GetSenderID()									 	
{ 
	TRef refID;

	TPtr<TBinaryTree>& pData = GetChild( TLMessaging::g_SenderIDRef );

	if(pData.IsValid())
		pData->Read(refID);

	return refID;
}

/*
	Adds a (singular) sender ID if one doesn't exist already
*/
TPtr<TBinaryTree>& TMessage::SetSenderID(TRefRef SenderRef)	
{
	TPtr<TBinaryTree>& pData = GetChild( TLMessaging::g_SenderIDRef );
	
	// Does the sender ID object exist already??
	if(pData.IsValid())
	{
		TLDebug_Break("TODO:  Alter the ID here or ignore it??");

		// return the first element - there should *only* ever be one unless the sender ID has been added by another means
		return pData;
	}

	return AddChildAndData( TLMessaging::g_SenderIDRef, SenderRef);
}




/*
	Adds a new timestamp to the message
*/
TPtr<TBinaryTree>& TMessage::AddTimeStamp(TRefRef TimeStampRef)			
{
	TLTime::TTimestamp tTimeStamp(TRUE);

	return AddChildAndData(TimeStampRef, tTimeStamp);
}


/*
	Returns True if the specified channel ID is assigned to this message, otherwise returns False.
*/
Bool TMessage::HasChannelID(TRefRef ChannelRef)									 	
{
	//	gr: dont enumurate all the channels, jsut search them
	TPtrArray<TBinaryTree>& DataArray = GetChildren();
	s32 ChannelDataIndex = DataArray.FindIndex( TLMessaging::g_ChannelIdRef );
	while ( ChannelDataIndex != -1 )
	{
		TPtr<TBinaryTree>& pChannelData = DataArray[ChannelDataIndex];
		if ( !pChannelData )
		{
			TLDebug_Break("Data expected");
		}
		else
		{
			pChannelData->ResetReadPos();
			const TRef* pChannelRef = pChannelData->ReadNoCopy<TRef>();
			if ( pChannelRef && *pChannelRef == ChannelRef )
				return TRUE;
		}

		//	find next channel data for match
		ChannelDataIndex = DataArray.FindIndex( TLMessaging::g_ChannelIdRef, ChannelDataIndex+1 );
	}

	//	no channel with this ref found
	return FALSE;

	/*
	//	gr: just for speed, i've put this array on the stack
	TFixedArray<TRef,TLMessaging::MaxChannels> Data;
	GetChannelIDs(Data);

	// Does our channel ID exist int he list of message channel ID's??
	if(Data.GetSize() > 0)
		return Data.Exists(ChannelRef);

	// No channel ID's
	return FALSE;
	*/
}

/*
	Returns all channel ID's specified for the message
*/
void TMessage::GetChannelIDs(TArray<TRef>& ChannelRefs)									 	
{
	TPtrArray<TBinaryTree> pArrayData;
	GetChildren(TLMessaging::g_ChannelIdRef, pArrayData);

	for(u32 uIndex = 0; uIndex < pArrayData.GetSize(); uIndex++)
	{
		TPtr<TBinaryTree>& pData = pArrayData.ElementAt(uIndex);
		if ( !pData )
			continue;

		pData->ResetReadPos();

		// If we read the channel ID correctly write it to the array of ID's
		TRef refID;
		if ( !pData->Read(refID) )
			continue;

		ChannelRefs.Add(refID);
	}
}



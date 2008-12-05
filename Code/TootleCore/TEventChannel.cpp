#include "TEventChannel.h"

namespace TLMessaging
{
	TPtr<TEventChannelManager>	g_pEventChannelManager = NULL;
}


using namespace TLMessaging;



/*
	Adds an event channel for the specified publisher
*/
Bool TEventChannelManager::RegisterEventChannel(TPublisher* pPublisher, TRef refPublisherID, TRef refChannelID)
{
	// Does the group exist?
	TPtr<TEventChannelGroup> pEventChannelGroup = FindEventChannelGroup(refPublisherID);
	TPtr<TEventChannel> pEventChannel = NULL;
	if(pEventChannelGroup.IsValid())
	{
		// Group exists.  Now check the channel to see if it already exists
		pEventChannel = pEventChannelGroup->FindEventChannel(refChannelID);

		// Event channel already exists within this group
		if(pEventChannel.IsValid())
			return FALSE;
	}
	else
	{
		// Create the group
		s32 sIndex = m_EventChannelGroups.Add(new TEventChannelGroup(refPublisherID));

		pEventChannelGroup = m_EventChannelGroups.ElementAt(sIndex);

	}

	// Create the channel for the group
	pEventChannel = pEventChannelGroup->RegisterEventChannel(refChannelID);

	if(!pEventChannel.IsValid())
		return FALSE;

	// Subscribe the newly created channel tot he publisher
	pEventChannel->SubscribeTo(pPublisher);
	
	// Send a message to all subscribers saying that a new event channel has been created
	TPtr<TLMessaging::TMessage> pMessage = new TLMessaging::TMessage("Channel");
	
	if(pMessage)
	{
		pMessage->Write(TRef("Added"));
		pMessage->Write(refPublisherID);
		pMessage->Write(refChannelID);
		PublishMessage(pMessage);
	}

	return TRUE;
}

/*
	Removes the specified event channel for the publisher
*/
Bool TEventChannelManager::UnregisterEventChannel(TPublisher* pPublisher, TRef refPublisherID, TRef refChannelID)
{
	// Does the group exist?
	TPtr<TEventChannelGroup> pEventChannelGroup = FindEventChannelGroup(refPublisherID);
	
	if(!pEventChannelGroup.IsValid())
		return FALSE;

	// Remove the channel from the group
	Bool bResult = pEventChannelGroup->UnregisterEventChannel(refChannelID);

	if(bResult)
	{
		// The channel was successfully remove - now check to see if any further channels exist in the group.  
		// If not remove the group automatically
		if(!pEventChannelGroup->HasEventChannels())
		{
			// Remove the group
			m_EventChannelGroups.Remove(pEventChannelGroup.GetObject());
			pEventChannelGroup = NULL;
		}
	}

	return bResult;
}

/*
*/
Bool TEventChannelManager::SubscribeTo(TSubscriber* pSubscriber, TRef refPublisherID, TRef refChannelID)
{
	TPtr<TEventChannel> pEventChannel = FindEventChannel(refPublisherID, refChannelID);

	if(!pEventChannel.IsValid())
		return FALSE;

	return pEventChannel->Subscribe(pSubscriber);
}

/*
*/
Bool TEventChannelManager::UnsubscribeFrom(TSubscriber* pSubscriber, TRef refPublisherID, TRef refChannelID)
{
	TPtr<TEventChannel> pEventChannel = FindEventChannel(refPublisherID, refChannelID);

	if(!pEventChannel.IsValid())
		return FALSE;

	return pEventChannel->Unsubscribe(pSubscriber);
}


Bool TEventChannelManager::BroadcastMessage(TPtr<TMessage> pData, TRef refPublisherID, TRef refChannelID)
{
	TPtr<TEventChannel> pEventChannel = FindEventChannel(refPublisherID, refChannelID);

	if(!pEventChannel.IsValid())
		return FALSE;

	pEventChannel->PublishMessage(pData);

	return TRUE;
}

/*
	Finds the event channel with the specified ID within the specified channel group
*/
TPtr<TEventChannel>	TEventChannelManager::FindEventChannel(TRef refPublisherID, TRef refChannelID)
{
	TPtr<TEventChannelGroup> pGroup = FindEventChannelGroup(refPublisherID);

	// Was the group found?
	if(!pGroup.IsValid())
		return NULL;

	// Find the event channel
	return pGroup->FindEventChannel(refChannelID);
}

/*
	Finds the event channel index with the specified ID within the specified channel group
*/
s32	TEventChannelManager::FindEventChannelIndex(TRef refPublisherID, TRef refChannelID)
{
	TPtr<TEventChannelGroup> pGroup = FindEventChannelGroup(refPublisherID);

	// Was the group found?
	if(!pGroup.IsValid())
		return -1;

	// Find the event channel
	return pGroup->FindEventChannelIndex(refChannelID);
}


/*
	Finds the event channel groupassociated witht eh specified publisher ID
*/
TPtr<TEventChannelGroup> TEventChannelManager::FindEventChannelGroup(TRef refPublisherID)
{
	//	gr: use the array functions! they will be faster than a for loop in the future :)
	return m_EventChannelGroups.FindPtr( refPublisherID );
	/*
	for(u32 uIndex = 0; uIndex < m_EventChannelGroups.GetSize(); uIndex++)
	{
		TPtr<TEventChannelGroup> pGroup = m_EventChannelGroups.ElementAt(uIndex);

		if(pGroup->GetPublisherID() == refPublisherID)
			return pGroup;
	}

	// Not found
	return NULL;
	*/
}


/*
	Finds the event channel groupassociated witht eh specified publisher ID
*/
s32 TEventChannelManager::FindEventChannelGroupIndex(TRef refPublisherID)
{
	//	gr: use the array functions! they will be faster than a for loop in the future :)
	return m_EventChannelGroups.FindIndex( refPublisherID );
/*	for(u32 uIndex = 0; uIndex < m_EventChannelGroups.GetSize(); uIndex++)
	{
		TPtr<TEventChannelGroup> pGroup = m_EventChannelGroups.ElementAt(uIndex);

		if(pGroup->GetPublisherID() == refPublisherID)
			return (s32) uIndex;
	}

	// Not found
	return -1;
	*/
}


/*
	Finds a specified event channel within the group
*/
TPtr<TEventChannel> TEventChannelGroup::FindEventChannel(TRef refChannelID)
{
	for(u32 uIndex = 0; uIndex < m_EventChannels.GetSize(); uIndex++)
	{
		TPtr<TEventChannel> pChannel = m_EventChannels.ElementAt(uIndex);

		if(pChannel->GetChannelID() == refChannelID)
			return pChannel;
	}

	// Not found
	return NULL;
}

/*
	Finds the index of the specified event channel
*/
s32 TEventChannelGroup::FindEventChannelIndex(TRef refChannelID)
{
	for(u32 uIndex = 0; uIndex < m_EventChannels.GetSize(); uIndex++)
	{
		TPtr<TEventChannel> pChannel = m_EventChannels.ElementAt(uIndex);

		if(pChannel->GetChannelID() == refChannelID)
			return (s32) uIndex;
	}

	// Not found
	return -1;
}


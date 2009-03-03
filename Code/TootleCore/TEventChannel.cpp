#include "TEventChannel.h"

namespace TLMessaging
{
	TPtr<TEventChannelManager>	g_pEventChannelManager = NULL;
}


using namespace TLMessaging;



/*
	Adds an event channel for the specified publisher
*/
Bool TEventChannelManager::RegisterEventChannel(TPublisher* pPublisher, TRefRef refPublisherID, TRefRef refChannelID)
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
		pEventChannelGroup = m_EventChannelGroups.AddPtr(new TEventChannelGroup(refPublisherID));
	}

	// Create the channel for the group
	pEventChannel = pEventChannelGroup->RegisterEventChannel(refChannelID);

	if(!pEventChannel.IsValid())
		return FALSE;

	// Subscribe the newly created channel tot he publisher
	pEventChannel->SubscribeTo(pPublisher);
	
	// Send a message to all subscribers saying that a new event channel has been created
	TLMessaging::TMessage Message("Channel");
	
	Message.Write(TRef("Added"));
	Message.Write(refPublisherID);
	Message.Write(refChannelID);
	PublishMessage(Message);

	return TRUE;
}

/*
	Removes the specified event channel for the publisher
*/
Bool TEventChannelManager::UnregisterEventChannel(TPublisher* pPublisher, TRefRef refPublisherID, TRefRef refChannelID)
{
	// Does the group exist?
	TPtr<TEventChannelGroup>& pEventChannelGroup = FindEventChannelGroup(refPublisherID);
	
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
			m_EventChannelGroups.Remove( pEventChannelGroup->GetPublisherID() );
			pEventChannelGroup = NULL;
		}
	}

	return bResult;
}

/*
*/
Bool TEventChannelManager::SubscribeTo(TSubscriber* pSubscriber, TRefRef refPublisherID, TRefRef refChannelID)
{
	TPtr<TEventChannel>& pEventChannel = FindEventChannel(refPublisherID, refChannelID);

	if(!pEventChannel.IsValid())
		return FALSE;

	return pEventChannel->Subscribe(pSubscriber);
}

/*
*/
Bool TEventChannelManager::UnsubscribeFrom(TSubscriber* pSubscriber, TRefRef refPublisherID, TRefRef refChannelID)
{
	TPtr<TEventChannel>& pEventChannel = FindEventChannel(refPublisherID, refChannelID);

	if(!pEventChannel.IsValid())
		return FALSE;

	return pEventChannel->Unsubscribe(pSubscriber);
}


Bool TEventChannelManager::BroadcastMessage(TLMessaging::TMessage& pData, TRefRef refPublisherID, TRefRef refChannelID)
{
	TPtr<TEventChannel>& pEventChannel = FindEventChannel(refPublisherID, refChannelID);

	if(!pEventChannel.IsValid())
		return FALSE;

	pEventChannel->PublishMessage(pData);

	return TRUE;
}

/*
	Finds the event channel with the specified ID within the specified channel group
*/
TPtr<TEventChannel>& TEventChannelManager::FindEventChannel(TRefRef refPublisherID, TRefRef refChannelID)
{
	TPtr<TEventChannelGroup>& pGroup = FindEventChannelGroup(refPublisherID);

	// Was the group found?
	if(!pGroup.IsValid())
		return TLPtr::GetNullPtr<TEventChannel>();

	// Find the event channel
	return pGroup->FindEventChannel(refChannelID);
}

/*
	Finds the event channel index with the specified ID within the specified channel group
*/
s32	TEventChannelManager::FindEventChannelIndex(TRefRef refPublisherID, TRefRef refChannelID)
{
	TPtr<TEventChannelGroup>& pGroup = FindEventChannelGroup(refPublisherID);

	// Was the group found?
	if(!pGroup.IsValid())
		return -1;

	// Find the event channel
	return pGroup->FindEventChannelIndex(refChannelID);
}



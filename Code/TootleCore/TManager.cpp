
#include "TManager.h"

#include "TEventChannel.h"
/*
	Process basic messages a manager will receive
*/
void TManager::ProcessMessage(TPtr<TLMessaging::TMessage>& pMessage)
{
	TRefRef MessageRef = pMessage->GetMessageRef();

	if ( MessageRef == TLCore::InitialiseRef )
	{
		// Check to see if the manager is ready - no need to call init anymore if so
		// Errors should be picked up at a higher level and the app should bail out
		if(GetState() == TLManager::S_Ready)
			return;

		SyncBool Result = Initialise();
		
		if(Result == SyncFalse)
		{
			TLCore::Platform::DoQuit();

			SetState(TLManager::S_Error);
		}
		else if (Result == SyncWait)
			SetState(TLManager::S_Initialising);
		else
			SetState(TLManager::S_Ready);
	}
	else if ( MessageRef == TLCore::UpdateRef )
	{
		//	get timestep
		float ThisTimestep = 0.f;
		float fThisTimeStepModifier = 1.0f;
		pMessage->ImportData( TLCore::TimeStepRef, ThisTimestep );
		pMessage->ImportData( TLCore::TimeStepModRef, fThisTimeStepModifier );
		
		// Check to see if this manager uses the modifier.  If so apply it tot he timestep now
		//if()
		{
			ThisTimestep *= fThisTimeStepModifier;
		}

		SyncBool Result = Update( ThisTimestep );

		if(Result == SyncTrue)
			SetState(TLManager::S_Ready);
		else
		{
			TLCore::Platform::DoQuit();
			SetState(TLManager::S_Error);
		}
	}
	else if ( MessageRef == TLCore::ShutdownRef )
	{
		// Check to see if the manager is shutdown - no need to process the shutdown message any further if so
		// Errors should be handled at a higher level and the app should quit
		if(GetState() == TLManager::S_Shutdown)
			return;

		SyncBool Result = Shutdown();

		if(Result == SyncTrue)
			SetState(TLManager::S_Shutdown);
		else if(Result == SyncFalse)
			SetState(TLManager::S_Error);
		else 
			SetState(TLManager::S_ShuttingDown);
	}
	else if( MessageRef == "Channel")
	{
		// Event channel changes
		TRef refChange;
		pMessage->Read(refChange);
		
		if(refChange == "Added")
		{
			TRef refPublisherID;
			TRef refChannelID;
			
			pMessage->Read(refPublisherID);
			pMessage->Read(refChannelID);
		
			OnEventChannelAdded(refPublisherID, refChannelID);
		}
	}

}

void TManager::OnEventChannelAdded(TRefRef refPublisherID, TRefRef refChannelID)
{
	if(refPublisherID == "CORE")
	{
		// Subscribe to the initialise messages
		if(refChannelID == TLCore::InitialiseRef)
			TLMessaging::g_pEventChannelManager->SubscribeTo(this, refPublisherID, refChannelID); 
		
		// Subscribe to the shutdown messages
		if(refChannelID == TLCore::ShutdownRef)
			TLMessaging::g_pEventChannelManager->SubscribeTo(this, refPublisherID, refChannelID); 
	}	
}




void TManager::SetState(TLManager::ManagerState NewState)
{ 
	if(m_ManagerState != NewState)
	{
		m_ManagerState = NewState; 

		// Broadcast a message telling things the managers state has changed
		TPtr<TLMessaging::TMessage> pMessage = new TLMessaging::TMessage("Manager");

		if(pMessage.IsValid())
		{
			pMessage->SetSenderID(GetManagerID());
			pMessage->AddChannelID("STATECHANGE");

			pMessage->Write(NewState);
			PublishMessage( pMessage );
		}
	}
}


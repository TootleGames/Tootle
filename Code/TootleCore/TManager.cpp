#include "TManager.h"
#include "TEventChannel.h"

#include "TLCore.h"
#include "TCoreManager.h"

//------------------------------------------------------
//	Process basic messages a manager will receive
//------------------------------------------------------
void TLCore::TManager::ProcessMessage(TLMessaging::TMessage& Message)
{
	switch ( Message.GetMessageRef().GetData() )
	{
		case TRef_Static(U,p,d,a,t):
		{
			//	get timestep
			float ThisTimestep = 0.f;
			float fThisTimeStepModifier = 1.0f;
			//Message.ImportData( TLCore::TimeStepRef, ThisTimestep );
			//Message.ImportData( TLCore::TimeStepModRef, fThisTimeStepModifier );
			Message.Read( ThisTimestep );
			Message.Read( fThisTimeStepModifier );
			
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
				TLCore::Quit();
				SetState(TLManager::S_Error);
			}
		}
		break;
			
		case TRef_Static(I,n,i,t,i):
		{
			// Check to see if the manager is ready - no need to call init anymore if so
			// Errors should be picked up at a higher level and the app should bail out
			if(GetState() == TLManager::S_Ready)
				return;
			
			SyncBool Result = Initialise();
			
			if(Result == SyncFalse)
			{
				TLCore::Quit();
				
				SetState(TLManager::S_Error);
			}
			else if (Result == SyncWait)
				SetState(TLManager::S_Initialising);
			else
				SetState(TLManager::S_Ready);
		}
		break;
			
		case TRef_Static(C,h,a,n,n):	//	"Channel"
		{
			// Event channel changes
			TRef refChange;
			Message.Read(refChange);
			
			if(refChange == TRef_Static(A,d,d,e,d))
			{
				TRef refPublisherID;
				TRef refChannelID;
				
				Message.Read(refPublisherID);
				Message.Read(refChannelID);
				
				OnEventChannelAdded(refPublisherID, refChannelID);
			}
		}
		break;
	
		case TRef_Static(S,h,u,t,d):
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
		break;

		// Reflection for setting property data on a manager via the messaging system
		case TRef_Static(S,e,t,P,r):	//	TLCore::SetPropertyRef:
		{
			SetProperty(Message);
			return;
		}

		// Reflection for getting property data from a manager via the messaging system
		case TRef_Static(G,e,t,P,r):	//	TLCore::GetPropertyRef:
		{
			TRef Manager;
			Message.ImportData(TLCore::ManagerRef, Manager);

			// Valid manager?  If so we can send a response
			if(Manager.IsValid())
			{
				TLMessaging::TMessage Response(TLCore::PropertyRef, GetManagerRef());

				// generate a response with the property information requested
				GetProperty(Message, Response);

				// Now send the response message 
				TRefRef Sender = Message.GetSenderRef();

				TLCore::g_pCoreManager->SendMessageTo(Sender, Manager, Response);
			}
			return;
		}


	}

}

void TLCore::TManager::OnEventChannelAdded(TRefRef PublisherRef, TRefRef ChannelRef)
{
	if(PublisherRef == TRef_Static4(C,o,r,e) )
	{
		// Subscribe to the initialise messages
		if(ChannelRef == TLCore::InitialiseRef)
			TLMessaging::g_pEventChannelManager->SubscribeTo(this, PublisherRef, ChannelRef); 
		
		// Subscribe to the shutdown messages
		if(ChannelRef == TLCore::ShutdownRef)
			TLMessaging::g_pEventChannelManager->SubscribeTo(this, PublisherRef, ChannelRef); 
	}	
}




void TLCore::TManager::SetState(TLManager::ManagerState NewState)
{ 
	if(m_ManagerState != NewState)
	{
		m_ManagerState = NewState; 

		// Broadcast a message telling things the managers state has changed
		TLMessaging::TMessage Message("StateChange", GetManagerRef());
		Message.Write(NewState);
		PublishMessage( Message );
	}
}


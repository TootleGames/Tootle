
#include "TAudiograph.h"
#include "TLAudio.h"
#include <TootleCore/TEventChannel.h>

namespace TLAudio
{
	TPtr<TAudiograph> g_pAudiograph;
};


using namespace TLAudio;

/*
TAudioNode* TAudioNodeFactory::CreateObject(TRefRef InstanceRef,TRefRef TypeRef)
{
	// Create engine/middleware side audio nodes
	if(TypeRef == "Audio")
		return new TAudioNode(InstanceRef,TypeRef);
	
	return NULL;
}
*/



TAudiograph::TAudiograph(TRefRef refManagerID) :
	TLGraph::TGraph<TLAudio::TAudioNode>	( refManagerID )
{
}



SyncBool TAudiograph::Initialise()
{
	if(TLMessaging::g_pEventChannelManager)
	{
		if(InitDevices() != SyncTrue)
			return SyncFalse;

		TLMessaging::g_pEventChannelManager->RegisterEventChannel(this, "AUDIOGRAPH", "START");
		TLMessaging::g_pEventChannelManager->RegisterEventChannel(this, "AUDIOGRAPH", "PAUSE");
		TLMessaging::g_pEventChannelManager->RegisterEventChannel(this, "AUDIOGRAPH", "STOP");

		return TLGraph::TGraph<TLAudio::TAudioNode>::Initialise();

		/*
		if(TLGraph::TGraph<TLAudio::TAudioNode>::Initialise() == SyncTrue)
		{	
		
			//	create generic render node factory
			TPtr<TClassFactory<TLAudio::TAudioNode,FALSE> > pFactory = new TAudioNodeFactory();
			AddFactory(pFactory);
			
			return SyncTrue;
		}
		 */
		
	}

	return SyncWait;
}

SyncBool TAudiograph::Update(float fTimeStep)
{
	TArray<TRef> refArray;

	if(Platform::DetermineFinishedAudio(refArray))
	{
		// Send out notification of each audio item that has come to an end
		for(u32 uIndex = 0; uIndex < refArray.GetSize(); uIndex++)
		{
			TRef AudioRef = refArray.ElementAt(uIndex);

			TPtr<TLMessaging::TMessage> pMessage = new TLMessaging::TMessage("AUDIO");

			if(pMessage)
			{
				pMessage->AddChannelID("STOP");
				pMessage->Write(AudioRef);

				PublishMessage(pMessage);
			}
			else
			{
				TLDebug_Break("Failed to create message for audio stop event so audio may be left in memory");
			}
		}
	}

	return TGraph::Update(fTimeStep);
}

SyncBool TAudiograph::Shutdown()
{
	if(Platform::Shutdown() != SyncTrue)
		return SyncFalse;

	return TLGraph::TGraph<TLAudio::TAudioNode>::Shutdown();
}


void TAudiograph::ProcessMessage(TPtr<TLMessaging::TMessage>& pMessage)
{
	// Super class process message
	TLGraph::TGraph<TLAudio::TAudioNode>::ProcessMessage(pMessage);
}

SyncBool TAudiograph::InitDevices()
{
	return TLAudio::Platform::Init();
}


	
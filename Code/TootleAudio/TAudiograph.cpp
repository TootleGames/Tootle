
#include "TAudiograph.h"
#include "TLAudio.h"
#include <TootleCore/TEventChannel.h>

namespace TLAudio
{
	TPtr<TAudiograph> g_pAudiograph;
};


using namespace TLAudio;


TAudiograph::TAudiograph(TRefRef refManagerID) :
	TLGraph::TGraph<TLAudio::TAudioNode>	( refManagerID )
{
}



SyncBool TAudiograph::Initialise()
{
	if(TLMessaging::g_pEventChannelManager)
	{
		//TLMessaging::g_pEventChannelManager->RegisterEventChannel(this, GetManagerID(), "ACTION");
		//TLMessaging::g_pEventChannelManager->RegisterEventChannel(this, GetManagerID(), "DEVICE");

		if(InitDevices() != SyncTrue)
			return SyncFalse;

		return TLGraph::TGraph<TLAudio::TAudioNode>::Initialise();
	}

	return SyncWait;
}

SyncBool TAudiograph::Update(float fTimeStep)
{
	return SyncTrue;
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


	
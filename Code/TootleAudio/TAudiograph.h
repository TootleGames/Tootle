
#pragma once

#include <TootleCore/TLGraph.h>
#include "TAudioNode.h"

namespace TLAudio
{
	class TAudiograph;

	extern TPtr<TAudiograph> g_pAudiograph;
};

/*
	TAudiograph class
*/
class TLAudio::TAudiograph : public TLGraph::TGraph<TLAudio::TAudioNode>
{
public:
	TAudiograph(TRefRef refManagerID);

protected:
	virtual SyncBool		Initialise();
	virtual SyncBool		Update(float fTimeStep);
	virtual SyncBool		Shutdown();

	virtual void			ProcessMessage(TPtr<TLMessaging::TMessage>& pMessage);

	SyncBool				InitDevices();
};
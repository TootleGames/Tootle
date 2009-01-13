
#pragma once

#include <TootleCore/TLGraph.h>
#include "TAudioNode.h"

namespace TLAudio
{
	class TAudiograph;
	//class TAudioNodeFactory;


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


/*
// [12/01/09] DB - NOT NEEDED JUST YET
//----------------------------------------------------------
//	Generic physics node factory
//----------------------------------------------------------
class TLAudio::TAudioNodeFactory : public TClassFactory<TAudioNode,FALSE>
{
public:
	virtual TAudioNode*	CreateObject(TRefRef InstanceRef,TRefRef TypeRef);
};
*/

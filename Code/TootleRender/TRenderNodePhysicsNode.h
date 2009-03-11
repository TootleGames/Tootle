/*------------------------------------------------------

	Render node that renders stuff on a physics node

-------------------------------------------------------*/
#pragma once

#include "TRenderNode.h"
#include "TRenderNodeDebugMesh.h"
#include <TootleCore/TSubscriber.h>


namespace TLPhysics
{
	class TPhysicsNode;
}

namespace TLRender
{
	class TRenderNodePhysicsNode;
}


//----------------------------------------------------
//	
//----------------------------------------------------
class TLRender::TRenderNodePhysicsNode : public TLRender::TRenderNodeDebugMesh
{
public:
	TRenderNodePhysicsNode(TRefRef RenderNodeRef,TRefRef TypeRef);

	virtual void			Initialise(TLMessaging::TMessage& Message);
	virtual void			Shutdown()									{	SetPhysicsNode( TRef() );	}

	void					SetPhysicsNode(TRefRef PhysicsNodeRef);		//	change the physics node we're monitoring

protected:
	virtual void			ProcessMessage(TLMessaging::TMessage& Message);
	void					OnPhysicsNodeChanged(TLPhysics::TPhysicsNode& PhysicsNode);	//	rebuild the mesh when the physics details change
	
	SyncBool				SubscribeToPhysicsNode();		//	keep trying to subscribe to physics node if we need to - returns wait if we still need to

public:
	TRef					m_PhysicsNodeRef;	//	node we're debugging
	Bool					m_Subscribed;		//	subscribed to node... if failed, and node ref is valid then keep trying to subscribe
};



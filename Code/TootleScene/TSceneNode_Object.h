#pragma once

#include <TootleRender/TLRender.h>
#include <TootlePhysics/TLPhysics.h>
#include <TootleAudio/TLAudio.h>

#include "TSceneNode_Transform.h"

namespace TLScene
{
	class TSceneNode_Object;
};


//	use this flag with OnTransformChanged to NOT pass the change [back]onto the physics node. ie. OR this flag when the physics changes our transform
#define TLSceneNodeObject_FromPhysicsTransform		(TLMaths_TransformBitScale<<1)	//	next availible flag

//-----------------------------------------------------
//	The Object is our basic node type that is linked to a
//	render node and/or physics node so does some quite 
//	generic routines (update to match physics pos, then 
//	updates render object)
//-----------------------------------------------------
class TLScene::TSceneNode_Object : public TLScene::TSceneNode_Transform
{
public:
	TSceneNode_Object(TRefRef NodeRef,TRefRef TypeRef);
	~TSceneNode_Object();

	virtual Bool					HasRender()		{ return TRUE; }
	virtual Bool					HasPhysics()	{ return TRUE; }

	// Distance checks
	virtual float					GetDistanceTo(const TLMaths::TLine& Line);

	// Physics Object access
	virtual TRef					GetPhysicsNodeRef() const					{ return m_PhysicsNodeRef; }
	TPtr<TLPhysics::TPhysicsNode>&	GetPhysicsNode(Bool InitialisedOnly=FALSE);

	// Render object access
	virtual TRef					GetRenderNodeRef() const					{ return m_RenderNodeRef; }
	TPtr<TLRender::TRenderNode>&	GetRenderNode(Bool InitialisedOnly=FALSE);

protected:
	virtual void					Initialise(TLMessaging::TMessage& Message);
	virtual void					SetProperty(TLMessaging::TMessage& Message);
	virtual void					Shutdown();
	virtual void					ProcessMessage(TLMessaging::TMessage& Message);

	virtual void					OnTransformChanged(u8 TransformChangedBits);	//	this checks to see if we're asleep first and delays sending a transform until we are awake. gr: see TLSceneNodeObject_FromPhysicsTransform
	
	virtual void					OnZoneWake(SyncBool ZoneActive);		//	re-enable physics and render nodes
	virtual void					OnZoneSleep();							//	disable physics and render nodes

	virtual Bool					CreatePhysicsNode(TRefRef PhysicsNodeType=TRef(),TLMessaging::TMessage* pInitMessage=NULL);
	virtual void					OnPhysicsNodeAdded(TPtr<TLPhysics::TPhysicsNode>& pPhysicsNode);
	void							DeletePhysicsNode();
	virtual void					OnPhysicsNodeRemoved(TRefRef PhysicsNodeRef)			{}	//	called when we get a message from the graph that our node has been removed - NOT invoked by DeletePhysicsNode()
	void							EnablePhysicsNode(Bool Enable,SyncBool EnableCollision=SyncWait);		//	enable/disable physics node - can seperately enable collision, syncwait doesn't change collision setting
	void							EnablePhysicsNodeCollision(Bool Enable);								//	enable/disable physics node's collisin

	FORCEINLINE Bool				CreateRenderNode(TPtr<TLRender::TRenderNode> pParentRenderNode)	{	return CreateRenderNode( pParentRenderNode ? pParentRenderNode->GetNodeRef() : TRef() );	}
	virtual Bool					CreateRenderNode(TRefRef ParentRenderNode,TRefRef RenderNodeType=TRef(),TLMessaging::TMessage* pInitMessage=NULL);
	virtual void					OnRenderNodeAdded(TPtr<TLRender::TRenderNode>& pRenderNode);
	void							DeleteRenderNode();
	virtual void					OnRenderNodeRemoved(TRefRef RenderNodeRef)			{}	//	called when we get a message from the graph that our node has been removed - NOT invoked by DeleteRenderNode()
	void							EnableRenderNode(Bool Enable);						//	enable/disable render node

	TRef							CreateAudioNode(TRefRef AudioRef, TRefRef AudioAsset);
	TRef							CreateAudioNode(TRefRef AudioRef, TRefRef AudioAsset, const TLAudio::TAudioProperties& Props);
	Bool							RemoveAudioNode(TRefRef AudioRef);

	void							Debug_EnableRenderDebugPhysics(Bool Enable);	//	turn on/off debug render node for physics

private:
	TRef					m_RenderNodeRef;
	TRef					m_PhysicsNodeRef;
	TRef					m_Debug_RenderDebugPhysicsNodeRef;	//	debug rendernode to render our physics
	u8						m_PublishTransformOnWake;		//	TTransform bitmask - true if our transform was changed by the physics whilst we were asleep. When we wake, we send our latest transform to the render node
	
	SyncBool				m_OnEditPhysicsWasEnabled;			//	when we start editing this node we disable the physics, this is what it was set to before so we don't enable a previously disabled node. If Wait then we haven't initialised this so shouldn't be restoring it
};


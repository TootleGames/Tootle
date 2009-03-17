#pragma once

#include <TootleRender/TLRender.h>
#include <TootlePhysics/TLPhysics.h>
#include <TootleAudio/TLAudio.h>

#include "TSceneNode_Transform.h"

namespace TLScene
{
	class TSceneNode_Object;
};



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

	virtual Bool	HasRender()		{ return TRUE; }
	virtual Bool	HasPhysics()	{ return TRUE; }

	virtual void	Shutdown();

	//	explicit changes change the physics nodes transform
	virtual void					SetTransform(const TLMaths::TTransform& Transform);
	virtual void					SetTranslate(const float3& Translate);
	virtual void					SetRotation(const TLMaths::TQuaternion& Rotation);
	virtual void					SetScale(const float3& Scale);

	// Distance checks
	virtual float					GetDistanceTo(const TLMaths::TLine& Line);

	// Physics Object access
	TRefRef							GetPhysicsNodeRef()					{ return m_PhysicsNodeRef; }
	TPtr<TLPhysics::TPhysicsNode>&	GetPhysicsNode(Bool InitialisedOnly=FALSE);

	// Render object access
	TRefRef							GetRenderNodeRef()					{ return m_RenderNodeRef; }
	TPtr<TLRender::TRenderNode>&	GetRenderNode(Bool InitialisedOnly=FALSE);

protected:
	virtual void					ProcessMessage(TLMessaging::TMessage& Message);

	virtual Bool					CreatePhysicsNode(TRefRef PhysicsNodeType=TRef());
	virtual void					OnPhysicsNodeAdded(TPtr<TLPhysics::TPhysicsNode>& pPhysicsNode);
	void							DeletePhysicsNode();
	virtual void					OnPhysicsNodeRemoved(TRefRef PhysicsNodeRef)			{}	//	called when we get a message from the graph that our node has been removed - NOT invoked by DeletePhysicsNode()
	void							EnablePhysicsNode(Bool Enable,Bool EnableCollision);		//	enable/disable physics node - can seperately enable collision

	FORCEINLINE Bool				CreateRenderNode(TPtr<TLRender::TRenderNode> pParentRenderNode)	{	return CreateRenderNode( pParentRenderNode ? pParentRenderNode->GetNodeRef() : TRef() );	}
	virtual Bool					CreateRenderNode(TRefRef ParentRenderNode,TLMessaging::TMessage* pInitMessage=NULL);
	virtual void					OnRenderNodeAdded(TPtr<TLRender::TRenderNode>& pRenderNode);
	void							DeleteRenderNode();
	virtual void					OnRenderNodeRemoved(TRefRef RenderNodeRef)			{}	//	called when we get a message from the graph that our node has been removed - NOT invoked by DeleteRenderNode()
	void							EnableRenderNode(Bool Enable);						//	enable/disable render node

	TRef							CreateAudioNode(TRefRef AudioRef, TRefRef AudioAsset);
	TRef							CreateAudioNode(TRefRef AudioRef, TRefRef AudioAsset, const TLAudio::TAudioProperties& Props);
	Bool							RemoveAudioNode(TRefRef AudioRef);

	// Transformation
	virtual void					Translate(float3 vTranslation);
	virtual void					OnTransformChanged(Bool bTranslation, Bool bRotation, Bool bScale);	//	this checks to see if we're asleep first and delays sending a transform until we are awake
	
	virtual void					OnZoneWake(SyncBool ZoneActive);		//	re-enable physics and render nodes
	virtual void					OnZoneSleep();							//	disable physics and render nodes

private:
	TRef					m_RenderNodeRef;
	TRef					m_PhysicsNodeRef;
	u8						m_PublishTransformOnWake;		//	TTransform bitmask - true if our transform was changed by the physics whilst we were asleep. When we wake, we send our latest transform to the render node
};


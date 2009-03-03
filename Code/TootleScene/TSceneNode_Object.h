#pragma once

#include <TootleRender/TLRender.h>
#include <TootlePhysics/TLPhysics.h>
#include <TootleAudio/TAudioInterface.h>

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
class TLScene::TSceneNode_Object : public TLScene::TSceneNode_Transform, public TLAudio::TAudioInterface
{
public:
	TSceneNode_Object(TRefRef NodeRef,TRefRef TypeRef);

	virtual Bool	HasRender()		{ return TRUE; }
	virtual Bool	HasPhysics()	{ return TRUE; }

	virtual void	Shutdown();

	// Distance checks
	virtual float				GetDistanceTo(const TLMaths::TLine& Line);

	void						SetAllNodesTranslate(const float3& Translate);

	// Physics Object access
	TRefRef							GetPhysicsNodeRef()					{ return m_PhysicsNodeRef; }
	TPtr<TLPhysics::TPhysicsNode>&	GetPhysicsNode();

	// Render object access
	TRefRef							GetRenderNodeRef()					{ return m_RenderNodeRef; }
	TPtr<TLRender::TRenderNode>&	GetRenderNode();

protected:

	virtual Bool					CreatePhysicsNode(TRefRef PhysicsNodeType=TRef());
	virtual void					OnPhysicsNodeAdded(TPtr<TLPhysics::TPhysicsNode>& pPhysicsNode)	{}
	void							DeletePhysicsNode();

	FORCEINLINE Bool				CreateRenderNode(TPtr<TLRender::TRenderNode> pParentRenderNode)	{	return CreateRenderNode( pParentRenderNode ? pParentRenderNode->GetNodeRef() : TRef() );	}
	virtual Bool					CreateRenderNode(TRefRef ParentRenderNode,TLMessaging::TMessage* pInitMessage=NULL);
	virtual void					OnRenderNodeAdded(TPtr<TLRender::TRenderNode>& pRenderNode)	{}
	void							DeleteRenderNode();

	virtual void 					Update(float fTimestep);
	virtual void					UpdateObjectFromPhysics();				//	update game object to match physics node

	// Transformation
	virtual void					Translate(float3 vTranslation);

protected:
	void					SetRenderNode(TRefRef RenderNodeRef)		{	m_RenderNodeRef = RenderNodeRef;	}
	void					SetPhysicsObject(TRefRef PhysicsNodeRef)	{	m_PhysicsNodeRef = PhysicsNodeRef;	}

private:
	TRef					m_RenderNodeRef;
	TRef					m_PhysicsNodeRef;
};


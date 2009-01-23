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
	TSceneNode_Object(TRefRef NodeRef,TRefRef TypeRef) :
	  TSceneNode_Transform	(NodeRef,TypeRef)
	{
	}

	virtual Bool	HasRender()		{ return TRUE; }
	virtual Bool	HasPhysics()	{ return TRUE; }

	virtual void	Shutdown();

	// Distance checks
	virtual float				GetDistanceTo(const TLMaths::TLine& Line);

	void						SetAllNodesTranslate(const float3& Translate);

	// Physics Object access
	TRefRef							GetPhysicsObjectRef()					{ return m_PhysicsObjectRef; }
	TPtr<TLPhysics::TPhysicsNode>&	GetPhysicsObject();

	// Render object access
	TRefRef							GetRenderNodeRef()					{ return m_RenderNodeRef; }
	TPtr<TLRender::TRenderNode>&	GetRenderNode();

protected:

	virtual Bool					CreatePhysicsObject();
	virtual void					OnPhysicsObjectAdded(TPtr<TLPhysics::TPhysicsNode>& pPhysicsObject)	{}

	virtual Bool					CreateRenderNode(TPtr<TLRender::TRenderNode> pParentRenderNode = NULL);
	virtual Bool					CreateRenderNode(TRefRef ParentRenderNode);
	virtual void					OnRenderNodeAdded(TPtr<TLRender::TRenderNode>& pRenderNode)	{}

	virtual void 					Update(float fTimestep);
	virtual void					UpdateObjectFromPhysics();				//	update game object to match physics node

	// Transformation
	virtual void					Translate(float3 vTranslation);

protected:
	void					SetRenderNode(TRefRef RenderNodeRef)		{	m_RenderNodeRef = RenderNodeRef;	}
	void					SetPhysicsObject(TRefRef PhysicsObjectRef)	{	m_PhysicsObjectRef = PhysicsObjectRef;	}

private:
	TRef					m_RenderNodeRef;
	TRef					m_PhysicsObjectRef;
};


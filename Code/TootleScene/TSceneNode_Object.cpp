#include "TSceneNode_Object.h"



using namespace TLScene;


void TSceneNode_Object::Shutdown()
{
	TPtr<TLPhysics::TPhysicsNode>& pPhysics = GetPhysicsObject();

	if(pPhysics)
	{
		TLPhysics::g_pPhysicsgraph->RemoveNode(pPhysics);
	}

	TPtr<TLRender::TRenderNode>& pRender = GetRenderNode();

	if(pRender)
	{
		TLRender::g_pRendergraph->RemoveNode(pRender);
	}
}



TPtr<TLPhysics::TPhysicsNode>& TSceneNode_Object::GetPhysicsObject()
{
	return TLPhysics::g_pPhysicsgraph->FindNode( m_PhysicsObjectRef );
}

TPtr<TLRender::TRenderNode>& TSceneNode_Object::GetRenderNode()
{
	return TLRender::g_pRendergraph->FindNode( m_RenderNodeRef );
}

//--------------------------------------------------------
//	Requests a physics node to be created
//--------------------------------------------------------
Bool TSceneNode_Object::CreatePhysicsObject()
{
	TPtr<TLPhysics::TPhysicsNode> pPhysicsObject = new TLPhysics::TPhysicsNode( GetNodeRef() );
	if ( TLPhysics::g_pPhysicsgraph->AddNode( pPhysicsObject ) )
	{
		SetPhysicsObject(pPhysicsObject->GetNodeRef());

		// NOTE: This may eventually become an async type routine
		//		 so no guarantees it happens on the same frmae of creation
		OnPhysicsObjectAdded(pPhysicsObject);

		return TRUE;
	}

	return FALSE;
}


/*
	Requests a render object to be created
*/
Bool TSceneNode_Object::CreateRenderNode(TPtr<TLRender::TRenderNode> pParentRenderNode)
{
	//TLMemory::Platform::MemValidate();
	TPtr<TLRender::TRenderNode> pRenderNode = new TLRender::TRenderNode( GetNodeRef() );
	//TLMemory::Platform::MemValidate();

	if ( TLRender::g_pRendergraph->AddNode( pRenderNode, pParentRenderNode ) )
	{
	//TLMemory::Platform::MemValidate();
		SetRenderNode(pRenderNode->GetNodeRef());
	//TLMemory::Platform::MemValidate();

		// NOTE: This may eventually become an async type routine
		//		 so no guarantees it happens on the same frmae of creation
		OnRenderNodeAdded(pRenderNode);
	//TLMemory::Platform::MemValidate();

		return TRUE;
	}

	return FALSE;
}


void TSceneNode_Object::DoUpdate(float fTimestep)
{
	// Update the pbject using phsycis
	UpdateObjectFromPhysics();

	// Super class do update
	TSceneNode_Transform::DoUpdate(fTimestep);
}


void TSceneNode_Object::UpdateObjectFromPhysics()
{
	TPtr<TLPhysics::TPhysicsNode>& pPhysicsNode = GetPhysicsObject();
	TPtr<TLRender::TRenderNode>& pRenderNode = GetRenderNode();

	//	update game object from physics node
	if ( pPhysicsNode )
	{
		//	set pos to (new) physics pos
		SetTransform( pPhysicsNode->GetRenderTransform() );
	}

	//	update render node from game object
	if ( pRenderNode )
	{
		pRenderNode->SetTransform( GetTransform() );
	}
}


void TSceneNode_Object::Translate(float3 vTranslation)
{
	// Manipulate the physics by adding a force in the direction required
	TPtr<TLPhysics::TPhysicsNode>& pPhysicsNode = GetPhysicsObject();

	//	update game object from physics node
	if ( pPhysicsNode )
	{
		// If we add a force it doesn;t necessarily move the object how we want when running
		// and when paused the physics doesn;t update.
		//pPhysicsNode->AddForce(vTranslation);

		// Set the position explicitly using the delta
		float3 vPos = pPhysicsNode->GetPosition();

		vPos += vTranslation;

		pPhysicsNode->SetPosition(vPos);

	}
	else
	{
		// No physics?
		TSceneNode_Transform::Translate(vTranslation);
	}
}


float TSceneNode_Object::GetDistanceTo(const TLMaths::TLine& Line)
{
	float3 vPos = GetPosition();

	// Do distance check from node to line
	float fDistance = Line.GetDistanceSq(vPos);

	/*
	// Check the physics if the node has any
	TPtr<TLPhysics::TPhysicsNode> pPhysicsNode = pNodeObject->GetPhysics();

	if(pPhsicsNode)
	{
	}
	*/

	TPtr<TLRender::TRenderNode>& pRenderNode = GetRenderNode();

	if(pRenderNode)
	{
		const TLMaths::TBox& Bounds = pRenderNode->GetWorldBoundsBox();

		if(Bounds.GetIntersection(Line))
		{
			float fBoundsDistance = Bounds.GetDistanceSq(Line);

			if(fBoundsDistance < fDistance)
				fDistance = fBoundsDistance;
		}
	}

	return fDistance;
}


	
void TSceneNode_Object::SetAllNodesTranslate(const float3& Translate)
{
	TPtr<TLRender::TRenderNode>& pRenderNode = GetRenderNode();
	if ( pRenderNode )
		pRenderNode->SetTranslate( Translate );

	TPtr<TLPhysics::TPhysicsNode>& pPhysicsNode = GetPhysicsObject();
	if ( pPhysicsNode )
		pPhysicsNode->SetPosition( Translate );

	SetTranslate( Translate );
}


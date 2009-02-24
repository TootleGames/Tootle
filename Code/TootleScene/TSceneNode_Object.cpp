#include "TSceneNode_Object.h"



using namespace TLScene;

TSceneNode_Object::TSceneNode_Object(TRefRef NodeRef,TRefRef TypeRef) :
	TSceneNode_Transform	(NodeRef,TypeRef)
{
}




void TSceneNode_Object::Shutdown()
{
	DeletePhysicsNode();

	DeleteRenderNode();
}


void TSceneNode_Object::DeletePhysicsNode()
{
	if ( m_PhysicsObjectRef.IsValid() )
	{
#ifdef _DEBUG
		TTempString Debug_String("Attempting to remove physics for node ");
		GetNodeRef().GetString( Debug_String );
		TLDebug_Print(Debug_String);
#endif
		
		TLPhysics::g_pPhysicsgraph->RemoveNode(m_PhysicsObjectRef);
		m_PhysicsObjectRef.SetInvalid();
	}
}


void TSceneNode_Object::DeleteRenderNode()
{
	if ( m_RenderNodeRef.IsValid() )
	{		
#ifdef _DEBUG
		TTempString Debug_String("Attempting to remove render object for node ");
		GetNodeRef().GetString( Debug_String );
		TLDebug_Print(Debug_String);
#endif
		
		TLRender::g_pRendergraph->RemoveNode(m_RenderNodeRef);
		m_RenderNodeRef.SetInvalid();
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
	// [20/02/09] DB - Currently we only allow one physics node to be associated with the object node
	if(!m_PhysicsObjectRef.IsValid())
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
	}

	return FALSE;
}


//--------------------------------------------------------
//	Requests a render object to be created
//--------------------------------------------------------
Bool TSceneNode_Object::CreateRenderNode(TRefRef ParentRenderNodeRef,TPtr<TLMessaging::TMessage> pInitMessage)
{
	// [20/02/09] DB - Currently we only allow one render node to be associated with the object node
	if( m_RenderNodeRef.IsValid() )
		return FALSE;

	//	no init message? make one up
	if ( !pInitMessage )
	{
		pInitMessage = new TLMessaging::TMessage( TLCore::InitialiseRef );

		//	add transform info from this scene node
		const TLMaths::TTransform& Transform = GetTransform();

		if ( Transform.HasTranslate() )
			pInitMessage->AddChildAndData("Translate", Transform.GetTranslate() );

		if ( Transform.HasScale() )
			pInitMessage->AddChildAndData("Scale", Transform.GetScale() );

		if ( Transform.HasRotation() )
			pInitMessage->AddChildAndData("Rotation", Transform.GetRotation() );
	}

	//	gr: specify in params?
	TRef RenderNodeType = TRef();

	//	create node
	m_RenderNodeRef = TLRender::g_pRendergraph->CreateNode( GetNodeRef(), RenderNodeType, ParentRenderNodeRef, pInitMessage );

	//	failed
	if ( !m_RenderNodeRef.IsValid() )
		return FALSE;

	// NOTE: This may eventually become an async type routine
	//		 so no guarantees it happens on the same frmae of creation
	TPtr<TLRender::TRenderNode>& pRenderNode = TLRender::g_pRendergraph->FindNode( m_RenderNodeRef );
	OnRenderNodeAdded( pRenderNode );

	return TRUE;
}


void TSceneNode_Object::Update(float fTimestep)
{
	//	do base update
	TSceneNode_Transform::Update(fTimestep);

	// Update the object using phsyics
	UpdateObjectFromPhysics();
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


#include "TSceneNode_Object.h"
#include <TootlePhysics/TPhysicsGraph.h>


#define DISABLE_RENDER_ON_SLEEP


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
	if ( m_PhysicsNodeRef.IsValid() )
	{
#ifdef _DEBUG
		TTempString Debug_String("Attempting to remove physics for node ");
		GetNodeRef().GetString( Debug_String );
		TLDebug_Print(Debug_String);
#endif
		
		TLPhysics::g_pPhysicsgraph->RemoveNode(m_PhysicsNodeRef);
		m_PhysicsNodeRef.SetInvalid();
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


void TSceneNode_Object::ProcessMessage(TLMessaging::TMessage& Message)
{
	//	gr: apply change from physics node ONLY
	if(Message.GetMessageRef() == "OnTransform" && Message.GetSenderRef() == m_PhysicsNodeRef )
	{
		float3 vVector;
		TLMaths::TQuaternion qRot;
		Bool bTranslation, bRotation, bScale;
		bTranslation = bRotation = bScale = FALSE;

		// Absolute position/rotation/scale setting
		if(Message.ImportData("Translate", vVector))
		{
			GetTransform().SetTranslate(vVector);
			bTranslation = TRUE;
		}

		if(Message.ImportData("Rotation", qRot))
		{
			GetTransform().SetRotation(qRot);
			bRotation = TRUE;
		}

		if(Message.ImportData("Scale", vVector))
		{
			GetTransform().SetScale(vVector);
			bScale = TRUE;
		}

		// If any part of the transform changed forward the message on
		if(bTranslation || bRotation || bScale)
		{
			//	send out notification our transform has changed
			OnTransformChanged(bTranslation, bRotation, bScale);
		}
		
	}
	else if ( Message.GetMessageRef() == "NodeAdded" )	//	catch when our render node or physics node has been added to the graph
	{
		//	read node, graph, and added/removed state
		TRef NodeRef;
		
		if( !Message.Read( NodeRef ) )
		{
			TLDebug_Break("Error reading graph change message");
			return;
		}

		// Get the graph reference
		TRef GraphRef = Message.GetSenderRef();

		if(NodeRef == m_RenderNodeRef && GraphRef == "RenderGraph" )
		{
			//	gr: change OnRenderNodeAdded to just take a TRef so functions that don't overload it skip this node search... or just make the callback an option to save the original subscription
			TPtr<TLRender::TRenderNode>& pRenderNode = TLRender::g_pRendergraph->FindNode( NodeRef );
			OnRenderNodeAdded( pRenderNode );
			
			//	no need to subscribe to this graph any more
			this->UnsubscribeFrom( TLRender::g_pRendergraph );
			return;
		}
		else if(NodeRef == m_PhysicsNodeRef && GraphRef == "PhysicsGraph" )
		{
			//	gr: change OnRenderNodeAdded to just take a TRef so functions that don't overload it skip this node search... or just make the callback an option to save the original subscription
			TPtr<TLPhysics::TPhysicsNode>& pPhysicsNode = TLPhysics::g_pPhysicsgraph->FindNode( NodeRef );
			OnPhysicsNodeAdded( pPhysicsNode );
			
			//	no need to subscribe to this graph any more
			this->UnsubscribeFrom( TLPhysics::g_pPhysicsgraph );
			return;
		}
	}
	else if( Message.GetMessageRef() == "NodeRemoved" )
	{
		//	read node, graph, and added/removed state
		TRef NodeRef;

		if ( !Message.Read( NodeRef ) )
		{
			TLDebug_Break("Error reading graph change message");
			return;
		}

		TRef GraphRef = Message.GetSenderRef();

		if(NodeRef == m_RenderNodeRef && GraphRef == "RenderGraph")
		{
			m_RenderNodeRef.SetInvalid();
			OnRenderNodeRemoved( NodeRef );
			return;
		}
		else if ( NodeRef == m_PhysicsNodeRef && GraphRef == "PhysicsGraph" )
		{
			m_PhysicsNodeRef.SetInvalid();
			OnPhysicsNodeRemoved( NodeRef );
			return;
		}
	}

	//	do normal process
	TSceneNode_Transform::ProcessMessage( Message );
}


TPtr<TLPhysics::TPhysicsNode>& TSceneNode_Object::GetPhysicsNode(Bool InitialisedOnly)
{
	return TLPhysics::g_pPhysicsgraph->FindNode( m_PhysicsNodeRef, !InitialisedOnly );
}

TPtr<TLRender::TRenderNode>& TSceneNode_Object::GetRenderNode(Bool InitialisedOnly)
{
	return TLRender::g_pRendergraph->FindNode( m_RenderNodeRef, !InitialisedOnly );
}

//--------------------------------------------------------
//	Requests a physics node to be created
//--------------------------------------------------------
Bool TSceneNode_Object::CreatePhysicsNode(TRefRef PhysicsNodeType)
{
	// [20/02/09] DB - Currently we only allow one render node to be associated with the object node
	if( m_PhysicsNodeRef.IsValid() )
		return FALSE;

	TLMessaging::TMessage Message( TLCore::InitialiseRef );

	//	add transform info from this scene node
	const TLMaths::TTransform& Transform = GetTransform();

	if ( Transform.HasTranslate() )
		Message.AddChildAndData("Translate", Transform.GetTranslate() );

	if ( Transform.HasScale() )
		Message.AddChildAndData("Scale", Transform.GetScale() );

	if ( Transform.HasRotation() )
		Message.AddChildAndData("Rotation", Transform.GetRotation() );

	//	export ownership info
	Message.ExportData("Owner", GetNodeRef());

	//	if we're in a zone, and the zone is asleep, disable node at initialise
	TPtr<TLMaths::TQuadTreeZone>& pZone = GetZone();
	if ( pZone && !pZone->IsActive() )
	{
		Message.ExportData("PFClear", TLPhysics::TPhysicsNode::Flag_Enabled );
	}

	//	create node
	TRef ParentNode = TRef();

	m_PhysicsNodeRef = TLPhysics::g_pPhysicsgraph->CreateNode( GetNodeRef(), PhysicsNodeType, ParentNode, &Message );

	//	failed
	if ( !m_PhysicsNodeRef.IsValid() )
		return FALSE;

	//	subscribe to the physics graph to catch when our node has been added
	this->SubscribeTo( TLPhysics::g_pPhysicsgraph );

	return TRUE;
}


//--------------------------------------------------------
//	Requests a render object to be created
//--------------------------------------------------------
Bool TSceneNode_Object::CreateRenderNode(TRefRef ParentRenderNodeRef,TLMessaging::TMessage* pInitMessage)
{
	// [20/02/09] DB - Currently we only allow one render node to be associated with the object node
	if( m_RenderNodeRef.IsValid() )
		return FALSE;

	//	gr: specify in params?
	TRef RenderNodeType = TRef();

	//	gr; changed to "merge" messages
	TLMessaging::TMessage TempMessage( TLCore::InitialiseRef );

	//	no init message? make one up
	if ( !pInitMessage )
	{
		pInitMessage = &TempMessage;
	}

	//	add standard scene node object stuff
	//	add transform info from this scene node
	if ( GetTransform().HasAnyTransform() )
	{
		const TLMaths::TTransform& Transform = GetTransform();

		if ( Transform.HasTranslate() )
			pInitMessage->AddChildAndData("Translate", Transform.GetTranslate() );

		if ( Transform.HasScale() )
			pInitMessage->AddChildAndData("Scale", Transform.GetScale() );

		if ( Transform.HasRotation() )
			pInitMessage->AddChildAndData("Rotation", Transform.GetRotation() );
	}

	//	export scene-node-ownership info
	pInitMessage->ExportData("Owner", GetNodeRef());

	//	if we're in a zone, and the zone is asleep, disable node at initialise
	TPtr<TLMaths::TQuadTreeZone>& pZone = GetZone();
	if ( pZone && !pZone->IsActive() )
	{
		pInitMessage->ExportData("RFClear", TLRender::TRenderNode::RenderFlags::Enabled );
	}

	//	create node
	m_RenderNodeRef = TLRender::g_pRendergraph->CreateNode( GetNodeRef(), RenderNodeType, ParentRenderNodeRef, pInitMessage );

	//	failed
	if ( !m_RenderNodeRef.IsValid() )
		return FALSE;

	//	subscribe to the render graph to catch when our node has been added
	this->SubscribeTo( TLRender::g_pRendergraph );

	return TRUE;
}


TRef TSceneNode_Object::CreateAudioNode(TRefRef AudioRef, TRefRef AudioAsset)
{
	TLMessaging::TMessage Message(TLCore::InitialiseRef);

	Message.ExportData("Asset", AudioAsset);
	Message.ExportData("Play", TRUE);
	Message.ExportData("Translate", GetTranslate());
	Message.ExportData("Owner", GetNodeRef());

	// Create an audio node for the specified audio reference
	return TLAudio::g_pAudiograph->StartAudio(AudioRef, Message);
}

TRef TSceneNode_Object::CreateAudioNode(TRefRef AudioRef, TRefRef AudioAsset, const TLAudio::TAudioProperties& Props)
{
	TLMessaging::TMessage Message(TLCore::InitialiseRef);

	Message.ExportData("Asset", AudioAsset);
	Message.ExportData("Play", TRUE);
	Message.ExportData("Props", Props);
	Message.ExportData("Translate", GetTranslate());
	Message.ExportData("Owner", GetNodeRef());

	// Create an audio node for the specified audio reference
	return TLAudio::g_pAudiograph->StartAudio(AudioRef, Message);
}


Bool TSceneNode_Object::RemoveAudioNode(TRefRef AudioRef)
{
	return TLAudio::g_pAudiograph->RemoveNode(AudioRef);
}



void TSceneNode_Object::Translate(float3 vTranslation)
{
	// Manipulate the physics by adding a force in the direction required
	TPtr<TLPhysics::TPhysicsNode>& pPhysicsNode = GetPhysicsNode();

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


//--------------------------------------------------------
//	re-enable physics and render nodes
//--------------------------------------------------------
void TLScene::TSceneNode_Object::OnZoneWake()
{
	//	enable physics node
	if ( GetPhysicsNodeRef().IsValid() )
	{
		//	enable node directly if we have it
		TLPhysics::TPhysicsNode* pPhysicsNode = GetPhysicsNode(TRUE);
		if ( pPhysicsNode )
		{
			pPhysicsNode->SetEnabled( TRUE );
		}
		else
		{
			//	send message to disable
			TLMessaging::TMessage Message(TLCore::InitialiseRef);
			Message.ExportData("PFSet", TLPhysics::TPhysicsNode::Flag_Enabled );
			TLPhysics::g_pPhysicsgraph->SendMessageToNode( GetPhysicsNodeRef(), Message );
		}
	}

	//	enable render node
	if ( GetRenderNodeRef().IsValid() )
	{
		//	enable node directly if we have it
		TLRender::TRenderNode* pRenderNode = GetRenderNode(TRUE);
		if ( pRenderNode )
		{
			pRenderNode->SetEnabled( TRUE );
		}
		else
		{
			//	send message to disable
			TLMessaging::TMessage Message(TLCore::InitialiseRef);
			Message.ExportData("RFSet", TLRender::TRenderNode::RenderFlags::Enabled );
			TLRender::g_pRendergraph->SendMessageToNode( GetRenderNodeRef(), Message );
		}
	}

}


//--------------------------------------------------------
//	disable physics and render nodes
//--------------------------------------------------------
void TLScene::TSceneNode_Object::OnZoneSleep()
{
	//	disable physics node
	if ( GetPhysicsNodeRef().IsValid() )
	{
		//	disable node directly if we have it
		TLPhysics::TPhysicsNode* pPhysicsNode = GetPhysicsNode(TRUE);
		if ( pPhysicsNode )
		{
			pPhysicsNode->SetEnabled( FALSE );
		}
		else
		{
			//	send message to disable
			TLMessaging::TMessage Message(TLCore::InitialiseRef);
			Message.ExportData("PFClear", TLPhysics::TPhysicsNode::Flag_Enabled );
			TLPhysics::g_pPhysicsgraph->SendMessageToNode( GetPhysicsNodeRef(), Message );
		}
	}

#ifdef DISABLE_RENDER_ON_SLEEP
	//	disable render node
	if ( GetRenderNodeRef().IsValid() )
	{
		//	disable node directly if we have it
		TLRender::TRenderNode* pRenderNode = GetRenderNode(TRUE);
		if ( pRenderNode )
		{
			pRenderNode->SetEnabled( FALSE );
		}
		else
		{
			//	send message to disable
			TLMessaging::TMessage Message(TLCore::InitialiseRef);
			Message.ExportData("RFClear", TLRender::TRenderNode::RenderFlags::Enabled );
			TLRender::g_pRendergraph->SendMessageToNode( GetRenderNodeRef(), Message );
		}
	}
#endif

}



#include "TSceneNode_Object.h"
#include <TootlePhysics/TPhysicsGraph.h>


#define DISABLE_RENDER_ON_SLEEP
#define DISABLE_RENDER_ON_HALFWAKE


using namespace TLScene;

TSceneNode_Object::TSceneNode_Object(TRefRef NodeRef,TRefRef TypeRef) :
	TSceneNode_Transform		( NodeRef, TypeRef ),
	m_PublishTransformOnWake	( 0x0 )
{
}


TSceneNode_Object::~TSceneNode_Object()
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


void TLScene::TSceneNode_Object::Initialise(TLMessaging::TMessage& Message)
{
	//	do super init first
	TLScene::TSceneNode_Transform::Initialise( Message );

	//	create a render node if a mesh ref or specific render node type exists
	Bool DoCreateRenderNode = FALSE;
	TRef MeshRef,RenderNodeType;
	DoCreateRenderNode |= Message.ImportData("Meshref", MeshRef);
	DoCreateRenderNode |= Message.ImportData("RNType", RenderNodeType);

	if ( DoCreateRenderNode )
	{
		TRef ParentRenderNode;
		Message.ImportData("RNParent", ParentRenderNode );
		//	re-use the message to create the render node
		CreateRenderNode( ParentRenderNode, RenderNodeType, &Message );
	}


	//	create a physics node if a collision shape exists

	//	pull out collision shape from data if specified
	TPtr<TBinaryTree> pCollisionShapeData = Message.GetChild("colshape");

	//	if collision shape data exists, we re-use it
	//	no collision shape data, look for a datum
	if ( !pCollisionShapeData )
	{
		TRef CollisionShapeDatum;
		if ( Message.ImportData("coldatum", CollisionShapeDatum ) )
		{
			TPtr<TLMaths::TShape> pCollisionShape;
			
			//	get a datum from the mesh for the collision shape
			if ( MeshRef.IsValid() )
			{
				TLAsset::TMesh* pMesh = TLAsset::LoadAsset( MeshRef, TRUE, "Mesh" ).GetObject<TLAsset::TMesh>();
				if ( pMesh )
				{
					pCollisionShape = pMesh->GetDatum( CollisionShapeDatum );
					if ( !pCollisionShape )
					{
						TTempString Debug_String("Collision datum (");
						CollisionShapeDatum.GetString( Debug_String );
						Debug_String.Append(") is missing from mesh ");
						MeshRef.GetString( Debug_String );
						TLDebug_Break( Debug_String );
					}
				}
				else
				{
					TTempString Debug_String("Collision datum specified (");
					CollisionShapeDatum.GetString( Debug_String );
					Debug_String.Append(") but missing mesh ");
					MeshRef.GetString( Debug_String );
					TLDebug_Break( Debug_String );
				}
			}
			else
			{
				TTempString Debug_String("Collision datum specified (");
				CollisionShapeDatum.GetString( Debug_String );
				Debug_String.Append(") to create a physics node on a Scene Object but no mesh specified");
				TLDebug_Break( Debug_String );
			}

			//	if we got a shape, then export it to data we're going to use
			if ( pCollisionShape )
			{
				pCollisionShapeData = Message.AddChild("colshape");
				if ( !TLMaths::ExportShapeData( pCollisionShapeData, *pCollisionShape, FALSE ) )
				{
					//	failed - remove that data again
					Message.RemoveChild("Colshape");
					pCollisionShapeData = NULL;
				}
			}
		}
	}

	//	pull out physics node type if specified
	TRef PhysicsNodeType;
	Message.ImportData("PNType", PhysicsNodeType );

	//	if we did get a valid collision shape or a specific node type then create the physics node
	if ( pCollisionShapeData || PhysicsNodeType.IsValid() )
	{
		//	re-use the message to create the physics node
		CreatePhysicsNode( PhysicsNodeType, &Message );
	}


	//	enable/disable things
	TPtr<TBinaryTree>& pEnableData = Message.GetChild("Enable");
	if ( pEnableData )
	{
		Bool RenderEnable = TRUE;
		Bool PhysicsEnable = TRUE;
		Bool CollisionEnable = TRUE;
		
		if ( pEnableData->ImportData("Render", RenderEnable ) )
			EnableRenderNode( RenderEnable );

		Bool ChangePhysics = pEnableData->ImportData("Physics", PhysicsEnable );
		ChangePhysics |= pEnableData->ImportData("Collision", CollisionEnable );
		if ( ChangePhysics )
		{
			EnablePhysicsNode( PhysicsEnable, CollisionEnable );
		}
	}
}



void TSceneNode_Object::ProcessMessage(TLMessaging::TMessage& Message)
{
	//	gr: apply change from our physics node ONLY
	if(Message.GetMessageRef() == TRef_Static(O,n,T,r,a) && Message.GetSenderRef() == m_PhysicsNodeRef )
	{
		u8 TransformChangedBits = GetTransform().ImportData( Message );
		OnTransformChanged(TransformChangedBits);
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
Bool TSceneNode_Object::CreatePhysicsNode(TRefRef PhysicsNodeType,TLMessaging::TMessage* pInitMessage)
{
	// [20/02/09] DB - Currently we only allow one render node to be associated with the object node
	if( m_PhysicsNodeRef.IsValid() )
		return FALSE;

	//	gr: same as the render node system, merge these messages
	TLMessaging::TMessage TempMessage( TLCore::InitialiseRef );

	//	no init message? make one up
	if ( !pInitMessage )
	{
		pInitMessage = &TempMessage;
	}

	//	add transform info from this scene node
	const TLMaths::TTransform& Transform = GetTransform();

	if ( Transform.HasTranslate() )
		pInitMessage->ExportData(TRef_Static(T,r,a,n,s), Transform.GetTranslate() );

	if ( Transform.HasScale() )
		pInitMessage->ExportData(TRef_Static(S,c,a,l,e), Transform.GetScale() );

	if ( Transform.HasRotation() )
		pInitMessage->ExportData(TRef_Static(R,o,t,a,t), Transform.GetRotation() );

	//	export ownership info
	pInitMessage->ExportData("Owner", GetNodeRef());

	//	create node
	TRef ParentNode = TRef();

	m_PhysicsNodeRef = TLPhysics::g_pPhysicsgraph->CreateNode( GetNodeRef(), PhysicsNodeType, ParentNode, pInitMessage );

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
Bool TSceneNode_Object::CreateRenderNode(TRefRef ParentRenderNodeRef,TRefRef RenderNodeType,TLMessaging::TMessage* pInitMessage)
{
	// [20/02/09] DB - Currently we only allow one render node to be associated with the object node
	if( m_RenderNodeRef.IsValid() )
		return FALSE;

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
			pInitMessage->ExportData(TRef_Static(T,r,a,n,s), Transform.GetTranslate() );

		if ( Transform.HasScale() )
			pInitMessage->ExportData(TRef_Static(S,c,a,l,e), Transform.GetScale() );

		if ( Transform.HasRotation() )
			pInitMessage->ExportData(TRef_Static(R,o,t,a,t), Transform.GetRotation() );
	}

	//	export scene-node-ownership info
	pInitMessage->ExportData("Owner", GetNodeRef());

	//	create node
	m_RenderNodeRef = TLRender::g_pRendergraph->CreateNode( GetNodeRef(), RenderNodeType, ParentRenderNodeRef, pInitMessage );

	//	failed
	if ( !m_RenderNodeRef.IsValid() )
		return FALSE;

	//	subscribe to the render graph to catch when our node has been added
	this->SubscribeTo( TLRender::g_pRendergraph );

	return TRUE;
}


void TSceneNode_Object::OnPhysicsNodeAdded(TPtr<TLPhysics::TPhysicsNode>& pPhysicsNode)
{
	//	re-enable/disable render node based on zone state
	SyncBool ZoneActive = IsZoneAwake();

	if ( ZoneActive == SyncFalse )
		OnZoneSleep();
	else
		OnZoneWake( ZoneActive );
}


void TSceneNode_Object::OnRenderNodeAdded(TPtr<TLRender::TRenderNode>& pRenderNode)
{
	//	re-enable/disable render node based on zone state
	SyncBool ZoneActive = IsZoneAwake();

	if ( ZoneActive == SyncFalse )
		OnZoneSleep();
	else
		OnZoneWake( ZoneActive );
}


TRef TSceneNode_Object::CreateAudioNode(TRefRef AudioRef, TRefRef AudioAsset)
{
	TLMessaging::TMessage Message(TLCore::InitialiseRef);

	Message.ExportData("Asset", AudioAsset);
	Message.ExportData("Play", TRUE);
	Message.ExportData(TRef_Static(T,r,a,n,s), GetTranslate());
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
	Message.ExportData(TRef_Static(T,r,a,n,s), GetTranslate());
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
		const TLMaths::TShapeBox& Bounds = pRenderNode->GetWorldBoundsBox();

		if(Bounds.GetBox().GetIntersection(Line))
		{
			float fBoundsDistance = Bounds.GetBox().GetDistanceSq(Line);

			if(fBoundsDistance < fDistance)
				fDistance = fBoundsDistance;
		}
	}

	return fDistance;
}


//--------------------------------------------------------
//	re-enable physics and render nodes
//--------------------------------------------------------
void TLScene::TSceneNode_Object::OnZoneWake(SyncBool ZoneActive)
{
	EnablePhysicsNode( TRUE, TRUE );
	
#ifdef DISABLE_RENDER_ON_HALFWAKE
	EnableRenderNode( ZoneActive == SyncTrue );
#else
	EnableRenderNode( TRUE );
#endif
}


//--------------------------------------------------------
//	disable physics and render nodes
//--------------------------------------------------------
void TLScene::TSceneNode_Object::OnZoneSleep()
{
	EnablePhysicsNode( FALSE, FALSE );

#ifdef DISABLE_RENDER_ON_SLEEP
	EnableRenderNode( FALSE );
#endif
}


//--------------------------------------------------------
//	this checks to see if we're asleep first and delays sending a transform until we are awake
//--------------------------------------------------------
void TLScene::TSceneNode_Object::OnTransformChanged(u8 TransformChangedBits)
{
	//	gr: this still needs doing even if we're asleep
	//	if translation changed then set zone out of date
	if ( TransformChangedBits & TLMaths_TransformBitTranslate )
		TLMaths::TQuadTreeNode::SetZoneOutOfDate();	

	//	don't send and don't save off a change if we have no subscribers
	if ( !HasSubscribers() )
		return;

	//	if asleep then don't send a message until we wake up again
	if ( IsAwake() == SyncFalse )
	{
		m_PublishTransformOnWake |= TransformChangedBits;
		return;
	}

	//	publish transform changes
	TSceneNode_Transform::OnTransformChanged( TransformChangedBits );

	//	subscribers are up to date
	m_PublishTransformOnWake = 0x0;
}


//--------------------------------------------------------
//	enable/disable physics node - can seperately enable collision
//--------------------------------------------------------
void TLScene::TSceneNode_Object::EnablePhysicsNode(Bool Enable,Bool EnableCollision)
{
	//	enable physics node
	if ( !GetPhysicsNodeRef().IsValid() )
		return;

	//	get node
	TLPhysics::TPhysicsNode* pPhysicsNode = GetPhysicsNode();
	if ( !pPhysicsNode )
		return;

	//	enable/disable collision
	pPhysicsNode->EnableCollision( EnableCollision );

	//	enable/disable
	pPhysicsNode->SetEnabled( Enable );
}


//--------------------------------------------------------
//	enable/disable render node
//--------------------------------------------------------
void TLScene::TSceneNode_Object::EnableRenderNode(Bool Enable)
{
	if ( !GetRenderNodeRef().IsValid() )
		return;

	//	enable node directly if we have it
	TLRender::TRenderNode* pRenderNode = GetRenderNode();
	if ( !pRenderNode )
		return;

	//	enable
	pRenderNode->SetEnabled( Enable );

	//	transform has changed whilst we were asleep, send transform-changed message to render node
	if ( Enable && (m_PublishTransformOnWake != 0x0) )
	{
		OnTransformChanged( m_PublishTransformOnWake );
	}

}


//--------------------------------------------------------
//	when we do an explicit transform on our node - change the physics' transform too
//--------------------------------------------------------
void TLScene::TSceneNode_Object::SetTransform(const TLMaths::TTransform& Transform)
{
	//	do inherited change
	TSceneNode_Transform::SetTransform( Transform );

	//	change physics
	TLPhysics::TPhysicsNode* pPhysicsNode = GetPhysicsNode();
	if ( pPhysicsNode )
		pPhysicsNode->SetTransform( GetTransform(), FALSE );
}


//--------------------------------------------------------
//	when we do an explicit transform on our node - change the physics' transform too
//--------------------------------------------------------
void TLScene::TSceneNode_Object::SetTranslate(const float3& Translate)
{
	//	do inherited change
	TSceneNode_Transform::SetTranslate( Translate );

	//	change physics
	TLPhysics::TPhysicsNode* pPhysicsNode = GetPhysicsNode();
	if ( pPhysicsNode )
		pPhysicsNode->SetTransform( GetTransform(), FALSE );
}


//--------------------------------------------------------
//	when we do an explicit transform on our node - change the physics' transform too
//--------------------------------------------------------
void TLScene::TSceneNode_Object::SetRotation(const TLMaths::TQuaternion& Rotation)
{
	//	do inherited change
	TSceneNode_Transform::SetRotation( Rotation );

	//	change physics
	TLPhysics::TPhysicsNode* pPhysicsNode = GetPhysicsNode();
	if ( pPhysicsNode )
		pPhysicsNode->SetTransform( GetTransform(), FALSE );
}


//--------------------------------------------------------
//	when we do an explicit transform on our node - change the physics' transform too
//--------------------------------------------------------
void TLScene::TSceneNode_Object::SetScale(const float3& Scale)
{
	//	do inherited change
	TSceneNode_Transform::SetScale( Scale );

	//	change physics
	TLPhysics::TPhysicsNode* pPhysicsNode = GetPhysicsNode();
	if ( pPhysicsNode )
		pPhysicsNode->SetTransform( GetTransform(), FALSE );
}




#include "TSceneNode_Object.h"
#include <TootlePhysics/TPhysicsGraph.h>


#define DISABLE_RENDER_ON_SLEEP
#define DISABLE_RENDER_ON_HALFWAKE
#define DEBUG_PHYSICS_RENDERNODE_COLOUR		TColour( 1.f, 0.f, 1.f, 0.5f )	//	pink

using namespace TLScene;

TSceneNode_Object::TSceneNode_Object(TRefRef NodeRef,TRefRef TypeRef) :
	TSceneNode_Transform		( NodeRef, TypeRef ),
	m_PublishTransformOnWake	( 0x0 ),
	m_OnEditPhysicsWasEnabled	( SyncWait )
{
}


TSceneNode_Object::~TSceneNode_Object()
{
}

/*
// [29/06/09] Duane's version - changed because Graham added the same thing at the same time...
//			  Leaving code in case I miss something after commit.  Will remove if everything works as before on
//			  all platforms.
void TSceneNode_Object::Initialise(TLMessaging::TMessage& Message)
{	
	// Super class intialise
	TSceneNode_Transform::Initialise(Message);
	
	// Check for a render node data sent to node.  Use the data to 
	// automatically setup a render node
	TPtr<TBinaryTree>& pRenderData = Message.GetChild("RenderData");

	if(pRenderData.IsValid())
	{
		TRef ParentRenderNodeRef;
		TRef RenderNodeTypeRef;

		pRenderData->ImportData("Parent", ParentRenderNodeRef);
		pRenderData->ImportData("Type", RenderNodeTypeRef);


		// Import the render node init message data
		TLMessaging::TMessage RenderNodeInitMessage(TLCore::InitialiseRef);
		TLMessaging::TMessage* pMessage = NULL;

		TPtr<TBinaryTree>& pRenderInitMessageData = pRenderData->GetChild("Message");
		
		if(pRenderInitMessageData)
		{
			RenderNodeInitMessage.CopyDataTree(*pRenderInitMessageData, FALSE);
			pMessage = &RenderNodeInitMessage;
		}

		// Create Render node
		CreateRenderNode(ParentRenderNodeRef, RenderNodeTypeRef, pMessage);
	}
}
*/


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


void TSceneNode_Object::Initialise(TLMessaging::TMessage& Message)
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

	//	look for mesh datums to convert into collision shapes
	TPtrArray<TBinaryTree> CollisionDatumDatas;
	Message.GetChildren("coldatum", CollisionDatumDatas );
	TLAsset::TMesh* pMesh = NULL;

	if ( CollisionDatumDatas.GetSize() > 0 )
	{
		pMesh = TLAsset::LoadAsset( MeshRef, TRUE, "Mesh" ).GetObject<TLAsset::TMesh>();
		if ( !pMesh )
		{
			TTempString Debug_String("Collision datums specified, but mesh \"");
			MeshRef.GetString( Debug_String );
			Debug_String.Append("\" is missing. Cannot create collision shapes for physics node");
			TLDebug_Break( Debug_String );
		}
	}

	for ( u32 d=0;	d<CollisionDatumDatas.GetSize() && pMesh;	d++ )
	{
		TBinaryTree& CollisionDatumData = *(CollisionDatumDatas[d]);

		//	read datum ref
		TRef CollisionShapeDatum;
		CollisionDatumData.ResetReadPos();
		if ( !CollisionDatumData.Read( CollisionShapeDatum ) )
			continue;

		//	get a datum from the mesh for the collision shape
		TPtr<TLMaths::TShape>& pCollisionShape = pMesh->GetDatum( CollisionShapeDatum );
		if ( !pCollisionShape )
		{
			TTempString Debug_String("Collision datum (");
			CollisionShapeDatum.GetString( Debug_String );
			Debug_String.Append(") is missing from mesh ");
			MeshRef.GetString( Debug_String );
			TLDebug_Break( Debug_String );
			continue;
		}
		
		//	if we got a shape, then export it to data we're going to use
		TPtr<TBinaryTree>& pCollisionShapeData = Message.AddChild("colshape");
		if ( TLMaths::ExportShapeData( pCollisionShapeData, *pCollisionShape, FALSE ) )
		{
			//	export ref - use the same ref as the datum
			pCollisionShapeData->ExportData("Ref", CollisionShapeDatum );

			//	and copy any extra data (eg. sensor state)
			pCollisionShapeData->AddUnreadChildren( CollisionDatumData, FALSE );
		}
		else
		{
			//	failed - remove that data again
			Message.RemoveChild( pCollisionShapeData );
			pCollisionShapeData = NULL;
		}
	}

	//	pull out physics node type if specified
	TRef PhysicsNodeType;
	Message.ImportData("PNType", PhysicsNodeType );

	//	if we have any collision shapes or a specific node type then create the physics node
	if ( PhysicsNodeType.IsValid() || Message.GetChild("colshape").IsValid() )
	{
		//	re-use the message to create the physics node
		CreatePhysicsNode( PhysicsNodeType, &Message );
	}

}


//------------------------------------------------
//	set node properties
//------------------------------------------------
void TSceneNode_Object::SetProperty(TLMessaging::TMessage& Message)
{
	//	enable/disable things
	TPtr<TBinaryTree>& pEnableData = Message.GetChild("Enable");
	if ( pEnableData )
	{
		Bool bEnable = TRUE;
		pEnableData->ResetReadPos();

		if(pEnableData->Read(bEnable))
			SetEnable(bEnable);

		Bool RenderEnable = TRUE;
		Bool PhysicsEnable = TRUE;
		SyncBool CollisionEnable = SyncWait;	//	syncwait = no changes
		Bool DebugPhysicsEnable = FALSE;
		
		if ( pEnableData->ImportData("Render", RenderEnable ) )
			EnableRenderNode( RenderEnable );

		if ( pEnableData->ImportData("DbgPhys", DebugPhysicsEnable ) )
			Debug_EnableRenderDebugPhysics( DebugPhysicsEnable );

		Bool ChangePhysics = FALSE;
		if ( pEnableData->ImportData("Physics", PhysicsEnable ) )		
			ChangePhysics |= TRUE;

		Bool TmpCollisionEnable = FALSE;
		if ( pEnableData->ImportData("Collision", TmpCollisionEnable ) )	
		{
			CollisionEnable = TmpCollisionEnable ? SyncTrue : SyncFalse;
			ChangePhysics |= TRUE;
		}

		if ( ChangePhysics )
		{
			EnablePhysicsNode( PhysicsEnable, CollisionEnable );
		}
	}

	//	read super-properties
	TSceneNode_Transform::SetProperty( Message );
}


void TSceneNode_Object::ProcessMessage(TLMessaging::TMessage& Message)
{
	//	gr: apply change from our physics node ONLY
	if(Message.GetMessageRef() == TRef_Static(O,n,T,r,a) && Message.GetSenderRef() == m_PhysicsNodeRef )
	{
		u8 TransformChangedBits = GetTransform().ImportData( Message );
		OnTransformChanged( TransformChangedBits|TLSceneNodeObject_FromPhysicsTransform );
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
	else if ( Message.GetMessageRef() == "EdtStart" )
	{
		//	 when this node starts to be edited we disable the physics
		if ( m_OnEditPhysicsWasEnabled != SyncWait )
		{
			TLDebug_Break("Warning, editing has already started on node?");
		}

		TLPhysics::TPhysicsNode* pPhysicsNode = GetPhysicsNode();
		if ( pPhysicsNode )
		{
			//	save old state of physics
			m_OnEditPhysicsWasEnabled = (SyncBool)pPhysicsNode->IsEnabled();

			//	disable
			EnablePhysicsNode( FALSE, SyncWait );
		}
		return;
	}
	else if ( Message.GetMessageRef() == "EdtEnd" )
	{
		//	 stopped editing node, restore physics state
		if ( m_OnEditPhysicsWasEnabled != SyncWait )
		{
			TLPhysics::TPhysicsNode* pPhysicsNode = GetPhysicsNode();
			if ( pPhysicsNode )
			{
				//	restore enabled state
				EnablePhysicsNode( m_OnEditPhysicsWasEnabled==SyncTrue ? TRUE : FALSE, SyncWait );
			}
			else
			{
				TLDebug_Break("Restoring physics node state, but node missing?");
			}

			//	reset saved state
			m_OnEditPhysicsWasEnabled = SyncWait;
		}
		return;
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

	//	add transform info from this scene node if the data doesn't already exist
	const TLMaths::TTransform& Transform = GetTransform();
	if ( Transform.HasTranslate() && !pInitMessage->HasChild(TRef_Static(T,r,a,n,s)) )
		pInitMessage->ExportData( TRef_Static(T,r,a,n,s), Transform.GetTranslate() );
	
	if ( Transform.HasScale() && !pInitMessage->HasChild(TRef_Static(T,r,a,n,s)) )
		pInitMessage->ExportData( TRef_Static(S,c,a,l,e), Transform.GetScale() );
	
	if ( Transform.HasRotation() && !pInitMessage->HasChild(TRef_Static(T,r,a,n,s)) )
		pInitMessage->ExportData( TRef_Static(R,o,t,a,t), Transform.GetRotation() );

	//	export ownership info
	if ( !pInitMessage->HasChild( TRef_Static(O,w,n,e,r) ) )
		pInitMessage->ExportData( TRef_Static(O,w,n,e,r), GetNodeRef());

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

	//	add transform info from this scene node if the data doesn't already exist
	const TLMaths::TTransform& Transform = GetTransform();
	if ( Transform.HasTranslate() && !pInitMessage->HasChild(TRef_Static(T,r,a,n,s)) )
		pInitMessage->ExportData( TRef_Static(T,r,a,n,s), Transform.GetTranslate() );
	
	if ( Transform.HasScale() && !pInitMessage->HasChild(TRef_Static(T,r,a,n,s)) )
		pInitMessage->ExportData( TRef_Static(S,c,a,l,e), Transform.GetScale() );
	
	if ( Transform.HasRotation() && !pInitMessage->HasChild(TRef_Static(T,r,a,n,s)) )
		pInitMessage->ExportData( TRef_Static(R,o,t,a,t), Transform.GetRotation() );

	//	export ownership info
	if ( !pInitMessage->HasChild( TRef_Static(O,w,n,e,r) ) )
		pInitMessage->ExportData( TRef_Static(O,w,n,e,r), GetNodeRef());

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
	//	when physics is disabled via editor, don't enable physics on wake
	if ( m_OnEditPhysicsWasEnabled == SyncWait )
		EnablePhysicsNode( TRUE, SyncWait );
	
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
	//	when physics is disabled via editor, don't enable physics on wake
	if ( m_OnEditPhysicsWasEnabled == SyncWait )
		EnablePhysicsNode( FALSE, SyncWait );

#ifdef DISABLE_RENDER_ON_SLEEP
	EnableRenderNode( FALSE );
#endif
}


//--------------------------------------------------------
//	this checks to see if we're asleep first and delays sending a transform until we are awake
//--------------------------------------------------------
void TLScene::TSceneNode_Object::OnTransformChanged(u8 TransformChangedBits)
{
	//	change the physics transform when the change has NOT come from the physics
	if ( (TransformChangedBits & TLSceneNodeObject_FromPhysicsTransform) == 0x0 )
	{
		//	change physics
		TLPhysics::TPhysicsNode* pPhysicsNode = GetPhysicsNode();
		if ( pPhysicsNode )
			pPhysicsNode->SetTransform( GetTransform(), FALSE );
	}

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
		m_PublishTransformOnWake |= TransformChangedBits & ~TLSceneNodeObject_FromPhysicsTransform;
		return;
	}

	//	publish transform changes
	TSceneNode_Transform::OnTransformChanged( TransformChangedBits & ~TLSceneNodeObject_FromPhysicsTransform );

	//	subscribers are up to date
	m_PublishTransformOnWake = 0x0;
}


//--------------------------------------------------------
//	enable/disable physics node - can seperately enable collision
//--------------------------------------------------------
void TLScene::TSceneNode_Object::EnablePhysicsNode(Bool Enable,SyncBool EnableCollision)
{
	//	enable physics node
	if ( !GetPhysicsNodeRef().IsValid() )
		return;

	//	get node
	TLPhysics::TPhysicsNode* pPhysicsNode = GetPhysicsNode(TRUE);
	if ( !pPhysicsNode )
	{
		//	send message instead if it's not been created yet
		TLMessaging::TMessage SetMessage(TLCore::SetPropertyRef);
		SetMessage.ExportData( Enable ? TRef("PFSet") : TRef("PFClear"), TLPhysics::TPhysicsNode::Flag_Enabled );
		
		if ( EnableCollision != SyncWait )
			SetMessage.ExportData( (EnableCollision==SyncTrue) ? TRef("PFSet") : TRef("PFClear"), TLPhysics::TPhysicsNode::Flag_HasCollision );

		TLPhysics::g_pPhysicsgraph->SendMessageToNode( GetPhysicsNodeRef(), SetMessage );
		return;
	}

	//	enable/disable collision
	if ( EnableCollision != SyncWait )
		pPhysicsNode->EnableCollision( (EnableCollision==SyncTrue) ? TRUE : FALSE );

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
//	turn on/off debug render node for physics
//--------------------------------------------------------
void TLScene::TSceneNode_Object::Debug_EnableRenderDebugPhysics(Bool Enable)
{
#ifdef _DEBUG
	if ( Enable && !m_Debug_RenderDebugPhysicsNodeRef.IsValid() )
	{
		//	create debug render node - requires render node and physics node
		if ( !GetRenderNodeRef().IsValid() || !GetPhysicsNodeRef().IsValid() )
			return;

		TLMessaging::TMessage InitMessage(TLCore::InitialiseRef);
		InitMessage.ExportData("PhNode", GetPhysicsNodeRef() );
		InitMessage.ExportData("Colour", DEBUG_PHYSICS_RENDERNODE_COLOUR );		//	gr: use a colour so we can have transparency
		m_Debug_RenderDebugPhysicsNodeRef = TLRender::g_pRendergraph->CreateNode( GetNodeRef(), "DbgPhys", GetRenderNodeRef(), &InitMessage );
	}
	else if ( !Enable && m_Debug_RenderDebugPhysicsNodeRef.IsValid() )
	{
		TLRender::g_pRendergraph->RemoveNode( m_Debug_RenderDebugPhysicsNodeRef );
	}
#endif
}


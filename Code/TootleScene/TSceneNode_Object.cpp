#include "TSceneNode_Object.h"
#include "TScenegraph.h"
#include <TootlePhysics/TPhysicsGraph.h>


#define DISABLE_RENDER_ON_SLEEP
#define DISABLE_RENDER_ON_HALFWAKE
#define DEBUG_PHYSICS_RENDERNODE_COLOUR		TColour( 1.f, 0.f, 1.f, 0.5f )	//	pink

using namespace TLScene;

TSceneNode_Object::TSceneNode_Object(TRefRef NodeRef,TRefRef TypeRef) :
	TSceneNode_Transform		( NodeRef, TypeRef ),
	m_fLife(100.0f),
	m_PublishTransformOnWake	( 0x0 ),
	m_OnEditPhysicsWasEnabled	( SyncWait )
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


void TSceneNode_Object::Initialise(TLMessaging::TMessage& Message)
{
	Message.Debug_PrintTree();

	//	do super init first
	TLScene::TSceneNode_Transform::Initialise( Message );

	//	auto-create render node
	InitialiseRenderNode(Message);

	//	auto create physics node
	InitialisePhysicsNode(Message);
}



//------------------------------------------------
//	create render node from init message
//------------------------------------------------
Bool TSceneNode_Object::InitialiseRenderNode(TLMessaging::TMessage& Message)
{
	//	if explicit node data exists, create and init a node with that
	TPtr<TBinaryTree>& pNodeData = Message.GetChild("RNode");
	if ( pNodeData.IsValid() )
	{
		// NEW SYSTEM
		// Uses render node data under a RNode subtree
		TRef ParentNodeRef;
		if ( !pNodeData->ImportData("Parent", ParentNodeRef) )
			Message.ImportData("RNParent", ParentNodeRef);

		TRef NodeTypeRef;
		pNodeData->ImportData("Type", NodeTypeRef);

		// Reference the node data as an initialise message
		TLMessaging::TMessage InitMessage(TLCore::InitialiseRef);
		InitMessage.ReferenceDataTree(*pNodeData);

		// Create node
		CreateRenderNode( ParentNodeRef, NodeTypeRef, &InitMessage );
		return TRUE;
	}


	// OLD SYSTEM 
	// Uses render node data within the nodes initialise message
	//	create a render node if a mesh ref or specific render node type exists
	TRef MeshRef,NodeTypeRef;
	Bool DoCreateNode = Message.ImportData("MeshRef", MeshRef);
	DoCreateNode |= Message.ImportData("RNType", NodeTypeRef);

	//	nothing specified to say "create a render node"
	if ( !DoCreateNode )
		return FALSE;

	TRef ParentNodeRef;
	Message.ImportData("RNParent", ParentNodeRef );

	//	Re-use the message to create the render node
	CreateRenderNode( ParentNodeRef, NodeTypeRef, &Message );
	
	return TRUE;
}


//------------------------------------------------
//	create physics node from init message
//------------------------------------------------
Bool TSceneNode_Object::InitialisePhysicsNode(TLMessaging::TMessage& Message)
{
	//	if explicit node data exists, create and init a node with that
	TPtr<TBinaryTree>& pNodeData = Message.GetChild("PNode");
	if ( pNodeData.IsValid() )
	{
		// NEW SYSTEM
		// Uses node data under a XNode subtree

		TRef NodeTypeRef;
		pNodeData->ImportData("Type", NodeTypeRef);

		// Reference the node data as an initialise message
		TLMessaging::TMessage InitMessage(TLCore::InitialiseRef);
		InitMessage.ReferenceDataTree(*pNodeData);

		// Create node
		CreatePhysicsNode( NodeTypeRef, &InitMessage );
		return TRUE;
	}


	// OLD SYSTEM 
	// Uses render node data within the nodes initialise message


	//	detect and convert collision datums into collision shapes
	TPtrArray<TBinaryTree> CollisionDatumDatas;
	Message.GetChildren("coldatum", CollisionDatumDatas );
	TPtr<TBinaryTree>& pMeshData = CollisionDatumDatas.GetSize() ? Message.GetChild("MeshRef") : TLPtr::GetNullPtr<TBinaryTree>();

	//	add the mesh data to the datum data (to make it backwards compatible)
	for ( u32 d=0;	d<CollisionDatumDatas.GetSize() && pMeshData;	d++ )
	{
		TBinaryTree& CollisionDatumData = *CollisionDatumDatas.ElementAt(d);
		CollisionDatumData.AddChild( pMeshData );
	}

	//	create a physics node if a collision shape or specific node type exists
	TRef NodeTypeRef;
	Bool DoCreateNode = Message.HasChild("colshape");
	DoCreateNode |= Message.HasChild("coldatum");
	DoCreateNode |= Message.ImportData("PNType", NodeTypeRef);

	//	nothing specified to say "create a node"
	if ( !DoCreateNode )
		return FALSE;

	//	Re-use the message to create the render node
	CreatePhysicsNode( NodeTypeRef, &Message );
	
	return TRUE;
}


//------------------------------------------------
//	set node properties
//------------------------------------------------
void TSceneNode_Object::SetProperty(TLMessaging::TMessage& Message)
{
	//	gr: changed this so individual things have specific enable messages/properties
	Bool Enable;

	if ( Message.ImportData("EnRender", Enable ) )
		EnableRenderNode( Enable );

	if ( Message.ImportData("EnDbg", Enable ) )
		Debug_EnableRenderDebugPhysics( Enable );

	if ( Message.ImportData("EnPhysics", Enable ) )
		EnablePhysicsNode( Enable );

	if ( Message.ImportData("EnCollision", Enable ) )
		EnablePhysicsNodeCollision( Enable );

	// Import life
	float fLife;
	if(Message.ImportData("Life", fLife))
		SetLife(fLife);

	//	read super-properties
	TSceneNode_Transform::SetProperty( Message );
}

void TSceneNode_Object::UpdateNodeData()
{
	// Update and serialise the render node data
	TPtr<TLRender::TRenderNode>& pRenderNode = GetRenderNode();

	//	gr: clear current render node data
	GetNodeData().RemoveChild("RNode");
	if(pRenderNode)
	{
		//	recursivly add the render node and all it's children to our node data
		UpdateOwnedNodeData( *pRenderNode, GetNodeData(), "RNode" );
	}

	// Update and serialise the physics node data
	TPtr<TLPhysics::TPhysicsNode>& pPhysicsNode = GetPhysicsNode();

	//	gr: clear current physics node data
	GetNodeData().RemoveChild("PNode");
	if(pPhysicsNode)
	{
		// Update the Physics node data
		pPhysicsNode->UpdateNodeData();

		TPtr<TBinaryTree>& pPhysicsData = GetNodeData().AddChild("PNode");
		if(pPhysicsData)
			pPhysicsData->ReferenceDataTree(pPhysicsNode->GetNodeData());
	}

/*	
	// Update and serialise the audio node data
	TPtr<TLAudio::TAudioNode>& pAudioNode = GetAudioNode();


	if(pAudioNode)
	{
		// Update the Audio node data
		pAudioNode->UpdateNodeData();

		GetNodeData().RemoveChild("ANode");
		TPtr<TBinaryTree>& pAudioData = GetNodeData().AddChild("ANode");

		if(pAudioData)
			pAudioData->ReferenceDataTree(pAudioNode->GetNodeData());
	}
*/
	GetNodeData().RemoveChild("Life");
	GetNodeData().ExportData("Life", m_fLife);

	TLScene::TSceneNode_Transform::UpdateNodeData();
}


//------------------------------------------------
//	recursivly store an owned-node's data to this data
//------------------------------------------------
void TSceneNode_Object::UpdateOwnedNodeData(TLRender::TRenderNode& RenderNode,TBinaryTree& NodeData,TRefRef NodeDataRef)
{
	// Update the node's own data (ie. save it's state)
	RenderNode.UpdateNodeData();

	//	add some data to write to
	TPtr<TBinaryTree>& pNewNodeData = GetNodeData().AddChild( NodeDataRef );
	if ( !pNewNodeData )
		return;

	//	copy the node's data
	pNewNodeData->ReferenceDataTree( RenderNode.GetNodeData() );

	//	now store this node's children too
	for ( u32 c=0;	c<RenderNode.GetChildren().GetSize();	c++ )
	{
		TPtr<TLRender::TRenderNode>& pChildRenderNode = RenderNode.GetChildren().ElementAt(c);
		UpdateOwnedNodeData( *pChildRenderNode, *pNewNodeData, "Child" );
	}
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
	else if( Message.GetMessageRef() == STRef(L,i,f,e,C) )	// "LifeChange"
	{
		float fLifeChange;
		if(Message.Read(fLifeChange))
			DoChangeLife(fLifeChange);
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

	//	render node has been created (not added yet)
	OnRenderNodeCreated();

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

	//	gr: changed to always send a message
	TLMessaging::TMessage SetMessage(TLCore::SetPropertyRef);
	SetMessage.ExportData( Enable ? TRef("PFSet") : TRef("PFClear"), TLPhysics::TPhysicsNode::Flag_Enabled );
		
	if ( EnableCollision != SyncWait )
		SetMessage.ExportData( (EnableCollision==SyncTrue) ? TRef("PFSet") : TRef("PFClear"), TLPhysics::TPhysicsNode::Flag_HasCollision );

	TLPhysics::g_pPhysicsgraph->SendMessageToNode( GetPhysicsNodeRef(), SetMessage );
}

//--------------------------------------------------------
//	enable/disable physics node collision
//--------------------------------------------------------
void TLScene::TSceneNode_Object::EnablePhysicsNodeCollision(Bool Enable)
{
	if ( !GetPhysicsNodeRef().IsValid() )
		return;

	//	gr: changed to always send a message
	TLMessaging::TMessage SetMessage(TLCore::SetPropertyRef);
	SetMessage.ExportData( Enable ? TRef("PFSet") : TRef("PFClear"), TLPhysics::TPhysicsNode::Flag_HasCollision );

	TLPhysics::g_pPhysicsgraph->SendMessageToNode( GetPhysicsNodeRef(), SetMessage );
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


void TLScene::TSceneNode_Object::DoChangeLife(const float& fLifeChange)
{
	if((fLifeChange != 0.0f) && CanChangeLife(fLifeChange))
	{
		m_fLife += fLifeChange;

		OnLifeChange(fLifeChange);
	}
}


void TLScene::TSceneNode_Object::OnDeath()
{
	// Remove the node from the scenegraph as default behaviour
	TLScene::g_pScenegraph->RemoveNode(GetNodeRef());
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


#include "TPhysicsNode.h"
#include "TPhysicsGraph.h"
#include <TootleCore/TLMaths.h>
#include <TootleCore/TLTime.h>
#include <TootleScene/TScenegraph.h>
#include <TootleCore/TEventChannel.h>
#include <TootleMaths/TShapeSphere.h>
#include <TootleMaths/TShapeBox.h>
#include <box2d/include/box2d.h>




#define USE_ZERO_GROUP



//	if something moves less than this amount then dont apply the change - 
//	this stops the world collision sphere from being invalidated more than we need to
#define MIN_CHANGE_AMOUNT			(TLMaths_NearZero*1000.f)
//#define MIN_CHANGE_AMOUNT			(TLMaths_NearZero)

#define HAS_MIN_CHANGE3(v)			HAS_MIN_CHANGE( v.LengthSq() )
//#define HAS_MIN_CHANGE3(v)			TRUE

#define HAS_MIN_CHANGE(f)			( f > MIN_CHANGE_AMOUNT )
//#define HAS_MIN_CHANGE(f)			TRUE

#define DEBUG_FLOAT_CHECK(v)		TLDebug_CheckFloat(v)
//#define DEBUG_FLOAT_CHECK(v)		TRUE

//#define FORCE_SQUIDGE				0.0f

#define FRICTION_SCALAR				PHYSICS_SCALAR
#define MOVEMENT_SCALAR				PHYSICS_SCALAR


namespace TLPhysics
{
	float3		g_WorldUp( 0.f, -1.f, 0.f );
	float3		g_WorldUpNormal( 0.f, -1.f, 0.f );

	float		g_GravityMetresSec	= 30.f;	//	gravity in metres per second (1unit being 1metre)
//	float		g_GravityMetresSec	= 9.81f;	//	gravity in metres per second (1unit being 1metre)
}


namespace TLRef
{
	TLArray::SortResult		RefSort(const TRef& aRef,const TRef& bRef,const void* pTestVal);	//	simple ref-sort func - for arrays of TRef's
}


//---------------------------------------------------------
//	update the bodyshape to match our current shape - fails if it must be recreated, or doesnt exist etc
//---------------------------------------------------------
Bool TLPhysics::TCollisionShape::UpdateBodyShape()
{
	//	missing shape - gr: auto delete the body shape here?
	if ( !m_pShape )
	{
		TLDebug_Break("shape expected");
		return FALSE;
	}

	//	don't have an old body shape - nothing to "update"
	if ( !m_pBodyShape )
	{
		TLDebug_Break("Bodyshape expected(? or not? is it okay to fail here and continue?");
		return FALSE;
	}

	//	get a new shape def
	b2CircleDef TempCircleDef;
	b2PolygonDef TempPolygonDef;
	b2FixtureDef* pNewShapeDef = TLPhysics::GetShapeDefFromShape(TempCircleDef,TempPolygonDef, *m_pShape );
	
	//	failed to get a definition from this shape
	if ( !pNewShapeDef )
		return FALSE;

	//	check we can update the polygon...

	//	different type (eg. circle vs polygon)
	if ( m_pBodyShape->GetType() != pNewShapeDef->type )
		return FALSE;

	//	if polygon, make sure we have same number of verts (otherwise corrupts/crashes box)
	if ( m_pBodyShape->GetType() == b2_polygonShape )
	{
		b2PolygonShape& CurrentPolygonShape = static_cast<b2PolygonShape&>( *m_pBodyShape->GetShape() );

		//	different number of verts, can't do a simple update
		//	must have used the TempPolygon shape def - saves casting pNewShapeDef
		if ( TempPolygonDef.vertexCount != CurrentPolygonShape.GetVertexCount() )
			return FALSE;

		//	can just update vertex positions 
		//	gr: I dont *think* this needs a refilter...
		CurrentPolygonShape.Set( TempPolygonDef.vertices, TempPolygonDef.vertexCount );
		return TRUE;
	}
	else if ( m_pBodyShape->GetType() == b2_circleShape )
	{
		b2CircleShape& CurrentCircleShape = static_cast<b2CircleShape&>( *m_pBodyShape->GetShape() );
		
		//	update circle shape 
		//	must have used the TempCircle shape def - saves casting pNewShapeDef
		//	gr: I dont *think* this needs a refilter...
		CurrentCircleShape.m_p = TempCircleDef.localPosition;
		CurrentCircleShape.m_radius = TempCircleDef.radius;
		return TRUE;
	}
	else
	{
		TLDebug_Break("updating unhandled/unknown shape type");
		return FALSE;
	}

}


//---------------------------------------------------------
//	delete body shape from body - returns if any changes made
//---------------------------------------------------------
Bool TLPhysics::TCollisionShape::DestroyBodyShape(b2Body* pBody)
{
	//	nothing to destroy
	if ( !m_pBodyShape )
		return FALSE;

	//	check bodies match - use correct body if wrong
	if ( GetBody() != pBody )
	{
		TLDebug_Break("Tried to delete bodyshape from body which it's not attached to");
		pBody = GetBody();
	}

	if ( !pBody )
	{
		TLDebug_Break("Body expected");
		return FALSE;
	}

	//	destroy shape
	pBody->DestroyFixture( m_pBodyShape );
	m_pBodyShape = NULL;

	return TRUE;
}






/////////////////////////////////////////////////////////////////////////////////////////





TLPhysics::TPhysicsNode::TPhysicsNode(TRefRef NodeRef,TRefRef TypeRef) :
	TLGraph::TGraphNode<TPhysicsNode>	( NodeRef, TypeRef ),
	m_Bounce						( 0.0f ),
	m_Damping						( 0.0f ),
	m_Friction						( 0.4f ),
	m_TransformChangedBits			( 0x0 ),
	m_pBody							( NULL ),
	m_BodyTransformChanged			( FALSE )
{
#ifdef CACHE_ACCUMULATED_MOVEMENT
	m_AccumulatedMovementValid = FALSE;
#endif
	m_PhysicsFlags.Set( TPhysicsNode::Flag_Enabled );
	m_PhysicsFlags.Set( TPhysicsNode::Flag_Rotate );
}


//---------------------------------------------------------
//	cleanup
//---------------------------------------------------------
void TLPhysics::TPhysicsNode::Shutdown()
{
	//	remove body
	if ( m_pBody )
	{
		m_pBody->GetWorld()->DestroyBody( m_pBody );
		m_pBody = NULL;

		//	invalidate pointers to all body's shapes
		for ( u32 s=0;	s<m_CollisionShapes.GetSize();	s++ )
			m_CollisionShapes[s].SetBodyShape( NULL );
	}

	//	regular shutdown
	TLGraph::TGraphNode<TLPhysics::TPhysicsNode>::Shutdown();
}

	
//---------------------------------------------------------
//	generic render node init
//---------------------------------------------------------
void TLPhysics::TPhysicsNode::Initialise(TLMessaging::TMessage& Message)
{
	TRef SceneNodeRef;

	//	need to subscribe to a scene node - todo: expand to get all children like this
	if ( Message.ImportData("SubTo",SceneNodeRef) )
	{
		TPtr<TLScene::TSceneNode>& pSceneNode = TLScene::g_pScenegraph->FindNode(SceneNodeRef);
		if ( pSceneNode )
		{
			this->SubscribeTo( pSceneNode );
		}
		else
		{
			TLDebug_Break("Node instructed to subscribe to a scene node that doesn't exist");
		}
	}

	//	need to publish to a scene node - todo: expand to get all children like this
	if ( Message.ImportData("PubTo",SceneNodeRef) )
	{
		TPtr<TLScene::TSceneNode>& pSceneNode = TLScene::g_pScenegraph->FindNode(SceneNodeRef);
		if ( pSceneNode )
		{
			pSceneNode->SubscribeTo( this );
		}
		else
		{
			TLDebug_Break("Node instructed to publish to a scene node that doesn't exist");
		}
	}


	if(Message.ImportData("Owner", m_OwnerSceneNode))
	{
		// Get the scenegraph node
		TPtr<TLScene::TSceneNode> pOwner = TLScene::g_pScenegraph->FindNode(m_OwnerSceneNode);

		if(pOwner.IsValid())
		{
			pOwner->SubscribeTo(this);
			SubscribeTo(pOwner);

			/*
			TPtr<TLMessaging::TEventChannel>& pEventChannel = pOwner->FindEventChannel(TRef_Static(O,n,T,r,a));

			if(pEventChannel)
			{
				// Subscribe to the scene node owners transform channel
				SubscribeTo(pEventChannel);

				// Subscribe the 'scene' node owner to this node so we can sen audio change messages
				pOwner->SubscribeTo(this);
			}
			*/
		}
	}


	//	get physics flags to set
	TPtrArray<TBinaryTree> FlagChildren;
	if ( Message.GetChildren("PFSet", FlagChildren ) )
	{
		u32 FlagIndex = 0;
		for ( u32 f=0;	f<FlagChildren.GetSize();	f++ )
		{
			FlagChildren[f]->ResetReadPos();
			if ( FlagChildren[f]->Read( FlagIndex ) )
			{
				//	handle special case flags
				if ( FlagIndex == Flag_Enabled )
					SetEnabled( TRUE );
				else if ( FlagIndex == Flag_HasCollision )
					EnableCollision( TRUE );
				else
					GetPhysicsFlags().Set( (Flags)FlagIndex );
			}
		}
		FlagChildren.Empty();
	}

	//	get render flags to clear
	if ( Message.GetChildren("PFClear", FlagChildren ) )
	{
		u32 FlagIndex = 0;
		for ( u32 f=0;	f<FlagChildren.GetSize();	f++ )
		{
			FlagChildren[f]->ResetReadPos();
			if ( FlagChildren[f]->Read( FlagIndex ) )
			{
				//	handle special case flags
				if ( FlagIndex == Flag_Enabled )
					SetEnabled( FALSE );
				else if ( FlagIndex == Flag_HasCollision )
					EnableCollision( FALSE );
				else
					GetPhysicsFlags().Clear( (Flags)FlagIndex );
			}
		}
		FlagChildren.Empty();
	}

	//	read physics properties
	if ( Message.ImportData("Friction", m_Friction ) )
		OnFrictionChanged();

	if ( Message.ImportData("Bounce", m_Bounce ) )
		OnBounceChanged();

	if ( Message.ImportData("Damping", m_Damping ) )
		OnDampingChanged();

	//	read collision shapes
	TPtrArray<TBinaryTree> CollisionShapeDatas;
	Message.GetChildren("colshape", CollisionShapeDatas );
	for ( u32 i=0;	i<CollisionShapeDatas.GetSize();	i++ )
	{
		TBinaryTree& ColShapeData = *(CollisionShapeDatas[i]);
		ColShapeData.ResetReadPos();
		TPtr<TLMaths::TShape> pCollisionShape = TLMaths::ImportShapeData( ColShapeData );
		if ( pCollisionShape )
		{
			//	import ref and sensor settings
			TRef ShapeRef;
			Bool IsSensor = FALSE;
			ColShapeData.ImportData("Ref", ShapeRef );
			ColShapeData.ImportData("Sensor", IsSensor );

			//	add the collison shape
			AddCollisionShape( pCollisionShape, IsSensor, ShapeRef );

			//	gr: by default we'll enable collision when a collision shape is specified
			EnableCollision();
		}
	}

	//	read transform
	//	gr: this code needs to change to use SetTransform() to change the body as appropriate too
	u8 TransformChanges = m_Transform.ImportData( Message );
	OnTransformChanged( TransformChanges );
	
	//	broadcast changes in transform NOW
	PublishTransformChanges();

	//	has a joint[s] to create
	TPtrArray<TBinaryTree> JointDatas;
	Message.GetChildren( "Joint", JointDatas );
	for ( u32 i=0;	i<JointDatas.GetSize();	i++ )
	{
		TBinaryTree& JointData = *(JointDatas[i]);
		TJoint NewJoint;
		NewJoint.m_NodeA = this->GetNodeRef();
		
		//	must has an other-node to link to
		if ( !JointData.ImportData("Node", NewJoint.m_NodeB ) )
			continue;
		
		JointData.ImportData("AncPos", NewJoint.m_JointPosA );
		JointData.ImportData("OthAncPos", NewJoint.m_JointPosB );

		//	create new joint
		TLPhysics::g_pPhysicsgraph->AddJoint( NewJoint );
	}


	TLGraph::TGraphNode<TPhysicsNode>::Initialise(Message);
}


//----------------------------------------------------
//	
//----------------------------------------------------
void TLPhysics::TPhysicsNode::ProcessMessage(TLMessaging::TMessage& Message)
{
	//	inherited process message
	TLGraph::TGraphNode<TLPhysics::TPhysicsNode>::ProcessMessage( Message );

	//	add a force 
	if ( Message.GetMessageRef() == "Force" )
	{
		//	read out float3 force
		float3 Force( 0.f, 0.f, 0.f );
		Bool MassRelative = FALSE;
		Message.ImportData( "MassRelative", MassRelative );

		if ( Message.Read( Force ) )
		{
			AddForce( Force, MassRelative );
		}
	}
	else if ( Message.GetMessageRef() == "ResetForces" )
	{
		//	reset forces command
		ResetForces();
	}
}


//----------------------------------------------------
//	before collisions are processed
//----------------------------------------------------
void TLPhysics::TPhysicsNode::Update(float fTimeStep)
{
	if ( !IsEnabled() )
	{
		TLDebug_Print("Shouldnt get here");
		return;
	}

	//	do base update to process messages
	TLGraph::TGraphNode<TLPhysics::TPhysicsNode>::Update( fTimeStep );

	//	add gravity force
	if ( m_PhysicsFlags( Flag_HasGravity ) )
	{
		//	negate as UP is opposite to the direction of gravity.
		float3 GravityForce = g_WorldUpNormal * -g_GravityMetresSec;

		AddForce( GravityForce );
	}
}


//----------------------------------------------------
//	after collisions are handled
//----------------------------------------------------
void TLPhysics::TPhysicsNode::PostUpdate(float fTimeStep,TLPhysics::TPhysicsgraph& Graph,TPtr<TPhysicsNode>& pThis)
{
	if ( m_PhysicsFlags( Flag_HasCollision ) && !HasCollision() )
	{
		PublishTransformChanges();
		return;
	}

	//	get change in transform
	if ( m_pBody )
	{
		u8 ChangedBits = 0x0;
		
		//	get new translation from box2d
		const b2Vec2& BodyPosition = m_pBody->GetPosition();
		float3 NewTranslate( BodyPosition.x, BodyPosition.y, 0.f );
		ChangedBits |= m_Transform.SetTranslateHasChanged( NewTranslate, TLMaths_NearZero );

		//	todo: some how allow existing 3D rotation and the box2D rotation.... 
		//	maybe somehting specificly for the scenenode to handle?
		//	http://grahamgrahamreeves.getmyip.com:1984/Trac/ticket/90
		//	gr: if the object has no rotation, then ignore rotation from box2D
		if ( GetPhysicsFlags().IsSet( Flag_Rotate ) )
		{
			//	get new rotation; todo: store angle for quicker angle-changed test?
			float32 BodyAngleRad = m_pBody->GetAngle();
			TLMaths::TQuaternion NewRotation( float3( 0.f, 0.f, -1.f ), BodyAngleRad );
			ChangedBits |= m_Transform.SetRotationHasChanged( NewRotation, TLMaths_NearZero );
		}

		//	notify of changes
		if ( ChangedBits != 0x0 )
			OnBodyTransformChanged(ChangedBits);
	}

	//	send out transform-changed messages
	PublishTransformChanges();

	//	send out collision messages
	PublishCollisions();
}



//----------------------------------------------------
//	work out our position
//----------------------------------------------------
float3 TLPhysics::TPhysicsNode::GetPosition() const					
{
	float3 Position(0,0,0);

	m_Transform.Transform( Position );
	
	return Position;	
}

//----------------------------------------------------
//	
//----------------------------------------------------
void TLPhysics::TPhysicsNode::SetPosition(const float3& Position) 
{
	if ( GetTransform().HasScale() )
	{
		TLDebug_Break("handle this...");
	}

	//	exclusivly set transform
	m_Transform.SetTranslate( Position );

	//	notify change (will set box body)
	OnTranslationChanged();
}


//----------------------------------------------------
//	
//----------------------------------------------------
void TLPhysics::TPhysicsNode::MovePosition(const float3& Movement,float Timestep)
{
#ifdef _DEBUG
	//	tiny change, dont apply it
	float MovementLengthSq = Movement.LengthSq();
	if ( MovementLengthSq < 0.0001f )
	{
		TLDebug_Break("Tiny movement - should have been caught by caller");
		return;
	}
#endif

	//	change translation
	if ( m_Transform.HasTranslate() )
	{
		float3 NewPosition = m_Transform.GetTranslate();
		NewPosition += Movement * Timestep * MOVEMENT_SCALAR;
		if ( NewPosition.z != 0.f )
		{
			TLDebug_Break("z?");
		}
		m_Transform.SetTranslate( NewPosition );
	}
	else
	{
		m_Transform.SetTranslate( Movement * Timestep * MOVEMENT_SCALAR );
	}

	//	translation has changed
	OnTranslationChanged();
}



//----------------------------------------------------
//	send transform changes as per m_TransformChanges
//----------------------------------------------------
void TLPhysics::TPhysicsNode::PublishTransformChanges()
{
	//	rule is "cant have changed if invalid value" - need to cater for when it has changed TO an invalid value...
	//	so this removes a change if the value is invalid
	m_TransformChangedBits &= m_Transform.GetHasTransformBits();

	//	no changes
	if ( m_TransformChangedBits == 0x0 )
		return;

	//	store changes and reset changed value - done JUST IN CASE this message triggers another message which alters the physics system
	u8 Changes = m_TransformChangedBits;
	m_TransformChangedBits = 0x0;

	//	no one to send a message to 
	if ( !HasSubscribers( TRef_Static(O,n,T,r,a) ) )
		return;

	TLMessaging::TMessage Message( TRef_Static(O,n,T,r,a), GetNodeRef() );

	//	write out transform data - send only if something was written
	if ( m_Transform.ExportData( Message, Changes ) != 0x0 )
	{
		//	send out message
		PublishMessage(Message);
	}
}



//----------------------------------------------------------
//	update tree: update self, and children and siblings
//----------------------------------------------------------
void TLPhysics::TPhysicsNode::PostUpdateAll(float fTimestep,TLPhysics::TPhysicsgraph& Graph,TPtr<TLPhysics::TPhysicsNode>& pThis)
{
	if ( !IsEnabled() )
		return;

	// Update this
	PostUpdate( fTimestep, Graph, pThis );

#ifdef TLGRAPH_OWN_CHILDREN

	TPtrArray<TPhysicsNode>& NodeChildren = GetChildren();
	for ( u32 c=0;	c<NodeChildren.GetSize();	c++ )
	{
		TPtr<TPhysicsNode>& pNode = NodeChildren.ElementAt(c);
		pNode->PostUpdateAll( fTimestep, Graph, pNode );
	}

#else

	// Update children recursivly
	if ( HasChildren() )
	{
		TPtr<TLPhysics::TPhysicsNode>& pChildFirst = GetChildFirst();
		pChildFirst->PostUpdateAll( fTimestep, pGraph, pChildFirst );
	}

	//	update siblings recursivly
	if ( HasNext() )
	{
		TPtr<TLPhysics::TPhysicsNode>& pNext = GetNext();
		pNext->PostUpdateAll( fTimestep, pGraph, pNext );
	}
#endif
}



//----------------------------------------------------------
//	setup collision shape from a shape, add to list, replace existing shape if it already exists
//----------------------------------------------------------
TRef TLPhysics::TPhysicsNode::AddCollisionShape(const TPtr<TLMaths::TShape>& pShape,Bool IsSensor,TRef ShapeRef)
{
	//	check params
	if ( !pShape )
	{
		TLDebug_Break("Shape expected");
		return TRef();
	}

	//	no ref provided, find a free one
	if ( !ShapeRef.IsValid() )
	{
		do
		{
			ShapeRef.Increment();
		}
		while ( m_CollisionShapes.Exists( ShapeRef ) );
	}

	//	find existing shape to repalce
	TCollisionShape* pExistingCollisionShape = GetCollisionShape( ShapeRef );
	if ( pExistingCollisionShape )
	{
#ifdef _DEBUG
		TTempString Debug_String("Collision shape ");
		ShapeRef.GetString( Debug_String );
		Debug_String.Append(" already exists on node ");
		GetNodeRef().GetString( Debug_String );
		Debug_String.Append("... replacing shape");
		TLDebug_Print( Debug_String );
#endif

		//	set new shape
		pExistingCollisionShape->SetShape( pShape );

		//	update the shape by just calling create
		//	failed to update shape, remove it
		if ( CreateBodyShape( *pExistingCollisionShape ) == SyncFalse)
		{
			RemoveCollisionShape( ShapeRef );
			return TRef();
		}

		return ShapeRef;		
	}

	//	else create a new one 
	TCollisionShape NewCollisionShape;
	NewCollisionShape.SetShape( pShape );
	NewCollisionShape.SetShapeRef( ShapeRef );
	NewCollisionShape.SetIsSensor( IsSensor );

	//	create box2d shape
	SyncBool CreateResult = CreateBodyShape( NewCollisionShape );

	//	failed
	if ( CreateResult == SyncFalse )
		return TRef();
	
	//	success! add to list of shapes
	m_CollisionShapes.Add( NewCollisionShape );

	return NewCollisionShape.GetShapeRef();
}


//----------------------------------------------------------
//	remove a collision shape, and it's body shape from the body. returns false if doesn't exist
//----------------------------------------------------------
Bool TLPhysics::TPhysicsNode::RemoveCollisionShape(TCollisionShape& CollisionShape)
{
	//	get index to remove from collision shape list - security check really before removing the body
	s32 CollisionShapeIndex = m_CollisionShapes.FindIndex( CollisionShape );
	if ( CollisionShapeIndex == -1 )
	{
		TLDebug_Break("Collison shape tried to be removed from node which it's not a member of");
		return FALSE;
	}

	//	delete shape from body
	if ( m_pBody )
	{
		if ( CollisionShape.DestroyBodyShape( m_pBody ) )
			OnBodyShapeRemoved();
	}

	//	get index to remove from collision shape list
	m_CollisionShapes.RemoveAt( CollisionShapeIndex );

	return TRUE;
}


//----------------------------------------------------
//	called when we get a collision. return a collision info to write data into. return NULL to pre-empt not sending any collision info out (eg. if no subscribers)
//----------------------------------------------------
TLPhysics::TCollisionInfo* TLPhysics::TPhysicsNode::OnCollision()
{
	//	no subscribers for message, so dont bother making up data
	if ( !HasSubscribers("OnCollision") )
		return NULL;

	//	add new collision info and return it
	TLPhysics::TCollisionInfo* pNewCollisionInfo = m_Collisions.AddNew();
	
	//	initialise a little
	if ( pNewCollisionInfo )
		pNewCollisionInfo->SetIsNewCollision( TRUE );

	return pNewCollisionInfo;
}


//----------------------------------------------------
//	called when we are no longer colliding with a node
//----------------------------------------------------
void TLPhysics::TPhysicsNode::OnEndCollision(TRefRef ShapeRef,TLPhysics::TPhysicsNode& OtherNode,TRefRef OtherShapeRef)
{
	//	create a special TCollisionInfo with end-of-collision info
	TLPhysics::TCollisionInfo* pEndCollisionInfo = TLPhysics::TPhysicsNode::OnCollision();
	if ( !pEndCollisionInfo )
		return;

	pEndCollisionInfo->SetIsEndOfCollision( ShapeRef, OtherNode, OtherShapeRef );
}


//----------------------------------------------------
//	send out collision message
//----------------------------------------------------
void TLPhysics::TPhysicsNode::PublishCollisions()
{
	//	no collisions occured
	if ( !m_Collisions.GetSize() )
		return;

	//	if no subscribers, just ditch the data
	if ( !HasSubscribers("OnCollision") )
	{
		m_Collisions.Empty();
		return;
	}

	//	make message
	TLMessaging::TMessage Message("OnCollision", GetNodeRef() );
	
	//	write our owner to the message
	if ( GetOwnerSceneNodeRef().IsValid() )
		Message.ExportData("owner", GetOwnerSceneNodeRef() );

	//	add an entry for each collision
	for ( u32 c=0;	c<m_Collisions.GetSize();	c++ )
	{
		TLPhysics::TCollisionInfo& CollisionInfo = m_Collisions[c];

		//	write the data as "end collision" data if it's not a new collison
		TRef CollisionDataRef = CollisionInfo.IsEndOfCollision() ? "EndCollision" : "Collision";
		TPtr<TBinaryTree>& pCollisionData = Message.AddChild( CollisionDataRef );

		//	write data
		m_Collisions[c].ExportData( *pCollisionData );
	}

	//	send!
	PublishMessage( Message );

	//	reset array list
	m_Collisions.Empty();
}


//-------------------------------------------------------------
//	explicit change of transform
//-------------------------------------------------------------
void TLPhysics::TPhysicsNode::SetTransform(const TLMaths::TTransform& NewTransform,Bool PublishChanges)
{
	//	copy transform
	u8 Changes = m_Transform.SetHasChanged( NewTransform );

	//	no changes
	if ( Changes == 0x0 )
		return;

	//	note changes and set body transform
	if ( PublishChanges )
	{
		OnTransformChanged( Changes );
	}
	else
	{
		OnTransformChangedNoPublish();
	}
}


	
//-------------------------------------------------------------
//	create the body in the world
//-------------------------------------------------------------
Bool TLPhysics::TPhysicsNode::CreateBody(b2World& World)
{
	const TLMaths::TTransform& Transform = GetTransform();

	b2BodyDef BodyDef;

	//	get values for the box2D body transform
	GetBodyTransformValues( BodyDef.position, BodyDef.angle );

	//	fix rotation as neccessary
	if ( GetPhysicsFlags().IsSet( Flag_Rotate ) )
		BodyDef.fixedRotation = FALSE;
	else
		BodyDef.fixedRotation = TRUE;

	//	set user data as a pointer back to self (could use ref...)
	//	if this changes, change TLPhysics::GetPhysicsNodeFromBody()
	BodyDef.userData = GetBodyUserDataFromPhysicsNode( this );

	BodyDef.linearDamping = m_Damping;

	//	allocate body
	m_pBody = World.CreateBody(&BodyDef);
	if ( !m_pBody  )
		return FALSE;

	//	created body with correct transform, so cannot be out of date
	m_BodyTransformChanged = FALSE;

	//	create shape definition from our existing collision shape
	for ( u32 s=0;	s<m_CollisionShapes.GetSize();	s++ )
	{
		if ( CreateBodyShape( m_CollisionShapes[s] ) != SyncTrue )
		{
			TLDebug_Break("todo: handle this - wait means try again sometime - false means shape is invalid");
		}
	}


	return TRUE;
}

//-------------------------------------------------------------
//	get the body shape (fixture) for this shape 
//-------------------------------------------------------------
b2Fixture* TLPhysics::TPhysicsNode::GetBodyShape(TRefRef ShapeRef)
{
	//	need a body
	if ( !m_pBody )
		return NULL;
	
	//	loop through list to find fixture with this shape ref
	b2Fixture* pShape = m_pBody->GetFixtureList();
	while ( pShape )
	{
		//	get check ref
		TRef BodyShapeRef = TLPhysics::GetShapeRefFromShape( pShape );
		if ( BodyShapeRef == ShapeRef )
			return pShape;

		pShape = pShape->GetNext();
	}

	//	no matches
	return NULL;
}

//-------------------------------------------------------------
//	create/recreate this shape on the body. if FALSE - it failed, if WAIT then we're waiting for a body to add it to
//-------------------------------------------------------------
SyncBool TLPhysics::TPhysicsNode::CreateBodyShape(TCollisionShape& CollisionShape)
{
	//	need a box2d body
	if ( !m_pBody )
		return SyncWait;

	//	update existing shape
	if ( CollisionShape.GetBodyShape() )
	{
		if ( CollisionShape.UpdateBodyShape() )
			return SyncTrue;
		
		//	update of shape failed, delete it
		CollisionShape.DestroyBodyShape( m_pBody );
	}

	//	create new shape
	TLMaths::TShape* pShape = CollisionShape.GetShape();
	if ( !pShape )
	{
		TLDebug_Break("shape expected");
		return SyncFalse;
	}

	//	if we have to scale shape because of our node transform, make up a new shape
	TPtr<TLMaths::TShape> pScaledCollisionShape = NULL;
	if ( GetTransform().HasScale() )
	{
		TLMaths::TTransform ScaleTransform;
		ScaleTransform.SetScale( GetTransform().GetScale() );
		pScaledCollisionShape = pShape->Transform( ScaleTransform, TLPtr::GetNullPtr<TLMaths::TShape>() );
		pShape = pScaledCollisionShape;
	}

	//	get a new shape def
	b2CircleDef TempCircleDef;
	b2PolygonDef TempPolygonDef;
	b2FixtureDef* pNewShapeDef = TLPhysics::GetShapeDefFromShape(TempCircleDef,TempPolygonDef, *pShape );
	if ( !pNewShapeDef )
	{
#ifdef _DEBUG
		TTempString Debug_String("failed to make shape definition for new body shape, unsupported TShape -> box2d shape? ");
		pShape->GetShapeType().GetString( Debug_String );
		TLDebug_Break( Debug_String );
#endif
		return SyncFalse;
	}

	//	initialise shape

	//	node properties
	pNewShapeDef->density = 1.f;		//	make everything rigid
	pNewShapeDef->friction = m_Friction;
	pNewShapeDef->restitution = m_Bounce;	//	zero restitution = no bounce

	//	set shape properties
	pNewShapeDef->isSensor = CollisionShape.IsSensor();
	pNewShapeDef->userData = TLPhysics::GetShapeUserDataFromShapeRef( CollisionShape.GetShapeRef() );

	//	create shape
	b2Fixture* pNewBodyShape = m_pBody->CreateFixture( pNewShapeDef );

	//	update collision shape
	CollisionShape.SetBodyShape( pNewBodyShape );

	if ( pNewBodyShape )
	{
		OnBodyShapeAdded( CollisionShape );
		return SyncTrue;
	}
	else
	{
		//	everything's okay, but failed to add to body... maybe body is disabled?
		//	queue up to add again
		TLDebug_Break("Find out what this circumstance might be");
		return SyncWait;
	}
}


//-------------------------------------------------------------
//	reset the body's transform
//-------------------------------------------------------------
void TLPhysics::TPhysicsNode::SetBodyTransform()
{
	if ( !m_pBody )
	{
		m_BodyTransformChanged = TRUE;
		return;
	}

	b2Vec2 Translate( 0.f, 0.f );
	float32 AngleRadians( 0.f );
	GetBodyTransformValues( Translate, AngleRadians );

	//	set new transform (BEFORE refiltering contact points)
	if ( !m_pBody->SetXForm( Translate, AngleRadians ) )
	{
		//	failed to set the transform, must be frozen
		m_BodyTransformChanged = TRUE;
		return;
	}
	
	//	transform has been set, is now valid
	m_BodyTransformChanged = FALSE;

	//	wake up body (Not sure if this has to be BEFORE the transforms...)
	//	gr: don't think this needs to be done before the transform, SO, because we don't want 
	//	to wake up the body when disabled/frozen (ie. failed SetXForm above) then I've moved
	//	this to after SetXForm (has/hasnt failed)
	m_pBody->WakeUp();

	//	refilter on world to reset contact points of the shapes
	TPtr<b2World>& pWorld = TLPhysics::g_pPhysicsgraph->GetWorld();
	if ( pWorld )
	{
		b2Fixture* pShape = m_pBody->GetFixtureList();
		while ( pShape )
		{
			//	tell graph to refilter
			TLPhysics::g_pPhysicsgraph->RefilterShape( pShape );
			pShape = pShape->GetNext();
		}
	}
}


//-------------------------------------------------------------
//	called when collision is enabled/disabled 
//-------------------------------------------------------------
void TLPhysics::TPhysicsNode::OnCollisionEnabledChanged(Bool IsNowEnabled)
{
	//	gr: group stuff all ditched. Collision handled in the contact listener on the graph
}



//-------------------------------------------------------------
//	called when node is enabled/disabled - hide from box world when disabled
//-------------------------------------------------------------
void TLPhysics::TPhysicsNode::OnNodeEnabledChanged(Bool IsNowEnabled)
{
	//	freeze body with box2d's functionality (added by me)
	if ( m_pBody )
	{
		if ( IsNowEnabled )
		{
			m_pBody->UnFreeze();

			//	if body transform is out of date, (eg. changed when disabled) set it
			if ( m_BodyTransformChanged )
				SetBodyTransform();
		}
		else
		{
			m_pBody->Freeze();
		}
	}	
}


//-------------------------------------------------------------
//	shape properties changed
//-------------------------------------------------------------
void TLPhysics::TPhysicsNode::OnShapeDefintionChanged()
{
	//	no changes to implement as shape isn't created
	if ( !GetBody() )
		return;

	TLDebug_Break("todo: alter body shape's properties (friction, restitution etc)");
}


//-------------------------------------------------------------
//	get a more exact array of all the box 2D body shapes
//-------------------------------------------------------------
void TLPhysics::TPhysicsNode::GetCollisionShapesWorld(TPtrArray<TLMaths::TShape>& ShapeArray) const
{
	if ( !m_pBody )
		return;

	//	for each shape in the body, get a world-transformed shape
	b2Fixture* pBodyShape = m_pBody->GetFixtureList();
	while ( pBodyShape )
	{
		ShapeArray.Add( TLPhysics::GetShapeFromBodyShape( *pBodyShape, GetTransform() ) );
		pBodyShape = pBodyShape->GetNext();
	}

}



//-------------------------------------------------------------
//	apply a force to the body
//-------------------------------------------------------------
void TLPhysics::TPhysicsNode::AddForce(const float3& Force,Bool MassRelative)
{
	//	nothing to apply
	if ( Force.IsZero() )
		return;

	if ( !m_pBody )
	{
		TLDebug_Warning("Force applied when no body... accumulate into force buffer to apply when body is created?");
		return;
	}
	
	//	I'm assuming force won't be applied when disabled (body is frozen)
	if ( !IsEnabled() )
	{
		TLDebug_Warning("Force applied when disabled... not sure this will apply");
	}

	//	multiply by the mass if it's not mass relative otherwise box will scale down the effect of the force. 
	//	eg. gravity doesn't want to be mass related otherwise things will fall at the wrong rates
	float Mass = MassRelative ? 1.f : m_pBody->GetMass();

	//	gr: apply the force at the world center[mass center] of the body
	m_pBody->ApplyForce( b2Vec2(Force.x*Mass,Force.y*Mass) , m_pBody->GetWorldCenter() );	
}


//-------------------------------------------------------------
//	get values to put INTO the box2D body transform from our transform
//-------------------------------------------------------------
void TLPhysics::TPhysicsNode::GetBodyTransformValues(b2Vec2& Translate,float32& AngleRadians)
{
	const TLMaths::TTransform& Transform = GetTransform();

	//	get simple translation
	if ( Transform.HasTranslate() )
		Translate.Set( Transform.GetTranslate().x, Transform.GetTranslate().y );
	else
		Translate.Set( 0.f, 0.f );

	//	work out Z axis rotation
	if ( Transform.HasRotation() )
		AngleRadians = Transform.GetRotation().GetAngle2D().GetRadians();
	else
		AngleRadians = TLMaths::TAngle::DegreesToRadians( 0.f );

}


//-------------------------------------------------------------
//
//-------------------------------------------------------------
void TLPhysics::TPhysicsNode::SetCollisionNone()
{
	TLDebug_Break("if required - remove all collision shapes with a function that removes the shape from the body");
}


//-------------------------------------------------------------
//	body shape has been added
//-------------------------------------------------------------
void TLPhysics::TPhysicsNode::OnBodyShapeAdded(TCollisionShape& CollisionShape)
{
	//	gr: same code atm
	OnBodyShapeRemoved();

}

//-------------------------------------------------------------
//	body shape has been removed
//-------------------------------------------------------------
void TLPhysics::TPhysicsNode::OnBodyShapeRemoved()
{
	if ( !m_pBody )
	{
		TLDebug_Break("Body expected");
		return;
	}

	//	generate new mass value for the body. If static mass must be zero
	if ( IsStatic() )
		m_pBody->SetStatic();
	else
		m_pBody->SetMassFromShapes();
}


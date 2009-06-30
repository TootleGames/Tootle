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





TLPhysics::TPhysicsNode::TPhysicsNode(TRefRef NodeRef,TRefRef TypeRef) :
	TLGraph::TGraphNode<TPhysicsNode>	( NodeRef, TypeRef ),
	m_Bounce						( 0.0f ),
	m_Damping						( 0.0f ),
	m_Friction						( 0.4f ),
	m_Temp_ExtrudeTimestep			( 0.f ),
	m_TransformChangedBits			( 0x0 ),
	m_pBody							( NULL )
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

	//	read collision shape
	TPtr<TBinaryTree>& pColShapeData = Message.GetChild("Colshape");
	if ( pColShapeData )
	{
		pColShapeData->ResetReadPos();
		TPtr<TLMaths::TShape> pCollisionShape = TLMaths::ImportShapeData( *pColShapeData );
		if ( pCollisionShape )
		{
			SetCollisionShape( pCollisionShape );

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

	//	init per-frame stuff
	m_Temp_ExtrudeTimestep = fTimeStep;
	SetAccumulatedMovementInvalid();

	if ( m_PhysicsFlags( Flag_HasCollision ) && !HasCollision() )
		return;

	//	set the pre-update pos
//	m_LastPosition = m_Position;

	if ( m_PhysicsFlags( Flag_HasGravity ) )
	{
		//	add gravity
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
		const b2Vec2& BodyPosition = m_pBody->GetPosition();
		float32 BodyAngleRad = m_pBody->GetAngle();

		//	get new transform from box2d
		float3 NewTranslate( BodyPosition.x, BodyPosition.y, 0.f );
		
		//	get new rotation; todo: store angle for quicker angle-changed test?
		TLMaths::TQuaternion NewRotation( float3( 0.f, 0.f, -1.f ), BodyAngleRad );

		u8 ChangedBits = 0x0;
		ChangedBits |= m_Transform.SetTranslateHasChanged( NewTranslate, TLMaths_NearZero );
		ChangedBits |= m_Transform.SetRotationHasChanged( NewRotation, TLMaths_NearZero );

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
	if ( GetCollisionShapeTransform().HasScale() )
	{
		TLDebug_Break("handle this...");
	}

	//	exclusivly set transform
	m_Transform.SetTranslate( Position );

	//	set transform on box2d body
	SetBodyTransform();

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
//	setup collision shape from a shape
//----------------------------------------------------------
void TLPhysics::TPhysicsNode::SetCollisionShape(const TPtr<TLMaths::TShape>& pShape)
{
	//	set shape
	m_pCollisionShape = pShape;

	//	create box2d shape
	CreateBodyShape();
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
	
	return pNewCollisionInfo;
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
		TPtr<TBinaryTree>& pCollisionData = Message.AddChild("Collision");

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

	//	set initial transform
	if ( Transform.HasTranslate() )
		BodyDef.position.Set( Transform.GetTranslate().x, Transform.GetTranslate().y );
	else
		BodyDef.position.Set( 0.f, 0.f );

	if ( Transform.HasRotation() )
		BodyDef.angle = Transform.GetRotation().GetAngle2D().GetRadians();

	//	fix rotation as neccessary
	if ( GetPhysicsFlags().IsSet( Flag_Rotate ) )
		BodyDef.fixedRotation = FALSE;
	else
		BodyDef.fixedRotation = TRUE;

	//	set user data as a pointer back to self (could use ref...)
	//	if this changes, change TLPhysics::GetPhysicsNodeFromBody()
	BodyDef.userData = this;

	BodyDef.linearDamping = m_Damping;

	//	allocate body
	m_pBody = World.CreateBody(&BodyDef);
	if ( !m_pBody  )
		return FALSE;

	//	create shape definition from our existing collision shape
	if ( m_pCollisionShape )
	{
		if ( !CreateBodyShape() )
			return FALSE;
	}

	return TRUE;
}


//-------------------------------------------------------------
//	when our collision shape changes we recreate the shape on the body
//-------------------------------------------------------------
Bool TLPhysics::TPhysicsNode::CreateBodyShape()
{
	//	need a box2d body
	if ( !m_pBody )
		return FALSE;

	//	gr: optimisation here; according to code documentation, you can change a shape's vertexes at runtime (needs same number of verts)
	//		which would save all the filtering etc

	//	destroy existing shapes/fixtures
	b2Fixture* pExistingShape = m_pBody->GetFixtureList();
	while ( pExistingShape )
	{
		//	store next
		b2Fixture* pNextShape = pExistingShape->GetNext();

		//	remove from body
		m_pBody->DestroyFixture( pExistingShape );

		pExistingShape = pNextShape;
	}

	//	no collision shape to work from
	if ( !m_pCollisionShape )
		return FALSE;

	//	create new shape
	if ( !m_pCollisionShape->IsValid() )
	{
		TLDebug_Break("Trying to create box2d shape from invalid shape");
		return FALSE;
	}

	//	try to get a circle definition first...
	b2CircleDef CircleDef;
	b2PolygonDef PolygonDef;
	b2FixtureDef* pShapeDef = NULL;
	TLMaths::TShape* pCollisionShape = m_pCollisionShape;
	TPtr<TLMaths::TShape> pScaledCollisionShape = NULL;

	//	gr: not decided how to do this yet... if the scale changes we will need to re-create the body shape...
	//		as a scale doesnt come OUT of box2d body, we should retain this scale
	//		but SCALE is the ONLY thing to apply to the shape
	if ( GetTransform().HasScale() )
	{
		TLMaths::TTransform ScaleTransform;
		ScaleTransform.SetScale( GetTransform().GetScale() );
		pScaledCollisionShape = m_pCollisionShape->Transform( ScaleTransform, TLPtr::GetNullPtr<TLMaths::TShape>() );
		pCollisionShape = pScaledCollisionShape;
	}

	//	apply scale to the shape

	if ( TLPhysics::GetCircleDefFromShape( CircleDef, *pCollisionShape ) )
	{
		pShapeDef = &CircleDef;
	}
	else if ( TLPhysics::GetPolygonDefFromShape( PolygonDef, *pCollisionShape ) )
	{
		pShapeDef = &PolygonDef;
	}
	else
	{
#ifdef _DEBUG
		TTempString Debug_String("Unsupported TShape -> box2d shape? ");
		m_pCollisionShape->GetShapeType().GetString( Debug_String );
		TLDebug_Print( Debug_String );
#endif
		return FALSE;
	}

	//	setup generic shape definition stuff
	b2FixtureDef& ShapeDef = *pShapeDef;

	//	make everything rigid
	ShapeDef.density = 1.f;

	ShapeDef.friction = m_Friction;

	//	zero restitution = no bounce
	ShapeDef.restitution = m_Bounce;

	//	make sensor
	if ( GetPhysicsFlags().IsSet( Flag_IsSensor ) )
		ShapeDef.isSensor = TRUE;
	else
		ShapeDef.isSensor = FALSE;

	//	create shape
	m_pBody->CreateFixture( &ShapeDef );

	//	generate new mass value for the body. If static mass must be zero
	if ( !IsStatic() )
		m_pBody->SetMassFromShapes();

	return TRUE;
}


//-------------------------------------------------------------
//	reset the body's transform
//-------------------------------------------------------------
void TLPhysics::TPhysicsNode::SetBodyTransform()
{
	if ( !m_pBody )
		return;

	const TLMaths::TTransform& Transform = GetTransform();
	float AngleRadians = Transform.GetRotation().GetAngle2D().GetRadians();

	//	wake up body (Not sure if this has to be BEFORE the transforms...)
	m_pBody->WakeUp();

	//	set new transform (BEFORE refiltering contact points)
	m_pBody->SetXForm( b2Vec2( Transform.GetTranslate().x, Transform.GetTranslate().y ), AngleRadians );
	
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
			m_pBody->UnFreeze();
		else
			m_pBody->Freeze();
	}	
}


//-------------------------------------------------------------
//	shape properties changed
//-------------------------------------------------------------
void TLPhysics::TPhysicsNode::OnShapeDefintionChanged()
{
	//	no changes to implement as shape isn't created
	//if ( !GetBodyShape() )
	if ( !GetBody() )
		return;

	TLDebug_Break("todo: alter body shape's properties (friction, restitution etc)");
}


//-------------------------------------------------------------
//	get a more exact array of all the box 2D body shapes
//-------------------------------------------------------------
void TLPhysics::TPhysicsNode::GetBodyWorldShapes(TPtrArray<TLMaths::TShape>& ShapeArray)
{
	//	get all the bodies
	TFixedArray<b2Body*,100> Bodies;
	GetBodys( Bodies );

	//	for each shape in each body, get a world-transformed shape
	for ( u32 b=0;	b<Bodies.GetSize();	b++ )
	{
		b2Body* pBody = Bodies[b];
		b2Fixture* pBodyShape = pBody->GetFixtureList();
		while ( pBodyShape )
		{
			ShapeArray.Add( TLPhysics::GetShapeFromBodyShape( *pBodyShape, GetTransform() ) );
			pBodyShape = pBodyShape->GetNext();
		}
	}

}



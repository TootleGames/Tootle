#include "TPhysicsNode.h"
#include "TPhysicsGraph.h"
#include <TootleCore/TLMaths.h>
#include <TootleCore/TLTime.h>
#include <TootleScene/TScenegraph.h>
#include <TootleCore/TEventChannel.h>
#include <TootleMaths/TShapeSphere.h>
#include <TootleMaths/TShapeBox.h>


#define USE_ZERO_GROUP


//namespace Box2D
	#include <box2d/include/box2d.h>


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

//	gr: simple is faster, possibly could make non-simple one save doing same in-zone-shape checks over and over..
#define SIMPLE_UPDATE_ZONE	//	changes update zone to just traverse from root down, instead of current zone up
//#define COMPLEX_UPDATE_ZONE	//	changes update zone to just traverse from root down, instead of current zone up
//#define NEW_UPDATE_ZONE	//	changes update zone to just traverse from root down, instead of current zone up


namespace TLPhysics
{
	float3		g_WorldUp( 0.f, -1.f, 0.f );
	float3		g_WorldUpNormal( 0.f, -1.f, 0.f );

	float		g_GravityMetresSec	= 20.f;	//	gravity in metres per second (1unit being 1metre)
//	float		g_GravityMetresSec	= 9.81f;	//	gravity in metres per second (1unit being 1metre)
}


namespace TLRef
{
	TLArray::SortResult		RefSort(const TRef& aRef,const TRef& bRef,const void* pTestVal);	//	simple ref-sort func - for arrays of TRef's
}





TLPhysics::TPhysicsNode::TPhysicsNode(TRefRef NodeRef,TRefRef TypeRef) :
	TLGraph::TGraphNode<TPhysicsNode>	( NodeRef, TypeRef ),
	m_Mass							( 1.0f ),
	m_Bounce						( 1.0f ),
	m_Squidge						( 0.f ),
#ifndef USE_BOX2D
	m_Friction						( 0.4f ),
#endif
	m_Temp_ExtrudeTimestep			( 0.f ),
	m_InitialisedZone				( FALSE ),
	m_WorldCollisionShapeChanged	( FALSE ),
	m_TransformChangedBits			( 0x0 ),
	m_pBody							( NULL )
{
#ifdef CACHE_ACCUMULATED_MOVEMENT
	m_AccumulatedMovementValid = FALSE;
#endif
	m_PhysicsFlags.Set( TPhysicsNode::Flag_Enabled );


}


//---------------------------------------------------------
//	cleanup
//---------------------------------------------------------
void TLPhysics::TPhysicsNode::Shutdown()
{
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


	//	read collision shape
	TPtr<TBinaryTree>& pColShapeData = Message.GetChild("Colshape");
	if ( pColShapeData )
	{
		pColShapeData->ResetReadPos();
		TPtr<TLMaths::TShape> pCollisionShape = TLMaths::ImportShapeData( *pColShapeData );
		if ( pCollisionShape )
			SetCollisionShape( pCollisionShape );		
	}

	//	read transform
	u8 TransformChanges = m_Transform.ImportData( Message );
	OnTransformChanged( TransformChanges );
	
	//	broadcast changes in transform NOW
	PublishTransformChanges();

	//	get physics flags to set
	TPtrArray<TBinaryTree> FlagChildren;
	if ( Message.GetChildren("PFSet", FlagChildren ) )
	{
		u32 FlagIndex = 0;
		for ( u32 f=0;	f<FlagChildren.GetSize();	f++ )
		{
			FlagChildren[f]->ResetReadPos();
			if ( FlagChildren[f]->Read( FlagIndex ) )
				GetPhysicsFlags().Set( (Flags)FlagIndex );
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
				GetPhysicsFlags().Clear( (Flags)FlagIndex );
		}
		FlagChildren.Empty();
	}

	TLGraph::TGraphNode<TPhysicsNode>::Initialise(Message);
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
//	m_WorldCollisionShapeChanged = FALSE;	//	gr: not reset PER FRAME, this can be changed externally too. reset once we send out our shape-changed message
	m_Temp_ExtrudeTimestep = fTimeStep;
	SetAccumulatedMovementInvalid();

	if ( m_PhysicsFlags( Flag_HasCollision ) && !HasCollision() )
		return;

	//	gr: doesn't apply if we have no zone
	if ( m_PhysicsFlags( Flag_ZoneExpected ) &&!m_InitialisedZone )
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
	if ( m_PhysicsFlags( Flag_ZoneExpected ) && !m_InitialisedZone )
	{
		if ( HasCollision() )
		{
			UpdateNodeCollisionZone( pThis, Graph );
			m_InitialisedZone = TRUE;
		}
	}

	if ( m_PhysicsFlags( Flag_HasCollision ) && !HasCollision() )
	{
		PublishTransformChanges();
		return;
	}

	#ifdef USE_BOX2D
	{
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
			ChangedBits |= m_Transform.SetRotationHasChanged( NewRotation );

			//	notify of changes
			if ( ChangedBits != 0x0 )
			{
				if ( ChangedBits == TLMaths_TransformBitRotation )
				{
					OnRotationChanged();
				}
				else
				{
					OnTransformChanged(ChangedBits);
				}

				//	after moving node, mark that the zone needs updating
				SetCollisionZoneNeedsUpdate();
			}
		}
	}
	#else // USE_BOX2D
	{
		//TLDebug_Print( TString("Velocity(%3.3f,%3.3f,%3.3f) Force(%3.3f,%3.3f,%3.3f) \n", m_Velocity.x, m_Velocity.y, m_Velocity.z, m_Force.x, m_Force.y, m_Force.z ) );

		//	update physics movement
		if ( DEBUG_FLOAT_CHECK( m_Force ) && HAS_MIN_CHANGE3(m_Force) )
		{
			m_Velocity += m_Force;
			m_Force.Set( 0.f, 0.f, 0.f );
			OnVelocityChanged();
			OnForceChanged();
		}
		
		//	move pos
		float VelocityLengthSq = m_Velocity.LengthSq();
		if ( VelocityLengthSq > TLMaths_NearZero )
		{
			if ( DEBUG_FLOAT_CHECK( m_Velocity ) )
			{
				MovePosition( m_Velocity, fTimeStep );
			}

			//	reduce velocity
			DEBUG_FLOAT_CHECK( m_Velocity );
			float Dampening = 1.f - ( GetFriction() * fTimeStep * FRICTION_SCALAR );
			if ( Dampening >= 1.f )
			{
				if ( Dampening > 1.f )
				{
					TLDebug_Break("Dampening should not increase...");
					Dampening = 1.f;
				}

				//	no change to velocity
			}
			else if ( Dampening < TLMaths_NearZero )
			{
				//	dampening is tiny, so stop
				Dampening = 0.f;
				m_Velocity.Set( 0.f, 0.f, 0.f );
				OnVelocityChanged();
			}
			else
			{
				DEBUG_FLOAT_CHECK( Dampening );
				m_Velocity *= Dampening;
				OnVelocityChanged();
			}
		}

		//	reset force
		if ( HAS_MIN_CHANGE3( m_Force ) )
		{
			m_Force.Set( 0.f, 0.f, 0.f );
			OnForceChanged();
		}
	}
	#endif // USE_BOX2D

	//	update collision zone
	if ( m_PhysicsFlags( Flag_ZoneExpected ) && GetCollisionZoneNeedsUpdate() )
	{
#ifndef USE_BOX2D
		UpdateNodeCollisionZone( pThis, Graph );
#endif

		//	no longer needs update
		SetCollisionZoneNeedsUpdate( FALSE );
	}


#ifdef USE_BOX2D
	m_WorldCollisionShapeChanged |= (m_TransformChangedBits!=0x0);
#endif

	//	notify that world collison shape has changed
	if ( m_WorldCollisionShapeChanged )
	{
		if ( HasSubscribers() )
		{
			TLMessaging::TMessage Message("ColShape", GetNodeRef() );

			//	write whether we have a shape - if not, then we've invalidated our shape, but no use for it yet so it hasnt been re-calculated
			#ifdef USE_BOX2D
				Bool HasShape = GetCollisionShape().IsValid();
			#else
				Bool HasShape = GetWorldCollisionShape().IsValid();
			#endif
			Message.Write( HasShape );

			PublishMessage( Message );
		}

		m_WorldCollisionShapeChanged = FALSE;
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

	//	after moving node, mark that the zone needs updating
	SetCollisionZoneNeedsUpdate();
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

	//	after moving node, mark that the zone needs updating
	SetCollisionZoneNeedsUpdate();
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
	if ( !HasSubscribers() )
		return;

	TLMessaging::TMessage Message(TRef_Static(O,n,T,r,a), GetNodeRef() );

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
//	called after iteration, return TRUE to do another iteration
//----------------------------------------------------------
Bool TLPhysics::TPhysicsNode::PostIteration(u32 Iteration)
{	
	//SetWorldCollisionShapeInvalid();
	return (Iteration+1 <= 1);
}

/*
//----------------------------------------------------------
//	setup a polygon collision shape with this mesh
//----------------------------------------------------------
void TLPhysics::TPhysicsNode::SetCollisionShape(TRefRef MeshRef)
{
	//	todo: see if it's already a polygon and just update it
	TPtr<TCollisionMesh> pCollisionMesh = new TCollisionMesh;
	m_pCollisionShape = pCollisionMesh;
	pCollisionMesh->SetMeshRef( MeshRef );

	//	invalidate zone
	SetCollisionZoneNeedsUpdate();

	SetWorldCollisionShapeInvalid();
}
*/

//----------------------------------------------------------
//	setup collision shape from a shape
//----------------------------------------------------------
void TLPhysics::TPhysicsNode::SetCollisionShape(const TPtr<TLMaths::TShape>& pShape)
{
	//	set shape
	m_pCollisionShape = pShape;

	//	invalidate zone
	SetCollisionZoneNeedsUpdate();

	//	invalidate WORLD shape
	SetWorldCollisionShapeInvalid();

	//	create box2d shape
	CreateBodyShape();
}


//----------------------------------------------------------
//	handle collision with other object - returns TRUE if we changed anything in the collison
//----------------------------------------------------------
Bool TLPhysics::TPhysicsNode::OnCollision(const TPhysicsNode& OtherNode)
{
	TLMaths::TIntersection& Intersection = m_Temp_Intersection;
	const TLMaths::TIntersection& OtherIntersection = OtherNode.m_Temp_Intersection;
	Bool bChanges = FALSE;

	Bool ForceToEdge = TRUE;
	Bool AddImpulse = FALSE;

	float3 vReboundForce(0,0,0);
	float3 vImpulse(0,0,0);

	//	bouncy stuff
	if ( AddImpulse )
	{
	//	float3 Dist = pOtherNode->GetWorldCollisionShape()->GetCenter() - this->GetWorldCollisionShape()->GetCenter();
		float3 Dist = Intersection.m_Intersection - this->GetWorldCollisionShape()->GetCenter();
		float DistDot2 = Dist.DotProduct(Dist);  

		// balls are too embedded to be of any use    
		if ( DistDot2 >= TLMaths_NearZero )
		{
			//	normalise distance
			Dist /= sqrtf(DistDot2);  
			
			// relative velocity of ball2 to ball1   
			float3 CollisionForce = OtherNode.GetVelocity() - this->GetVelocity();
			float VdotN = CollisionForce.DotProduct(Dist);        

			// balls are separating, no need to add impulse 
			// else make them bounce against each other.  
			// there is a small threshold value in case they are moving 
			// very very slowly towards each other.    
			if ( VdotN < -TLMaths_NearZero ) 
			{
				// calculate the amount of impulse
				float BounceFactor = m_Bounce + OtherNode.m_Bounce;
				float htotal = (-BounceFactor * VdotN) / (m_Mass + OtherNode.m_Mass); 
			//	float htotal = (-BounceFactor * VdotN) / (m_Mass); 

				//	hit strength relative to mass of other object
			//	float htotal *= pOtherNode->m_Mass;    
				
				//	apply power to velocity rather than force
			//	m_Velocity -= Dist * htotal;

				vImpulse = CollisionForce.Normal() * htotal;

#ifndef USE_BOX2D
				m_Velocity += vImpulse;
#endif
				bChanges = TRUE;
			}
		}
	}

	//	move away from intersection via force
	if ( ForceToEdge )
	{
		//	we want to move our intersection point up to the edge of where the other object intersected
		float3& Delta =Intersection.m_PostCollisionDelta;
		Delta = OtherIntersection.m_Intersection - Intersection.m_Intersection;

		float Factor = 0.f;

		//	if colliding with something that doesn't move then force ourselves to move as far as possible(the required amoutn)
		if ( OtherNode.IsStatic() )
		{
			//	gr: no squidge against static objects...
			Factor = 1.0f;
			//	gr: to combat bouncing when stuck between two static objects we move a little less than all-our-force
			Factor = 0.95f;
			//	gr: heavily reduce squidge rather than none
			//Factor *= 1.f - (m_Squidge*0.25f);
		}
		else
		{
			//	gr: todo: change the factor to a mass ratio so the lighter object moves by a larger factor than the heavier object
			Factor = 0.5f;

			//	squidge against other objects (todo: incorporate squidge of other object)
			#ifdef FORCE_SQUIDGE
			float Squidge = FORCE_SQUIDGE;
			#else
			float Squidge = m_Squidge;
			#endif

			Factor *= 1.f - Squidge;
		}

		//	change the movement delta if we're against a static object (so definately is moved)
		//	if it's a soft object (not static) then change the velocity
		if ( HAS_MIN_CHANGE( Factor ) )
		{
			//	if we don't have a normal from the intersection, make up our own
			if ( !Intersection.m_HasNormal )
			{
				Intersection.m_Normal = Delta;
				Intersection.m_HasNormal = TRUE;
			}

 			if ( DEBUG_FLOAT_CHECK( Factor ) && DEBUG_FLOAT_CHECK( Delta ) )
			{
				Delta *= Factor;
			}

			vReboundForce = Delta;

			AddForce( vReboundForce );

			bChanges = TRUE;
		}
	}

	// For now only send a message when colliding with static objects - this will need changing in the future but also
	// needs to be done elsewhere
	if(bChanges && OtherNode.IsStatic())
	{
		// Publish a message to all subscribers to say that this node has collided with something
		TLMessaging::TMessage Message("COLLISION");

		Message.Write(GetNodeRef());
		//Message.Write(bChanges);
		Message.Write( GetVelocity() );		// Velocity of object
		Message.Write(vImpulse);			// Velocity change due to collision
		Message.Write(vReboundForce);		// Force applied in moving this object via collision
		//Message.Write(pOtherNode->IsStatic());	// Is the other node static?

		PublishMessage(Message);
	}

	return bChanges;
}


//----------------------------------------------------------
//	calculate transformed collision shape 
//----------------------------------------------------------
TLMaths::TShape* TLPhysics::TPhysicsNode::CalcWorldCollisionShape()
{
	//	doesn't need calculating
	if ( m_pWorldCollisionShape )
		return m_pWorldCollisionShape.GetObject();

	//	no collision shape
	if ( !m_pCollisionShape )
	{
		TLDebug_Break("Collision shape expected");
		return NULL;
	}

#ifndef USE_BOX2D
	//	check to see if we need to add movement to the transform calculation
	const float3& Movement = GetAccumulatedMovement();
	Bool HasMovement = ( Movement.LengthSq() > 0.f );

	static TLMaths::TTransform g_Temp_Transform;
//	TLMaths::TTransform& Transform = HasMovement ? g_Temp_Transform : m_Transform;
	TLMaths::TTransform Transform = HasMovement ? g_Temp_Transform : m_Transform;

	//	if we do have movement then setup the temp transform
	if ( HasMovement )
	{
		TLMemory::CopyData( &Transform, &m_Transform, 1 );
		//g_Temp_Transform = m_Transform;

		if ( Transform.HasTranslate() )
		{
			Transform.GetTranslate() += Movement;
		}
		else
		{		
			Transform.SetTranslate( Movement );
		}
	}

	//	gr: rotation was ignored for the old snowway-spheres, but needed now
//	if ( Transform.HasRotation() )
//		Transform.SetRotationInvalid();

	//	gr: scale in the shape transform was what was breaking the positional/intersection stuff of the spheres...
	if ( Transform.HasScale() )
	{
		TLDebug_Break("Collision shape transform with scale doesn't work!");
	}
#else
	TLMaths::TTransform& Transform = m_Transform;
#endif

	//	transform the collision shape into a new shape
	m_pWorldCollisionShape = m_pCollisionShape->Transform( Transform, m_pCollisionShape, m_pLastWorldCollisionShape );

	//	world collision shape has changed
	//	gr: need a more comprehensive has-changed check?
	m_WorldCollisionShapeChanged = TRUE;

	if ( m_pWorldCollisionShape )
	{
		//	whether it was used or not, the last world collision shape is now redundant
		m_pLastWorldCollisionShape = NULL;
	}
	else
	{
#ifdef _DEBUG
		TTempString Debug_String("Failed to transform collision shape type ");
		m_pCollisionShape->GetShapeType().GetString( Debug_String );
		Debug_String.Append(" on node ");
		GetNodeRef().GetString( Debug_String );
		Debug_String.Append('(');
		GetNodeTypeRef().GetString( Debug_String );
		Debug_String.Append(')');
		TLDebug_Break( Debug_String );
#endif
	}

	return m_pWorldCollisionShape.GetObject();
}


//----------------------------------------------------------
//	move node into/out of collision zones
//----------------------------------------------------------
void TLPhysics::TPhysicsNode::UpdateNodeCollisionZone(TPtr<TLPhysics::TPhysicsNode>& pThis,TLPhysics::TPhysicsgraph& Graph)
{
	//	use the generic quad tree zone update
	TPtr<TLMaths::TQuadTreeNode> pThisZoneNode = pThis;
	UpdateZone( pThisZoneNode, Graph.GetRootCollisionZone() );
}


//-------------------------------------------------------------
//	
//-------------------------------------------------------------
void TLPhysics::TPhysicsNode::AddCollisionInfo(const TLPhysics::TPhysicsNode& OtherNode,const TLMaths::TIntersection& Intersection)
{
	//	alloc a new info
	TCollisionInfo* pCollisionInfo = m_Collisions.AddNew();
	if ( !pCollisionInfo )
		return;

	//	set collision info
	pCollisionInfo->Set( OtherNode, Intersection );
}



//----------------------------------------------------
//	send transform changes as per m_TransformChanges
//----------------------------------------------------
void TLPhysics::TPhysicsNode::PublishCollisions()
{
	//	no collisions occured
	if ( !m_Collisions.GetSize() )
		return;

	//	if no subscribers, just ditch the data
	if ( !HasSubscribers() )
	{
		m_Collisions.Empty();
		return;
	}

	//	make message
	TLMessaging::TMessage Message("Collision", GetNodeRef() );
	
	//	write our owner to the message
	Message.Write( GetOwnerSceneNodeRef() );

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
//	
//-------------------------------------------------------------
SyncBool TLPhysics::TPhysicsNode::IsInShape(const TLMaths::TBox2D& Shape)
{
#ifdef _DEBUG
	if ( !HasZoneShape() )
	{
		TLDebug_Break("Doing shape test on a physics node when we shouldn't... must be missing a HasZoneShape() test");
		return SyncWait;
	}
#endif

	//	gr: always recalc world collision shape, previous break worked, but if the shape was invalidated between zone tests then
	//		it wouldn't be recalculated and trigger the break below
	TLMaths::TShape* pWorldShape = CalcWorldCollisionShape();

	if ( !pWorldShape )
	{
		TLDebug_Break("World collision shape expected; this nodes shape-testability should have already been checked with HasZoneShape()");
		return SyncWait;
	}

	//	make up a Collision shape for the test-shape and do an intersection test with it
	TLMaths::TShapeBox2D ShapeCollisionShape( Shape );
	
	return pWorldShape->HasIntersection( ShapeCollisionShape ) ? SyncTrue : SyncFalse;
//	return ShapeCollisionShape.HasIntersection( *pWorldShape ) ? SyncTrue : SyncFalse;
}



//-------------------------------------------------------------
//	
//-------------------------------------------------------------
Bool TLPhysics::TPhysicsNode::HasZoneShape()
{
	//	not yet initialised with a collision shape
	if ( !HasCollision() )
		return FALSE;

	//	pre-calc the world collision shape (we're going to use it anyway)
	//	and if that fails we can't do any tests anyway
	TLMaths::TShape* pShape = CalcWorldCollisionShape();

	//	no shape atm, wait
	if ( !pShape )
		return FALSE;

	return TRUE;
}


//-------------------------------------------------------------
//	explicit change of transform
//-------------------------------------------------------------
void TLPhysics::TPhysicsNode::SetTransform(const TLMaths::TTransform& NewTransform,Bool PublishChanges)
{
	//	copy transform
	m_Transform = NewTransform;

	//	invalidate stuff
	if ( PublishChanges )
	{
		OnTransformChanged( TRUE, TRUE, TRUE );
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
	BodyDef.position.Set( Transform.GetTranslate().x, Transform.GetTranslate().y );
	if ( Transform.HasRotation() )
		BodyDef.angle = Transform.GetRotation().GetAngle2D().GetRadians();

	//	set user data as a pointer back to self (could use ref...)
	BodyDef.userData = this;

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

	//	destroy existing shape[s]
	b2Shape* pExistingShape = m_pBody->GetShapeList();
	while ( pExistingShape )
	{
		//	store next
		b2Shape* pNextShape = pExistingShape->GetNext();

		//	remove from body
		m_pBody->DestroyShape( pExistingShape );

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
	b2ShapeDef* pShapeDef = NULL;

	if ( TLPhysics::GetCircleDefFromShape( CircleDef, *m_pCollisionShape ) )
	{
		pShapeDef = &CircleDef;
	}
	else if ( TLPhysics::GetPolygonDefFromShape( PolygonDef, *m_pCollisionShape ) )
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
	b2ShapeDef& ShapeDef = *pShapeDef;

	//	make everything rigid
	ShapeDef.density = 1.f;

	//	gr: todo: use our node's friction values...
	ShapeDef.friction = 0.3f;

	//	create shape
	m_pBody->CreateShape( &ShapeDef );

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
		b2Shape* pShape = m_pBody->GetShapeList();
		while ( pShape )
		{
			pWorld->Refilter( pShape );
			pShape = pShape->GetNext();
		}
	}
}


//-------------------------------------------------------------
//	called when collision is enabled/disabled - changes group of box2D body so it won't do collision checks
//-------------------------------------------------------------
void TLPhysics::TPhysicsNode::OnCollisionEnabledChanged(Bool IsNowEnabled)
{
	b2Shape* pShape = GetBodyShape();
	if ( !pShape )
		return;

	//	detect change of group index
	s16 GroupIndex = pShape->GetFilterData().groupIndex;

	//	gr: due to the use of this system, all our objects must be in group 1
#ifndef USE_ZERO_GROUP
	if ( GroupIndex == 0 )
	{
		TLDebug_Break("Gr: no objects should be in the zero group");
		GroupIndex = 1;
	}
#endif

	if ( IsNowEnabled && GroupIndex < 0 )
	{
		//	enable collision by making group positive
		b2FilterData NewFilterData = pShape->GetFilterData();
		#ifdef USE_ZERO_GROUP
			NewFilterData.groupIndex = 0;
		#else
			NewFilterData.groupIndex = -GroupIndex;
		#endif
		pShape->SetFilterData( NewFilterData );
		TLPhysics::g_pPhysicsgraph->GetWorld()->Refilter( pShape );
	}
#ifdef USE_ZERO_GROUP
	else if ( !IsNowEnabled && GroupIndex >= 0 )
#else
	else if ( !IsNowEnabled && GroupIndex > 0 )
#endif
	{
		//	disable collision by making group negative
		b2FilterData NewFilterData = pShape->GetFilterData();
		#ifdef USE_ZERO_GROUP
			NewFilterData.groupIndex = -1;
		#else
			NewFilterData.groupIndex = -GroupIndex;
		#endif
		pShape->SetFilterData( NewFilterData );
		TLPhysics::g_pPhysicsgraph->GetWorld()->Refilter( pShape );
	}


}

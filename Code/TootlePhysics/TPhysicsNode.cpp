#include "TPhysicsNode.h"
#include "TPhysicsGraph.h"
#include <TootleCore/TLMaths.h>
#include <TootleCore/TLTime.h>

#include <TootleScene/TScenegraph.h>
#include <TootleCore/TEventChannel.h>



//	if something moves less than this amount then dont apply the change - 
//	this stops the world collision sphere from being invalidated more than we need to
#define MIN_CHANGE_AMOUNT			(TLMaths::g_NearZero*1000.f)
//#define MIN_CHANGE_AMOUNT			(TLMaths::g_NearZero)

#define HAS_MIN_CHANGE3(v)			HAS_MIN_CHANGE( v.LengthSq() )
//#define HAS_MIN_CHANGE3(v)			TRUE

#define HAS_MIN_CHANGE(f)			( f > MIN_CHANGE_AMOUNT )
//#define HAS_MIN_CHANGE(f)			TRUE

#define DEBUG_FLOAT_CHECK(v)		TLDebug_CheckFloat(v)
//#define DEBUG_FLOAT_CHECK(v)		TRUE

//#define FORCE_SQUIDGE				0.0f

//	gr: simple is faster, possibly could make non-simple one save doing same in-zone-shape checks over and over..
#define SIMPLE_UPDATE_ZONE	//	changes update zone to just traverse from root down, instead of current zone up
//#define COMPLEX_UPDATE_ZONE	//	changes update zone to just traverse from root down, instead of current zone up
//#define NEW_UPDATE_ZONE	//	changes update zone to just traverse from root down, instead of current zone up


namespace TLPhysics
{
	float3		g_WorldUp( 0.f, -1.f, 0.f );
	float3		g_WorldUpNormal( 0.f, -1.f, 0.f );
	
	float		g_GravityMetresSec	= 9.81f;	//	gravity in metres per second (1unit being 1metre)
}


namespace TLRef
{
	TLArray::SortResult		RefSort(const TRef& aRef,const TRef& bRef,const void* pTestVal);	//	simple ref-sort func - for arrays of TRef's
}





TLPhysics::TPhysicsNode::TPhysicsNode(TRefRef NodeRef,TRefRef TypeRef) :
	TLGraph::TGraphNode<TPhysicsNode>	( NodeRef, TypeRef ),
	m_Friction						( 0.4f ),
	m_Mass							( 1.0f ),
	m_Bounce						( 1.0f ),
	m_Squidge						( 0.f ),
	m_Temp_ExtrudeTimestep			( 0.f ),
	m_InitialisedZone				( FALSE ),
	m_Debug_StaticCollisions		( 0 ),
	m_WorldCollisionShapeChanged	( FALSE )
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
			TPtr<TLMessaging::TEventChannel>& pEventChannel = pOwner->FindEventChannel("OnTransform");

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

	if ( Message.ImportData("Translate", m_Transform.GetTranslate() ) == SyncTrue )
	{
		m_Transform.SetTranslateValid();
		OnTranslationChanged();
	}

	if ( Message.ImportData("Scale", m_Transform.GetScale() ) == SyncTrue )
	{
		m_Transform.SetScaleValid();
		OnScaleChanged();
	}

	if ( Message.ImportData("Rotation", m_Transform.GetRotation() ) == SyncTrue )
	{
		m_Transform.SetRotationValid();
		OnRotationChanged();
	}

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
	m_WorldCollisionShapeChanged = FALSE;
	m_Debug_StaticCollisions = 0;
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
		float GravityPerFrame = g_GravityMetresSec * (1.f/60.f) * fTimeStep;
		m_GravityForce = g_WorldUpNormal;
		m_GravityForce *= -GravityPerFrame;	//	negate as UP is opposite to the direction of gravity.

		m_Force += m_GravityForce;// * m_Mass;
	}

	//	make last force applied relative
//	m_Force *= 1.f / 60.f;

	m_Velocity += m_Force;
	m_Force.Set(0.f,0.f,0.f);

}


//----------------------------------------------------
//	after collisions are handled
//----------------------------------------------------
void TLPhysics::TPhysicsNode::PostUpdate(float fTimeStep,TLPhysics::TPhysicsgraph* pGraph,TPtr<TPhysicsNode>& pThis)
{
	if ( m_PhysicsFlags( Flag_ZoneExpected ) && !m_InitialisedZone )
	{
		if ( HasCollision() )
		{
			UpdateNodeCollisionZone( pThis, pGraph );
			m_InitialisedZone = TRUE;
		}
	}

	if ( m_PhysicsFlags( Flag_HasCollision ) && !HasCollision() )
	{
		PublishTransformChanges();
		return;
	}

	//TLDebug_Print( TString("Velocity(%3.3f,%3.3f,%3.3f) Force(%3.3f,%3.3f,%3.3f) \n", m_Velocity.x, m_Velocity.y, m_Velocity.z, m_Force.x, m_Force.y, m_Force.z ) );

	//float fFrameStep = fTimestep * TLTime::GetUpdatesPerSecond();

	//	update physics movement
	if ( DEBUG_FLOAT_CHECK( m_Force ) && HAS_MIN_CHANGE3(m_Force) )
	{
		m_Velocity += m_Force;
		m_Force.Set( 0.f, 0.f, 0.f );
		OnVelocityChanged();
		OnForceChanged();
	}
	
	//	move pos
	if ( DEBUG_FLOAT_CHECK( m_Velocity ) )
	{
		MovePosition( m_Velocity, fTimeStep );
	}

	//	reduce velocity
	DEBUG_FLOAT_CHECK( m_Velocity );
	float Dampening = 1.f - ( GetFriction() * fTimeStep);
	if ( Dampening >= 1.f )
	{
		if ( Dampening > 1.f )
		{
			TLDebug_Break("Dampening should not increase...");
			Dampening = 1.f;
		}

		//	no change to velocity
	}
	else if ( Dampening < TLMaths::g_NearZero )
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

	//	reset force
	if ( HAS_MIN_CHANGE3( m_Force ) )
	{
		m_Force.Set( 0.f, 0.f, 0.f );
		OnForceChanged();
	}

	//	update collision zone
	if ( m_PhysicsFlags( Flag_ZoneExpected ) && GetCollisionZoneNeedsUpdate() )
	{
		UpdateNodeCollisionZone( pThis, pGraph );

		//	no longer needs update
		SetCollisionZoneNeedsUpdate( FALSE );
	}

	//	notify that world collison shape has changed
	if ( m_WorldCollisionShapeChanged )
	{
		TLMessaging::TMessage Message("ColShape", GetNodeRef() );

		//	write whether we have a shape - if not, then we've invalidated our shape, but no use for it yet so it hasnt been re-calculated
		Bool HasShape = GetWorldCollisionShape().IsValid();
		Message.Write( HasShape );

		PublishMessage( Message );

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

	m_Transform.TransformVector( Position );
	
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

	m_Transform.SetTranslate( Position );

	OnTranslationChanged();

	//	after moving node, mark that the zone needs updating
	SetCollisionZoneNeedsUpdate();
}



//----------------------------------------------------
//	
//----------------------------------------------------
void TLPhysics::TPhysicsNode::MovePosition(const float3& Movement,float Timestep)
{
	//	tiny change, dont apply it
	float MovementLengthSq = Movement.LengthSq();
	if ( MovementLengthSq < 0.0001f )
		return;

	//	change translation
	if ( m_Transform.HasTranslate() )
	{
		float3 NewPosition = m_Transform.GetTranslate();
		NewPosition += Movement * Timestep;
		if ( NewPosition.z != 0.f )
		{
			TLDebug_Break("z?");
		}
		m_Transform.SetTranslate( NewPosition );
	}
	else
	{
		m_Transform.SetTranslate( Movement * Timestep );
	}


	//if ( MovementLengthSq > 0.1f )
	{
		OnTranslationChanged();

		//	after moving node, mark that the zone needs updating
		SetCollisionZoneNeedsUpdate();
	}
}



//----------------------------------------------------
//	send transform changes as per m_TransformChanges
//----------------------------------------------------
void TLPhysics::TPhysicsNode::PublishTransformChanges()
{
	//	rule is "cant have changed if invalid value" - need to cater for when it has changed TO an invalid value...
	//	so this removes a change if the value is invalid
	Bool TranslationChanged = m_TransformChanges[0] && m_Transform.HasTranslate();
	Bool RotationChanged = m_TransformChanges[1] && m_Transform.HasRotation();
	Bool ScaleChanged = m_TransformChanges[2] && m_Transform.HasScale();

	//	un-mark changes now that we've got what changes we're gonna send
	m_TransformChanges.SetAll( FALSE );

	//	no changes
	if ( !TranslationChanged && !RotationChanged && !ScaleChanged )
		return;

	//	no one to send a message to 
	if ( !HasSubscribers() )
		return;

	TLMessaging::TMessage Message("OnTransform", GetNodeRef() );

	if ( TranslationChanged )
	{
		Message.ExportData("Translate", m_Transform.GetTranslate());
		TLDebug_CheckFloat( m_Transform.GetTranslate() );
	}

	if( RotationChanged )
	{
		Message.ExportData("Rotation", m_Transform.GetRotation());
		TLDebug_CheckFloat( m_Transform.GetRotation() );
	}

	if( ScaleChanged )
	{
		Message.ExportData("Scale", m_Transform.GetScale());
		TLDebug_CheckFloat( m_Transform.GetScale() );
	}

	PublishMessage(Message);
}



//----------------------------------------------------------
//	update tree: update self, and children and siblings
//----------------------------------------------------------
void TLPhysics::TPhysicsNode::PostUpdateAll(float fTimestep,TLPhysics::TPhysicsgraph* pGraph,TPtr<TLPhysics::TPhysicsNode>& pThis)
{
	if ( !IsEnabled() )
		return;

	// Update this
	PostUpdate( fTimestep, pGraph, pThis );

#ifdef TLGRAPH_OWN_CHILDREN

	TPtrArray<TPhysicsNode>& NodeChildren = GetChildren();
	for ( u32 c=0;	c<NodeChildren.GetSize();	c++ )
	{
		TPtr<TPhysicsNode>& pNode = NodeChildren.ElementAt(c);
		pNode->PostUpdateAll( fTimestep, pGraph, pNode );
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

//----------------------------------------------------------
//	setup a sphere collision shape
//----------------------------------------------------------
void TLPhysics::TPhysicsNode::SetCollisionShape(const TLMaths::TSphere& Sphere)
{
	//	todo: see if it's already a sphere and just update it
	TPtr<TCollisionSphere> pCollisionSphere = new TCollisionSphere;
	m_pCollisionShape = pCollisionSphere;
	pCollisionSphere->SetSphere( Sphere );

	//	invalidate zone
	SetCollisionZoneNeedsUpdate();

	SetWorldCollisionShapeInvalid();
}


//----------------------------------------------------------
//	setup an oblong collision shape
//----------------------------------------------------------
void TLPhysics::TPhysicsNode::SetCollisionShape(const TLMaths::TOblong2D& Oblong)
{
	//	todo: see if it's already an oblong and just update it
	TPtr<TCollisionOblong2D> pCollisionShape = new TCollisionOblong2D( Oblong );
	m_pCollisionShape = pCollisionShape;

	//	invalidate zone
	SetCollisionZoneNeedsUpdate();

	SetWorldCollisionShapeInvalid();
}

//----------------------------------------------------------
//	setup a capsule collision shape
//----------------------------------------------------------
void TLPhysics::TPhysicsNode::SetCollisionShape(const TLMaths::TCapsule2D& Capsule)
{
	//	todo: see if it's already an oblong and just update it
	TPtr<TCollisionCapsule2D> pCollisionShape = new TCollisionCapsule2D( Capsule );
	m_pCollisionShape = pCollisionShape;

	//	invalidate zone
	SetCollisionZoneNeedsUpdate();

	SetWorldCollisionShapeInvalid();
}

//----------------------------------------------------------
//	handle collision with other object - returns TRUE if we changed anything in the collison
//----------------------------------------------------------
Bool TLPhysics::TPhysicsNode::OnCollision(const TPhysicsNode& OtherNode)
{
	TIntersection& Intersection = m_Temp_Intersection;
	const TIntersection& OtherIntersection = OtherNode.m_Temp_Intersection;
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
		if ( DistDot2 >= TLMaths::g_NearZero )
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
			if ( VdotN < -TLMaths::g_NearZero ) 
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
				m_Velocity += vImpulse;
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

			m_Force += vReboundForce;
			OnForceChanged();
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
		Message.Write(m_Velocity);		// Velocity of object
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
TLPhysics::TCollisionShape* TLPhysics::TPhysicsNode::CalcWorldCollisionShape()
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

	//	transform the collision shape into a new shape
	m_pWorldCollisionShape = m_pCollisionShape->Transform( Transform, m_pCollisionShape, m_pLastWorldCollisionShape );

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
void TLPhysics::TPhysicsNode::UpdateNodeCollisionZone(TPtr<TLPhysics::TPhysicsNode>& pThis,TLPhysics::TPhysicsgraph* pGraph)
{
	//	use the generic quad tree zone update
	TPtr<TLMaths::TQuadTreeNode> pThisZoneNode = pThis;
	pThisZoneNode->UpdateZone( pThisZoneNode, pGraph->GetRootCollisionZone() );

	/*
#ifdef NEW_UPDATE_ZONE


#endif

#ifdef SIMPLE_UPDATE_ZONE
	//	simple mode
	TPtr<TLMaths::TQuadTreeZone> pParentZone = pGraph->GetRootCollisionZone();

	//	re-add to parent to evaluate if we now span multiple zones
	if ( pParentZone )
	{
		while ( !pParentZone->AddNode( pThis->GetZoneNodePtr(), pParentZone, TRUE ) )
		{
			//	no longer in parent zone, try parent of parent
			pParentZone = pParentZone->GetParentZone();
			if ( !pParentZone )
			{
				//	not in ANY zone any more
				GetPhysicsZoneNode().SetZone( pParentZone, pThis->GetZoneNodePtr(), NULL );
				//SetCollisionZone( pParentZone, pThisQuadTreeNode, NULL );
				return;
			}
		}
	}
#endif 

#ifdef COMPLEX_UPDATE_ZONE

	TPtr<TCollisionZone>& pCurrentZone = GetCollisionZone();

	//	no current zone, just try to add at the root
	if ( !pCurrentZone )
	{
		TPtr<TCollisionZone>& pRootZone = pGraph->GetRootCollisionZone();

		//	just return, if it worked it worked, if it didn't, we're still out of the zones
		pRootZone->AddNode( pThis, pRootZone, TRUE );
		return;
	}

	//	find the first zone we're in
	TPtr<TCollisionZone> pTestZone = pCurrentZone;
	TPtr<TCollisionZone> pInZone;
	while ( !pInZone && pTestZone )
	{
		if ( pTestZone->IsNodeInZoneShape( pThis ) )
			pInZone = pTestZone;
		else
			pTestZone = pTestZone->GetParentZone();
	}

	//	pInZone is the first zone we're definately in
	while ( pInZone )
	{
		//	get the parent of InZone
		TPtr<TCollisionZone> pInZoneParent = pInZone->GetParentZone();
		if ( !pInZoneParent )
		{
			//	pInZone must be root. add to that and exit
			pInZone->AddNode( pThis, pInZone, FALSE );
			return;
		}

		//	see if we're in multiple children of the parent zone, i.e. crossing a border, if we are, go up again
		TFixedArray<u32,4> InZoneParentChildrenZones;
		pInZoneParent->GetInChildZones( this, InZoneParentChildrenZones );
		if ( InZoneParentChildrenZones.GetSize() == 0 )
		{
			//	error, in zone, but not in any children...
			TLDebug_Break("in zone, but not in any children...");
			pInZone->AddNode( pThis, pInZone, FALSE );
			return;
		}

		//	just in one zone (not crossing borders) so just add to InZone
		//	todo: dont need to check children?
		if ( InZoneParentChildrenZones.GetSize() == 1 )
		{
			pInZone->AddNode( pThis, pInZone, FALSE );
			return;
		}

		//	crossing borders of InZone's parents, so go up again (hopefully wont be on another border...)
		pInZone = pInZoneParent;
	}

	//	no longer in any zone, not even root
	pInZone = NULL;
	SetCollisionZone( pInZone, pThis, NULL );

#endif
	*/
}

/*

//-------------------------------------------------------------
//	attempt to add this node to this zone. checks with children 
//	first to see if it fits into just one child better. returns FALSE if not in this zone
//-------------------------------------------------------------
Bool TLPhysics::TPhysicsNode::SetCollisionZone(TPtr<TLMaths::TQuadTreeZone>& pCollisionZone,TPtr<TPhysicsNode> pThis,const TFixedArray<u32,4>* pChildZoneList)
{
	//	already in this zone
	TPtr<TLMaths::TQuadTreeZone>& pOldZone = GetZone();
	if ( pOldZone == pCollisionZone )
	{
		//	just update child list
		if ( !pChildZoneList )
			SetChildZonesNone();
		else
			SetChildZones( *pChildZoneList );

		return TRUE;
	}

	//	remove from old zone
	if ( pOldZone )
	{
		TPtr<TLMaths::TQuadTreeNode> pThisQuadTreeNode = pThis;
		pOldZone->DoRemoveNode( pThisQuadTreeNode );
	}

	//	add to this zone
	if ( pCollisionZone )
	{
		if ( pCollisionZone->GetNodes().Exists( pThis ) )
		{
			TLDebug_Break("Node shouldnt be in this list");
		}
		else
		{
			//	add node to collision zone
			TPtr<TLMaths::TQuadTreeNode> pThisQuadTreeNode = pThis;
			pCollisionZone->DoAddNode( pThisQuadTreeNode );
		}
	}

	//	set new zone on node
	m_pCollisionZone = pCollisionZone;
	m_InitialisedZone = TRUE;

	//	update child list
	if ( !pChildZoneList )
		SetChildZonesNone();
	else
		SetChildZones( *pChildZoneList );

	return TRUE;
}
*/
	
/*
//-------------------------------------------------------------
//	
//-------------------------------------------------------------
void TLPhysics::TPhysicsNode::SetChildZones(const TFixedArray<u32,4>& InZones)
{
	if ( !InZones.GetSize() )
	{
		m_ChildCollisionZones.Empty();
		return;
	}

	if ( !m_pCollisionZone )
	{
		TLDebug_Break("Should be in a zone when trying to assign children");
		return;
	}

	//	add child zones
	m_ChildCollisionZones = InZones;
}
*/



//-------------------------------------------------------------
//	
//-------------------------------------------------------------
void TLPhysics::TPhysicsNode::AddCollisionInfo(const TLPhysics::TPhysicsNode& OtherNode,const TIntersection& Intersection)
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
	//	not yet initialised with a collision shape
	if ( !HasCollision() )
	{
		//	wait for it
		if ( GetPhysicsFlags()( TLPhysics::TPhysicsNode::Flag_HasCollision ) )
			return SyncWait;

		//	has no collision...
		return SyncFalse;
	}

	TLPhysics::TCollisionShape* pShape = CalcWorldCollisionShape();

	//	no shape atm, wait
	if ( !pShape )
		return SyncWait;

	TLPhysics::TCollisionBox2D ShapeCollisionShape( Shape );
	if ( !pShape->HasIntersection( &ShapeCollisionShape ) )
		return SyncFalse;

	return SyncTrue;
}






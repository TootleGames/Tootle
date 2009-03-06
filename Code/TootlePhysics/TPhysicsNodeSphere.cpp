#include "TPhysicsNodeSphere.h"



namespace TLPhysics
{
	extern float3		g_WorldUp;
}
	

#define MAX_ROLL		1.f		//	cap roll on quaternion


TLPhysics::TPhysicsNodeSphere::TPhysicsNodeSphere(TRefRef NodeRef,TRefRef TypeRef) :
	TPhysicsNode		( NodeRef, TypeRef ),
	m_LastNormalValid	( FALSE )
{
}


//----------------------------------------------------
//	physics update
//----------------------------------------------------
void TLPhysics::TPhysicsNodeSphere::Update(float Timestep)
{
	m_LastNormalIsStatic = FALSE;
	TPhysicsNode::Update( Timestep );
}



//----------------------------------------------------
//	roll our transform. 
//	Movement is FORCE (not timestepped) movement. 
//	Normal is surface normal that we've rolled on. Assume valid
//----------------------------------------------------
void TLPhysics::TPhysicsNodeSphere::RollTransform(const float3& MovementForce,const float3& Normal,float Timestep)
{
	//	relative to size of sphere?
	TCollisionSphere* pCollisionSphere = GetCollisionShape().GetObject<TCollisionSphere>();

	//	make the number below BIGGER to increase rotation speed (if it rolls too mch for movemnt)
	//	make the number below SMALLER to decrease speed (if slides)
	float Radius = pCollisionSphere->GetSphere().GetRadius();
	float MovementScale = 2.f / Radius;
	
	//	scale movement to rotation amount
	float3 Movement( MovementForce * (MovementScale * Timestep) );

	//	roll sphere
	float MovementLenSq = Movement.LengthSq();
	if ( MovementLenSq < TLMaths::g_NearZero )
		return;

	//	get floor drag/friction/collision from floor collision
	//	this works as a mask to get the axis to rotate around that wont be our normal
	float3 FloorDrag( 1.f-TLMaths::Absf(Normal.x), 1.f-TLMaths::Absf(Normal.y), 1.f-TLMaths::Absf(Normal.z) );
	Movement *= FloorDrag;

	//	if we're on a surface, we must roll with our movement
	//	roll in the direction of the movement
	float MovementLen = TLMaths::Sqrtf( MovementLenSq );
	float Roll = MovementLen;
	float3 Axis = Movement.Normal( MovementLen, 1.f );
	Axis = Axis.CrossProduct( float3(0,-1,0) );	//	cross against world up? or normal we're on or what?
		
	float AxisLengthSq = Axis.LengthSq();
	if ( AxisLengthSq < TLMaths::g_NearZero )
		return;
	
	Axis.Normalise( TLMaths::Sqrtf(AxisLengthSq), 1.f );
#ifdef MAX_ROLL
	if ( Roll > MAX_ROLL )	
		Roll = MAX_ROLL;
	else if ( Roll < -MAX_ROLL )	
		Roll = -MAX_ROLL;
#endif

	TLMaths::TQuaternion RollQuat( Axis, Roll );
	RollQuat.Normalise();
	
	//	get new rotation
	if ( RollQuat.IsValid() )
	{
		TLMaths::TQuaternion& Rotation = m_RenderTransform.GetRotation();
		if ( m_RenderTransform.HasRotation() )
			Rotation *= RollQuat;
		else
		{
			m_RenderTransform.SetRotation( RollQuat );
		}

		//	normalise rotation
		Rotation.Normalise();
	}

}


//----------------------------------------------------
//	after collisions are handled
//----------------------------------------------------
void TLPhysics::TPhysicsNodeSphere::PostUpdate(float Timestep,TLPhysics::TPhysicsgraph* pGraph,TPtr<TLPhysics::TPhysicsNode>& pThis)
{
	float3 OldPos = m_Transform.GetTranslate();
	TPhysicsNode::PostUpdate( Timestep, pGraph, pThis );
	float3& NewPos = m_Transform.GetTranslate();

	//	copy bits out of normal transform
	if ( m_Transform.HasTranslate() )
	{
		m_RenderTransform.SetTranslate( m_Transform.GetTranslate() );
	}
	else
		m_RenderTransform.SetTranslateInvalid();

		
	if ( m_LastNormalValid )
	{
		float3 Movement = NewPos - OldPos;
		RollTransform( Movement, m_LastNormal, 1.f );
	}

	// Broadcast transform change
	OnRenderTransformChange();
}

// [06/03/09] DB - Specific sphere physics transform change
void TLPhysics::TPhysicsNodeSphere::OnRenderTransformChange()
{
	TLMessaging::TMessage Message("TRANSFORM");
	Message.ExportData("Translate", m_RenderTransform.GetTranslate());
	Message.ExportData("Rotation", m_RenderTransform.GetRotation());
	PublishMessage(Message);
}


//----------------------------------------------------
//	handle collision with other object
//----------------------------------------------------
Bool TLPhysics::TPhysicsNodeSphere::OnCollision(const TPhysicsNode* pOtherNode)
{
	if ( !TPhysicsNode::OnCollision( pOtherNode ) )
		return FALSE;
	
	
	//	do roll if we moved during collision
	TIntersection& Intersection = m_Temp_Intersection;
	if ( Intersection.m_HasNormal )
	{
		if ( !m_LastNormalIsStatic || pOtherNode->IsStatic() )
		{
			if ( Intersection.m_Normal.LengthSq() > 0.f )
			{
				m_LastNormal = Intersection.m_Normal.Normal();
				TLDebug_CheckFloat( m_LastNormal );
				m_LastNormalValid = TRUE;
				m_LastNormalIsStatic = pOtherNode->IsStatic();
				//RollTransform( Intersection.m_PostCollisionDelta, Intersection.m_Normal.Normal(), m_Temp_ExtrudeTimestep );
			}
		}
	}
	

	return TRUE;
}

/*


void GPhysicsSphere::DoIntersection(float3& From, float3& Dir, float3& MeshPos, GPlane& TrianglePlane, float3& TriangleNormal, float3& TriangleV1, float3& TriangleV2, float3& TriangleV3)
{
//	#define DRAW_FAILED
	#define DRAW_SUCCEED
	float4 DebugTriangleColourFailed(1,1,0,1);
	float4 DebugTriangleColourSucceed(1,0,0,1);

	float3 PosLocalToTriangle = From - MeshPos;

	//	get intersection
	float3 IntersectionFrom = PosLocalToTriangle;
	float3 IntersectionDir = TriangleNormal * -m_SphereRadius;
	float IntersectLength;

	//	draw dir
	#ifdef DEBUG_PHYSICS
		//if ( DrawInfo.Flags & GDrawInfoFlags::DebugPhysicsCollisions )
			GDisplay::DebugLine( IntersectionFrom, float4(1,1,1,1), (IntersectionFrom+IntersectionDir) );
	#endif

	//	check intersection point
	if (! TrianglePlane.Intersection( IntersectLength, PosLocalToTriangle, IntersectionDir ) )
	{
		#ifdef DEBUG_PHYSICS_COLLISION_POINT
			//if ( DrawInfo.Flags & GDrawInfoFlags::DebugPhysicsCollisions )
			#ifdef DRAW_FAILED
				//	draw failed triangle
				GDisplay::DebugLine( TriangleV1 + MeshPos, DebugTriangleColourFailed, TriangleV2 + MeshPos );
				GDisplay::DebugLine( TriangleV2 + MeshPos, DebugTriangleColourFailed, TriangleV3 + MeshPos );
				GDisplay::DebugLine( TriangleV3 + MeshPos, DebugTriangleColourFailed, TriangleV1 + MeshPos );
			#endif
		#endif
		return;
	}

	//	intersection too far away
	if ( IntersectLength<0.f || IntersectLength>1.f )
	{
		#ifdef DEBUG_PHYSICS_COLLISION_POINT
			//if ( DrawInfo.Flags & GDrawInfoFlags::DebugPhysicsCollisions )
			#ifdef DRAW_FAILED
				//	draw failed triangle
				GDisplay::DebugLine( TriangleV1 + MeshPos, DebugTriangleColourFailed, TriangleV2 + MeshPos );
				GDisplay::DebugLine( TriangleV2 + MeshPos, DebugTriangleColourFailed, TriangleV3 + MeshPos );
				GDisplay::DebugLine( TriangleV3 + MeshPos, DebugTriangleColourFailed, TriangleV1 + MeshPos );
			#endif
		#endif
		return;
	}

	//	get the intersection point
	float3 IntersectionPoint = PosLocalToTriangle;
	if ( IntersectLength == 1.f )
		IntersectionPoint += IntersectionDir;
	else if ( IntersectLength > 0.f )
		IntersectionPoint += IntersectionDir * IntersectLength;

	//	check point is actually on the triangle and not just the plane
	if ( !PointInsideTriangle( IntersectionPoint, TriangleV1, TriangleV2, TriangleV3, TrianglePlane ) )
	{
		#ifdef DEBUG_PHYSICS_COLLISION_POINT
			//if ( DrawInfo.Flags & GDrawInfoFlags::DebugPhysicsCollisions )
			#ifdef DRAW_FAILED
				//	draw failed triangle
				GDisplay::DebugLine( TriangleV1 + MeshPos, DebugTriangleColourFailed, TriangleV2 + MeshPos );
				GDisplay::DebugLine( TriangleV2 + MeshPos, DebugTriangleColourFailed, TriangleV3 + MeshPos );
				GDisplay::DebugLine( TriangleV3 + MeshPos, DebugTriangleColourFailed, TriangleV1 + MeshPos );
			#endif
		#endif
		return;
	}

	#ifdef DEBUG_PHYSICS_COLLISION_POINT
		//if ( DrawInfo.Flags & GDrawInfoFlags::DebugPhysicsCollisions )
		#ifdef DRAW_SUCCEED
			//	draw succeeded triangle
			GDisplay::DebugLine( TriangleV1 + MeshPos, DebugTriangleColourSucceed, TriangleV2 + MeshPos );
			GDisplay::DebugLine( TriangleV2 + MeshPos, DebugTriangleColourSucceed, TriangleV3 + MeshPos );
			GDisplay::DebugLine( TriangleV3 + MeshPos, DebugTriangleColourSucceed, TriangleV1 + MeshPos );
		#endif
	#endif



	//	react
	float3 Dist( IntersectionPoint - From );	//	dir to intersection

	float DistDot2 = Dist.DotProduct(Dist);  

	// balls are too embedded to be of any use    
    if ( DistDot2 < NEAR_ZERO )     
		return;
	
	float n = sqrtf(DistDot2);  
	Dist /= n;  

	float3 FloorReflectedDir = Dir;
	FloorReflectedDir.Reflect( TriangleNormal );
	float3 FloorReflectedVelocity = m_Velocity;
	FloorReflectedVelocity.Reflect( TriangleNormal );

	//	need a physics object to collide with
	GPhysicsObject WallPhysicsObject;
	WallPhysicsObject.m_Bounce = 0.5f;
	WallPhysicsObject.m_Mass = 1.f;
	WallPhysicsObject.m_Friction = 0.4f;
	WallPhysicsObject.m_Velocity = float3( FloorReflectedVelocity*(1.f-WallPhysicsObject.m_Friction) );
	WallPhysicsObject.m_Force = float3( FloorReflectedDir*(1.f-WallPhysicsObject.m_Friction) );//(0,0,0);



	// relative velocity of ball2 to ball1   
	float3 V = WallPhysicsObject.AccumulatedMovement() - AccumulatedMovement();
    float VdotN = V.DotProduct(Dist);        

	if (VdotN < -NEAR_ZERO)
	{
		DoCollision( &WallPhysicsObject, Dist, VdotN );
	}      

	float     d  = Dist.Length() - n;

	// relative amount of displacement to make the ball touch         
	float ratio1 = (WallPhysicsObject.m_Mass) / (m_Mass + WallPhysicsObject.m_Mass);   
	float ratio2 = 1.0f - ratio1;    

	// move the balls to theire ideal positon.  
	m_pOwner->m_Position -= Dist * d * ratio1;  

	
	if ( m_LastFloorNormal.LengthSq() == 0.f )
	{
		m_LastFloorNormal = TriangleNormal;
	}
	else
	{
		m_LastFloorNormal += TriangleNormal;
		m_LastFloorNormal.Normalise();
	}

}

void GPhysicsSphere::PostUpdate(GWorld* pWorld)
{
	//	do base update
	GPhysicsObject::PostUpdate(pWorld);

	//	roll sphere
	float3 Movement( m_Velocity * g_App->FrameDelta() );
	float MovementLenSq = Movement.LengthSq();

	Bool DoRotation = (MovementLenSq > NEAR_ZERO);

	if ( m_PhysicsFlags	& GPhysicsFlags::DontRotate )
		DoRotation = FALSE;

	//	if there is any movement, apply
	if ( DoRotation )
	{
		float3 Axis = m_LastRollAxis;
		float Roll = 0.f;

		float3 FloorMovement( Movement );

		//	get floor drag/friction/collision from floor collision
		float3 FloorDrag( 1.f-m_LastFloorNormal.x, 1.f-m_LastFloorNormal.y, 1.f-m_LastFloorNormal.z );
		FloorMovement *= FloorDrag;

		float FloorMovementLenSq = FloorMovement.LengthSq();

		//	if we're on a surface, we must roll with our movement
		if ( m_LastFloorNormal.LengthSq() != 0.f && FloorMovementLenSq != 0.f )
		{
			//	roll in the direction of the movement
			Roll += sqrtf( FloorMovementLenSq ) * 2.f;	//	div radius of sphere?
			Axis = FloorMovement.Normal();
			Axis = Axis.CrossProduct( float3(0,1,0) );
		}
		else
		{
			Roll = m_LastRoll;
		}

		#ifdef DEBUG_PHYSICS
			GDisplay::DebugLine( m_pOwner->m_Position, float4(1,1,1,1), m_pOwner->m_Position + Axis );
		#endif

		GQuaternion RollQuat( Axis, Roll );
		RollQuat.Normalise();

		m_LastRoll = Roll;
		m_LastRollAxis = Axis;
		
		//	get new rotation
		if ( RollQuat.IsValid() )
		{
			if ( m_pOwner->m_Rotation.IsValid() )
				m_pOwner->m_Rotation *= RollQuat;
			else
				m_pOwner->m_Rotation = RollQuat;

			m_pOwner->m_Rotation.Normalise();
		}
	
	}
	
}

*/

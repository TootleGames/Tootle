#include "TPhysicsNodeSphere.h"
#include <TootleMaths/TShapeSphere.h>



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
	TLMaths::TShapeSphere* pCollisionSphere = GetCollisionShape().GetObject<TLMaths::TShapeSphere>();

	//	make the number below BIGGER to increase rotation speed (if it rolls too mch for movemnt)
	//	make the number below SMALLER to decrease speed (if slides)
	float Radius = pCollisionSphere->GetSphere().GetRadius();
	float MovementScale = 2.f / Radius;
	
	//	scale movement to rotation amount
	float3 Movement( MovementForce * (MovementScale * Timestep) );

	//	roll sphere
	float MovementLenSq = Movement.LengthSq();
	if ( MovementLenSq < TLMaths_NearZero )
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
	if ( AxisLengthSq < TLMaths_NearZero )
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
void TLPhysics::TPhysicsNodeSphere::PostUpdate(float Timestep,TLPhysics::TPhysicsgraph& Graph,TPtr<TLPhysics::TPhysicsNode>& pThis)
{
	float3 OldPos = m_Transform.GetTranslate();
	TPhysicsNode::PostUpdate( Timestep, Graph, pThis );
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
	TLMessaging::TMessage Message(TRef_Static(O,n,T,r,a),GetNodeRef());

	//	gr: previous code DIDN'T include scale...
	if ( m_RenderTransform.ExportData( Message, TLMaths_TransformBitTranslate|TLMaths_TransformBitRotation ) != 0x0 )
	{
		PublishMessage(Message);
	}
}


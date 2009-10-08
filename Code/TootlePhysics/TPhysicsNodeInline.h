
#pragma once

#include "TPhysicsNode.h"

#include <box2d/include/box2d.h>


FORCEINLINE void TLPhysics::TPhysicsNode::AddTorque(float AngleRadians)
{
	if ( m_pBody && AngleRadians != 0.f )	
	{
		//	gr: apply the torque
		m_pBody->ApplyTorque( AngleRadians );	
	}
}
	
FORCEINLINE void TLPhysics::TPhysicsNode::SetVelocity(const float3& Velocity)	
{
	if ( m_pBody )
	{
		m_pBody->SetLinearVelocity( b2Vec2( Velocity.x, Velocity.y ) );
	}
}


FORCEINLINE float3 TLPhysics::TPhysicsNode::GetVelocity() const			
{
	if ( m_pBody )
	{
		const b2Vec2& BodyVelocity = m_pBody->GetLinearVelocity();
		return float3( BodyVelocity.x, BodyVelocity.y, 0.f );
	}
	else
	{
		return float3( 0.f, 0.f, 0.f );
	}
}


FORCEINLINE void TLPhysics::TPhysicsNode::ResetForces()
{
	if ( m_pBody )
	{
		//	gr: quick fudge version, seems to work fine
		m_pBody->PutToSleep();
		m_pBody->WakeUp();
	}
}


FORCEINLINE void TLPhysics::TPhysicsNode::SetFriction(float Friction)
{
	b2Body* pBody = GetBody();
	if ( pBody )
	{
		b2Fixture* pFixture = pBody->GetFixtureList();

		while(pFixture != NULL)
		{
			pFixture->SetFriction( Friction );
			pFixture = pFixture->GetNext();
		}
	}
}


	
FORCEINLINE void TLPhysics::TPhysicsNode::SetLinearDamping(float Damping)
{
	m_Damping = Damping;
	if ( m_pBody )
	{
		m_pBody->SetLinearDamping( m_Damping );
	}
}


FORCEINLINE void TLPhysics::TPhysicsNode::SetAngularDamping(float Damping)
{
	if ( m_pBody )
	{
		m_pBody->SetAngularDamping( Damping );
	}
}

//	this re-sets it on the body if it exists
FORCEINLINE void TLPhysics::TPhysicsNode::OnDampingChanged()
{	
	SetLinearDamping( m_Damping );
}	


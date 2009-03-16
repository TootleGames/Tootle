/*------------------------------------------------------

	Physics node for a sphere, it rotates our transform
	as we collide with surfaces to make it look like it's 
	rolling

-------------------------------------------------------*/
#pragma once

#include "TPhysicsNode.h"


namespace TLPhysics
{
	class TPhysicsNodeSphere;
};




class TLPhysics::TPhysicsNodeSphere : public TLPhysics::TPhysicsNode
{
public:
	TPhysicsNodeSphere(TRefRef NodeRef,TRefRef TypeRef);

	virtual const TLMaths::TTransform&	GetRenderTransform() const		{	return m_RenderTransform;	}

	virtual void	Update(float Timestep);				//	physics update
	virtual void	PostUpdate(float Timestep,TLPhysics::TPhysicsgraph* pGraph,TPtr<TLPhysics::TPhysicsNode>& pThis);			//	after collisions are handled

	virtual Bool	OnCollision(const TPhysicsNode& OtherNode);	//	handle collision with other object

protected:
	void			RollTransform(const float3& MovementForce,const float3& Normal,float Timestep);	//	roll our transform. Movement is FORCE (not timestepped) movement. Normal is surface normal that we've rolled on. Assume valid

	void			OnRenderTransformChange();

protected:
	TLMaths::TTransform	m_RenderTransform;

	float3			m_LastNormal;
	Bool			m_LastNormalValid;
	Bool			m_LastNormalIsStatic;
};



/*------------------------------------------------------

	Self contained physics node. Handles physics movement,
	collision etc

-------------------------------------------------------*/
#pragma once

#include <TootleCore/TLGraph.h>
#include <TootleCore/TFlags.h>
#include <TootleCore/TLMaths.h>
#include "TCollisionShape.h"
#include "TCollisionZone.h"



namespace TLMaths
{
	class TSphere;
	class TBox;
	class TCapsule;
};

namespace TLPhysics
{
	class TPhysicsNode;
	class TPhysicsgraph;
	class TCollisionZone;
	class TIntersection;

	extern float3		g_WorldUp;		//	gr: currently a global, change to be on the graph, or per node at some point so individual nodes can have their own gravity direction. Depends on what we need it for
	extern float3		g_WorldUpNormal;		//	gr: currently a global, change to be on the graph, or per node at some point so individual nodes can have their own gravity direction. Depends on what we need it for
};




class TLPhysics::TPhysicsNode : public TLGraph::TGraphNode<TPhysicsNode>
{
	friend class TLPhysics::TPhysicsgraph;
	friend class TLPhysics::TCollisionZone;
public:
	enum Flags
	{
		Flag_HasGravity = 0,	//	has generic gravity force applied
		Flag_RestOnStatic,		//	stick to floor when we hit it (stops tiny vibration bounce)
		Flag_Static,			//	does not move when collided wtih
		//Flag_CollideBothSides,	//	collide with inside of shape, or dont check normal in polygon 
		Flag_CollisionExpected,	//	expecting a valid collision shape
		Flag_ZoneExpected,		//	expecting to be in a collision zone
	};

public:
	TPhysicsNode(TRefRef NodeRef,TRefRef TypeRef=TRef());

	virtual void			Update(float Timestep);				//	physics update
	virtual void			PostUpdate(float Timestep,TLPhysics::TPhysicsgraph* pGraph,TPtr<TLPhysics::TPhysicsNode>& pThis);			//	after collisions are handled
	virtual Bool			PostIteration(u32 Iteration);		//	called after iteration, return TRUE to do another iteration

	float3						GetPosition() const;
	void						SetPosition(const float3& Position);
	void						MovePosition(const float3& Movement,float Timestep);
	const TLMaths::TTransform&	GetTransform() const					{	return m_Transform;	}
	virtual const TLMaths::TTransform&	GetRenderTransform() const		{	return GetTransform();	}

	TFlags<Flags>&			GetPhysicsFlags()					{	return m_PhysicsFlags;	}
	const TFlags<Flags>&	GetPhysicsFlags() const				{	return m_PhysicsFlags;	}
	Bool					IsStatic() const					{	return m_PhysicsFlags.IsSet( TPhysicsNode::Flag_Static );	}

	void					AddForce(const float3& Force)		{	if ( Force.LengthSq() == 0.f )	return;	m_Force += Force;	OnForceChanged();	}
	void					SetVelocity(const float3& Velocity)	{	m_Velocity = Velocity;	OnVelocityChanged();	}
	const float3&			GetVelocity() const					{	return (m_Velocity);	}
	
	void					OnVelocityChanged()					{	SetAccumulatedMovementInvalid();	SetWorldCollisionShapeInvalid();	}
	void					OnForceChanged()					{	SetAccumulatedMovementInvalid();	SetWorldCollisionShapeInvalid();	}
	void					OnTransformChanged()				{	SetWorldCollisionShapeInvalid();	}

	Bool					HasCollision() const				{	return m_pCollisionShape.IsValid() ? m_pCollisionShape->IsValid() : FALSE;	}
	void					SetCollisionNone()					{	m_pCollisionShape = NULL;	SetWorldCollisionShapeInvalid();	SetCollisionZoneNeedsUpdate();	}
	void					SetCollisionShape(TRefRef MeshRef);						//	setup polygon collision with a mesh
	void					SetCollisionShape(const TLMaths::TSphere& Sphere);		//	setup a sphere collision
	//void					SetCollisionShape(const TLMaths::TBox& Box);			//	setup a box collision
	//void					SetCollisionShape(const TLMaths::TCapsule& Capsule);	//	setup a capsule collision
	TLMaths::TTransform&	GetCollisionShapeTransform()					{	return m_Transform;	}
	TPtr<TCollisionShape>&	GetCollisionShape()								{	return m_pCollisionShape;	}
	
	TPtr<TCollisionShape>&			GetWorldCollisionShape()				{	return m_pWorldCollisionShape;	}
	const TPtr<TCollisionShape>&	GetWorldCollisionShape() const			{	return m_pWorldCollisionShape;	}
	TCollisionShape*				CalcWorldCollisionShape();				//	calculate transformed collision shape 
	FORCEINLINE void				SetWorldCollisionShapeInvalid()			{	if ( m_pWorldCollisionShape )	m_pLastWorldCollisionShape = m_pWorldCollisionShape;	m_pWorldCollisionShape = NULL;	}

	virtual Bool				OnCollision(const TPhysicsNode* pOtherNode);	//	handle collision with other object

	Bool						SetCollisionZone(TPtr<TCollisionZone>& pCollisionZone,TPtr<TPhysicsNode> pThis,const TFixedArray<u32,4>* pChildZoneList);
	TPtr<TCollisionZone>&		GetCollisionZone()										{	return m_pCollisionZone;	}
	TFixedArray<u32,4>&			GetChildCollisionZones()								{	return m_ChildCollisionZones;	}
	void						SetChildZonesNone()										{	m_ChildCollisionZones.Empty();	}
	void						SetChildZones(const TFixedArray<u32,4>& InZones);		//	

	FORCEINLINE void			SetCollisionZoneNeedsUpdate(Bool NeedsUpdate=TRUE)		{	m_CollisionZoneNeedsUpdate = NeedsUpdate;	}
	FORCEINLINE Bool			GetCollisionZoneNeedsUpdate() const						{	return m_CollisionZoneNeedsUpdate;	}
	void						UpdateNodeCollisionZone(TPtr<TLPhysics::TPhysicsNode>& pThis,TLPhysics::TPhysicsgraph* pGraph);	//	update what collision zone we're in

protected:
	const float3&				GetWorldUp() const							{	return HasParent() ? GetParent()->GetWorldUp() : TLPhysics::g_WorldUpNormal;	}
	
	void						PostUpdateAll(float Timestep,TLPhysics::TPhysicsgraph* pGraph,TPtr<TLPhysics::TPhysicsNode>& pThis);		//	update tree: update self, and children and siblings

	FORCEINLINE const float3&	GetAccumulatedMovement()					{	return m_AccumulatedMovementValid ? m_Temp_Intersection.m_Movement : CalcAccumulatedMovement();	}
	FORCEINLINE const float3&	CalcAccumulatedMovement()				 	{	m_AccumulatedMovementValid = TRUE;	return (m_Temp_Intersection.m_Movement = (m_Velocity + m_Force)*m_Temp_ExtrudeTimestep );	}
	FORCEINLINE void			SetAccumulatedMovementInvalid()				{	m_AccumulatedMovementValid = FALSE;	}
	FORCEINLINE Bool			IsAccumulatedMovementValid() const			{	return m_AccumulatedMovementValid;	}

	const float3&				GetForce() const					{	return (m_Force);	}
	float3						GetVelocityAndForce() const			{	return (m_Velocity + m_Force);	}

public:
	float					m_Friction;			//	
	float					m_Mass;				//	used for varying impact of two objects, larger object bounces less
	float					m_Bounce;			//	elasticity :)
	float					m_Squidge;			//	the amount (factor) the collision shape can be overlapped by (opposite to rigiditty)

	TArray<TIntersection>	m_Debug_Collisions;		//	temporary for debugging - list of all the collisions on the last physics update
	u32						m_Debug_StaticCollisions;	//	

protected:
	TLMaths::TTransform		m_Transform;			//	world transform of shape

	TFlags<Flags>			m_PhysicsFlags;
	float3					m_Velocity;
	float3					m_Force;
	float3					m_GravityForce;				//	gravity force applied this frame

	TPtr<TCollisionShape>	m_pCollisionShape;			//	collision shape
	TPtr<TCollisionShape>	m_pWorldCollisionShape;		//	transformed collision shape, delete/invalidated when pos or collision shape changes
	TPtr<TCollisionShape>	m_pLastWorldCollisionShape;	//	to save re-allocations of the same object, when we invalidate the world collision shape we set it to this. then when we recalc it, we try to reuse this pointer

	Bool					m_CollisionZoneNeedsUpdate;	//	does the collision zone need updating?
	TPtr<TCollisionZone>	m_pCollisionZone;			//	collision zone we're in
	TFixedArray<u32,4>		m_ChildCollisionZones;		//	collision zone we're intersecting, these are direct children of m_pCollisionZone
	Bool					m_InitialisedZone;			//	

	float					m_Temp_ExtrudeTimestep;		//	timestep for this frame... just saves passing around, used when calculating world collision shape for this frame
	TIntersection			m_Temp_Intersection;		//	current intersection. assume is invalid unless we're in an OnCollision func
	Bool					m_AccumulatedMovementValid;	//	accumulated movement float3 is now in m_Temp_Intersection, this bool dictates if it needs to be updated
};


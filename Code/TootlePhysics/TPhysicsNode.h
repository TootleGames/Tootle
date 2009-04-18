/*------------------------------------------------------

	Self contained physics node. Handles physics movement,
	collision etc

-------------------------------------------------------*/
#pragma once

#include <TootleCore/TLGraph.h>
#include <TootleCore/TFlags.h>
#include <TootleCore/TLMaths.h>
#include "TCollisionShape.h"
#include <TootleMaths/TQuadTree.h>
#include "TLPhysics.h"


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

	extern float3		g_WorldUp;			//	gr: currently a global, change to be on the graph, or per node at some point so individual nodes can have their own gravity direction. Depends on what we need it for
	extern float3		g_WorldUpNormal;	//	gr: currently a global, change to be on the graph, or per node at some point so individual nodes can have their own gravity direction. Depends on what we need it for
};


//------------------------------------------------------
//	
//------------------------------------------------------
class TLPhysics::TPhysicsNode : public TLGraph::TGraphNode<TLPhysics::TPhysicsNode>, public TLMaths::TQuadTreeNode
{
	friend class TLPhysics::TPhysicsgraph;
public:
	enum Flags
	{
		Flag_HasGravity = 0,	//	has generic gravity force applied
		Flag_RestOnStatic,		//	stick to floor when we hit it (stops tiny vibration bounce)
		Flag_Static,			//	does not move when collided wtih
		//Flag_CollideBothSides,	//	collide with inside of shape, or dont check normal in polygon 
		Flag_HasCollision,		//	expecting a valid collision shape - clear this to DISABLE collision, but still keep shape etc
		Flag_ZoneExpected,		//	expecting to be in a collision zone
		Flag_Enabled,			//	if not enabled, graph does not update this node
	};

public:
	TPhysicsNode(TRefRef NodeRef,TRefRef TypeRef=TRef());

	virtual void				Update(float Timestep);						//	physics update
	virtual void				Shutdown();									//	cleanup
	virtual void				PostUpdate(float Timestep,TLPhysics::TPhysicsgraph& Graph,TPtr<TLPhysics::TPhysicsNode>& pThis);			//	after collisions are handled
	virtual Bool				PostIteration(u32 Iteration);				//	called after iteration, return TRUE to do another iteration

	FORCEINLINE TRefRef			GetOwnerSceneNodeRef() const				{	return m_OwnerSceneNode;	}

	float3						GetPosition() const;
	void						SetPosition(const float3& Position);
	void						MovePosition(const float3& Movement,float Timestep);
	virtual void				SetTransform(const TLMaths::TTransform& NewTransform,Bool PublishChanges=TRUE);	//	explicit change of transform
	const TLMaths::TTransform&	GetTransform() const					{	return m_Transform;	}
	virtual const TLMaths::TTransform&	GetRenderTransform() const		{	return GetTransform();	}

	TFlags<Flags>&				GetPhysicsFlags()					{	return m_PhysicsFlags;	}
	const TFlags<Flags>&		GetPhysicsFlags() const				{	return m_PhysicsFlags;	}
	virtual Bool				IsStatic() const					{	return m_PhysicsFlags.IsSet( TPhysicsNode::Flag_Static );	}
	FORCEINLINE Bool			IsEnabled() const					{	return m_PhysicsFlags.IsSet( TPhysicsNode::Flag_Enabled );	}
	FORCEINLINE void			SetEnabled(Bool Enabled)			{	return m_PhysicsFlags.Set( TPhysicsNode::Flag_Enabled, Enabled );	}

	void					AddForce(const float3& Force)		{	if ( Force.LengthSq() == 0.f )	return;	m_Force += Force;	OnForceChanged();	}
	void					SetVelocity(const float3& Velocity)	{	m_Velocity = Velocity;	OnVelocityChanged();	}
	const float3&			GetVelocity() const					{	return (m_Velocity);	}
	
	void					OnVelocityChanged()					{	SetAccumulatedMovementInvalid();	SetWorldCollisionShapeInvalid();	}
	void					OnForceChanged()					{	SetAccumulatedMovementInvalid();	SetWorldCollisionShapeInvalid();	}

	FORCEINLINE void		OnTransformChanged(Bool TransChanged,Bool ScaleChanged,Bool RotationChanged)	{	m_TransformChangedBits |= TransChanged * TLMaths_TransformBitTranslate;	m_TransformChangedBits = ScaleChanged * TLMaths_TransformBitScale;	m_TransformChangedBits |= RotationChanged * TLMaths_TransformBitRotation;	SetWorldCollisionShapeInvalid();	}
	FORCEINLINE void		OnTransformChanged(u8 TransformChangedBits)	{	m_TransformChangedBits |= TransformChangedBits;	}
	FORCEINLINE void		OnTransformChangedNoPublish()		{	SetWorldCollisionShapeInvalid();	}
	FORCEINLINE void		OnTranslationChanged()				{	m_TransformChangedBits |= TLMaths_TransformBitTranslate;	SetWorldCollisionShapeInvalid();	}
	FORCEINLINE void		OnRotationChanged()					{	m_TransformChangedBits |= TLMaths_TransformBitRotation;		SetWorldCollisionShapeInvalid();	}
	FORCEINLINE void		OnScaleChanged()					{	m_TransformChangedBits |= TLMaths_TransformBitScale;		SetWorldCollisionShapeInvalid();	}

	//void					OnTransformChanged(Bool bTranslation, Bool bRotation, Bool bScale);

	Bool					HasCollision() const				{	return (IsEnabled() && HasCollisionFlag() && m_pCollisionShape.IsValid()) ? m_pCollisionShape->IsValid() : FALSE;	}
	Bool					HasCollisionFlag() const			{	return m_PhysicsFlags( Flag_HasCollision );	}
	void					SetCollisionNone()					{	m_pCollisionShape = NULL;	SetWorldCollisionShapeInvalid();	SetCollisionZoneNeedsUpdate();	}
	void					SetCollisionShape(TRefRef MeshRef);							//	setup polygon collision with a mesh
	void					SetCollisionShape(const TPtr<TLMaths::TShape>& pShape);		//	setup collision shape from a shape
	void					SetCollisionShape(const TLMaths::TSphere& Sphere);			//	setup a sphere collision
	void					SetCollisionShape(const TLMaths::TSphere2D& Sphere);		//	setup a sphere collision
	void					SetCollisionShape(const TLMaths::TCapsule2D& Capsule);		//	setup a capsule collision
	void					SetCollisionShape(const TLMaths::TOblong2D& Oblong);		//	setup an oblong collision
	void					SetCollisionShape(const TLMaths::TBox2D& Box);				//	setup an oblong collision
	TLMaths::TTransform&	GetCollisionShapeTransform()					{	return m_Transform;	}
	TPtr<TLMaths::TShape>&	GetCollisionShape()								{	return m_pCollisionShape;	}
	
	TPtr<TLMaths::TShape>&			GetWorldCollisionShape()				{	return m_pWorldCollisionShape;	}
	const TPtr<TLMaths::TShape>&	GetWorldCollisionShape() const			{	return m_pWorldCollisionShape;	}
	TLMaths::TShape*				CalcWorldCollisionShape();				//	calculate transformed collision shape 
	FORCEINLINE void				SetWorldCollisionShapeInvalid()			{	if ( m_pWorldCollisionShape )	m_pLastWorldCollisionShape = m_pWorldCollisionShape;	m_pWorldCollisionShape = NULL;	m_WorldCollisionShapeChanged = TRUE;	}

//	Bool						SetCollisionZone(TPtr<TLMaths::TQuadTreeZone>& pCollisionZone,TPtr<TPhysicsNode> pThis,const TFixedArray<u32,4>* pChildZoneList);

	FORCEINLINE void						SetCollisionZoneNeedsUpdate(Bool NeedsUpdate=TRUE)		{	SetZoneOutOfDate( NeedsUpdate );	}
	FORCEINLINE Bool						GetCollisionZoneNeedsUpdate() const						{	return IsZoneOutOfDate();	}
	void									UpdateNodeCollisionZone(TPtr<TLPhysics::TPhysicsNode>& pThis,TLPhysics::TPhysicsgraph& Graph);	//	update what collision zone we're in

	FORCEINLINE Bool			operator==(TRefRef Ref) const							{	return GetNodeRef() == Ref;	}

protected:
	virtual void				Initialise(TLMessaging::TMessage& Message);	
	void						PostUpdateAll(float Timestep,TLPhysics::TPhysicsgraph& Graph,TPtr<TLPhysics::TPhysicsNode>& pThis);		//	update tree: update self, and children and siblings
	const float3&				GetWorldUp() const							{	return HasParent() ? GetParent()->GetWorldUp() : TLPhysics::g_WorldUpNormal;	}

	FORCEINLINE const float3&	GetAccumulatedMovement()					{	return m_AccumulatedMovementValid ? m_Temp_Intersection.m_Movement : CalcAccumulatedMovement();	}
	FORCEINLINE const float3&	CalcAccumulatedMovement()				 	{	m_AccumulatedMovementValid = TRUE;	return (m_Temp_Intersection.m_Movement = (m_Velocity + m_Force)*m_Temp_ExtrudeTimestep );	}
	FORCEINLINE void			SetAccumulatedMovementInvalid()				{	m_AccumulatedMovementValid = FALSE;	}
	FORCEINLINE Bool			IsAccumulatedMovementValid() const			{	return m_AccumulatedMovementValid;	}
	void						PublishTransformChanges();					//	send transform changes as per m_TransformChanges

	const float3&				GetForce() const							{	return (m_Force);	}
	float3						GetVelocityAndForce() const					{	return (m_Velocity + m_Force);	}
	virtual float				GetFriction() const							{	return m_Friction;	}

	virtual Bool				OnCollision(const TPhysicsNode& OtherNode);	//	handle collision with other object
	void						AddCollisionInfo(const TLPhysics::TPhysicsNode& OtherNode,const TLMaths::TIntersection& Intersection);
	void						PublishCollisions();						//	send out our list of collisions

	virtual SyncBool			IsInShape(const TLMaths::TBox2D& Shape);
	virtual Bool				HasZoneShape();								//	return validity of shape for early bail out tests.

public:
	float					m_Friction;			//	
	float					m_Mass;				//	used for varying impact of two objects, larger object bounces less
	float					m_Bounce;			//	elasticity :)
	float					m_Squidge;			//	the amount (factor) the collision shape can be overlapped by (opposite to rigiditty)

protected:
	TLMaths::TTransform		m_Transform;				//	world transform of shape
	u8						m_TransformChangedBits;		//	dont broadcast trasnform changes until post update - TRANSFORM_BIT_XXX

	TFlags<Flags>			m_PhysicsFlags;
	float3					m_Velocity;
	float3					m_Force;
	float3					m_GravityForce;				//	gravity force applied this frame

	Bool					m_WorldCollisionShapeChanged;	//	bool to say if the collision shape changed during the physics update. reset at node pre update, and message is sent in post update
	TPtr<TLMaths::TShape>	m_pCollisionShape;			//	collision shape
	TPtr<TLMaths::TShape>	m_pWorldCollisionShape;		//	transformed collision shape, delete/invalidated when pos or collision shape changes
	TPtr<TLMaths::TShape>	m_pLastWorldCollisionShape;	//	to save re-allocations of the same object, when we invalidate the world collision shape we set it to this. then when we recalc it, we try to reuse this pointer
	TArray<TCollisionInfo>	m_Collisions;				//	list of collisions during our last update - published in PostUpdate to subscribers

	Bool					m_InitialisedZone;			//	

	float					m_Temp_ExtrudeTimestep;		//	timestep for this frame... just saves passing around, used when calculating world collision shape for this frame
	TLMaths::TIntersection	m_Temp_Intersection;		//	current intersection. assume is invalid unless we're in an OnCollision func
	Bool					m_AccumulatedMovementValid;	//	accumulated movement float3 is now in m_Temp_Intersection, this bool dictates if it needs to be updated

	TRef					m_OwnerSceneNode;			//	"Owner" scene node - if this is set then we automaticcly process some stuff
};



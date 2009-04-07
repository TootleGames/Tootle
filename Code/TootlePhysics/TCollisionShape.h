/*------------------------------------------------------

	collision object which is a wrapper for shapes

-------------------------------------------------------*/
#pragma once

#include <TootleCore/TLTypes.h>
#include <TootleCore/TLMaths.h>
#include <TootleCore/TPtr.h>
#include <TootleCore/TRef.h>
#include <TootleAsset/TMesh.h>


namespace TLAsset
{
	class TMesh;
}


namespace TLPhysics
{
	class TPhysicsgraph;
	class TIntersection;

	class TCollisionShape;			//	base collision type
	class TCollisionSphere;			//	sphere collision shape
	class TCollisionBox;			//	box collision shape
	class TCollisionBox2D;			//	2D box collision shape
	class TCollisionOblong2D;		//	2D oblong box collision shape
	class TCollisionCapsule2D;
	class TCollisionMesh;			//	collision type that uses a mesh asset to test intersections with
	class TCollisionMeshWithBounds;	//	mesh collision type with transformed bounds sphere/box


	//	non standard "shape" type refs
	#define TLMaths_ShapeRef_Mesh				TRef_Static3(M,s,h)
	#define TLMaths_ShapeRef_MeshWithBounds		TRef_Static(M,s,h,W,B)
	FORCEINLINE TRef		GetMeshShapeTypeRef()			{	return TLMaths_ShapeRef(Mesh);	}
	FORCEINLINE TRef		GetMeshWithBoundsShapeTypeRef()	{	return TLMaths_ShapeRef(MeshWithBounds);	}
}





//-----------------------------------------------------
//	intersection information
//-----------------------------------------------------
class TLPhysics::TIntersection
{
public:
	FORCEINLINE TIntersection() : 
		m_HasNormal		( FALSE )
	{
	}

	void		Transform(const TLMaths::TTransform& Transform);	//	transform applied to node A when we did the intersection test
	void		Untransform(const TLMaths::TTransform& Transform);	//	undo a transform applied to node A when we did the intersection test

	void		Reset()					{	m_HasNormal = FALSE;	}	//	reset vars before collision detection

public:
	float3		m_Intersection;			//	intersection point of NodeA
	float3		m_Movement;				//	node's accumulated movement
	float3		m_PostCollisionDelta;	//	the force change after collision
	
	Bool		m_HasNormal;			//	the intersection normal is valid
	float3		m_Normal;				//	normal of the plane we intersected with
};




class TLPhysics::TCollisionShape
{
	friend class TLPhysics::TPhysicsgraph;
public:
	TCollisionShape()															{	}
	virtual ~TCollisionShape()													{	}

	virtual const TRef				GetShapeType() const = 0;					//	return a shape type

	virtual Bool					IsValid() const = 0;						//	check if the shape is valid
	virtual float3					GetCenter() const = 0;						//	get the center of the shape

	virtual TPtr<TCollisionShape>	Transform(const TLMaths::TTransform& Transform,TPtr<TLPhysics::TCollisionShape>& pThis,TPtr<TLPhysics::TCollisionShape>& pOldShape)	{	return NULL;	}	//	transform this collision shape into a world-relative shape

	Bool							HasIntersection(TCollisionShape* pCollisionShape);
	Bool							GetIntersection(TCollisionShape* pCollisionShape,TIntersection& NodeAIntersection,TIntersection& NodeBIntersection);

protected:
	//	simple fast intersection tests which don't need intersection information - default behaviour uses more expensive "GetIntersection" code
	virtual Bool					HasIntersection_Sphere(TCollisionSphere* pCollisionShape)					{	TIntersection a,b;	return GetIntersection_Sphere( pCollisionShape, a, b );	}
	virtual Bool					HasIntersection_Box(TCollisionBox* pCollisionShape)							{	TIntersection a,b;	return GetIntersection_Box( pCollisionShape, a, b );	}
	virtual Bool					HasIntersection_Box2D(TCollisionBox2D* pCollisionShape)						{	TIntersection a,b;	return GetIntersection_Box2D( pCollisionShape, a, b );	}
	virtual Bool					HasIntersection_Oblong2D(TCollisionOblong2D* pCollisionShape)				{	TIntersection a,b;	return GetIntersection_Oblong2D( pCollisionShape, a, b );	}
	virtual Bool					HasIntersection_Capsule2D(TCollisionCapsule2D* pCollisionShape)				{	TIntersection a,b;	return GetIntersection_Capsule2D( pCollisionShape, a, b );	}
	virtual Bool					HasIntersection_Mesh(TCollisionMesh* pCollisionShape)						{	TIntersection a,b;	return GetIntersection_Mesh( pCollisionShape, a, b );	}
	virtual Bool					HasIntersection_MeshWithBounds(TCollisionMeshWithBounds* pCollisionShape)	{	TIntersection a,b;	return GetIntersection_MeshWithBounds( pCollisionShape, a, b );	}

	//	full intersection tests which return intersection info
	virtual Bool					GetIntersection_Sphere(TCollisionSphere* pCollisionShape,TIntersection& NodeAIntersection,TIntersection& NodeBIntersection);
	virtual Bool					GetIntersection_Box(TCollisionBox* pCollisionShape,TIntersection& NodeAIntersection,TIntersection& NodeBIntersection);
	virtual Bool					GetIntersection_Box2D(TCollisionBox2D* pCollisionShape,TIntersection& NodeAIntersection,TIntersection& NodeBIntersection);
	virtual Bool					GetIntersection_Oblong2D(TCollisionOblong2D* pCollisionShape,TIntersection& NodeAIntersection,TIntersection& NodeBIntersection);
	virtual Bool					GetIntersection_Capsule2D(TCollisionCapsule2D* pCollisionShape,TIntersection& NodeAIntersection,TIntersection& NodeBIntersection);
	virtual Bool					GetIntersection_Mesh(TCollisionMesh* pCollisionMesh,TIntersection& NodeAIntersection,TIntersection& NodeBIntersection);
	virtual Bool					GetIntersection_MeshWithBounds(TCollisionMeshWithBounds* pCollisionMesh,TIntersection& NodeAIntersection,TIntersection& NodeBIntersection);

	Bool							Debug_IntersectionTodo(TRefRef WithShapeType);

protected:
};




class TLPhysics::TCollisionSphere : public TLPhysics::TCollisionShape
{
public:
	TCollisionSphere()															{}
	TCollisionSphere(const TLMaths::TSphere& Sphere) : m_Sphere ( Sphere )		{}

	virtual const TRef				GetShapeType() const						{	return TLMaths_ShapeRef(TSphere);	}

	void							SetSphere(const TLMaths::TSphere& Sphere)	{	m_Sphere = Sphere;	}
	const TLMaths::TSphere&			GetSphere() const							{	return m_Sphere;	}
	virtual Bool					IsValid() const								{	return GetSphere().IsValid();	}
	virtual float3					GetCenter() const							{	return GetSphere().GetPos();	}

	virtual TPtr<TCollisionShape>	Transform(const TLMaths::TTransform& Transform,TPtr<TLPhysics::TCollisionShape>& pThis,TPtr<TLPhysics::TCollisionShape>& pOldShape);

protected:
	virtual Bool					GetIntersection_Sphere(TCollisionSphere* pCollisionShape,TIntersection& NodeAIntersection,TIntersection& NodeBIntersection);
	virtual Bool					GetIntersection_Mesh(TCollisionMesh* pCollisionMesh,TIntersection& NodeAIntersection,TIntersection& NodeBIntersection);
	virtual Bool					GetIntersection_MeshWithBounds(TCollisionMeshWithBounds* pCollisionMesh,TIntersection& NodeAIntersection,TIntersection& NodeBIntersection);

	virtual Bool					HasIntersection_Box2D(TCollisionBox2D* pCollisionShape);

protected:
	TLMaths::TSphere				m_Sphere;	//	sphere collision object
};



class TLPhysics::TCollisionBox : public TLPhysics::TCollisionShape
{
public:
	TCollisionBox()												{}
	TCollisionBox(const TLMaths::TBox& Box) : m_Box ( Box )		{	GenerateBoundsSphere();	}

	virtual const TRef				GetShapeType() const						{	return TLMaths_ShapeRef(TBox);	}

	void							SetBox(const TLMaths::TBox& Box)			{	m_Box = Box;	GenerateBoundsSphere();	}
	const TLMaths::TBox&			GetBox() const								{	return m_Box;	}
	virtual Bool					IsValid() const								{	return GetBox().IsValid();	}
	virtual float3					GetCenter() const							{	return GetBox().GetCenter();	}

	virtual TPtr<TCollisionShape>	Transform(const TLMaths::TTransform& Transform,TPtr<TLPhysics::TCollisionShape>& pThis,TPtr<TLPhysics::TCollisionShape>& pOldShape);

protected:
	void							GenerateBoundsSphere()						{	m_BoundsSphere.SetInvalid();	m_BoundsSphere.Accumulate( m_Box );	}

protected:
	virtual Bool					HasIntersection_Sphere(TCollisionSphere* pCollisionShape);
	virtual Bool					HasIntersection_Mesh(TCollisionMesh* pCollisionMesh);
	virtual Bool					HasIntersection_MeshWithBounds(TCollisionMeshWithBounds* pCollisionMesh);

protected:
	TLMaths::TSphere				m_BoundsSphere;	//	private faster sphere to test with before we do boxs test
	TLMaths::TBox					m_Box;			//	box collision object
};



class TLPhysics::TCollisionBox2D : public TLPhysics::TCollisionShape
{
	friend class TLPhysics::TCollisionCapsule2D;
public:
	TCollisionBox2D()												{}
	TCollisionBox2D(const TLMaths::TBox2D& Box) : m_Box ( Box )		{	GenerateBoundsSphere();	}

	virtual const TRef				GetShapeType() const						{	return TLMaths_ShapeRef(TBox2D);	}

	void							SetBox(const TLMaths::TBox2D& Box)			{	m_Box = Box;	GenerateBoundsSphere();	}
	const TLMaths::TBox2D&			GetBox() const								{	return m_Box;	}
	virtual Bool					IsValid() const								{	return GetBox().IsValid();	}
	virtual float3					GetCenter() const							{	return GetBox().GetCenter3();	}

	virtual TPtr<TCollisionShape>	Transform(const TLMaths::TTransform& Transform,TPtr<TLPhysics::TCollisionShape>& pThis,TPtr<TLPhysics::TCollisionShape>& pOldShape);

protected:
	void							GenerateBoundsSphere()						{	m_BoundsSphere.SetInvalid();	m_BoundsSphere.Accumulate( m_Box );	}

protected:
	virtual Bool					HasIntersection_Sphere(TCollisionSphere* pCollisionShape);
	virtual Bool					HasIntersection_Mesh(TCollisionMesh* pCollisionMesh);
	virtual Bool					HasIntersection_MeshWithBounds(TCollisionMeshWithBounds* pCollisionMesh);
	virtual Bool					HasIntersection_Capsule2D(TCollisionCapsule2D* pCollisionShape);

//	virtual Bool					GetIntersection_Sphere(TCollisionSphere* pCollisionShape,TIntersection& NodeAIntersection,TIntersection& NodeBIntersection);
//	virtual Bool					GetIntersection_Box(TCollisionBox* pCollisionShape,TIntersection& NodeAIntersection,TIntersection& NodeBIntersection);
//	virtual Bool					GetIntersection_Box2D(TCollisionBox2D* pCollisionShape,TIntersection& NodeAIntersection,TIntersection& NodeBIntersection);
//	virtual Bool					GetIntersection_Mesh(TCollisionMesh* pCollisionMesh,TIntersection& NodeAIntersection,TIntersection& NodeBIntersection);
//	virtual Bool					GetIntersection_MeshWithBounds(TCollisionMeshWithBounds* pCollisionMesh,TIntersection& NodeAIntersection,TIntersection& NodeBIntersection);

protected:
	TLMaths::TSphere2D				m_BoundsSphere;	//	private faster sphere to test with before we do boxs test
	TLMaths::TBox2D					m_Box;			//	box collision object
};



class TLPhysics::TCollisionOblong2D : public TLPhysics::TCollisionShape
{
public:
	TCollisionOblong2D()												{}
	TCollisionOblong2D(const TLMaths::TOblong2D& Oblong) : m_Oblong ( Oblong )	{	OnOblongChanged();	}

	virtual const TRef				GetShapeType() const						{	return TLMaths_ShapeRef(TOblong2D);	}

	void							SetOblong(const TLMaths::TOblong2D& Oblong)	{	m_Oblong = Oblong;	OnOblongChanged();	}
	const TLMaths::TOblong2D&		GetOblong() const							{	return m_Oblong;	}
	virtual Bool					IsValid() const								{	return GetOblong().IsValid();	}
	virtual float3					GetCenter() const							{	return m_Center;	}

//	virtual TPtr<TCollisionShape>	Transform(const TLMaths::TTransform& Transform,TPtr<TLPhysics::TCollisionShape>& pThis,TPtr<TLPhysics::TCollisionShape>& pOldShape);

protected:
	void							OnOblongChanged()							{	m_BoundsSphere.SetInvalid();	m_BoundsSphere.Accumulate( GetOblong().GetBoxCorners() );	m_Center = m_Oblong.GetCenter();	}

protected:
	TLMaths::TSphere2D				m_BoundsSphere;	//	private faster sphere to test with before we do boxs test
	TLMaths::TOblong2D				m_Oblong;		//	box collision object
	float2							m_Center;		//	"cached" center for the oblong
};


class TLPhysics::TCollisionCapsule2D : public TLPhysics::TCollisionShape
{
public:
	TCollisionCapsule2D()												{}
	TCollisionCapsule2D(const TLMaths::TCapsule2D& Capsule) : m_Capsule ( Capsule )	{	}

	virtual const TRef				GetShapeType() const						{	return TLMaths_ShapeRef(TCapsule2D);	}

	void							SetCapsule(const TLMaths::TCapsule2D& Capsule)	{	m_Capsule = Capsule;	}
	const TLMaths::TCapsule2D&		GetCapsule() const							{	return m_Capsule;	}
	virtual Bool					IsValid() const								{	return GetCapsule().IsValid();	}
	virtual float3					GetCenter() const							{	return GetCapsule().GetCenter();	}

	virtual TPtr<TCollisionShape>	Transform(const TLMaths::TTransform& Transform,TPtr<TLPhysics::TCollisionShape>& pThis,TPtr<TLPhysics::TCollisionShape>& pOldShape);

protected:
	virtual Bool					HasIntersection_Box2D(TCollisionBox2D* pCollisionShape)		{	return pCollisionShape->HasIntersection_Capsule2D( this );	}

	virtual Bool					GetIntersection_Capsule2D(TCollisionCapsule2D* pCollisionShape,TIntersection& NodeAIntersection,TIntersection& NodeBIntersection);

protected:
	TLMaths::TCapsule2D				m_Capsule;		//	capsule shape
};


class TLPhysics::TCollisionMesh : public TLPhysics::TCollisionShape
{
public:
	TCollisionMesh()											{	}
	TCollisionMesh(TRefRef MeshRef) : m_MeshRef ( MeshRef )		{	}

	virtual const TRef				GetShapeType() const		{	return TLMaths_ShapeRef(Mesh);	}

	TRefRef							GetMeshRef() const			{	return m_MeshRef;	}
	void							SetMeshRef(TRefRef MeshRef)	{	m_MeshRef = MeshRef;		m_pMeshCache = NULL;	}
	virtual Bool					GetMeshTransform(TLMaths::TTransform& Transform) const		{	return FALSE;	}
	virtual Bool					IsValid() const				{	return GetMeshAsset().IsValid();	}
	virtual float3					GetCenter() const;

	virtual TPtr<TCollisionShape>	Transform(const TLMaths::TTransform& Transform,TPtr<TLPhysics::TCollisionShape>& pThis,TPtr<TLPhysics::TCollisionShape>& pOldShape);
	
	TPtr<TLAsset::TMesh>&			GetMeshAsset();				//	get mesh asset and cache
	TPtr<TLAsset::TMesh>			GetMeshAsset() const;		//	get mesh asset, but cannot cache as const

protected:
//	virtual Bool					GetIntersection_Mesh(TCollisionMesh* pCollisionMesh,TIntersection& NodeAIntersection,TIntersection& NodeBIntersection);
//	virtual Bool					GetIntersection_MeshWithBounds(TCollisionMeshWithBounds* pCollisionMesh,TIntersection& NodeAIntersection,TIntersection& NodeBIntersection);
	
protected:
	TRef							m_MeshRef;		//	mesh asset 
	TPtr<TLAsset::TMesh>			m_pMeshCache;	//	
};



class TLPhysics::TCollisionMeshWithBounds : public TLPhysics::TCollisionMesh
{
public:
	virtual const TRef				GetShapeType() const						{	return TLMaths_ShapeRef(MeshWithBounds);	}
	
	void							SetSphere(const TLMaths::TSphere& Sphere)	{	m_BoundsSphere = Sphere;	}
	const TLMaths::TSphere&			GetSphere() const							{	return m_BoundsSphere;	}
	void							SetBox(const TLMaths::TBox& Box)			{	m_BoundsBox = Box;	}
	const TLMaths::TBox&			GetBox() const								{	return m_BoundsBox;	}
	
	virtual Bool					GetMeshTransform(TLMaths::TTransform& Transform) const		{	if ( !m_MeshTransform.HasAnyTransform() )	return FALSE;	Transform = m_MeshTransform;	return TRUE;	}
	void							SetMeshTransform(const TLMaths::TTransform& MeshTransform)	{	m_MeshTransform = MeshTransform;	}

	virtual float3					GetCenter() const;

protected:
//	virtual Bool					HasIntersection_Box(TCollisionBox* pCollisionShape);

	virtual Bool					GetIntersection_Sphere(TCollisionSphere* pCollisionShape,TIntersection& NodeAIntersection,TIntersection& NodeBIntersection);
	virtual Bool					GetIntersection_Box(TCollisionBox* pCollisionShape,TIntersection& NodeAIntersection,TIntersection& NodeBIntersection);
	virtual Bool					GetIntersection_Box2D(TCollisionBox2D* pCollisionShape,TIntersection& NodeAIntersection,TIntersection& NodeBIntersection);

protected:
	TLMaths::TSphere				m_BoundsSphere;		//	transformed sphere bounds
	TLMaths::TBox					m_BoundsBox;		//	transformed box bounds
	TLMaths::TTransform				m_MeshTransform;	//	transform for mesh
};



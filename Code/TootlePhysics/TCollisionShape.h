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
	class TCollisionZone;

	class TCollisionShape;			//	base collision type
	class TCollisionSphere;			//	sphere collision shape
	class TCollisionBox;			//	box collision shape
	class TCollisionBox2D;			//	2D box collision shape
	class TCollisionMesh;			//	collision type that uses a mesh asset to test intersections with
	class TCollisionMeshWithBounds;	//	mesh collision type with transformed bounds sphere/box

	enum ShapeType
	{
		Shape_Sphere,
		Shape_Box,
		Shape_Box2D,
		Shape_Mesh,
		Shape_MeshWithBounds,
	};

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
	friend class TLPhysics::TCollisionZone;
public:
	TCollisionShape()															{	}
	virtual ~TCollisionShape()													{	}

	virtual TLPhysics::ShapeType	GetShapeType() const = 0;					//	return a shape type

	virtual Bool					IsValid() const = 0;						//	check if the shape is valid
	virtual float3					GetCenter() const = 0;						//	get the center of the shape

	virtual TPtr<TCollisionShape>	Transform(const TLMaths::TTransform& Transform,TPtr<TLPhysics::TCollisionShape>& pThis,TPtr<TLPhysics::TCollisionShape>& pOldShape)	{	return NULL;	}	//	transform this collision shape into a world-relative shape

	Bool							HasIntersection(TCollisionShape* pCollisionShape);

protected:
	//	simple fast intersection tests which don't need intersection information
	virtual Bool					HasIntersection_Sphere(TCollisionSphere* pCollisionShape);
	virtual Bool					HasIntersection_Box(TCollisionBox* pCollisionShape);
	virtual Bool					HasIntersection_Box2D(TCollisionBox2D* pCollisionShape);
	virtual Bool					HasIntersection_Mesh(TCollisionMesh* pCollisionMesh);
	virtual Bool					HasIntersection_MeshWithBounds(TCollisionMeshWithBounds* pCollisionMesh);

	//	full intersection tests which return intersection info
	Bool							GetIntersection(TCollisionShape* pCollisionShape,TIntersection& NodeAIntersection,TIntersection& NodeBIntersection);
	virtual Bool					GetIntersection_Sphere(TCollisionSphere* pCollisionShape,TIntersection& NodeAIntersection,TIntersection& NodeBIntersection);
	virtual Bool					GetIntersection_Box(TCollisionBox* pCollisionShape,TIntersection& NodeAIntersection,TIntersection& NodeBIntersection);
	virtual Bool					GetIntersection_Box2D(TCollisionBox2D* pCollisionShape,TIntersection& NodeAIntersection,TIntersection& NodeBIntersection);
	virtual Bool					GetIntersection_Mesh(TCollisionMesh* pCollisionMesh,TIntersection& NodeAIntersection,TIntersection& NodeBIntersection);
	virtual Bool					GetIntersection_MeshWithBounds(TCollisionMeshWithBounds* pCollisionMesh,TIntersection& NodeAIntersection,TIntersection& NodeBIntersection);

protected:
};




class TLPhysics::TCollisionSphere : public TLPhysics::TCollisionShape
{
public:
	TCollisionSphere()															{}
	TCollisionSphere(const TLMaths::TSphere& Sphere) : m_Sphere ( Sphere )		{}

	virtual TLPhysics::ShapeType	GetShapeType() const						{	return TLPhysics::Shape_Sphere;	}

	void							SetSphere(const TLMaths::TSphere& Sphere)	{	m_Sphere = Sphere;	}
	const TLMaths::TSphere&			GetSphere() const							{	return m_Sphere;	}
	virtual Bool					IsValid() const								{	return GetSphere().IsValid();	}
	virtual float3					GetCenter() const							{	return GetSphere().GetPos();	}

	virtual TPtr<TCollisionShape>	Transform(const TLMaths::TTransform& Transform,TPtr<TLPhysics::TCollisionShape>& pThis,TPtr<TLPhysics::TCollisionShape>& pOldShape);

protected:
	virtual Bool					GetIntersection_Sphere(TCollisionSphere* pCollisionShape,TIntersection& NodeAIntersection,TIntersection& NodeBIntersection);
	virtual Bool					GetIntersection_Mesh(TCollisionMesh* pCollisionMesh,TIntersection& NodeAIntersection,TIntersection& NodeBIntersection);
	virtual Bool					GetIntersection_MeshWithBounds(TCollisionMeshWithBounds* pCollisionMesh,TIntersection& NodeAIntersection,TIntersection& NodeBIntersection);

protected:
	TLMaths::TSphere				m_Sphere;	//	sphere collision object
};



class TLPhysics::TCollisionBox : public TLPhysics::TCollisionShape
{
public:
	TCollisionBox()												{}
	TCollisionBox(const TLMaths::TBox& Box) : m_Box ( Box )		{	GenerateBoundsSphere();	}

	virtual TLPhysics::ShapeType	GetShapeType() const						{	return TLPhysics::Shape_Box;	}

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
public:
	TCollisionBox2D()												{}
	TCollisionBox2D(const TLMaths::TBox2D& Box) : m_Box ( Box )		{	GenerateBoundsSphere();	}

	virtual TLPhysics::ShapeType	GetShapeType() const						{	return TLPhysics::Shape_Box;	}

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

//	virtual Bool					GetIntersection_Sphere(TCollisionSphere* pCollisionShape,TIntersection& NodeAIntersection,TIntersection& NodeBIntersection);
//	virtual Bool					GetIntersection_Box(TCollisionBox* pCollisionShape,TIntersection& NodeAIntersection,TIntersection& NodeBIntersection);
//	virtual Bool					GetIntersection_Box2D(TCollisionBox2D* pCollisionShape,TIntersection& NodeAIntersection,TIntersection& NodeBIntersection);
//	virtual Bool					GetIntersection_Mesh(TCollisionMesh* pCollisionMesh,TIntersection& NodeAIntersection,TIntersection& NodeBIntersection);
//	virtual Bool					GetIntersection_MeshWithBounds(TCollisionMeshWithBounds* pCollisionMesh,TIntersection& NodeAIntersection,TIntersection& NodeBIntersection);

protected:
	TLMaths::TSphere2D				m_BoundsSphere;	//	private faster sphere to test with before we do boxs test
	TLMaths::TBox2D					m_Box;			//	box collision object
};



class TLPhysics::TCollisionMesh : public TLPhysics::TCollisionShape
{
public:
	TCollisionMesh()											{	}
	TCollisionMesh(TRefRef MeshRef) : m_MeshRef ( MeshRef )		{	}

	virtual TLPhysics::ShapeType	GetShapeType() const		{	return TLPhysics::Shape_Mesh;	}

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
	virtual TLPhysics::ShapeType	GetShapeType() const						{	return TLPhysics::Shape_MeshWithBounds;	}
	
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



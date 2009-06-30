/*------------------------------------------------------

	Overloadable shape type

-------------------------------------------------------*/
#pragma once

#include "TSphere.h"
#include "TBox.h"
#include "TOblong.h"
#include "TCapsule.h"
#include <TootleCore/TPtr.h>


namespace TLMaths
{
	class TShape;			//	base shape type
	class TShapeSphere2D;	//	sphere shape
	class TShapeSphere;		//	sphere shape
	class TShapeCapsule2D;	//	capsule shape
	class TShapeBox;		//	box shape
	class TShapeBox2D;		//	box shape
//	class TShapeMesh;		//	mesh "shape" for triangle/face, line intersection etc
	class TShapePolygon2D;	//	convex polygon shape

	class TIntersection;	//	resulting intersection information of two shapes

	TPtr<TShape>				ImportShapeData(TBinaryTree& Data);									//	create a shape type from TBinaryData
	Bool						ImportShapeData(TBinaryTree& Data,TLMaths::TShape& Shape);			//	Import shape data from a TBinarydata into an exisiting shape type. If the types don't match this will fail
	FORCEINLINE TPtr<TShape>	ImportShapeData(TBinaryTree* pData)									{	return pData ? ImportShapeData( *pData ) : TPtr<TShape>(NULL);	}
	FORCEINLINE Bool			ImportShapeData(TBinaryTree* pData,TLMaths::TShape& Shape)			{	return pData ? ImportShapeData( *pData, Shape ) : FALSE;	}
	Bool						ImportShapeData(TBinaryTree& Data,TLMaths::TShape& Shape);			//	Import shape data from a TBinarydata into an exisiting shape type. If the types don't match this will fail
	Bool						ExportShapeData(TBinaryTree& Data,const TLMaths::TShape& Shape,Bool WriteIfInvalid=TRUE);	//	export shape to data
	FORCEINLINE Bool			ExportShapeData(TBinaryTree* pData,const TLMaths::TShape& Shape,Bool WriteIfInvalid=TRUE)	{	return ExportShapeData( *pData, Shape, WriteIfInvalid );	}
	TPtr<TShape>				CreateShapeType(TRefRef ShapeType);									//	create shape instance from ref
};




class TLMaths::TIntersection
{
public:
	TIntersection(const float3& IntersectionPos=float3(0,0,0)) :
		m_Intersection	( IntersectionPos ),
		m_HasNormal		( FALSE )
	{
	}

	void			Transform(const TLMaths::TTransform& Transform);	//	transform applied to node A when we did the intersection test
	void			Untransform(const TLMaths::TTransform& Transform);	//	undo a transform applied to node A when we did the intersection test

	void			Reset()					{	m_HasNormal = FALSE;	}	//	reset vars before collision detection

	const float3&	GetPosition() const		{	return m_Intersection;	}

public:
	float3		m_Intersection;			//	intersection point of NodeA
	float3		m_Movement;				//	node's accumulated movement
	float3		m_PostCollisionDelta;	//	the force change after collision
	
	Bool		m_HasNormal;			//	the intersection normal is valid
	float3		m_Normal;				//	normal of the plane we intersected with
};




class TLMaths::TShape
{
	//	give these import/export functions access to our protected Import/Export routines. We keep these protected in 
	//	TShapes as we do NOT want users to access them directly.
	friend TPtr<TShape>	TLMaths::ImportShapeData(TBinaryTree& Data);
	friend Bool			TLMaths::ImportShapeData(TBinaryTree& Data,TLMaths::TShape& Shape);
	friend Bool			TLMaths::ExportShapeData(TBinaryTree& Data,const TLMaths::TShape& Shape,Bool WriteIfInvalid);	

public:
	TShape()															{	}
	virtual ~TShape()													{	}

	virtual TRef					GetShapeType() const = 0;					//	return a shape type
	virtual Bool					IsValid() const = 0;						//	check if the shape is valid
	virtual void					SetInvalid() = 0;							//	set shape as invalid
	virtual float3					GetCenter() const = 0;						//	get the center of the shape
	virtual float3					GetRandomPosition() const					{	return GetCenter();	}	//	return a random position inside the shape

	virtual void					Transform(const TLMaths::TTransform& Transform)												{	TLDebug_Break("Overload me");	}	//	transform self
	virtual TPtr<TShape>			Transform(const TLMaths::TTransform& Transform,TPtr<TShape>& pOldShape) const	{	return NULL;	}	//	transform this collision shape into a world-relative shape

	//	simple fast intersection tests which don't need intersection information - default behaviour uses more expensive "GetIntersection" code
	Bool							HasIntersection(TShape& OtherShape);								//	for the low level type we use our shape refs to work out which one to call... redun
	virtual Bool					HasIntersection(TShapeSphere& OtherShape)							{	TIntersection a,b;	return GetIntersection( OtherShape, a, b );	}
	virtual Bool					HasIntersection(TShapeSphere2D& OtherShape)							{	TIntersection a,b;	return GetIntersection( OtherShape, a, b );	}
	virtual Bool					HasIntersection(TShapeBox& OtherShape)								{	TIntersection a,b;	return GetIntersection( OtherShape, a, b );	}
	virtual Bool					HasIntersection(TShapeBox2D& OtherShape)							{	TIntersection a,b;	return GetIntersection( OtherShape, a, b );	}
	virtual Bool					HasIntersection(TShapePolygon2D& OtherShape)						{	TIntersection a,b;	return GetIntersection( OtherShape, a, b );	}
	virtual Bool					HasIntersection(TShapeCapsule2D& OtherShape)						{	TIntersection a,b;	return GetIntersection( OtherShape, a, b );	}
//	virtual Bool					HasIntersection(TShapeMesh* pCollisionShape)						{	TIntersection a,b;	return GetIntersection_Mesh( OtherShape, a, b );	}

	//	full intersection tests which return intersection info
	Bool							GetIntersection(TShape& OtherShape,TIntersection& NodeAIntersection,TIntersection& NodeBIntersection);
	virtual Bool					GetIntersection(TShapeSphere& OtherShape,TIntersection& NodeAIntersection,TIntersection& NodeBIntersection);
	virtual Bool					GetIntersection(TShapeSphere2D& OtherShape,TIntersection& NodeAIntersection,TIntersection& NodeBIntersection);
	virtual Bool					GetIntersection(TShapeBox& OtherShape,TIntersection& NodeAIntersection,TIntersection& NodeBIntersection);
	virtual Bool					GetIntersection(TShapeBox2D& OtherShape,TIntersection& NodeAIntersection,TIntersection& NodeBIntersection);
	virtual Bool					GetIntersection(TShapePolygon2D& OtherShape,TIntersection& NodeAIntersection,TIntersection& NodeBIntersection);
	virtual Bool					GetIntersection(TShapeCapsule2D& OtherShape,TIntersection& NodeAIntersection,TIntersection& NodeBIntersection);
//	virtual Bool					GetIntersection(TShapeMesh* pCollisionMesh,TIntersection& NodeAIntersection,TIntersection& NodeBIntersection);
	
	//template<class TYPE> 
	//FORCEINLINE void				operator=(const TYPE& Shape)				{	m_Box = Shape;	}
	
protected:
	virtual Bool					ImportData(TBinaryTree& Data)			{	TLDebug_Break("Shape ImportData required");	return FALSE;	}
	virtual Bool					ExportData(TBinaryTree& Data) const		{	TLDebug_Break("Shape ExportData required");	return FALSE;	}

	void							Debug_BreakOverloadThis(const char* pTestType,TShape& OtherShape);

protected:
};





/*
class TLMaths::TShapeMesh : public TLMaths::TShape
{
public:
	TShapeMesh()															{}
	TShapeMesh(TRefRef MeshRef) : m_MeshRef ( MeshRef )					{}

	static TRef						GetShapeType_Static()						{	return TLMaths_ShapeRef(Mesh);	}
	virtual TRef					GetShapeType() const						{	return TLMaths_ShapeRef(Mesh);	}
	virtual Bool					IsValid() const								{	return m_MeshRef.IsValid();	}
	virtual float3					GetCenter() const							{	return float3(0,0,0);	}
	
	const TLMaths::TBox2D&			GetBox() const								{	return m_Box;	}

protected:
	virtual Bool					ImportData(TBinaryTree& Data);
	virtual Bool					ExportData(TBinaryTree& Data) const;

protected:
	TLMaths::TBox2D					m_Box;
};

*/



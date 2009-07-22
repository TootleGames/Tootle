/*------------------------------------------------------

	collision object which is a wrapper for shapes

-------------------------------------------------------*/
#pragma once

#include <TootleCore/TLTypes.h>
#include <TootleCore/TRef.h>
#include <TootleCore/TLMaths.h>
#include "TLPhysics.h"
#include <box2d/include/box2d.h>


namespace TLPhysics
{
	class TCollisionShape;
}


//------------------------------------------------------
//	shape with a ref
//------------------------------------------------------
class TLPhysics::TCollisionShape
{
public:
	TCollisionShape() : m_pBodyShape ( NULL ), m_IsSensor ( FALSE )	{}

	FORCEINLINE TRefRef				GetShapeRef() const							{	return m_ShapeRef;	}
	FORCEINLINE void				SetShapeRef(TRefRef ShapeRef)				{	m_ShapeRef = ShapeRef;	}
	const TPtr<TLMaths::TShape>&	GetShape() const							{	return m_pShape;	}
	FORCEINLINE void				SetShape(const TPtr<TLMaths::TShape>& pNewShape)	{	m_pShape = pNewShape;	}	//	set new shape

	FORCEINLINE Bool				IsSensor() const							{	return m_IsSensor;	}
	FORCEINLINE void				SetIsSensor(Bool IsSensor)					{	m_IsSensor = IsSensor;	}

	FORCEINLINE b2Body*				GetBody()									{	return m_pBodyShape ? m_pBodyShape->GetBody() : NULL;	}
	FORCEINLINE b2Fixture*			GetBodyShape()								{	return m_pBodyShape;	}
	FORCEINLINE void				SetBodyShape(b2Fixture* pBodyShape)			{	m_pBodyShape = pBodyShape;	}
	Bool							UpdateBodyShape();							//	update the bodyshape to match our current shape - fails if it must be recreated, or doesnt exist etc
	Bool							DestroyBodyShape(b2Body* pBody);			//	delete body shape from body - returns if any changes made

	FORCEINLINE Bool				operator==(const TCollisionShape& Shape) const	{	return Shape.GetShapeRef() == GetShapeRef();	}
	FORCEINLINE Bool				operator==(TRefRef ShapeRef) const				{	return ShapeRef == GetShapeRef();	}

protected:
	b2Fixture*				m_pBodyShape;	//	shape created on body
	TRef					m_ShapeRef;
	TPtr<TLMaths::TShape>	m_pShape;
	Bool					m_IsSensor;		//	is a sensor shape - not a collision shape
};


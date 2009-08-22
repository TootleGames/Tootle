#pragma once

#include <box2d/include/box2d.h>

FORCEINLINE b2Body*	TLPhysics::TCollisionShape::GetBody()
{	
	return m_pBodyShape ? m_pBodyShape->GetBody() : NULL;	
}

FORCEINLINE b2Fixture* TLPhysics::TCollisionShape::GetBodyShape()								
{	
	return m_pBodyShape;	
}

FORCEINLINE void TLPhysics::TCollisionShape::SetBodyShape(b2Fixture* pBodyShape)
{
	m_pBodyShape = pBodyShape;	
}

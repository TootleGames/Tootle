
#pragma once

#include <box2d/include/box2d.h>

namespace TLPhysics
{
	class TPhysicsNode;

	FORCEINLINE TPhysicsNode*	GetPhysicsNodeFromBody(b2Body* pBody)			{	return pBody ? (TLPhysics::TPhysicsNode*)pBody->GetUserData() : NULL;	}	//	in case the user-data usage of the body changes, use this to access a physics node from a body
	FORCEINLINE TPhysicsNode*	GetPhysicsNodeFromShape(b2Fixture* pShape)		{	return pShape ? GetPhysicsNodeFromBody( pShape->GetBody() ) : NULL;	}
	FORCEINLINE void*			GetBodyUserDataFromPhysicsNode(TPhysicsNode* pNode)	{	return pNode ? (void*)pNode : NULL;	}

	FORCEINLINE TRef			GetShapeRefFromShape(b2Fixture* pShape)			{	return pShape ? TRef( TLCore::PointerToInteger( pShape->GetUserData() ) ) : TRef_Invalid;	}
	FORCEINLINE void*			GetShapeUserDataFromShapeRef(TRefRef ShapeRef)	{	return TLCore::IntegerToPointer( ShapeRef.GetData() );	}

}
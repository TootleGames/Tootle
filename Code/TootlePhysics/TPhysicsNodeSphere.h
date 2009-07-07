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

};



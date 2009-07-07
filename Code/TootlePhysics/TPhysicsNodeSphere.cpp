#include "TPhysicsNodeSphere.h"
#include <TootleMaths/TShapeSphere.h>



namespace TLPhysics
{
	extern float3		g_WorldUp;
}
	

#define MAX_ROLL		1.f		//	cap roll on quaternion


TLPhysics::TPhysicsNodeSphere::TPhysicsNodeSphere(TRefRef NodeRef,TRefRef TypeRef) :
	TPhysicsNode		( NodeRef, TypeRef )
	
{
}


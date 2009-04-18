/*------------------------------------------------------

	collision object which is a wrapper for shapes

-------------------------------------------------------*/
#pragma once

#include <TootleCore/TLTypes.h>
#include <TootleCore/TRef.h>
#include <TootleCore/TLMaths.h>



namespace TLPhysics
{
	//	non standard "shape" type refs
	#define TLMaths_ShapeRef_Mesh				TRef_Static3(M,s,h)
	#define TLMaths_ShapeRef_MeshWithBounds		TRef_Static(M,s,h,W,B)
	FORCEINLINE TRef		GetMeshShapeTypeRef()			{	return TLMaths_ShapeRef(Mesh);	}
	FORCEINLINE TRef		GetMeshWithBoundsShapeTypeRef()	{	return TLMaths_ShapeRef(MeshWithBounds);	}
}


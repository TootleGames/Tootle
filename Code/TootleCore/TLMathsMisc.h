/*
 *  TLMathsMisc.h
 *  TootleCore
 *
 *  Created by Duane Bradbury on 27/10/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */

#include "TRef.h"


namespace TLMaths
{
	//	gr: these macros are just here to keep the compiled ref usage, they're not NEEDED but saves us using TRef_Static(B,o,x,2) etc everywhere
	#define TLMaths_ShapeRef_TBox			TRef_Static3(B,o,x)
	#define TLMaths_ShapeRef_TBox2D			TRef_Static4(B,o,x,TWO)
	#define TLMaths_ShapeRef_TCapsule		TRef_Static3(C,a,p)
	#define TLMaths_ShapeRef_TCapsule2D		TRef_Static4(C,a,p,TWO)
	#define TLMaths_ShapeRef_TLine			TRef_Static4(L,i,n,e)
	#define TLMaths_ShapeRef_TLine2D		TRef_Static(L,i,n,e,TWO)
	#define TLMaths_ShapeRef_TOblong		TRef_Static3(O,b,l)
	#define TLMaths_ShapeRef_TOblong2D		TRef_Static4(O,b,l,TWO)
	#define TLMaths_ShapeRef_TPolygon		TRef_Static4(P,o,l,y)
	#define TLMaths_ShapeRef_TPolygon2D		TRef_Static5(P,o,l,y,TWO)
	#define TLMaths_ShapeRef_TSphere		TRef_Static3(S,p,h)
	#define TLMaths_ShapeRef_TSphere2D		TRef_Static4(S,p,h,TWO)
	#define TLMaths_ShapeRef_Polygon		TRef_Static4(P,o,l,y)
	#define TLMaths_ShapeRef_Polygon2D		TRef_Static5(P,o,l,y,TWO)
		
	#define TLMaths_ShapeRef(ShapeType)		TLMaths_ShapeRef_##ShapeType
	

}
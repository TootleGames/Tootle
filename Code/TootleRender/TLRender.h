/*------------------------------------------------------

	Render lib interface

-------------------------------------------------------*/
#pragma once

#include <TootleCore/TLTypes.h>
#include <TootleCore/TColour.h>

#include "TRendergraph.h"

namespace TLRender
{
	//	drawing results - they go from worst to best. so if your NewDrawResult > OldDrawResult then NewDrawResult is better
	//	gr: may change this to flags or something with more detail to catch specific errors (missing specific assets etc)
	enum DrawResult
	{
		Draw_Error	= 0,	//	error with render object
		Draw_Empty,			//	nothing in the render object to draw
		Draw_ZeroAlpha,		//	object was completely invisible due to colour
		Draw_Culled,		//	render object was entirely culled
		Draw_Clipped,		//	render object was drawn but partly clipped
		Draw_Missing,		//	render object was drawn but with missing asset[s]
		Draw_Okay,			//	render object drawn successfully
	};
	inline Bool		GetDrawResultSuccess(DrawResult Result)		{	return Result != Draw_Error;	}	//	quick func to see if we should continue rendering this object following this result
	inline void		GetMergedDrawResult(DrawResult& Result,const DrawResult& NewDrawResult)	{	Result = (NewDrawResult > Result) ? NewDrawResult : Result;	}

	//	forward declaration of platform specific implementations
	namespace Platform
	{
		SyncBool	Init();
		SyncBool	Update();
		SyncBool	Shutdown();
	}

	extern const TColour	g_DebugMissingColoursColour;
	extern u32				g_PolyCount;
	extern u32				g_VertexCount;
}

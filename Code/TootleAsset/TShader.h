/*------------------------------------------------------

	Shader asset interface. This format may change a little
	when we start loading programs, but the idea is that the base
	TShader asset is a little dumb and contains shader-specific 
	information (eg. platform/rasteriser support/language etc)

	Software and fixed-function shaders will overload this and 
	implement the runtime functions as required (pre/post render stuff)
	and be created by the rasteriser (for fixed function shaders)
	and whereever neccessary for the software shaders.

	Presumably (and this is where this may change slightly because of the interface)
	the rasteriser will use the base type to pull out the program 
	instructions (cg/hlsl/arb etc) and load the program when required at runtime.

	Therefore this Shader asset may or may not contain a program, but will be
	used in a "read-only" style to do whatever it needs to do.

	I want to see the shader assets as the interfaces to the shader functionality 
	(ie. shader ASSETS will be bound to render nodes -whether software, fixed function,
	or program skinning-) but the implementation may have to be moved to the rasteriser.
	In which case the hardcoded shadesr (ff and sw) may just have identifiers or 
	something in the shader asset.

-------------------------------------------------------*/
#pragma once

#include "TLAsset.h"
#include "TAsset.h"
#include <TootleCore/TTransform.h>

namespace TLAsset
{
	class TShader;
};

namespace TLRender
{
	class TRenderNode;
}


class TLAsset::TShader : public TLAsset::TAsset
{
public:
	TShader(TRefRef AssetRef) :	TAsset( GetAssetType_Static(), AssetRef )	{	}

	static TRef						GetAssetType_Static()			{	return TRef_Static(S,h,a,d,e);	}

	//	runtime implementations - these will only be accessed by the rasteriser
	//	gr: note: these are non-const for the overloaded hard-coded shaders
	virtual const TArray<float3>&	GetVertexes(const TArray<float3>& Vertexes)	{	return Vertexes;	}
	virtual Bool					PreRender(TBinaryTree& ShaderData)			{	return true;	}
	virtual void					PostRender()								{	}
};


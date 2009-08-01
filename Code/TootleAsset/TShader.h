/*------------------------------------------------------

	shader asset, two kinds, hardware shader (which is
	just an ARB program in opengl) and a software shader
	which is generated in code and derives from a software
	shader type which is executed when our object is rendered

-------------------------------------------------------*/
#pragma once

#include "TLAsset.h"
#include "TAsset.h"


namespace TLAsset
{
	class TShader;
};



class TLAsset::TShader : public TLAsset::TAsset
{
public:
	TShader(const TRef& AssetRef) :	TAsset( GetAssetType_Static(), AssetRef )	{	}

	static TRef						GetAssetType_Static()			{	return TRef_Static(S,h,a,d,e);	}
};


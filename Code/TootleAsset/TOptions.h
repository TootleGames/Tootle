

#pragma once

#include "TAsset.h"

namespace TLAsset
{
	class TOptions;
}

//---------------------------------------------------------
// Options is a collection of data so a normal asset but with a specific type
//---------------------------------------------------------
class TLAsset::TOptions : public TLAsset::TAsset
{
public:
	TOptions(TRefRef AssetRef) : 
		TLAsset::TAsset( GetAssetType_Static(), AssetRef )
	{
	}

	static TRef						GetAssetType_Static()				{	return TRef_Static(O,p,t,i,o);	}
};

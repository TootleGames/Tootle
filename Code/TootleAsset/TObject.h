

#pragma once

#include "TScheme.h"

namespace TLAsset
{
	class TObject;
}

//---------------------------------------------------------
//	scheme is a collection of node(tree)'s to be inserted in various
//	graphs
//---------------------------------------------------------
class TLAsset::TObject : public TLAsset::TScheme
{
public:
	TObject(TRefRef AssetRef) : 
		TLAsset::TScheme( AssetRef, GetAssetType_Static() )
	{
	}

	static TRef						GetAssetType_Static()				{	return TRef_Static(O,b,j,e,c);	}
};

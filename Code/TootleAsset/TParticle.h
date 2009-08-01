/*------------------------------------------------------

	Particle asset

-------------------------------------------------------*/
#pragma once

#include "TLAsset.h"
#include "TAsset.h"
#include <TootleCore/TBinaryTree.h>


namespace TLAsset
{
	class TParticle;
};



//---------------------------------------------------------
//	
//---------------------------------------------------------
class TLAsset::TParticle : public TLAsset::TAsset
{
public:
	TParticle(TRefRef AssetRef);

	static TRef					GetAssetType_Static()			{	return TRef_Static(P,a,r,t,i);	}

protected:
	virtual SyncBool			ImportData(TBinaryTree& Data);	//	load asset data out binary data
	virtual SyncBool			ExportData(TBinaryTree& Data);	//	save asset data to binary data
};





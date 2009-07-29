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

protected:
	virtual SyncBool			ImportData(TBinaryTree& Data);	//	load asset data out binary data
	virtual SyncBool			ExportData(TBinaryTree& Data);	//	save asset data to binary data
};





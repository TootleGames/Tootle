#include "TParticle.h"






TLAsset::TParticle::TParticle(TRefRef AssetRef) :
	TLAsset::TAsset( GetAssetType_Static(), AssetRef )
{
}


//----------------------------------------------------
//	save asset data to binary data
//----------------------------------------------------
SyncBool TLAsset::TParticle::ExportData(TBinaryTree& Data)
{
	//	write back any data we didn't recognise
	ExportUnknownData( Data );

	return SyncTrue;
}


//----------------------------------------------------
//	load asset data out binary data
//----------------------------------------------------
SyncBool TLAsset::TParticle::ImportData(TBinaryTree& Data)
{
	//	store off any data we haven't read to keep this data intact
	ImportUnknownData( Data );

	return SyncTrue;
}

#include "TLAudio.h"
#include <TootleAsset/TLAsset.h>


//------------------------------------------------
//	wrapper which loads the asset then does create buffer
//------------------------------------------------
Bool TLAudio::Platform::CreateBuffer(TRefRef AudioAssetRef)
{
	TLAsset::TAudio* pAudioAsset = TLAsset::GetAsset<TLAsset::TAudio>( AudioAssetRef );
	if ( !pAudioAsset )
		return FALSE;

	return CreateBuffer( *pAudioAsset );
}

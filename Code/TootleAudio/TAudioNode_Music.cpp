
#include "TAudioNode_Music.h"
#include "TAudiograph.h"

using namespace TLAudio;

TAudioNode_Music::TAudioNode_Music(TRefRef NodeRef,TRefRef TypeRef) :
	TAudioNode( NodeRef, TypeRef )
{
}


float TAudioNode_Music::GetGlobalVolume()
{ 
	return TLAudio::g_pAudiograph->GetMusicVolume(); 
}

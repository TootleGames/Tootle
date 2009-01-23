#include "TAudioInterface.h"

#include "TLAudio.h"


using namespace TLAudio;

Bool TAudioInterface::StartAudio(TRefRef AudioRef, TRefRef AudioAsset)
{
	// Create an audio node for the specified audio reference
	TPtr<TLAudio::TAudioNode> pAudioNode = TLAudio::g_pAudiograph->DoCreateNode(AudioRef, "Audio");

	if(pAudioNode)
	{
		pAudioNode->SetAudioAssetRef(AudioAsset);
		
		pAudioNode->Play();
		return TRUE;
	}

	return FALSE;
}

Bool TAudioInterface::StopAudio(TRefRef AudioRef)
{
	// Find the audio node
	TPtr<TLAudio::TAudioNode> pAudioNode = TLAudio::g_pAudiograph->FindNode(AudioRef);

	if(pAudioNode)
	{
		pAudioNode->Stop();
	}

	return FALSE;
}

Bool TAudioInterface::PauseAudio(TRefRef AudioRef, Bool bPause)
{
	// Find the audio node
	TPtr<TLAudio::TAudioNode> pAudioNode = TLAudio::g_pAudiograph->FindNode(AudioRef);

	if(pAudioNode)
	{
		pAudioNode->Pause();
		return TRUE;
	}

	return FALSE;
}

Bool TAudioInterface::IsAudioPlaying(TRefRef AudioRef)
{
	// Find the audio node
	TPtr<TLAudio::TAudioNode> pAudioNode = TLAudio::g_pAudiograph->FindNode(AudioRef);

	return pAudioNode.IsValid();
}

void TAudioInterface::SetAudioPitch(TRefRef AudioRef, float fPitch)
{
	// Find the audio node
	TPtr<TLAudio::TAudioNode> pAudioNode = TLAudio::g_pAudiograph->FindNode(AudioRef);

	if(pAudioNode)
		pAudioNode->SetPitch(fPitch);
}


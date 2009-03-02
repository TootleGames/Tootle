#include "TAudioInterface.h"

#include "TLAudio.h"


using namespace TLAudio;

Bool TAudioInterface::StartAudio(TRefRef AudioRef, TRefRef AudioAsset)
{
	// Create an audio node for the specified audio reference
	TPtr<TLAudio::TAudioNode> pAudioNode = TLAudio::g_pAudiograph->DoCreateNode(AudioRef, "Audio");

	if(pAudioNode)
	{
		if(pAudioNode->SetAudioAssetRef(AudioAsset))
		{
			if(pAudioNode->Play())
				return TRUE;
		}
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

Bool TAudioInterface::PauseAudio(TRefRef AudioRef, const Bool& bPause)
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



void TAudioInterface::SetAudioTranslate(TRefRef AudioRef, const float3& vTranslate)
{
	// Find the audio node
	TPtr<TLAudio::TAudioNode> pAudioNode = TLAudio::g_pAudiograph->FindNode(AudioRef);
	
	if(pAudioNode)
		pAudioNode->SetTranslate(vTranslate);
}

float3 TAudioInterface::GetAudioTranslate(TRefRef AudioRef)
{
	// Find the audio node
	TPtr<TLAudio::TAudioNode> pAudioNode = TLAudio::g_pAudiograph->FindNode(AudioRef);
	
	if(pAudioNode)
		return pAudioNode->GetTranslate();

	return float3(0.0f, 0.0f, 0.0f);
}


void TAudioInterface::SetAudioPitch(TRefRef AudioRef, const float& fPitch)
{
	// Find the audio node
	TPtr<TLAudio::TAudioNode> pAudioNode = TLAudio::g_pAudiograph->FindNode(AudioRef);

	if(pAudioNode)
		pAudioNode->SetPitch(fPitch);
}


float TAudioInterface::GetAudioPitch(TRefRef AudioRef)
{
	// Find the audio node
	TPtr<TLAudio::TAudioNode> pAudioNode = TLAudio::g_pAudiograph->FindNode(AudioRef);
	
	if(pAudioNode)
		return pAudioNode->GetPitch();
	
	// Failed?
	return -1.0f;
}



void TAudioInterface::SetAudioVolume(TRefRef AudioRef, const float& fVolume)
{
	// Find the audio node
	TPtr<TLAudio::TAudioNode> pAudioNode = TLAudio::g_pAudiograph->FindNode(AudioRef);
	
	if(pAudioNode)
		pAudioNode->SetVolume(fVolume);
}

float TAudioInterface::GetAudioVolume(TRefRef AudioRef)
{
	// Find the audio node
	TPtr<TLAudio::TAudioNode> pAudioNode = TLAudio::g_pAudiograph->FindNode(AudioRef);
	
	if(pAudioNode)
		return pAudioNode->GetVolume();

	return -1.0f;
}


void TAudioInterface::SetAudioLooping(TRefRef AudioRef, const Bool& bLooping)
{
	// Find the audio node
	TPtr<TLAudio::TAudioNode> pAudioNode = TLAudio::g_pAudiograph->FindNode(AudioRef);
	
	if(pAudioNode)
		pAudioNode->SetLooping(bLooping);
}


Bool TAudioInterface::GetAudioIsLooping(TRefRef AudioRef)
{
	// Find the audio node
	TPtr<TLAudio::TAudioNode> pAudioNode = TLAudio::g_pAudiograph->FindNode(AudioRef);
	
	if(pAudioNode)
		return pAudioNode->GetIsLooping();
	
	return FALSE;
}





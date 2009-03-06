
#pragma once

#include <TootleCore/TRef.h>
#include "TAudioNode.h"

namespace TLAudio
{
	class TAudioInterface;
}

/*
// Not sure it's worth making the audio interface use policies.  The only reason I can think of would be for if 
// you wanted to specialise some nodes to track the audio in some way but for now I am going to leave it
class TLAudio::AudioPolicyNone
{
	Bool	StartAudio(TRefRef AudioRef, TRefRef AudioAsset) 
	{
		return FALSE;
	}

	Bool	StopAudio(TRefRef AudioRef)
	{
		return FALSE;
	}

	Bool	PauseAudio(TRefRef AudioRef, Bool bPause)
	{
		return FALSE;
	}

}

class TLAudio::AudioPolicyOneShot
{
	Bool	StartAudio(TRefRef AudioRef, TRefRef AudioAsset);
	Bool	StopAudio(TRefRef AudioRef);

	Bool	PauseAudio(TRefRef AudioRef, Bool bPause);
}

class TLAudio::AudioPolicyTracking
{
	Bool	StartAudio(TRefRef AudioRef, TRefRef AudioAsset);
	Bool	StopAudio(TRefRef AudioRef);

	Bool	PauseAudio(TRefRef AudioRef, Bool bPause);

private:
	void	AddSound(TRefRef AudioRef)		{ m_SoundList.Add(AudioRef); }
	void	RemoveSound(TRefRef AudioRef)	{ m_SoundList.Remove(AudioRef); }

private:
	TArray<TRef>	m_SoundList;
}
*/

//template <class AudioPolicy>
//class TLAudio::TAudioInterface : public AudioPolicy

/*

// [04/02/09] DB - Removed as it's not flexible enough for sending messages between nodes

class TLAudio::TAudioInterface
{
public:

	// General Audio requests
	Bool	StartAudio(TRefRef AudioRef, TRefRef AudioAsset);
	Bool	StartAudio(TRefRef AudioRef, TRefRef AudioAsset, const TAudioProperties& Props);

	Bool	StopAudio(TRefRef AudioRef);

	Bool	PauseAudio(TRefRef AudioRef, const Bool& bPause);

	Bool	IsAudioPlaying(TRefRef AudioRef);

	// Audio transform access
	void	SetAudioTranslate(TRefRef AudioRef, const float3& vTranslate);
	float3	GetAudioTranslate(TRefRef AudioRef);

	// Audio properties
	void	SetAudioLooping(TRefRef AudioRef, const Bool& bLooping);
	Bool	GetAudioIsLooping(TRefRef AudioRef);
	
	void	SetAudioFrequency(TRefRef AudioRef, const float& fFrequency);
	float	GetFrequency(TRefRef AudioRef);

	void					SetAudioProperties(TRefRef AudioRef, const TAudioProperties& Props);
	const TAudioProperties&	GetAudioProperties(TRefRef AudioRef);
};

*/
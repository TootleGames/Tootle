
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

class TLAudio::TAudioInterface
{
public:

	// General Audio requests
	Bool	StartAudio(TRefRef AudioRef, TRefRef AudioAsset);
	Bool	StopAudio(TRefRef AudioRef);

	Bool	PauseAudio(TRefRef AudioRef, Bool bPause);

	Bool	IsAudioPlaying(TRefRef AudioRef);

	// Audio properties

	void	SetAudioPitch(TRefRef AudioRef, float fPitch);

	void	SetAudioVolume(TRefRef AudioRef, float fVolume);
	float	GetAudioVolume(TRefRef AudioRef);

	void	SetAudioFrequency(TRefRef AudioRef, float fFrequency);
	float	GetFrequency(TRefRef AudioRef);

	void					SetAudioProperties(TRefRef AudioRef, const TAudioProperties& Props);
	const TAudioProperties&	GetAudioProperties(TRefRef AudioRef);
};

#pragma once

namespace TLAudio
{
	class TAudioInterface;
}


class TLAudio::TAudioInterface
{
public:

	// General Audio requests
	Bool	StartAudio(TRef& refAudioID);
	Bool	StopAudio(TRef& refAudioID);

	// Audio properties
	void	SetAudioVolume(TRef& refAudioID, float fVolume);
	float	GetAudioVolume(TRef& refAudioID);

	void	SetAudioFrequency(TRef& refAudioID, float fFrequency);
	float	GetFrequency(TRef& refAudioID);

	void				SetAudioProperties(TRef& refAudioID, TAudioProperties& Props);
	TAudioProperties&	GetAudioProperties(TRef& refAudioID);

private:
	void	AddSound(TRef& refAudioID)		{ m_SoundList.Add(refAudioID); }
	void	RemoveSound(TRef& refAudioID)	{ m_SoundList.Remove(refAudioID); }

private:
	TArray<TRef>	m_AudioList;
}
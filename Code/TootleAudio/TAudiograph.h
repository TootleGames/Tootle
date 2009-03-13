
#pragma once

#include <TootleCore/TLGraph.h>
#include <TootleInput/TUser.h>
#include "TAudioNode.h"
#include "TAudioNode_Music.h"

namespace TLAudio
{
	class TListenerProperties;

	class TAudiograph;
	class TAudioNodeFactory;

	extern TPtr<TAudiograph> g_pAudiograph;
};


// Data for the audio system listener
class TLAudio::TListenerProperties
{
public:
	float3		m_vPosition;
	float3		m_vVelocity;
	float3		m_vLookAt;
	float3		m_vUp;
};


/*
	TAudiograph class
*/
class TLAudio::TAudiograph : public TLGraph::TGraph<TLAudio::TAudioNode>
{
public:
	TAudiograph(TRefRef refManagerID);

	////////////////////////////////////////////////////////
	// Audio system access
	////////////////////////////////////////////////////////

	// Music access
	Bool					StartMusic(TRefRef AudioAsset);

	// Volume access
	FORCEINLINE float		GetMusicVolume()	const	{ return m_fMusicVolume; }
	FORCEINLINE float		GetEffectsVolume()	const	{ return m_fEffectsVolume; }

	FORCEINLINE Bool		IsPaused()			const	{ return m_bPause; }
	FORCEINLINE Bool		IsEnabled()			const	{ return m_bEnabled; }
	FORCEINLINE Bool		IsMuted()			const	{ return m_bMute; }
	////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////
	// Audio object access
	////////////////////////////////////////////////////////
	TRef	StartAudio(TRefRef AudioRef, TRefRef AudioAddet);
	TRef	StartAudio(TRefRef AudioRef, TLMessaging::TMessage& Message);	

	Bool	StopAudio(TRefRef AudioRef);
	Bool	PauseAudio(TRefRef AudioRef, const Bool& bPause);

	FORCEINLINE Bool		IsAudioPlaying(TRefRef AudioRef)	{ return IsInGraph(AudioRef);}

	// Audio transform access
	void	SetAudioTranslate(TRefRef AudioRef, const float3& vTranslate);
	float3	GetAudioTranslate(TRefRef AudioRef);

	// Audio Properties
	void	SetAudioPitch(TRefRef AudioRef, const float& fPitch);
	float	GetAudioPitch(TRefRef AudioRef);

	void	SetAudioVolume(TRefRef AudioRef, const float& fVolume);
	float	GetAudioVolume(TRefRef AudioRef);

	void	SetAudioLooping(TRefRef AudioRef, const Bool& bLooping);
	Bool	GetAudioIsLooping(TRefRef AudioRef);

	// Audio offset access
	void	SetAudioOffsetNormalised(TRefRef AudioRef, float& fOffset);
	float	GetAudioOffsetNormalised(TRefRef AudioRef);

	void	SetAudioOffsetSeconds(TRefRef AudioRef, const float& fSeconds);
	float	GetAudioOffsetSeconds(TRefRef AudioRef);

	float	GetAudioLengthSeconds(TRefRef AudioRef);

	void					SetAudioProperties(TRefRef AudioRef, const TAudioProperties& Props);
	const TAudioProperties&	GetAudioProperties(TRefRef AudioRef);

	////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////
	// Audio listener
	////////////////////////////////////////////////////////

	void			SetListener(const TListenerProperties& Props);

	////////////////////////////////////////////////////////

protected:
	virtual SyncBool		Initialise();
	virtual SyncBool		Update(float fTimeStep);
	virtual SyncBool		Shutdown();

	void					Enable();
	void					Disable();

	virtual void			ProcessMessage(TLMessaging::TMessage& Message);
	virtual void			OnEventChannelAdded(TRefRef refPublisherID, TRefRef refChannelID);	

	SyncBool				InitDevices();

	void					OnMusicVolumeChanged();
	void					OnEffectsVolumeChanged();
	void					OnPauseStateChanged();

	TPtr<TAudioNode_Music>	GetMusicNode()	{ return FindNode(m_MusicRef); }

	void					OnInputDeviceAdded(TRefRef DeviceRef,TRefRef DeviceTypeRef);
	void					MapActions_Keyboard(TRefRef DeviceRef, TPtr<TLUser::TUser> pUser);

	FORCEINLINE	void		ToggleMute()	{ m_bMute = !m_bMute; OnMuteChanged(); }
	void					OnMuteChanged();


private:
	TRef					m_MusicRef;			// Music node reference

	float					m_fMusicVolume;		// Audio system master music volume
	float					m_fEffectsVolume;	// Audio system master effects volume

	TListenerProperties		m_Listener;			// The audio listener aka microphone

	Bool					m_bPause;			// Audio system pause - system will pause all audio
	Bool					m_bEnabled;			// Audio system enabled - system completely stopped from generating audio
	Bool					m_bMute;			// Audio system muted - all audio not audible
};


//----------------------------------------------------------
//	Generic audio node factory
//----------------------------------------------------------
class TLAudio::TAudioNodeFactory : public TClassFactory<TAudioNode,FALSE>
{
public:
	virtual TAudioNode*	CreateObject(TRefRef InstanceRef,TRefRef TypeRef);
};

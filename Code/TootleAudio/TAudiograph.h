
#pragma once

#include <TootleCore/TLGraph.h>
#include <TootleInput/TUser.h>
#include "TAudioNode.h"
#include "TAudioNode_Music.h"
#include "TAudioListener.h"
#include "TAudioDistance.h"

namespace TLAudio
{
	class TAudiograph;
	class TAudioNodeFactory;

	extern TPtr<TAudiograph> g_pAudiograph;
};




/*
	TAudiograph class
*/
class TLAudio::TAudiograph : public TLGraph::TGraph<TLAudio::TAudioNode>
{
public:
	TAudiograph();

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
	FORCEINLINE Bool		IsActive()			const	{ return m_bActive; }
	FORCEINLINE Bool		IsMuted()			const	{ return m_bMute; }
	
	FORCEINLINE DistanceModel		GetDistanceModel()	const	{ return m_DistanceModel; }
	FORCEINLINE float			GetDopplerFactor()		const	{ return m_fDopplerFactor; }
	FORCEINLINE float			GetDopplerVelocity()	const	{ return m_fDopplerVelocity; }

	////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////
	// Audio object access
	////////////////////////////////////////////////////////
	TRef	StartAudio(TRefRef AudioRef, TRefRef AssetRef);
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
	
	void					Activate();
	void					Deactivate();

	virtual void			ProcessMessage(TLMessaging::TMessage& Message);
	virtual void			OnEventChannelAdded(TRefRef refPublisherID, TRefRef refChannelID);	

	SyncBool				InitDevices();

	TAudioNode_Music*		GetMusicNode()	{ return static_cast<TAudioNode_Music*>( FindNode(m_MusicRef) ); }

	void					OnInputDeviceAdded(TRefRef DeviceRef,TRefRef DeviceTypeRef);
	void					MapActions_Keyboard(TRefRef DeviceRef, TPtr<TLUser::TUser> pUser);

	FORCEINLINE	void		ToggleMute()	{ m_bMute = !m_bMute; OnMuteChanged(); }
	
	FORCEINLINE void		SetMute(Bool bMute);			
	FORCEINLINE void		SetPause(Bool bPause);
	FORCEINLINE void		SetEnabled(Bool bEnabled);
	FORCEINLINE void		SetActive(Bool bActive);
	FORCEINLINE void		SetEffectsVolume(float fVolume);
	FORCEINLINE void		SetMusicVolume(float fVolume);
	
	void					SetDistanceModel(TLAudio::DistanceModel model);

	void					SetDopplerFactor(float fFactor);
	void					SetDopplerVelocity(float fVelocity);


	virtual void			SetProperty(TLMessaging::TMessage& Message);

private:
	void					OnMusicVolumeChanged();
	void					OnEffectsVolumeChanged();
	void					OnPauseStateChanged();
	void					OnMuteChanged();
	void					OnEnableChanged();

	void					OnDistanceModelChanged();
	void					OnDopplerEffectChanged();


private:
	TRef					m_MusicRef;			// Music node reference

	float					m_fMusicVolume;		// Audio system master music volume
	float					m_fEffectsVolume;	// Audio system master effects volume
	
	TListenerProperties		m_Listener;			// The audio listener aka microphone
	DistanceModel			m_DistanceModel;	// The audio system distance model
	float					m_fDopplerFactor;	// Doppler effect
	float					m_fDopplerVelocity;	// Doppler effect

	Bool					m_bPause;			// Audio system pause - system will pause all audio
	Bool					m_bEnabled;			// Audio system enabled - low level system is initialised for use
	Bool					m_bActive;			// Audio system active - sounds can be generated 
	Bool					m_bMute;			// Audio system muted - all audio not audible
};

TLGraph_DeclareGraph(TLAudio::TAudioNode);

FORCEINLINE void TLAudio::TAudiograph::SetMute(Bool bMute)			
{ 
	if(m_bMute != bMute) 
	{ 
		m_bMute = bMute; 
		OnMuteChanged();
	} 
}

FORCEINLINE void TLAudio::TAudiograph::SetPause(Bool bPause)
{
	if(m_bPause != bPause)
	{
		m_bPause = bPause; 
		OnPauseStateChanged(); 
	}
}


FORCEINLINE void TLAudio::TAudiograph::SetEnabled(Bool bEnable)
{
	if(m_bEnabled != bEnable)
	{
		if(bEnable)
			Enable();
		else 
			Disable();
	}
}


FORCEINLINE void TLAudio::TAudiograph::SetActive(Bool bActive)
{
	if(m_bActive != bActive)
	{
		if(bActive)
			Activate();
		else 
			Deactivate();
	}
}

FORCEINLINE void TLAudio::TAudiograph::SetEffectsVolume(float fVolume)
{
	if(m_fEffectsVolume != fVolume)
	{
		m_fEffectsVolume = fVolume;
		OnEffectsVolumeChanged();
	}
}

FORCEINLINE void TLAudio::TAudiograph::SetMusicVolume(float fVolume)
{
	if(m_fMusicVolume != fVolume)
	{
		m_fMusicVolume = fVolume;
		OnMusicVolumeChanged();
	}
}






//----------------------------------------------------------
//	Generic audio node factory
//----------------------------------------------------------
class TLAudio::TAudioNodeFactory : public TNodeFactory<TAudioNode>
{
public:
	virtual TAudioNode*	CreateObject(TRefRef InstanceRef,TRefRef TypeRef);
};

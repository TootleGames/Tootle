
#pragma once

#include <TootleCore/TLGraph.h>
#include "TAudioNode.h"
#include "TAudioNode_Music.h"

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
	////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////
	// Audio object access
	////////////////////////////////////////////////////////

	FORCEINLINE Bool		IsAudioPlaying(TRefRef AudioRef)	{ return IsInGraph(AudioRef);}

	// Audio Properties
	void	SetAudioPitch(TRefRef AudioRef, const float& fPitch);
	float	GetAudioPitch(TRefRef AudioRef);

	void	SetAudioVolume(TRefRef AudioRef, const float& fVolume);
	float	GetAudioVolume(TRefRef AudioRef);

	////////////////////////////////////////////////////////

protected:
	virtual SyncBool		Initialise();
	virtual SyncBool		Update(float fTimeStep);
	virtual SyncBool		Shutdown();

	virtual void			ProcessMessage(TLMessaging::TMessage& Message);

	SyncBool				InitDevices();

	void					OnMusicVolumeChanged();
	void					OnEffectsVolumeChanged();
	void					OnPauseStateChanged();

	TPtr<TAudioNode_Music>	GetMusicNode()	{ return FindNode(m_MusicRef); }

private:
	TRef					m_MusicRef;			// Music node reference

	float					m_fMusicVolume;		// Audio system master music volume
	float					m_fEffectsVolume;	// Audio system master effects volume

	Bool					m_bPause;			// Audio system pause
};


//----------------------------------------------------------
//	Generic audio node factory
//----------------------------------------------------------
class TLAudio::TAudioNodeFactory : public TClassFactory<TAudioNode,FALSE>
{
public:
	virtual TAudioNode*	CreateObject(TRefRef InstanceRef,TRefRef TypeRef);
};

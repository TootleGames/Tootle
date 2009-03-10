#include "TAudioNode.h"

#include "TLAudio.h"

#include <TootleCore/TLMaths.h>

#include <TootleScene/TScenegraph.h>


using namespace TLAudio;

TAudioNode::TAudioNode(TRefRef NodeRef,TRefRef TypeRef) :
	TLGraph::TGraphNode<TAudioNode>		( NodeRef, TypeRef )
{
	TLMessaging::g_pEventChannelManager->SubscribeTo(this, "AUDIOGRAPH", "Stop"); 
	TLMessaging::g_pEventChannelManager->SubscribeTo(this, "AUDIOGRAPH", "OnVolumeChanged"); 
}

// Initialise routine
void TAudioNode::Initialise(TLMessaging::TMessage& Message)	
{
	// Process the owner setup first
	TRef	OwnerRef;

	if(Message.ImportData("Owner", OwnerRef))
	{
		// Get the scenegraph node
		TPtr<TLScene::TSceneNode> pOwner = TLScene::g_pScenegraph->FindNode(OwnerRef);

		if(pOwner.IsValid())
		{
			pOwner->SubscribeTo(this);
			SubscribeTo(pOwner);

			/*
			TPtr<TLMessaging::TEventChannel>& pEventChannel = pOwner->FindEventChannel("OnTransform");

			if(pEventChannel)
			{
				// Subscribe tot he scene node owners ontransform channel
				SubscribeTo(pEventChannel);

				// Subscribe the 'scene' node owner to this node so we can sen audio change messages
				pOwner->SubscribeTo(this);
			}
			*/
		}
	}

	// Setup audio properties first
	TAudioProperties Props;

	Bool bHasProps = Message.ImportData("Props", Props);

	// Set Pre-Source creation properties
	if(bHasProps)
	{
		SetStreaming(Props.m_bStreaming);
	}

	TRef AudioAsset;
	if(Message.ImportData("Asset", AudioAsset))
	{
		SetAudioAssetRef(AudioAsset);
	}

	// Set Post-Source creation properties
	if(bHasProps)
	{
		SetFrequencyMult(Props.m_fFrequencyMult);
		SetPitch(Props.m_fPitch);
		SetVolume(Props.m_fVolume);
		SetMinRange(Props.m_fMinRange);
		SetRateOfDecay(Props.m_fRateOfDecay);
		SetLooping(Props.m_bLooping);
	}

	float3 vPosition;
	if(Message.ImportData("Translate", vPosition))
	{
		SetTranslate(vPosition);
	}


	Bool bPlay;
	if(Message.ImportData("Asset", bPlay))
	{
		Play();
	}

	TLGraph::TGraphNode<TAudioNode>::Initialise(Message);
}

// Main node update called once per frame
void TAudioNode::Update(float fTimestep)
{
	// Is the audio node flagged to try and create the source?  If so try again...
	if(m_AudioFlags.IsSet(Release))
	{
		// Set the volume to 0 before removing the node.  This is to try and prevent any pops and clicks
		// that can happen via the OpenAL sysetm when stopping the source
		SetVolume(0.0f);

		// Request shutdown of this node
		TLAudio::g_pAudiograph->RemoveNode(GetNodeRef());

		return;
	}

	TLGraph::TGraphNode<TAudioNode>::Update(fTimestep);
}
// Shutdown routine	- called before being removed form the graph
void TAudioNode::Shutdown()
{
	RemoveSource();
	
	TLGraph::TGraphNode<TAudioNode>::Shutdown();
}

void TAudioNode::ProcessMessage(TLMessaging::TMessage& Message)
{
	if(Message.GetMessageRef() == "Stop")
	{
		if(!m_AudioFlags.IsSet(Release))
		{
			// NOTE: May need to test for a flag of 'auto release' at some stage as we may not want this 
			// all of the time and the event would occur when 'Stopping' an audio object manually 
			// as there is no distinction at a lower level
			//if(m_AudioFlags.IsSet(AutoRelease)
			{
				TRef AudioRef;
				Message.Read(AudioRef);

				if(AudioRef == GetNodeRef())
				{
					// Flag the node to release
					m_AudioFlags.Set(Release, TRUE);
				}
			}
		}
	}
	else if(Message.GetMessageRef() == "OnVolumeChanged")
	{
		// Update the nodes audio volume
		float fVolume;

		if(Message.ImportData("Effects", fVolume))
		{
			// The volume change is passed through via the messaging so simply update
			// the volume for the paltform audio object
			float fFinalVolume = GetVolume() * fVolume;
			TLAudio::Platform::SetVolume(GetNodeRef(), fFinalVolume);
		}

		return;
	}
	else if(Message.GetMessageRef() == "Pause")
	{
		Bool bState;
		if(Message.ImportData("State", bState))
		{
			if(bState)
			{
				// Pause the audio
				TLAudio::Platform::PauseAudio(GetNodeRef());
			}
			else
			{
				// Unpause the audio
				TLAudio::Platform::StartAudio(GetNodeRef());
			}
		}

		return;
	}
	else if(Message.GetMessageRef() == "OnTransform")
	{
		float3 vVector;
		if(Message.ImportData("Translate", vVector))
		{
			UpdatePreviousPos();
			SetTranslate(vVector);

			// Set the low level audio position
			TLAudio::Platform::SetPosition(GetNodeRef(), vVector);

			float3 vVelocity = (vVector - m_vPreviousPos);
			TLAudio::Platform::SetVelocity(GetNodeRef(), vVelocity);
		}

		/*
		if(Message.ImportData("Rotation", vVector))
		{
			UpdatePreviousPos();
			SetTranslate(vVector);

			// Set the low level audio position
			TLAudio::Platform::SetPosition(GetNodeRef(), vVector);

			float3 vVelocity = (vVector - m_vPreviousPos);
			TLAudio::Platform::SetVelocity(GetNodeRef(), vVelocity);
		}
		*/


		return;
	}

	// Only pass on messages that might actually be processed otherwise we will relay the message
	// to subscribers and potentially enter an infinite loop
	if(	(Message.GetMessageRef() == TLCore::InitialiseRef) ||
		(Message.GetMessageRef() == TLCore::ShutdownRef))
	{
		// Pass the message onto the super class
		TLGraph::TGraphNode<TAudioNode>::ProcessMessage(Message);
	}
}


Bool TAudioNode::Play()
{
	if(m_AudioFlags.IsSet(Release)) 
	{
		// Source is flagged to release so ignore request to play
		return FALSE;
	}


	// Play this audio
	if(!TLAudio::Platform::StartAudio(GetNodeRef()))
	{
		// Failed to play - falg the node to release
		//TODO: May need to check for the auto-release flag
		m_AudioFlags.Set(Release, TRUE);
		return FALSE;
	}

	return TRUE;
}

void TAudioNode::Pause()
{
	// Pause this audio
	TLAudio::Platform::PauseAudio(GetNodeRef());
}

void TAudioNode::Stop()
{
	SetVolume(0.0f);

	// Stop this audio
	TLAudio::Platform::StopAudio(GetNodeRef());
}


void TAudioNode::Reset()
{
}

// Set the volume of this instance
void TAudioNode::SetVolume(float fVolume)
{
	// Clamp the volume to within range
	TLMaths::Limit(fVolume, 0.0f, 1.0f);
	
	if(m_AudioProperties.m_fVolume != fVolume)
	{
		// Get the global audio volume (effects or music) and multiply the new volume by the global volume
		float fEffectsVolume = GetGlobalVolume();

		float fFinalVolume = fVolume * fEffectsVolume;

		// Try and set the volume for the source
		// If successful set the volume for the node
		if(TLAudio::Platform::SetVolume(GetNodeRef(), fFinalVolume))
		{
			m_AudioProperties.m_fVolume = fVolume;
		}
	}
}

float TAudioNode::GetGlobalVolume()
{ 
	return TLAudio::g_pAudiograph->GetEffectsVolume(); 
}


// Set the frequency of this instance
void TAudioNode::SetFrequencyMult(float fFrequencyMult)
{
	// Clamp the frequency to within range
	TLMaths::Limit(fFrequencyMult, 0.0f, 10.0f);

	if(m_AudioProperties.m_fFrequencyMult != fFrequencyMult)
	{
		// Try and set the frequency for the source
		// If successful set the frequency for the node
		//if(TLAudio::Platform::SetFrequencyMult(GetNodeRef(), fFrequencyMult))
		{
			m_AudioProperties.m_fFrequencyMult = fFrequencyMult;
		}
	}
}

// Set the frequency of this instance
void TAudioNode::SetPitch(float fPitch)
{
	// Clamp the frequency to within range
	TLMaths::Limit(fPitch, 0.1f, 100.0f);

	if(m_AudioProperties.m_fPitch != fPitch)
	{
		// Try and set the pitch for the source
		// If successful set the pitch for the node
		if(TLAudio::Platform::SetPitch(GetNodeRef(), fPitch))
		{
			m_AudioProperties.m_fPitch = fPitch;
		}
	}
}


// Set the min range of this instance
void TAudioNode::SetMinRange(float fDistance)
{
	// Clamp the min range to within range
	TLMaths::Limit(fDistance, 0.0f, 100.0f);

	if(m_AudioProperties.m_fMinRange != fDistance)
	{
		// Try and set the min range for the source
		// If successful set the min range for the node
		if(TLAudio::Platform::SetMinRange(GetNodeRef(), fDistance))
		{
			m_AudioProperties.m_fMinRange = fDistance;
		}
	}
}


// Set the rate of volume decay of this instance
void TAudioNode::SetRateOfDecay(float fRateOfDecay)
{
	// Clamp the rate of decay to within range
	TLMaths::Limit(fRateOfDecay, 0.01f, 1.0f);

	if(m_AudioProperties.m_fRateOfDecay != fRateOfDecay)
	{
		// Try and set the rate of decay for the source
		// If successful set the rate of decay for the node
		if(TLAudio::Platform::SetRateOfDecay(GetNodeRef(), fRateOfDecay))
		{
			m_AudioProperties.m_fRateOfDecay = fRateOfDecay;
		}
	}
}


// Set this instance to be looping
void TAudioNode::SetLooping(const Bool& bLooping)
{
	if(m_AudioProperties.m_bLooping != bLooping)
	{
		// Try and set the looping state for the source
		// If successful set the looping state for the node
		if(TLAudio::Platform::SetLooping(GetNodeRef(), bLooping))
		{
			m_AudioProperties.m_bLooping = bLooping;
		}
	}
}

// Set this instance to be streamed
void TAudioNode::SetStreaming(const Bool& bStreaming)
{
	m_AudioProperties.m_bStreaming = bStreaming;
}


Bool TAudioNode::SetAudioAssetRef(TRefRef AssetRef)
{	
	if( m_AudioAssetRef == AssetRef )	
		return TRUE;

	// Destroy current audio source info
	if(IsSourceActive())
		RemoveSource();
#ifdef _DEBUG	
	TString str = "Setting AudioAsset: ";
	AssetRef.GetString(str);
	TLDebug_Print(str);
#endif
	m_AudioAssetRef = AssetRef;	
			
	// Create the new audio source info and bind to audio buffer
	return CreateSource();
}

// Generates the source audio data using the audio asset specified
Bool TAudioNode::CreateSource()
{
	if(Platform::CreateSource(GetNodeRef()))
	{
		// Does the buffer exist for the asset we are referencing?
		if(Platform::HasBuffer(m_AudioAssetRef))
		{
			// Bind the source to the buffer
			return Platform::AttachSourceToBuffer(GetNodeRef(), m_AudioAssetRef, GetIsStreaming());
		}
		else
		{
			TLDebug_Print("Audio buffer does not exist for audio node");
			//TODO: May need to check for the auto-release flag
			m_AudioFlags.Set(Release, TRUE);
		}
	}
	else
	{
		TLDebug_Print("Failed to create source for audio node");
		//TODO: May need to check for the auto-release flag
		m_AudioFlags.Set(Release, TRUE);
	}

	return FALSE;
}

// Removes the associated source via the platform specific code
void TAudioNode::RemoveSource()
{
	SetVolume(0.0f);
	Platform::RemoveSource(GetNodeRef());	
}

// Check the low level audio system to see if a source is active with the nodes ID
Bool TAudioNode::IsSourceActive()
{
	return Platform::HasSource(GetNodeRef());
}



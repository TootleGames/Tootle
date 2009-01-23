#include "TAudioNode.h"

#include "TLAudio.h"

#include <TootleCore/TLMaths.h>

using namespace TLAudio;

TAudioNode::TAudioNode(TRefRef NodeRef,TRefRef TypeRef) :
	TLGraph::TGraphNode<TAudioNode>		( NodeRef, TypeRef )
{
	TLMessaging::g_pEventChannelManager->SubscribeTo(this, "AUDIOGRAPH", "STOP"); 

}

// Initialise routine
void TAudioNode::Initialise(TPtr<TLMessaging::TMessage>& pMessage)	
{
	TLGraph::TGraphNode<TAudioNode>::Initialise(pMessage);
}

// Main node update called once per frame
void TAudioNode::Update(float fTimestep)
{
	TLGraph::TGraphNode<TAudioNode>::Update(fTimestep);
}
// Shutdown routine	- called before being removed form the graph
void TAudioNode::Shutdown()
{
	RemoveSource();
	
	TLGraph::TGraphNode<TAudioNode>::Shutdown();
}

void TAudioNode::ProcessMessage(TPtr<TLMessaging::TMessage>& pMessage)
{
	if(pMessage->GetMessageRef() == "AUDIO")
	{
		if(pMessage->HasChannelID("STOP"))
		{
			TRef AudioRef;
			pMessage->Read(AudioRef);

			if(AudioRef == GetNodeRef())
			{
				// Shutdown this node
				// NOTE: May need to test for a flag of 'auto release' at some stage as we may not want this 
				// all of the time and the event would occur when 'Stopping' an audio object manually 
				// as there is no distinction at a lower level
				TLAudio::g_pAudiograph->RemoveNode(GetNodeRef());
			}
		}

		return;
	}

	// Pass the message onto the super class
	TLGraph::TGraphNode<TAudioNode>::ProcessMessage(pMessage);
}


void TAudioNode::Play()
{
	// Play this audio
	TLAudio::Platform::StartAudio(GetNodeRef());
}

void TAudioNode::Pause()
{
	// Pause this audio
	TLAudio::Platform::PauseAudio(GetNodeRef());
}

void TAudioNode::Stop()
{
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
	
	// Try and set the volume for the source
	// If successful set the volume for the node
	
	m_AudioProperties.m_fVolume = fVolume;
}

// Set the frequency of this instance
void TAudioNode::SetFrequencyMult(float fFrequencyMult)
{
	// Clamp the frequency to within range
	TLMaths::Limit(fFrequencyMult, 0.0f, 10.0f);
	
	// Try and set the frequency for the source
	// If successful set the frequency for the node
	
	m_AudioProperties.m_fFrequencyMult = fFrequencyMult;
}

// Set the frequency of this instance
void TAudioNode::SetPitch(float fPitch)
{
	// Clamp the frequency to within range
	TLMaths::Limit(fPitch, 0.0f, 10.0f);

	// Try and set the pitch for the source
	// If successful set the pitch for the node
	if(TLAudio::Platform::SetPitch(GetNodeRef(), fPitch))
	{
		m_AudioProperties.m_fPitch = fPitch;
	}
}



void TAudioNode::SetAudioAssetRef(TRefRef AssetRef)
{	
	if( m_AudioAssetRef != AssetRef )	
	{	
		// Destroy current audio source info
		RemoveSource();
		
		m_AudioAssetRef = AssetRef;	
				
		// Create the new audio source info and bind to audio buffer
		CreateSource();
	}
}
// Generates the source audio data using the audio asset specified
void TAudioNode::CreateSource()
{
	if(Platform::CreateSource(GetNodeRef()))
	{
		// Does the buffer exist for the asset we are referencing?
		if(Platform::HasBuffer(m_AudioAssetRef))
		{
			// Bind the source to the buffer
			Platform::AttachSourceToBuffer(GetNodeRef(), m_AudioAssetRef, FALSE);
		}
		else
		{
			TLDebug_Print("Audio buffer does not exist for audio node");
		}
	}
}

// Removes the associated source via the platform specific code
void TAudioNode::RemoveSource()
{
		Platform::RemoveSource(GetNodeRef());	
}



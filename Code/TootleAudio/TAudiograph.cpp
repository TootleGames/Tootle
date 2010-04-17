#include "TAudiograph.h"
#include "TLAudio.h"
#include <TootleCore/TEventChannel.h>
#include <TootleCore/TLTime.h>
#include <TootleInput/TDevice.h>



namespace TLAudio
{
	TPtr<TAudiograph> g_pAudiograph;
	
	
	namespace Platform
	{
		SyncBool	Init();
		SyncBool	Update();
		SyncBool	Shutdown();
		
		// Low level audio routines		
		Bool		CreateSource(TRefRef SourceRef);
		Bool		RemoveSource(TRefRef SourceRef);
		
		Bool		CreateBuffer(TRefRef AudioAssetRef);		//	wrapper which loads the asset then does create buffer
		Bool		CreateBuffer(TLAsset::TAudio& AudioAsset);
		Bool		RemoveBuffer(TRefRef AudioAssetRef);
		
		Bool		HasSource(TRefRef AudioSourceRef);
		Bool		HasBuffer(TRefRef AudioAssetRef);
		
		Bool		AttachSourceToBuffer(TRefRef AudioSourceRef, TRefRef AudioAssetRef, Bool bStreaming);
		
		// Audio control
		Bool		StartAudio(TRefRef AudioSourceRef);
		Bool		StopAudio(TRefRef AudioSourceRef);
		Bool		PauseAudio(TRefRef AudioSourceRef);
		
		Bool		DetermineFinishedAudio(TArray<TRef>& refArray);
		
		// Audio Properties
		Bool		SetPitch(TRefRef AudioSourceRef, const float fPitch);
		Bool		GetPitch(TRefRef AudioSourceRef, float& fPitch);
		
		Bool		SetVolume(TRefRef AudioSourceRef, const float fVolume);
		Bool		GetVolume(TRefRef AudioSourceRef, float& fVolume);
		
		Bool		SetLooping(TRefRef AudioSourceRef, const Bool bLooping);
		Bool		GetIsLooping(TRefRef AudioSourceRef, Bool& bLooping);
		
		Bool		SetRelative(TRefRef AudioSourceRef, const Bool bRelative);
		Bool		GetIsRelative(TRefRef AudioSourceRef, Bool& bRelative);
		
		Bool		SetPosition(TRefRef AudioSourceRef, const float3 vPosition);
		Bool		GetPosition(TRefRef AudioSourceRef, float3& vPosition);
		
		Bool		SetVelocity(TRefRef AudioSourceRef, const float3 vVelocity);
		Bool		GetVelocity(TRefRef AudioSourceRef, float3& vVelocity);
		
		Bool		SetMinRange(TRefRef AudioSourceRef, const float fDistance);
		Bool		SetMaxRange(TRefRef AudioSourceRef, const float fDistance);
		Bool		SetRateOfDecay(TRefRef AudioSourceRef, const float fRateOfDecay);
		
		
		// Audio system listener (aka a virtual microphone)	
		void SetListener(const TListenerProperties& Props);
		
		Bool Enable();
		Bool Disable();
		
		Bool Activate();
		Bool Deactivate();
		
	}
	
};


using namespace TLAudio;

TAudioNode* TAudioNodeFactory::CreateObject(TRefRef InstanceRef,TRefRef TypeRef)
{
	// Create engine side audio nodes
	if(TypeRef == "Audio")
		return new TAudioNode(InstanceRef,TypeRef);
	else if(TypeRef == "Music")
		return new TAudioNode_Music(InstanceRef,TypeRef);
	
	return NULL;
}



TAudiograph::TAudiograph() :
	TLGraph::TGraph<TLAudio::TAudioNode>	( "Audio" ),
	m_fMusicVolume							(1.0f),
	m_fEffectsVolume						(1.0f),
	m_bPause								(FALSE),
	m_bEnabled								(FALSE),
	m_bActive								(TRUE),
	m_bMute									(FALSE)
{
}



SyncBool TAudiograph::Initialise()
{
	if(TLMessaging::g_pEventChannelManager)
	{
		if(InitDevices() != SyncTrue)
			return SyncFalse;

		TLMessaging::g_pEventChannelManager->RegisterEventChannel(this, GetManagerRef(), "Start");
		TLMessaging::g_pEventChannelManager->RegisterEventChannel(this, GetManagerRef(), "Stop");
		TLMessaging::g_pEventChannelManager->RegisterEventChannel(this, GetManagerRef(), "Pause");
		TLMessaging::g_pEventChannelManager->RegisterEventChannel(this, GetManagerRef(), "OnVolumeChanged");
		TLMessaging::g_pEventChannelManager->RegisterEventChannel(this, GetManagerRef(), "OnMuteChanged");
		TLMessaging::g_pEventChannelManager->RegisterEventChannel(this, GetManagerRef(), "OnEnableChanged");

		if(TLGraph::TGraph<TLAudio::TAudioNode>::Initialise() == SyncTrue)
		{	
			//	create generic audio node factory
			TPtr<TClassFactory<TLAudio::TAudioNode,FALSE> > pFactory = new TAudioNodeFactory();

			if(!pFactory)
			{
				TLDebug_Print("Failed to create audio node factory");
				return SyncFalse;
			}

			AddFactory(pFactory);
			
			return SyncTrue;
		}
		
	}

	return SyncWait;
}

void TAudiograph::OnEventChannelAdded(TRefRef refPublisherID, TRefRef refChannelID)
{
	if(refPublisherID == "SCREENMANAGER")
	{
		// Subscribe to the update messages
		if((refChannelID == "OnWindowChanged") ||
			(refChannelID == "ScreenChanged"))
		{
			TLMessaging::g_pEventChannelManager->SubscribeTo(this, refPublisherID, refChannelID); 
		}
	}
	else if(refPublisherID == "INPUT")
	{
		// Subscribe to the input device messages
		if(refChannelID == "DeviceChanged")
			TLMessaging::g_pEventChannelManager->SubscribeTo(this, refPublisherID, refChannelID); 
		
	}
	else if(refPublisherID == "USERMANAGER")
	{
		// Subscribe to the input action messages
		if(refChannelID == TRef_Static(A,c,t,i,o))
			TLMessaging::g_pEventChannelManager->SubscribeTo(this, refPublisherID, refChannelID); 
	}


	// Super event channel routine
	TLGraph::TGraph<TLAudio::TAudioNode>::OnEventChannelAdded(refPublisherID, refChannelID);
}



SyncBool TAudiograph::InitDevices()
{
	return TLAudio::Platform::Init();
}


SyncBool TAudiograph::Update(float fTimeStep)
{
	TLTime::TScopeTimer Timer( TRef_Static(A,u,d,i,o) );

	if(!IsEnabled() || !IsActive())
	{
		// Still need to process queued messages
		ProcessMessageQueue();
		return SyncTrue;
	}

	// Disable the audio whilst we make mass changes
	//Disable();

	TArray<TRef> refArray;

	if(Platform::DetermineFinishedAudio(refArray))
	{
		// Send out notification of each audio item that has come to an end
		for(u32 uIndex = 0; uIndex < refArray.GetSize(); uIndex++)
		{
			TRef AudioRef = refArray.ElementAt(uIndex);

			TLMessaging::TMessage Message("Stop");
			Message.Write(AudioRef);

			PublishMessage(Message);
		}
	}

	SyncBool Result = TLGraph::TGraph<TLAudio::TAudioNode>::Update(fTimeStep);

	// Update the low level system
	TLAudio::Platform::Update();
	
	// TODO: 
	// Determine audible audio nodes as a list using distance to listener position
	// OpenAL is limited to 16 'channels' so limit the list bearing in mind some nodes will need multiple
	// channels - i.e. music may need 3 or 4.
	// Remove any buffers for audio not now audible
	// Create buffers and sources dynamically per-frame
	// and bind
	// IMPORTANT: change low level system to allow for multiple buffers/sources for 
	//            one audio node - currently only allows for one of each per node.
	
	// Enable the audio again
	//Enable();

	return Result;
}

SyncBool TAudiograph::Shutdown()
{
	TLDebug_Print("Audiograph shutdown");

	if(Platform::Shutdown() != SyncTrue)
		return SyncFalse;

	return TLGraph::TGraph<TLAudio::TAudioNode>::Shutdown();
}


void TAudiograph::ProcessMessage(TLMessaging::TMessage& Message)
{
	TRef MessageRef = Message.GetMessageRef();

	if(MessageRef == "SetVolume")
	{
		TLDebug_Break("Redundant - use SetProperty");
		float fVolume; 
		if(Message.ImportData("Effects", fVolume))
		{
			// Clamp to within range
			TLMaths::Limit(fVolume, 0.0f, 1.0f);

			SetEffectsVolume(fVolume);
		}

		if(Message.ImportData("Music", fVolume))
		{
			// Clamp to within range
			TLMaths::Limit(fVolume, 0.0f, 1.0f);

			SetMusicVolume(fVolume);
		}

		return;
	}
	else if(MessageRef == "Pause")
	{
		SetPause(TRUE);
		return;
	}
	else if(MessageRef == "UnPause")
	{
		SetPause(FALSE);
		return;
	}
	else if(MessageRef == "Mute")
	{
		SetMute(TRUE);
		return;
	}
	else if(MessageRef == "UnMute")
	{
		SetMute(FALSE);
		return;
	}
	else if(MessageRef == "Enable")
	{
		// Enable the audio system, creating low-level device
		SetEnabled(TRUE);
		return;
	}
	else if(MessageRef == "Disable")
	{
		// Disable the audio system completely, destroying low-level device
		SetEnabled(FALSE);
		return;
	}	
	else if(MessageRef == TRef_Static(A,c,t,i,o))
	{
		TRef InputActionRef;
		int2 CursorPos;
		if ( Message.Read(InputActionRef) )
		{
			if ( Message.ImportData("CURSOR", CursorPos ) )
			{
				//	mouse has been pressed down, reset our last position of the mouse
				if ( InputActionRef == "Mute" )
				{
					ToggleMute();
				}
			}
		}
		return;
	}
	else if(MessageRef == "OnWindowChanged")
	{
		TRef State;

		if(Message.ImportData("State", State))
		{
			if(State == "Deactivate")
			{
				// Deactivate/suspend the audio
				SetActive(FALSE);
			}
			else
			{
				// Activate/unsuspend the audio
				SetActive(TRUE);
			}

			return;
		}

	}
	if(MessageRef == "DeviceChanged")
	{
		// Device message from the input system
		// Check for if the device has been added or removed
		TRef refState;
		if(Message.ImportData("State", refState))
		{
			TRef DeviceRef, DeviceTypeRef;
			if(!Message.ImportData("DevID", DeviceRef))
				return;

			if(!Message.ImportData("Type", DeviceTypeRef))
				return;

			if(refState == TRef("Added"))
			{
				// New device
				OnInputDeviceAdded( DeviceRef, DeviceTypeRef );
			}
			else if(refState == TRef("Removed"))
			{
				// Device removed
				// Remove any users that are using the device		
			}

			// return - no need to pass this message on
			return;
		}
	}

	// Super class process message
	TLGraph::TGraph<TLAudio::TAudioNode>::ProcessMessage(Message);
}


void TAudiograph::SetProperty(TLMessaging::TMessage& Message)
{
	if(Message.HasChild("Volume"))
	{
		TPtr<TBinaryTree> pChild = Message.GetChild("Volume");
		float fVolume; 
		if(pChild->ImportData("Effects", fVolume))
		{
			// Clamp to within range
			TLMaths::Limit(fVolume, 0.0f, 1.0f);

			SetEffectsVolume(fVolume);
		}

		if(pChild->ImportData("Music", fVolume))
		{
			// Clamp to within range
			TLMaths::Limit(fVolume, 0.0f, 1.0f);

			SetMusicVolume(fVolume);
		}
	}
	
	Bool bOption = FALSE;

	if(Message.ImportData("Pause", bOption))
	{
		SetPause(bOption);
	}

	if(Message.ImportData("Mute", bOption))
	{
		SetMute(bOption);
	}
	
	if(Message.ImportData("Enable", bOption))
	{
		SetEnabled(bOption);
	}
}


// Enables the low level audio system by initialising the device and allowing 
// audio objects to be created
void TAudiograph::Enable()
{
	if(Platform::Enable())
	{
		m_bEnabled = TRUE;
		OnEnableChanged();
	}
}

// Disables the low level audio system destroying all current sound objects
// and shutting down the audio device
void TAudiograph::Disable()
{
	if(Platform::Disable())
	{
		// Remove all nodes
		RemoveChildren(GetRootNode()->GetNodeRef());
		m_MusicRef.SetInvalid();

		m_bEnabled = FALSE;
		OnEnableChanged();
	}
}

// Switches the low level audio system 'on' to allow audio to be heard
// No changes are made to the device or the audio objects
void TAudiograph::Activate()
{
	if(Platform::Activate())
	{
		m_bActive = TRUE;
	}
}

// Switches the low level audio system 'off' so no audio will be heard
// No changes are made to the device or the audio objects
void TAudiograph::Deactivate()
{
	if(Platform::Deactivate())
	{
		m_bActive = FALSE;
	}
}


void TAudiograph::OnMuteChanged()
{
	TLMessaging::TMessage Message("OnMuteChanged", GetGraphRef());
	Message.ExportData("Mute", m_bMute);
	PublishMessage(Message);
}

void TAudiograph::OnEnableChanged()
{
	TLMessaging::TMessage Message("OnEnableChanged", GetGraphRef());
	Message.ExportData("Enable", m_bEnabled);

	PublishMessage(Message);
}

void TAudiograph::OnMusicVolumeChanged()
{
	TLMessaging::TMessage Message("OnVolumeChanged", GetGraphRef());
	Message.ExportData("Type", TRef("Music"));
	Message.ExportData("Volume", GetMusicVolume());

	PublishMessage(Message);
}

void TAudiograph::OnEffectsVolumeChanged()
{
	TLMessaging::TMessage Message("OnVolumeChanged", GetGraphRef());
	Message.ExportData("Type", TRef("Effects"));
	Message.ExportData("Volume", GetEffectsVolume());

	PublishMessage(Message);
}


void TAudiograph::OnPauseStateChanged()
{
	// Broadcast pause message to all subscribers
	TLMessaging::TMessage Message("Pause", GetGraphRef());
	Message.ExportData("State", m_bPause);

	PublishMessage(Message);
}


void TAudiograph::OnInputDeviceAdded(TRefRef DeviceRef,TRefRef DeviceTypeRef)
{
	TPtr<TLUser::TUser>& pGlobalUser = TLUser::g_pUserManager->GetUser("Global");
	
	if(!pGlobalUser.IsValid())
	{
		TLDebug_Break("Failed to find global user");
		return;
	}

	if(DeviceTypeRef == TLInput::KeyboardRef)
	{
		// Map some basic keyboard keys to control some of the audio system
		MapActions_Keyboard(DeviceRef, pGlobalUser);
	}
}

void TAudiograph::MapActions_Keyboard(TRefRef DeviceRef, TPtr<TLUser::TUser> pUser)
{
	// Add Mute keyboard shortcut
	if(pUser->AddAction("SIMPLE", "Mute"))	
	{
		pUser->MapAction("Mute", DeviceRef, "K_M");
		pUser->MapActionCondition("Mute", TLInput::GreaterThan, 0.0f);
	}
}


void TAudiograph::SetListener(const TListenerProperties& Props)
{
	// Copy the properties so the audio graph can use them
	m_Listener = Props;

	// Now pass on the info to the low level audio system
	Platform::SetListener(Props);
}


Bool TAudiograph::StartMusic(TRefRef AudioAsset)
{
	if(!IsEnabled())
		return FALSE;

	// Music already playing
	if(m_MusicRef.IsValid())
	{
		return FALSE;
	}

	// Create a music node
	TLMessaging::TMessage Message(TLCore::InitialiseRef);

	Message.ExportData("Asset", AudioAsset);
	Message.ExportData("Play", TRUE);

	TLAudio::TAudioProperties Props;

	//Props.m_bStreaming = TRUE;
	Props.m_bLooping = TRUE;
	Props.m_fVolume = 1.0f;


	// By setting the rate of decay to zero we effectively switch of the distance attenuation
	// calculations meaning the sound will always be heard at full volume.
	Props.m_fRateOfDecay = 0.0f;

	Message.ExportData("Props", Props);



	m_MusicRef = CreateNode("Music", "Music", "Root", &Message); 

	return m_MusicRef.IsValid();
}


TRef TAudiograph::StartAudio(TRefRef AudioRef, TRefRef AssetRef)
{
	if(!IsEnabled())
		return TRef();

	TLMessaging::TMessage Message(TLCore::InitialiseRef);

	Message.ExportData("Asset", AssetRef);
	Message.ExportData("Play", TRUE);

	return StartAudio(AudioRef, Message);
}


TRef TAudiograph::StartAudio(TRefRef AudioRef, TLMessaging::TMessage& Message)	
{
	if(!IsEnabled())
		return TRef();

	return CreateNode(AudioRef, "Audio", "Root", &Message);
}





Bool TAudiograph::StopAudio(TRefRef AudioRef)
{
	// Find the audio node
	TPtr<TLAudio::TAudioNode> pAudioNode = FindNode(AudioRef);

	if(pAudioNode)
	{
		pAudioNode->Stop();
		return TRUE;
	}

	return FALSE;
}

Bool TAudiograph::PauseAudio(TRefRef AudioRef, const Bool& bPause)
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


void TAudiograph::SetAudioTranslate(TRefRef AudioRef, const float3& vTranslate)
{
	// Find the audio node
	TPtr<TLAudio::TAudioNode> pAudioNode = FindNode(AudioRef);
	
	if(pAudioNode)
	{
		pAudioNode->UpdatePreviousPos();
		pAudioNode->SetTranslate(vTranslate);
	}
}

float3 TAudiograph::GetAudioTranslate(TRefRef AudioRef)
{
	// Find the audio node
	TPtr<TLAudio::TAudioNode> pAudioNode = FindNode(AudioRef);
	
	if(pAudioNode)
		return pAudioNode->GetTranslate();

	return float3(0.0f, 0.0f, 0.0f);
}

void TAudiograph::SetAudioPitch(TRefRef AudioRef, const float& fPitch)
{
	// Find the audio node
	TPtr<TLAudio::TAudioNode> pAudioNode = FindNode(AudioRef);

	if(pAudioNode)
		pAudioNode->SetPitch(fPitch);
}


float TAudiograph::GetAudioPitch(TRefRef AudioRef)
{
	// Find the audio node
	TPtr<TLAudio::TAudioNode> pAudioNode = FindNode(AudioRef);
	
	if(pAudioNode)
		return pAudioNode->GetPitch();
	
	// Failed?
	return -1.0f;
}



void TAudiograph::SetAudioVolume(TRefRef AudioRef, const float& fVolume)
{
	// Find the audio node
	TPtr<TLAudio::TAudioNode> pAudioNode = FindNode(AudioRef);
	
	if(pAudioNode)
		pAudioNode->SetVolume(fVolume);
}

float TAudiograph::GetAudioVolume(TRefRef AudioRef)
{
	// Find the audio node
	TPtr<TLAudio::TAudioNode> pAudioNode = FindNode(AudioRef);
	
	if(pAudioNode)
		return pAudioNode->GetVolume();

	return -1.0f;
}

void TAudiograph::SetAudioLooping(TRefRef AudioRef, const Bool& bLooping)
{
	// Find the audio node
	TPtr<TLAudio::TAudioNode> pAudioNode = FindNode(AudioRef);
	
	if(pAudioNode)
		pAudioNode->SetLooping(bLooping);
}


Bool TAudiograph::GetAudioIsLooping(TRefRef AudioRef)
{
	// Find the audio node
	TPtr<TLAudio::TAudioNode> pAudioNode = FindNode(AudioRef);
	
	if(pAudioNode)
		return pAudioNode->GetIsLooping();
	
	return FALSE;
}

// The audio offset is a normalised value 0.0f - being the start and 1.0f being the end
void TAudiograph::SetAudioOffsetNormalised(TRefRef AudioRef, float& fOffset)
{
	// Clamp to within range 
	TLMaths::Limit(fOffset, 0.0f, 1.0f);

	// Get the audio length form the current buffer being used

	// Set the time in secs
}

float TAudiograph::GetAudioOffsetNormalised(TRefRef AudioRef)
{
	return 0.0f;
}


void TAudiograph::SetAudioOffsetSeconds(TRefRef AudioRef, const float& fSeconds)
{

}

float TAudiograph::GetAudioOffsetSeconds(TRefRef AudioRef)
{
	return 0.0f;
}

float TAudiograph::GetAudioLengthSeconds(TRefRef AudioRef)
{
	return 0.0f;
}





	
#include "TEffect.h"


using namespace TLInput;


SyncBool TInputEffect_ForceFeedback::Initialise(TLMessaging::TMessage& Message)
{
	// Begin the vibration
	OnStartVibration();
	
	return TInputEffect::Initialise(Message);
}

SyncBool TInputEffect_ForceFeedback::Update(float fTimestep)
{
	// Update the duration timer
	m_fDuration -= fTimestep;
	
	if(m_fDuration <= 0.0f)
	{
		// Complete
		// TODO: Broadcast a message to say the effect has finished?
		return SyncTrue;
	}
	
	// Update the vibration effect
	OnUpdateVibration();
	
	return SyncWait;
}

SyncBool TInputEffect_ForceFeedback::Shutdown()
{
	// Stop the vibration
	OnStopVibration();


	return TInputEffect::Shutdown();
}


void TInputEffect_ForceFeedback::OnStartVibration()
{
	//TLInput::Platform::StartVibration()
}

void TInputEffect_ForceFeedback::OnUpdateVibration()
{
	//TLInput::Platform::UpdateVibration()
}

void TInputEffect_ForceFeedback::OnStopVibration()
{
	//TLInput::Platform::StopVibration()
}


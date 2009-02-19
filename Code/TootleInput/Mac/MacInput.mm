#include "MacInput.h"

#ifdef _DEBUG
	#define ENABLE_INPUTSYSTEM_TRACE
#endif

// [16/12/08] DB -	Possible responsiveness improvement
//					Calls the update manually from the touch events which should mean the inptu is processed immediately
//				    instead of during the next frame update
#define ENABLE_IMMEDIATE_TOUCHUPDATE

#define ACCEL_MAXPROCESS	10		//	at most, per device update only process the last N accelerometer data's
#define ACCEL_MINCHANGE		0.025f	//	minimum amount of change on an axis to register a change. anything smaller than this will be ignored and "jitter"



namespace TLInput
{
	namespace Platform 
	{
		namespace Mac 
		{						
			void	SetCursorPosition(u8 uIndex, int2 uPos);			
		}
	}
}


using namespace TLInput;

// Not used on the ipod
SyncBool Platform::Init()
{		
	return SyncTrue;	
}

// Create Mac input device
Bool Platform::Mac::CreateDevice()
{
	// On the Ipod the input is fixed hardware so create a generic device to handle this
	TRef InstanceRef = "IPOD";
	
	// Create the generic input object
	TPtr<TInputDevice>& pGenericDevice = g_pInputSystem->GetInstance(InstanceRef, TRUE, "Mouse");
	
	if(pGenericDevice.IsValid())
	{
		if(!InitialiseDevice(pGenericDevice))
		{
			// Failed to initialise the input device data						
			pGenericDevice = NULL;
			
			g_pInputSystem->RemoveInstance(InstanceRef);
			
			return FALSE;
		}
		else
		{
			// Notify to all subscribers of the input system that a new device was added
			TPtr<TLMessaging::TMessage> pMessage = new TLMessaging::TMessage("Input");			
			
			if(pMessage.IsValid())
			{
				TRef refState = "ADDED";
				pMessage->AddChannelID("DEVICE");									// device information message
				pMessage->AddChildAndData("STATE", refState);					// state change
				pMessage->AddChildAndData("DEVID", pGenericDevice->GetDeviceRef() );	// device ID
				pMessage->AddChildAndData("TYPE", pGenericDevice->GetDeviceType() );						// device type
				
				g_pInputSystem->PublishMessage(pMessage);
				
				// Success
				return TRUE;
			}
		}
	}
	
	return FALSE;
}

// Initialise the device
Bool Platform::Mac::InitialiseDevice(TPtr<TInputDevice> pDevice)
{
	return TRUE;
}



SyncBool Platform::Update()		
{	

			
	return SyncTrue;	
}

SyncBool Platform::Shutdown()	
{	
	// Shutdown the mac hardware

	
	return SyncTrue;
}

SyncBool Platform::EnumerateDevices()
{ 
	
	return SyncTrue; 
}

void Platform::RemoveAllDevices() 
{
}


Bool Platform::UpdateDevice(TPtr<TLInput::TInputDevice> pDevice)
{
#ifdef ENABLE_INPUTSYSTEM_TRACE
	TLDebug_Print("INPUT: Begin update");
#endif
		
#ifdef ENABLE_INPUTSYSTEM_TRACE
	TLDebug_Print("INPUT: End update");
#endif			
	
	return TRUE;
}



int2 Platform::GetCursorPosition(u8 uIndex)
{
	//TODO: Return cursor pos
	
	return int2(0,0);
}


// On Ipod set the cursor position
void Platform::Mac::SetCursorPosition(u8 uIndex, int2 uPos)
{
	//TODO: Set cursor pos
}



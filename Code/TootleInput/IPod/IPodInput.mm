#include "IPodInput.h"

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
		namespace IPod 
		{
			TPtrArray<TTouchData>		g_TouchData;
			TArray<TAccelerationData>	g_AccelerationData;
			float3						g_LastAccelData;		//	store the last accel data in case we dont have any for some immediate touch response, we will just send the last data we had. Also reduces the amount of processing done when values changes only slightly
			
			const u32 MAX_CURSOR_POSITIONS = 4;
			TFixedArray<int2, MAX_CURSOR_POSITIONS>		g_aCursorPositions;
			
			
			TFixedArray<float, MAX_CURSOR_POSITIONS>	g_aIpodButtonState;
						
			void	SetCursorPosition(u8 uIndex, int2 uPos);
			
			void	ProcessTouchData(TPtr<TLInput::TInputDevice> pDevice);
		}
	}
}


using namespace TLInput;

// Not used on the ipod
SyncBool Platform::Init()
{		
	return SyncTrue;	
}

// Create Ipod input device
Bool Platform::IPod::CreateDevice()
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
Bool Platform::IPod::InitialiseDevice(TPtr<TInputDevice> pDevice)
{
	// Create four 'buttons' and 'axis' sensors to be able to send data from
	// and for actions to be mapped to
	
	// Add button inputs
	u32 uIndex = 0;
	
	u32 uUniqueID = 0;
	
	TString stringLabel;
	TRef refLabel;
	
	
	for(uIndex = 0; uIndex < IPod::MAX_CURSOR_POSITIONS; uIndex++)
	{
		// For buttons we need to label them based on what type and the model
		// so get this information from a function in stead which will lookup the details required
		
		TPtr<TInputSensor>& pSensor = pDevice->AttachSensor(uUniqueID, Button);
		
		if(pSensor.IsValid())
		{
			//pSensor->SubscribeTo(pDXDevice);

#ifdef ENABLE_INPUTSYSTEM_TRACE
			TString str;
			str.Appendf("Attached button sensor - %d", uUniqueID);
			TLDebug_Print(str);
#endif
			
			uUniqueID++;
		}
	}
		
	TArray<TRef> AxisRefs;
	
	AxisRefs.Add("AXX1");
	AxisRefs.Add("AXY1");
	AxisRefs.Add("AXZ1");
	AxisRefs.Add("AXX2");
	AxisRefs.Add("AXY2");
	AxisRefs.Add("AXZ2");
	AxisRefs.Add("AXX3");
	AxisRefs.Add("AXY3");
	AxisRefs.Add("AXZ3");
	AxisRefs.Add("AXX4");
	AxisRefs.Add("AXY4");
	AxisRefs.Add("AXZ4");
	
	for(uIndex = 0; uIndex < AxisRefs.GetSize(); uIndex++)
	{		
		
		TPtr<TInputSensor>& pSensor = pDevice->AttachSensor(uUniqueID, Axis);
		
		if(pSensor.IsValid())
		{
			//	gr: wierd code? ref -> string -> ref ?
			//pSensor->SubscribeTo(pDXDevice);
			TRef temp = AxisRefs.ElementAt(uIndex);
			stringLabel.Empty();
			temp.GetString(stringLabel); // get the appropriate axis type x,y,z
			refLabel = stringLabel;
			
#ifdef ENABLE_INPUTSYSTEM_TRACE
			TString str;
			str.Appendf("Attached axis sensor - %d", uUniqueID);
			TLDebug_Print(str);
			TLDebug_Print(stringLabel);
#endif
			
			pSensor->SetLabel(refLabel);
			uUniqueID++;
		}
	}
	
	AxisRefs.Empty();
	AxisRefs.Add("ACCX");
	AxisRefs.Add("ACCY");
	AxisRefs.Add("ACCZ");
	
	for(uIndex = 0; uIndex < 3; uIndex++)
	{
		// Add accelerometer axis
		TPtr<TInputSensor>& pSensor = pDevice->AttachSensor(uUniqueID, Axis);
		
		if(pSensor.IsValid())
		{
			//	gr: wierd code? ref -> string -> ref ?
			TRef temp = AxisRefs.ElementAt(uIndex);
			stringLabel.Empty();
			temp.GetString(stringLabel);
			refLabel = stringLabel;
			pSensor->SetLabel(refLabel);
#ifdef ENABLE_INPUTSYSTEM_TRACE
			TString str;
			str.Appendf("Attached accelerometer sensor - %d", uUniqueID);
			TLDebug_Print(str);
			TLDebug_Print(stringLabel);

#endif
			
			uUniqueID++;
		}
	}
	
	return TRUE;
}



SyncBool Platform::Update()		
{	

			
	return SyncTrue;	
}

SyncBool Platform::Shutdown()	
{	
	// Shutdown the ipod hardware

	//Clear the touch data array
	IPod::g_TouchData.Empty();
	
	return SyncTrue;
}

// Not used on IPod
SyncBool Platform::EnumerateDevices()
{ 
	TRef InstanceRef = "IPOD";

	// Check to see if the single ipod input device alreay exists
	TPtr<TInputDevice>& pDevice = g_pInputSystem->GetInstance(InstanceRef, FALSE);
	
	if(!pDevice)
		IPod::CreateDevice();
	
	return SyncTrue; 
}

// Not used on IPod
void Platform::RemoveAllDevices() 
{
}


Bool Platform::UpdateDevice(TPtr<TLInput::TInputDevice> pDevice)
{
#ifdef ENABLE_INPUTSYSTEM_TRACE
	TLDebug_Print("INPUT: Begin update");
#endif
	TArray<IPod::TAccelerationData>& AccelerationDataArray = IPod::g_AccelerationData;
				
	
	if(IPod::g_TouchData.GetSize() > 0 )
	{
		// Process all touch data
		IPod::ProcessTouchData(pDevice);

		// Processed all touch data
		// Check to see if we still have any accelerometer data left to process
		// If so continue through otherwise return
		//if( AccelerationData.GetSize() == 0)
		//	return TRUE;
	}
	
	//	no accel data to process
	if ( AccelerationDataArray.GetSize() == 0 )
		return TRUE;
	
	//	gr: skip excessive accelleration data;
	u32 FirstData = 0;
	if ( AccelerationDataArray.GetSize() > ACCEL_MAXPROCESS )
		FirstData = AccelerationDataArray.GetLastIndex() - ACCEL_MAXPROCESS;
	
	//	process all our accell data
	for ( u32 i=FirstData;	i<AccelerationDataArray.GetSize();	i++ )
	{
		//	gr: check to see if we can ignore minor changes first
		IPod::TAccelerationData& AccelerationData = AccelerationDataArray.ElementAt(i);
		float3 Diff( IPod::g_LastAccelData - AccelerationData.m_vAcceleration );

		//	if every axis' change is less than the Min then skip this data
		if ( ( Diff.x < ACCEL_MINCHANGE && Diff.x > -ACCEL_MINCHANGE ) && 
			 ( Diff.y < ACCEL_MINCHANGE && Diff.y > -ACCEL_MINCHANGE ) && 
			 ( Diff.z < ACCEL_MINCHANGE && Diff.z > -ACCEL_MINCHANGE ) )
		{
			continue;
		}

#ifdef ENABLE_INPUTSYSTEM_TRACE
		TLDebug_Print( TString("INPUT: Process NON-TOUCH data: %d/%d", i, AccelerationDataArray.GetSize()-1 ) );
#endif
		// No touch data to process, so send a basic message with the current generic device state info		
		TPtr<TBinaryTree>& MainBuffer = pDevice->GetDataBuffer();
		MainBuffer->Empty();
		TPtr<TBinaryTree>& pDataBuffer = MainBuffer->AddChild("Input");

		// Setup the button data				
		for(u32 uButtonIndex = 0; uButtonIndex < IPod::MAX_CURSOR_POSITIONS; uButtonIndex++)
		{
			// Add data to the binary array - use current state information
			pDataBuffer->Write(IPod::g_aIpodButtonState.ElementAt(uButtonIndex));
		}

		// No deltas in position this frame
		for(u32 uAxisIndex = 0; uAxisIndex < IPod::MAX_CURSOR_POSITIONS; uAxisIndex++)
		{
			// No changes in position for the other axis on this run of the loop
			pDataBuffer->Write(0.0f);
			pDataBuffer->Write(0.0f);
			pDataBuffer->Write(0.0f);
		}
		
		// Add the acceleromter data
		//	gr: could possibly get away with just writing the float3 but won't for now
		pDataBuffer->Write( AccelerationData.m_vAcceleration.x );
		pDataBuffer->Write( AccelerationData.m_vAcceleration.y );
		pDataBuffer->Write( AccelerationData.m_vAcceleration.z );
		
		//	record last-used accell value
		IPod::g_LastAccelData = AccelerationData.m_vAcceleration;
			
		// Set the cursor information
		//SetCursorPosition(uSensorIndex, pTouchData->uCurrentPos);
		
		// Tell the device to process the data
		pDataBuffer->ResetReadPos();
		pDevice->ForceUpdate();
	
	}
	
	//	processed all this data
	AccelerationDataArray.Empty();
		
#ifdef ENABLE_INPUTSYSTEM_TRACE
	TLDebug_Print("INPUT: End update");
#endif			
	
	return TRUE;
}


void Platform::IPod::ProcessTouchData(TPtr<TLInput::TInputDevice> pDevice)
{	
	TPtr<TBinaryTree>& MainBuffer = pDevice->GetDataBuffer();

	// Go through the list of touch data we have received and 
	// process it like it was from sensors	
	for(u32 uIndex = 0; uIndex < g_TouchData.GetSize(); uIndex++)
	{		
#ifdef ENABLE_INPUTSYSTEM_TRACE
		TLDebug_Print("INPUT: Process TOUCH data");
#endif
		
		MainBuffer->Empty();
		
		TPtr<TBinaryTree>& pDataBuffer = MainBuffer->AddChild("Input");
		
		if(pDataBuffer)
		{
			
			TPtr<TTouchData>& pTouchData = g_TouchData.ElementAt(uIndex);
			
			// Get the index - this will correspond to both a button and axis index
			u32 uSensorIndex = pTouchData->uIndex;
			
			// Setup the button data				
			for(u32 uButtonIndex = 0; uButtonIndex < IPod::MAX_CURSOR_POSITIONS; uButtonIndex++)
			{
				// Set the current state of the button
				// NOTE: We are going to need to keep track of this in some object as if there are no messages then
				// we may send out the wrong data
				
				// if it was a begin or moved message write 1.0f to say the button is down/was pressed
				if(uButtonIndex == uSensorIndex)
				{
					if(pTouchData->uPhase == IPod::Begin || pTouchData->uPhase == IPod::Move)
						g_aIpodButtonState.ElementAt(uButtonIndex) = 1.0f;
					else
						g_aIpodButtonState.ElementAt(uButtonIndex) = 0.0f;
				}
				
				// Add data to the binary array
				pDataBuffer->Write(IPod::g_aIpodButtonState.ElementAt(uButtonIndex));
			}
			
			// Note: Axis will be a delta as per the PC which will be calculated				
			for(u32 uAxisIndex = 0; uAxisIndex < IPod::MAX_CURSOR_POSITIONS; uAxisIndex++)
			{
				
				if(uAxisIndex == uSensorIndex)
				{
					// Change in position
					int2	uDelta = pTouchData->uCurrentPos - pTouchData->uPreviousPos;
					
					pDataBuffer->Write((float)uDelta.x);
					pDataBuffer->Write((float)uDelta.y);
					pDataBuffer->Write(0.0f);	// No AXIS_Z motion on hte ipod but left in case we can do something with it
				}
				else
				{
					// No changes in position for the other axis on this run of the loop
					pDataBuffer->Write(0.0f);
					pDataBuffer->Write(0.0f);
					pDataBuffer->Write(0.0f);
				}
			}
			
			// Add the acceleromter data
			if(g_AccelerationData.GetSize() > 0)
			{
				TAccelerationData& AccelerationData = g_AccelerationData.ElementAt(0);
				
				pDataBuffer->Write( AccelerationData.m_vAcceleration.x );
				pDataBuffer->Write( AccelerationData.m_vAcceleration.y );
				pDataBuffer->Write( AccelerationData.m_vAcceleration.z );
				
				IPod::g_LastAccelData = AccelerationData.m_vAcceleration;
				g_AccelerationData.RemoveAt(0);
			}
			else
			{
				// No acceleration on this run of the loop - use the last accel values so no changes will be detected
				pDataBuffer->Write( IPod::g_LastAccelData.x );
				pDataBuffer->Write( IPod::g_LastAccelData.y );
				pDataBuffer->Write( IPod::g_LastAccelData.z );
			}
			
			
			// Set the cursor information
			SetCursorPosition(uSensorIndex, pTouchData->uCurrentPos);
			
			 // Tell the device to process the data
			 pDataBuffer->ResetReadPos();
			 pDevice->ForceUpdate();
		}
	}

	//Clear the touch data array
	IPod::g_TouchData.Empty();
}


int2 Platform::GetCursorPosition(u8 uIndex)
{
	if(uIndex < Platform::IPod::MAX_CURSOR_POSITIONS)
	{
		// Get cursor position from array
		return Platform::IPod::g_aCursorPositions[uIndex];
	}
	
	return int2(0,0);
}


// On Ipod set the cursor position
void Platform::IPod::SetCursorPosition(u8 uIndex, int2 uPos)
{
	if(uIndex < Platform::IPod::MAX_CURSOR_POSITIONS)
	{
		// Set cursor position 
		Platform::IPod::g_aCursorPositions[uIndex] = uPos;
	}
}




void TLInput::Platform::IPod::ProcessTouchBegin(TPtr<TTouchData>& pTouchData)
{
#ifdef ENABLE_INPUTSYSTEM_TRACE
	TLDebug_Print("TOUCH BEGIN");
#endif
	g_TouchData.Add(pTouchData);

#ifdef ENABLE_IMMEDIATE_TOUCHUPDATE
	TLInput::g_pInputSystem->ForceUpdate();
#endif
}

void TLInput::Platform::IPod::ProcessTouchMoved(TPtr<TTouchData>& pTouchData)
{
#ifdef ENABLE_INPUTSYSTEM_TRACE
	TLDebug_Print("TOUCH MOVED");
	TString str;
	str.Appendf("CurrrentPos %d %d", pTouchData->uCurrentPos.x, pTouchData->uCurrentPos.y);
	TLDebug_Print(str);
#endif
	
	g_TouchData.Add(pTouchData);
	
#ifdef ENABLE_IMMEDIATE_TOUCHUPDATE
	TLInput::g_pInputSystem->ForceUpdate();
#endif
}

void TLInput::Platform::IPod::ProcessTouchEnd(TPtr<TTouchData>& pTouchData)
{
#ifdef ENABLE_INPUTSYSTEM_TRACE
	TLDebug_Print("TOUCH END");
#endif
	g_TouchData.Add(pTouchData);

#ifdef ENABLE_IMMEDIATE_TOUCHUPDATE
	TLInput::g_pInputSystem->ForceUpdate();
#endif
}

void TLInput::Platform::IPod::ProcessAcceleration(TLInput::Platform::IPod::TAccelerationData& AccelerationData)
{
	// Handle acceleration data here
	//TLDebug_Print(TString("TLInput::Platform::IPod::ProcessAcceleration - Acceleration occured (%.3f, %.3f, %.3f)", pAccelerationData->vAcceleration.x, pAccelerationData->vAcceleration.y, pAccelerationData->vAcceleration.z));
	
	g_AccelerationData.Add( AccelerationData );
}



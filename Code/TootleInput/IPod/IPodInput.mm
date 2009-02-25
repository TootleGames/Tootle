#include "IPodInput.h"

#ifdef _DEBUG
//	#define ENABLE_INPUTSYSTEM_TRACE
#endif

// [16/12/08] DB -	Possible responsiveness improvement
//					Calls the update manually from the touch events which should mean the inptu is processed immediately
//				    instead of during the next frame update
//#define ENABLE_IMMEDIATE_TOUCHUPDATE

#define ACCEL_MAXPROCESS	10		//	at most, per device update only process the last N accelerometer data's
#define ACCEL_MINCHANGE		0.025f	//	minimum amount of change on an axis to register a change. anything smaller than this will be ignored and "jitter"

// [20/02/09] DB - Define to enable the multi-touch support in the input system.  Currenly always uses index 0 for the sensor index
//				   With this option defined the index should correspond to the appropriate finger index
//#define ENABLE_MULTI_TOUCH 


namespace TLInput
{
	namespace Platform 
	{
		namespace IPod 
		{
			TArray<TTouchData>			g_TouchData;
			TArray<float3>				g_AccelerationData;
			float3						g_LastAccelData = float3(0,0,0);		//	store the last accel data in case we dont have any for some immediate touch response, we will just send the last data we had. Also reduces the amount of processing done when values changes only slightly
			
			const u32 MAX_CURSOR_POSITIONS = 4;
			TFixedArray<int2, MAX_CURSOR_POSITIONS>					g_aCursorPositions;
			TFixedArray<float, MAX_CURSOR_POSITIONS>				g_aIpodButtonState;		// DB - MAY BE ABLE TO REMOVE THIS AND USE THE NEW TOUCH OBJECT ARRAY
						
			TFixedArray< TRef, MAX_CURSOR_POSITIONS>				g_ActiveTouchObjects;	// Fixed array of touch object ID's
			TArray<TTouchObject>									g_TouchObjects;			// Dynamic array of touch objects that persist over time
			
			void	SetCursorPosition(u8 uIndex, int2 uPos);
			
			void	ProcessTouchData(TPtr<TLInput::TInputDevice> pDevice);
			
			void	CheckRemoveTouchObjects();
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
		
			refLabel = GetDefaultButtonRef(uIndex);
			pSensor->AddLabel(refLabel);
			uUniqueID++;
		}
	}
		

	for(uIndex = 0; uIndex < IPod::MAX_CURSOR_POSITIONS; uIndex++)
	{		
		
		TPtr<TInputSensor>& pSensor = pDevice->AttachSensor(uUniqueID, Axis);
		
		if(pSensor.IsValid())
		{
			refLabel = GetDefaultAxisRef(uIndex);
			pSensor->AddLabel(refLabel);
			uUniqueID++;
		}
	}
	
	TArray<TRef> AxisRefs;
	AxisRefs.Add("ACCX");
	AxisRefs.Add("ACCY");
	AxisRefs.Add("ACCZ");
	
	for(uIndex = 0; uIndex < 3; uIndex++)
	{
		// Add accelerometer axis
		TPtr<TInputSensor>& pSensor = pDevice->AttachSensor(uUniqueID, Axis);
		
		if(pSensor.IsValid())
		{
			TRef refLabel = AxisRefs.ElementAt(uIndex);
			pSensor->AddLabel(refLabel);
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
	
	IPod::g_ActiveTouchObjects.Empty();
	IPod::g_TouchObjects.Empty();
	
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
	TArray<float3>& AccelerationDataArray = IPod::g_AccelerationData;
				
	
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
		const float3& vAccelerationData = AccelerationDataArray.ElementAt(i);

		//	gr: could possibly get away with just writing the float3 but won't for now
		pDataBuffer->Write( vAccelerationData.x );
		pDataBuffer->Write( vAccelerationData.y );
		pDataBuffer->Write( vAccelerationData.z );
		
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
			
			const TTouchData& TouchData = g_TouchData.ElementAt(uIndex);
			
#ifdef ENABLE_MULTI_TOUCH
			// Find the index using the touch object arrays
			s32 sSensorIndex = g_ActiveTouchObjects.FindIndex(TouchData.TouchRef);
			
			if(sSensorIndex == -1)
			{
				// If the index is invalid then we have touch data but no corresponding 
				// touch object in the arrays.  This may be because the touch object has been removed 
				// before it has had chance to be processed.  In this casea  deffered removal mechanism may be in order
				// but then we may run into issues where we run out of slots if lots of fingers are being pressed at once.
				TLDebug_Break("Touch data with no corresponding touch object");
			}
			
#else
			// Always use sensor index 0
			s32 sSensorIndex = 0;			
#endif
						
			// Setup the button data				
			for(s32 sButtonIndex = 0; sButtonIndex < IPod::MAX_CURSOR_POSITIONS; sButtonIndex++)
			{
				// Set the current state of the button
				// NOTE: We are going to need to keep track of this in some object as if there are no messages then
				// we may send out the wrong data
				
				// if it was a begin or moved message write 1.0f to say the button is down/was pressed
				if(sButtonIndex == sSensorIndex)
				{
					if(TouchData.uPhase == IPod::TTouchData::Begin || TouchData.uPhase == IPod::TTouchData::Move)
						g_aIpodButtonState.ElementAt(sButtonIndex) = 1.0f;
					else
						g_aIpodButtonState.ElementAt(sButtonIndex) = 0.0f;
				}
				
				// Add data to the binary array
				pDataBuffer->Write(IPod::g_aIpodButtonState.ElementAt(sButtonIndex));
			}
			
			// Note: Axis will be a delta as per the PC which will be calculated				
			for(s32 sAxisIndex = 0; sAxisIndex < IPod::MAX_CURSOR_POSITIONS; sAxisIndex++)
			{
				
				if(sAxisIndex == sSensorIndex)
				{
					// Change in position
					int2	uDelta = TouchData.uCurrentPos - TouchData.uPreviousPos;
					
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
				const float3& vAccelerationData = g_AccelerationData.ElementAt(0);
				
				pDataBuffer->Write( vAccelerationData.x );
				pDataBuffer->Write( vAccelerationData.y );
				pDataBuffer->Write( vAccelerationData.z );
				
				g_AccelerationData.RemoveAt(0);
			}
			else
			{
				//	No new acceleration data, the last accel values so no changes will be detected in the sensors
				pDataBuffer->Write( IPod::g_LastAccelData.x );
				pDataBuffer->Write( IPod::g_LastAccelData.y );
				pDataBuffer->Write( IPod::g_LastAccelData.z );
			}
			
			
			// Set the cursor information
			SetCursorPosition(sSensorIndex, TouchData.uCurrentPos);
			
			// Tell the device to process the data
			pDataBuffer->ResetReadPos();
			pDevice->ForceUpdate();
			
			// Check to see if we need to remove the touch objects
			CheckRemoveTouchObjects();
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




void TLInput::Platform::IPod::ProcessTouchBegin(const TTouchData& TouchData)
{
#ifdef ENABLE_INPUTSYSTEM_TRACE
	TLDebug_Print("TOUCH BEGIN");
#endif
	g_TouchData.Add(TouchData);

#ifdef ENABLE_MULTI_TOUCH	
	// Create a new touch object.  This will map exactly to the ipod touch event
	// so we can keep track of what touch data is which touch object
	if(!g_ActiveTouchObjects.Exists(TouchData.TouchRef))
	{
		Bool bFreeSlotFilled = FALSE;
		// Now find the first free slot in the fixed array of touch object references and add the TouchRef
		// This will be used for indexing the touch objects such that if a person presses two fingers and then releases the first
		// the second finger wills till correspond to a virtual 'button2' being held down and a further finger press would be classed
		// as 'button1'
		for(u32 uIndex = 0; uIndex < g_ActiveTouchObjects.GetSize(); uIndex++)
		{
			if(!g_ActiveTouchObjects.ElementAt(uIndex).IsValid())
			{
				// Found a free slot
				g_ActiveTouchObjects.ElementAt(uIndex) = TouchData.TouchRef;
				bFreeSlotFilled = TRUE;
				break;
			}
		}
		
		if(bFreeSlotFilled)
		{
			TTouchObject TouchObj(TouchData.TouchRef);
			
			// Copy the data from the touch event initially
			TouchObj = TouchData;
			
			g_TouchObjects.Add(TouchObj);			
		}
		else
		{
			TLDebug_Break("Unable to find free slot for ref");
		}
	}
	else
	{
		TLDebug_Break("Touch object exists for ref");
	}
#endif

#ifdef ENABLE_IMMEDIATE_TOUCHUPDATE
	TLInput::g_pInputSystem->ForceUpdate();
#endif
}

void TLInput::Platform::IPod::ProcessTouchMoved(const TTouchData& TouchData)
{
#ifdef ENABLE_INPUTSYSTEM_TRACE
	TLDebug_Print("TOUCH MOVED");
	TString str;
	str.Appendf("CurrrentPos %d %d", TouchData.uCurrentPos.x, TouchData.uCurrentPos.y);
	TLDebug_Print(str);
#endif
	
	g_TouchData.Add(TouchData);
	
	
#ifdef ENABLE_IMMEDIATE_TOUCHUPDATE
	TLInput::g_pInputSystem->ForceUpdate();
#endif
}

void TLInput::Platform::IPod::ProcessTouchEnd(const TTouchData& TouchData)
{
#ifdef ENABLE_INPUTSYSTEM_TRACE
	TLDebug_Print("TOUCH END");
#endif
	g_TouchData.Add(TouchData);

#ifdef ENABLE_MULTI_TOUCH	
	// Flag the touch object to be removed when it has been processed	
	s32 sIndex = g_ActiveTouchObjects.FindIndex(TouchData.TouchRef);
	
	if(sIndex != -1)
	{
		// Flag the touch object to be removed during the next update
		TTouchObject* pObj = g_TouchObjects.Find(TouchData.TouchRef);
		
		if(pObj)
			pObj->uFlags.Set(TTouchObject::Remove, TRUE);
	}
	else
	{
		TLDebug_Break("Active touch object doesn't exist for ref");
	}
#endif
	
	
#ifdef ENABLE_IMMEDIATE_TOUCHUPDATE
	TLInput::g_pInputSystem->ForceUpdate();
#endif
}

void TLInput::Platform::IPod::ProcessAcceleration(const float3& vAccelerationData)
{
	//	gr: moved the comparison with the last-data to here so minor changes don't even get close to the
	//		accell data buffer
	float3 Diff( IPod::g_LastAccelData - vAccelerationData );
	
	//	if every axis' change is less than the Min then ignore this data
	if ( ( Diff.x < ACCEL_MINCHANGE && Diff.x > -ACCEL_MINCHANGE ) && 
		( Diff.y < ACCEL_MINCHANGE && Diff.y > -ACCEL_MINCHANGE ) && 
		( Diff.z < ACCEL_MINCHANGE && Diff.z > -ACCEL_MINCHANGE ) )
	{
		return;
	}
	
	//	significant change, add the accelleration data
	g_AccelerationData.Add( vAccelerationData );

	//	record last-used accell value
	IPod::g_LastAccelData = vAccelerationData;
}



void TLInput::Platform::IPod::CheckRemoveTouchObjects()
{
	for(u32 uIndex = 0; uIndex < g_TouchObjects.GetSize(); uIndex++)
	{
		TTouchObject& Obj = g_TouchObjects.ElementAt(uIndex);

		// Flagged to be removed?
		if(Obj.uFlags.IsSet(TTouchObject::Remove))
		{			
			// Find the active ref for the touch object
			s32 sIndex = g_ActiveTouchObjects.FindIndex(Obj.TouchRef);
			
			if(sIndex != -1)
			{					
				// Remove the ref as being active
				g_ActiveTouchObjects.ElementAt(sIndex).SetInvalid();
			}
			else
			{
				// Error - we have a touch object but no ref to thi object?
				TLDebug_Break("Unable to find active touch object ref");
			}
			
			g_TouchObjects.RemoveAt(uIndex);
			uIndex--;
		}
	}
	
}



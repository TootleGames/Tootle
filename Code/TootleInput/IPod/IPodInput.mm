#include "IPodInput.h"

#ifdef _DEBUG
	#define ENABLE_INPUTSYSTEM_TRACE
#endif


#define ACCEL_MAXPROCESS	10		//	at most, per device update only process the last N accelerometer data's
#define ACCEL_MINCHANGE		0.025f	//	minimum amount of change on an axis to register a change. anything smaller than this will be ignored and "jitter"

// Define to be able to trace out the accelerometer data - produces a *lot* of output every frame
#ifdef _DEBUG
//	#define TRACE_ACCELEROMETER
#endif

// [20/02/09] DB - Define to enable the multi-touch support in the input system.  Currenly always uses index 0 for the sensor index
//				   With this option defined the index should correspond to the appropriate finger index
#define ENABLE_MULTI_TOUCH


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
			TFixedArray<int2, MAX_CURSOR_POSITIONS>					g_aCursorPositions(MAX_CURSOR_POSITIONS);
			//TFixedArray<float, MAX_CURSOR_POSITIONS>				g_aIpodButtonState;		// DB - MAY BE ABLE TO REMOVE THIS AND USE THE NEW TOUCH OBJECT ARRAY
						
			TFixedArray< TRef, MAX_CURSOR_POSITIONS>				g_ActiveTouchObjects(MAX_CURSOR_POSITIONS);	// Fixed array of touch object ID's
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
			TLMessaging::TMessage Message("DeviceChanged");			
			
			TRef refState = "ADDED";
			Message.AddChildAndData("State", refState);					// state change
			Message.AddChildAndData("DEVID", pGenericDevice->GetDeviceRef() );	// device ID
			Message.AddChildAndData("TYPE", pGenericDevice->GetDeviceType() );						// device type
			
			g_pInputSystem->PublishMessage(Message);
				
			// Success
			return TRUE;
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
			pSensor->SetCursorIndex(uIndex);
			uUniqueID++;
		}
	}

	u32 uAxisIndex = 0;
	for(uIndex = 0; uIndex < IPod::MAX_CURSOR_POSITIONS; uIndex++)
	{		
		uAxisIndex = uIndex * 3;
		
		// Add X axis sensor
		TPtr<TInputSensor> pSensor = pDevice->AttachSensor(TRef(uUniqueID), Axis);
		
		if(pSensor.IsValid())
		{
			refLabel = GetDefaultAxisRef(uAxisIndex);
			pSensor->AddLabel(refLabel);
			pSensor->SetCursorIndex(uIndex);
			uUniqueID++;			
		}
		
		// Add Y axis sensor
		pSensor = pDevice->AttachSensor(uUniqueID, Axis);
		
		if(pSensor.IsValid())
		{
			refLabel = GetDefaultAxisRef(uAxisIndex+1);
			pSensor->AddLabel(refLabel);
			pSensor->SetCursorIndex(uIndex);
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



Bool Platform::UpdateDevice(TLInput::TInputDevice& Device)
{
	// No touch or acceleration data?  nothing to do
	if((IPod::g_TouchData.GetSize() == 0) &&
		(IPod::g_AccelerationData.GetSize() == 0))
	{
		return FALSE;
	}
	
	// Get reference to the device input buffer
	TArray<TInputData>& InputBuffer = Device.GetInputBuffer();
	
	//	prealloc array data
	InputBuffer.AddAllocSize( IPod::g_TouchData.GetSize() );
	InputBuffer.AddAllocSize( IPod::g_AccelerationData.GetSize() );
	
	TInputData data;
	
	if(IPod::g_TouchData.GetSize() > 0)
	{
#ifdef _DEBUG
		TTempString inputinfo = "Input processing ";
		inputinfo.Appendf("%d touch items", IPod::g_TouchData.GetSize());
		TLDebug::Print(inputinfo);
#endif				
			
		// Process the touch data
		for(u32 uIndex = 0; uIndex < IPod::g_TouchData.GetSize(); uIndex++)
		{
			const IPod::TTouchData& TouchData = IPod::g_TouchData.ElementAt(uIndex);
			
#ifdef ENABLE_MULTI_TOUCH
			// Find the index using the touch object arrays
			s32 sTouchIndex = IPod::g_ActiveTouchObjects.FindIndex(TouchData.TouchRef);
			
			if(sTouchIndex == -1)
			{
				// If the index is invalid then we have touch data but no corresponding 
				// touch object in the arrays.  This may be because the touch object has been removed 
				// before it has had chance to be processed.  In this casea  deffered removal mechanism may be in order
				// but then we may run into issues where we run out of slots if lots of fingers are being pressed at once.
				TLDebug_Break("Touch data with no corresponding touch object");
				continue;
			}
			
#else
			// Always use sensor index 0
			s32 sTouchIndex = 0;			
#endif
			u32 uButtonIndex = (u32)sTouchIndex;
			
			data.m_SensorRef = uButtonIndex;
			
			// If the touch is a begin the data is a 1.0f for the button to represent 'pressed'
			// If the touch event is a cancel or end the data is a 0.0f for the button to represent 'released'			
			if(TouchData.uPhase == IPod::TTouchData::Begin)
			{
				data.m_fData = 1.0f;
				InputBuffer.Add(data);
#ifdef _DEBUG
				// In debug print what button was pressed
				TTempString inputinfo = "Touch input Begin: ";
				inputinfo.Appendf("%d %.4f", uButtonIndex, data.m_fData);
				TLDebug::Print(inputinfo);
				
#endif
				
			}
			else if((TouchData.uPhase == IPod::TTouchData::End) ||
					(TouchData.uPhase == IPod::TTouchData::Cancel))
			{
				data.m_fData = 0.0f;
				InputBuffer.Add(data);
#ifdef _DEBUG
				// In debug print what button was pressed
				TTempString inputinfo = "Touch input End: ";
				inputinfo.Appendf("%d %.4f", uButtonIndex, data.m_fData);
				TLDebug::Print(inputinfo);
#endif
				// Remove the ref as being active
				IPod::g_ActiveTouchObjects.ElementAt(sTouchIndex).SetInvalid();
				TLDebug_Print("Removing active touch reference");
				
				for(u32 uIndex = 0; uIndex < IPod::g_TouchObjects.GetSize(); uIndex++)
				{
					IPod::TTouchObject& Obj = IPod::g_TouchObjects.ElementAt(uIndex);
					
					if(Obj.TouchRef == TouchData.TouchRef)
					{
						TLDebug_Print("Removing touch object");
						IPod::g_TouchObjects.RemoveAt(uIndex);
						break;
					}
				}
				
			}
			
			
			// Change in position
			int2	uDelta = TouchData.uCurrentPos - TouchData.uPreviousPos;
			
			// Increment the sensor index to match the axis index for the touch x axis sensor
			u32 uAxisIndex = 4 + (uButtonIndex * 2);
			
			if(uDelta.x != 0.0f)
			{
				// Movement in the x axis so add to the buffer
				data.m_SensorRef = uAxisIndex;
				data.m_fData = uDelta.x/100.0f;
				InputBuffer.Add(data);
#ifdef _DEBUG
				TTempString inputinfo = "Touch X-axis movement: ";
				inputinfo.Appendf("%d %.4f", uAxisIndex, data.m_fData);
				TLDebug::Print(inputinfo);
#endif				
			}
			
			uAxisIndex++;
			
			if(uDelta.y != 0.0f)
			{
				// Movement in the y axis so add to the buffer
				data.m_SensorRef = uAxisIndex;
				data.m_fData = uDelta.y/100.0f;
				InputBuffer.Add(data);
#ifdef _DEBUG
				TTempString inputinfo = "Touch Y-axis movement: ";
				inputinfo.Appendf("%d %.4f", uAxisIndex, data.m_fData);
				TLDebug::Print(inputinfo);				
#endif
			}
			
			// Set the cursor information
			IPod::SetCursorPosition(sTouchIndex, TouchData.uCurrentPos);							
			
		}
			
		// Empty the touch data array
		IPod::g_TouchData.Empty();
	}
	
	if(IPod::g_AccelerationData.GetSize() > 0)
	{
#ifdef TRACE_ACCELEROMETER
		TTempString inputinfo = "Input processing ";
		inputinfo.Appendf("%d accelerometer items", IPod::g_AccelerationData.GetSize());
		TLDebug::Print(inputinfo);
#endif				
		
		
		//	Process the acceleration data
		for(u32 uIndex = 0; uIndex < IPod::g_AccelerationData.GetSize(); uIndex++)
		{
			const float3& vAccelerationData = IPod::g_AccelerationData.ElementAt(uIndex);
			
			// The accelerometer indexes are:
			// X - 12
			// Y - 13
			// Z - 14
			
			u32 uAccIndex = 12;
			data.m_SensorRef = uAccIndex;
			data.m_fData = vAccelerationData.x;
			InputBuffer.Add(data);
			uAccIndex++;

			data.m_SensorRef = uAccIndex;
			data.m_fData = vAccelerationData.y;
			InputBuffer.Add(data);
			uAccIndex++;

			data.m_SensorRef = uAccIndex;
			data.m_fData = vAccelerationData.z;
			InputBuffer.Add(data);

#ifdef TRACE_ACCELEROMETER
			// In debug print the accelermoeter data
			TTempString inputinfo = "Touch accelerometer: ";
			inputinfo.Appendf("%.3f %.3f %.3f", vAccelerationData.x, vAccelerationData.y, vAccelerationData.z);
			TLDebug::Print(inputinfo);
#endif				
			

		}

		// Empty the accelerometer data
		IPod::g_AccelerationData.Empty();
	}
	
	
	return TRUE;	
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
	TString str;
	str.Appendf("TOUCH BEGIN %d", TouchData.TouchRef.GetData() );
	TLDebug_Print(str);
#endif

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
			g_TouchData.Add(TouchData);
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
#else
	g_TouchData.Add(TouchData);
#endif

}

void TLInput::Platform::IPod::ProcessTouchMoved(const TTouchData& TouchData)
{
#ifdef ENABLE_INPUTSYSTEM_TRACE
	TString str;
	str.Appendf("TOUCH MOVED %d", TouchData.TouchRef.GetData() );
	TLDebug_Print(str);
	str.Empty();
	str.Appendf("CurrrentPos %d %d", TouchData.uCurrentPos.x, TouchData.uCurrentPos.y);
	TLDebug_Print(str);
#endif
	
#ifdef ENABLE_MULTI_TOUCH	
	
	// Create a new touch object.  This will map exactly to the ipod touch event
	// so we can keep track of what touch data is which touch object
	// Only do this if we have a valid gmaeside active touch object
	if(g_ActiveTouchObjects.Exists(TouchData.TouchRef))
	{
		g_TouchData.Add(TouchData);
	}
	else
	{
		TLDebug_Break("Active touch object doesn't exist for ref");
	}
#else	
	g_TouchData.Add(TouchData);
#endif
}

void TLInput::Platform::IPod::ProcessTouchEnd(const TTouchData& TouchData)
{
#ifdef ENABLE_INPUTSYSTEM_TRACE
	TString str;
	str.Appendf("TOUCH END %d", TouchData.TouchRef.GetData() );
	TLDebug_Print(str);
#endif

#ifdef ENABLE_MULTI_TOUCH	
	if(g_ActiveTouchObjects.Exists(TouchData.TouchRef))
	{
		g_TouchData.Add(TouchData);
	}
	else
	{
		TLDebug_Break("Active touch object doesn't exist for ref");
	}
#else 
	g_TouchData.Add(TouchData);
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


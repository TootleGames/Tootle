#include "TDevice.h"
#include "TLInput.h"

using namespace TLInput;

/*
	Device update
*/
void TInputDevice::Update()
{
	// The hardware information should be up to date
	// Update the sensors which will send out the data if changes have occured
	ProcessSensors();
}


void TInputDevice::ProcessSensors()
{
	for( u32 uIndex = 0; uIndex < m_InputBuffer.GetSize(); uIndex++ )
	{
		const TInputData& data = m_InputBuffer.ElementAt(uIndex);

		// Find the sensor with the required ID

		TPtr<TInputSensor>& pSensor = m_Sensors.FindPtr(data.m_SensorRef);

		if(pSensor.IsValid())
		{
			pSensor->Process(data.m_fData);
		}
		else
		{
#ifdef _DEBUG			
			TString str;
			str.Appendf("Unable to find sensor - %d ", data.m_SensorRef.GetData());			
			TString sensorstr;
			data.m_SensorRef.GetString(sensorstr);
			str.Append(sensorstr);
			TLDebug_Print(str);
#endif
			
			TLDebug::Break("Sensor not found");
		}
	}

	m_InputBuffer.Empty();
}

/*

// In non-buffered mode each input that comes in is the same amount as the number of sensors and 
// is also in the same order.
void TInputDevice::ProcessSensors()
{
#ifdef _DEBUG
	// Non-buffered mode the input buffer size should match the sensor size
	if(m_InputBuffer.GetSize() != m_Sensors.GetSize())
	{
		TLDebug::Break("Input buffer is different to number of sensors in non-buffered mode");
	}
#endif

	for( u32 uIndex = 0; uIndex < m_InputBuffer.GetSize(); uIndex++ )
	{
		const TInputData& data = m_InputBuffer.ElementAt(uIndex);

		// Get the sensor
		TPtr<TInputSensor> pSensor = m_Sensors.ElementAt(uIndex);

		pSensor->Process(data.m_fData);
	}

	m_InputBuffer.Empty();
}

*/


TPtr<TLInput::TInputSensor>& TInputDevice::AttachSensor(TRefRef SensorRef, TSensorType SensorType)
{
	if(!HasSensor(SensorRef))
	{
		TPtr<TInputSensor> pSensor = new TInputSensor(SensorRef, SensorType);

		if(pSensor.IsValid())
		{
			s32 SensorIndex = m_Sensors.Add(pSensor);

#ifdef _DEBUG
			TString str;
			str.Appendf("Attached sensor to device - %d %d - ", SensorRef.GetData(), SensorType);
			TString sensorstr;
			SensorRef.GetString(sensorstr);
			str.Append(sensorstr);
			TLDebug_Print(str);
#endif

			return m_Sensors[SensorIndex];
		}
	}

	return TLPtr::GetNullPtr<TLInput::TInputSensor>();
}

Bool TInputDevice::HasSensor(TRefRef SensorRef)
{
	TPtr<TInputSensor>& pSensor = GetSensor(SensorRef);

	return pSensor.IsValid();
}


TPtr<TLInput::TInputSensor>& TInputDevice::GetSensor(TRefRef SensorRef)
{
	for(u32 uIndex = 0; uIndex < m_Sensors.GetSize(); uIndex++)
	{
		TPtr<TInputSensor>& pSensor = m_Sensors.ElementAt(uIndex);

		if(pSensor->m_SensorRef == SensorRef)
			return pSensor;
	}

	return TLPtr::GetNullPtr<TLInput::TInputSensor>();
}


// Returns a sensor object for a given 'key' 'button' 'axis' reference or label
// To get the sensor for the key 'x' on a keyboard for example this routine should be used
// The sensor object returned is specific to the device so requesting 'x' on a mouse will return null, not found.
// On a gamepad, depending on the pad, it will return either null, not found, or the sensor object of a button labelled x.
TPtr<TLInput::TInputSensor>& TInputDevice::GetSensorFromLabel(TRefRef SensorLabelRef)
{
	for(u32 uIndex = 0; uIndex < m_Sensors.GetSize(); uIndex++)
	{
		TPtr<TInputSensor>& pSensor = m_Sensors.ElementAt(uIndex);

		if(pSensor->HasLabel(SensorLabelRef))
		{
			return pSensor;
		}
	}

	return TLPtr::GetNullPtr<TLInput::TInputSensor>();
}


Bool TInputDevice::GetSensorRefFromLabel(TRef SensorLabelRef, TRef& SensorRef)
{
	// Go through the sensors and look up whether any have the specified sensor ref as a label
	for(u32 uIndex = 0; uIndex < m_Sensors.GetSize(); uIndex++)
	{
		TPtr<TLInput::TInputSensor> pSensor = m_Sensors.ElementAt(uIndex);
		
		if(pSensor->HasLabel(SensorLabelRef))
		{
			SensorRef = pSensor->m_SensorRef;
			return TRUE;
		}
	}
	
	// Not found
	return FALSE;
}




u32	TInputDevice::GetSensorCount(TSensorType SensorType)
{
	u32 uCount = 0;

	for(u32 uIndex = 0; uIndex < m_Sensors.GetSize(); uIndex++)
	{
		const TPtr<TInputSensor>& pSensor = m_Sensors.ElementAtConst(uIndex);

		if(pSensor->m_Type == SensorType)
			uCount++;
	}

	return uCount;
}




TPtr<TLInput::TInputEffect>& TInputDevice::AttachEffect(TRef refEffectID)
{
	return TLPtr::GetNullPtr<TLInput::TInputEffect>();
}




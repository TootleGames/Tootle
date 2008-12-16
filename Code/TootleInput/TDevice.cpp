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
	GetDataBuffer()->ResetReadPos();
	ProcessSensors(GetDataBuffer());
}

void TInputDevice::ProcessSensors(TPtr<TBinaryTree>& MainBuffer)
{
	float fValue = 0.0f;
	TPtrArray<TBinaryTree> BufferArray;

	if(MainBuffer->GetChildren("Input", BufferArray))
	{
		for( u32 uIndex = 0; uIndex < BufferArray.GetSize(); uIndex++ )
		{
			TPtr<TBinaryTree> pDataBuffer = BufferArray.ElementAt(uIndex);

			if(pDataBuffer.IsValid())
			{
				pDataBuffer->ResetReadPos();

				for(u32 uIndex = 0; uIndex < m_Sensors.GetSize(); uIndex++)
				{
					if(pDataBuffer->Read(fValue))
					{
						TPtr<TInputSensor> pSensor = m_Sensors.ElementAt(uIndex);

						pSensor->Process(fValue);
					}
				}
			}
		}
	}
}


TPtr<TLInput::TInputSensor>	TInputDevice::AttachSensor(TRef refSensorID, TSensorType SensorType)
{
	if(!HasSensor(refSensorID))
	{
		TPtr<TInputSensor> pSensor = new TInputSensor(refSensorID, SensorType);

		if(pSensor.IsValid())
		{
			m_Sensors.Add(pSensor);
			return pSensor;
		}
	}

	return NULL;
}

Bool TInputDevice::HasSensor(TRef refSensorID)
{
	TPtr<TInputSensor> pSensor = GetSensor(refSensorID);

	return pSensor.IsValid();
}


TPtr<TLInput::TInputSensor>	TInputDevice::GetSensor(TRef refSensorID)
{
	for(u32 uIndex = 0; uIndex < m_Sensors.GetSize(); uIndex++)
	{
		TPtr<TInputSensor> pSensor = m_Sensors.ElementAt(uIndex);

		if(pSensor->m_refSensorID == refSensorID)
			return pSensor;
	}

	return TPtr<TInputSensor>(NULL);
}


// Returns a sensor index of a given sensor 'key' 'button' 'axis' reference or label
// To get the sensor index for the key 'x' on a keyboard for example this routine should be used
// The index returned is specific to the device so requesting 'x' on a mouse will return -1, not found.
// On a gamepad, depending on the pad, it will return either -1, not found, or the index of a button labelled x.
s32 TInputDevice::GetSensorIndex(TRef refSensorLabel)
{
	// Go through the sensors and look up whether any have the specified sensor ref as a label
	for(u32 uIndex = 0; uIndex < m_Sensors.GetSize(); uIndex++)
	{
		TPtr<TLInput::TInputSensor> pSensor = m_Sensors.ElementAt(uIndex);

		if(pSensor->GetLabel() == refSensorLabel)
			return (s32) uIndex;
	}

	return -1;
}

/*
TRef TInputDevice::GetSensorID(TRef refSensorLabel)
{
	// Go through the sensors and look up whether any have the specified sensor ref as a label
	for(u32 uIndex = 0; uIndex < m_Sensors.GetSize(); uIndex++)
	{
		TPtr<TLInput::TInputSensor> pSensor = m_Sensors.ElementAt(uIndex);
		
		if(pSensor->GetLabel() == refSensorLabel)
			return pSensor->m_refSensorID;
	}
	
	return TRef(0);
}
 */




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




TPtr<TLInput::TInputEffect>	TInputDevice::AttachEffect(TRef refEffectID)
{
	return NULL;
}




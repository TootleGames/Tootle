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
	BufferArray.SetAllocSize(100);

	if(MainBuffer->GetChildren("Input", BufferArray))
	{
		for( u32 uIndex = 0; uIndex < BufferArray.GetSize(); uIndex++ )
		{
			TBinaryTree* pDataBuffer = BufferArray.ElementAt(uIndex).GetObject();

			if(!pDataBuffer)
				continue;

			pDataBuffer->ResetReadPos();

			for(u32 uIndex = 0; uIndex < m_Sensors.GetSize(); uIndex++)
			{
				if(pDataBuffer->Read(fValue))
				{
					TPtr<TInputSensor>& pSensor = m_Sensors.ElementAt(uIndex);

					pSensor->Process(fValue);
				}
			}
		}
	}
}


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
			str.Appendf("Attached sensor to device - %d %d", SensorRef, SensorType);
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


// Returns a sensor index of a given sensor 'key' 'button' 'axis' reference or label
// To get the sensor index for the key 'x' on a keyboard for example this routine should be used
// The index returned is specific to the device so requesting 'x' on a mouse will return -1, not found.
// On a gamepad, depending on the pad, it will return either -1, not found, or the index of a button labelled x.
s32 TInputDevice::GetSensorIndex(TRefRef refSensorLabel)
{
	// Go through the sensors and look up whether any have the specified sensor ref as a label
	for(u32 uIndex = 0; uIndex < m_Sensors.GetSize(); uIndex++)
	{
		TPtr<TLInput::TInputSensor>& pSensor = m_Sensors.ElementAt(uIndex);

		if(pSensor->HasLabel(refSensorLabel))
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
		
		if(pSensor->HasLabel(refSensorLabel))
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




TPtr<TLInput::TInputEffect>& TInputDevice::AttachEffect(TRef refEffectID)
{
	return TLPtr::GetNullPtr<TLInput::TInputEffect>();
}




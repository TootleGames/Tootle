#include "TSensor.h"

#include <TootleCore/TLMaths.h>

#ifdef _DEBUG
//#define ENABLE_INPUTSENSOR_TRACE
#endif

using namespace TLInput;

void TInputSensor::ProcessMessage(TPtr<TLMessaging::TMessage>& pMessage)
{
	if(pMessage->HasChannelID(m_refSensorID))
	{
		float fValue;

		pMessage->Read(fValue);

		Process(fValue);
	}
}


void TInputSensor::Process(float fRawValue)
{
	float fValue = fRawValue;
	// Data within range?
//	if((fValue < m_fRangeLow) || (fValue > m_fRangeHigh))
//		fValue = 0.0f;// Out of range
	//Limit to within bounds
	TLMaths::Limit(fValue, m_fRangeLow, m_fRangeHigh);
	
	if(fabsf(fValue) <= m_fDeadZone) 		// Check deadzone
		fValue = 0.0f; // below deadzone threshold

	// Change in value?
	if(m_fValue != fValue)
	{
#ifdef ENABLE_INPUTSENSOR_TRACE
		TString sensor;
		m_refSensorID.GetString(sensor);
		
		TString sensorlabel;
		m_refSensorLabel.GetString(sensorlabel);
		
		TString str;
		str.Appendf("Sensor %s (%s) value changed - %.3f", sensor.GetData(), sensorlabel.GetData(), fValue);
		TLDebug_Print(str);
#endif
		
		TPtr<TLMessaging::TMessage> pMessage = new TLMessaging::TMessage("Input");

		// Relay message to all subscribers
		if(pMessage.IsValid())
		{
			pMessage->Write(fValue);

			// Add the raw data to the message - for things that may need access to this information
			pMessage->AddChildAndData("RAWDATA", fRawValue);

			// Add the sensor type so things know what type of sensor sent the information
			//pMessage->AddChildAndData("SENSOR", m_Type);
					
			PublishMessage(pMessage);

			m_fValue = fValue;
		}
	}
}
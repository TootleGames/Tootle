#include "TSensor.h"

#include <TootleCore/TLMaths.h>

#ifdef _DEBUG
	//#define ENABLE_INPUTSENSOR_TRACE
#endif

using namespace TLInput;

void TInputSensor::ProcessMessage(TLMessaging::TMessage& Message)
{
	if(Message.GetMessageRef() == m_SensorRef)
	{
		float fValue;
		if ( Message.Read(fValue) )
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
		TTempString Debug_SensorString("Sensor ");
		m_SensorRef.GetString(Debug_SensorString);
		Debug_SensorString.Append(" (");
		for ( u32 l=0;	l<m_SensorLabels.GetSize();	l++ )
		{
			m_SensorLabels[l].GetString(Debug_SensorString);
			if ( l != m_SensorLabels.GetLastIndex() )
				Debug_SensorString.Append(", ");
		}
		Debug_SensorString.Appendf(") value changed; %.3f", fValue);
		TLDebug_Print( Debug_SensorString );
#endif
		
		TLMessaging::TMessage Message("OnInput");

		// Relay message to all subscribers
		Message.Write(fValue);

		// Add the raw data to the message - for things that may need access to this information
		Message.ExportData("RAWDATA", fRawValue);
		
#if defined(TL_TARGET_IPOD) || defined(TL_TARGET_IPAD)
		
		Message.ExportData("CIDX", GetCursorIndex());
		
#endif
		
		// Add the sensor type so things know what type of sensor sent the information
		//Message.ExportData("SENSOR", m_Type);
				
		PublishMessage(Message);

		m_fValue = fValue;
	}
}


#pragma once

#include <TootleCore/TRelay.h>

namespace TLInput
{
	class TInputSensor;

	typedef enum TSensorType
	{
		Unknown = 0,
		Button,
		Axis,
		POV,
	};
}

/*
	Base sensor input class.  
	Used to represent sensors on a physical device within the generic device class
*/
class TLInput::TInputSensor : public TLMessaging::TRelay
{
	friend class TInputDevice;
public:
	explicit TInputSensor(TRef refSensorID, TSensorType SensorType)	:
		m_refSensorID(refSensorID),
			m_Type(SensorType),
			m_fValue(0.0f),
			m_fRangeLow(-1.0f),
			m_fRangeHigh(1.0f),
			m_fDeadZone(0.0f)
	{}

	inline Bool			operator==(const TRef& SensorRef)				const	{	return m_refSensorID == SensorRef;	}
	inline Bool			operator==(const TInputSensor& InputSensor)		const 	{	return m_refSensorID == InputSensor.m_refSensorID;	}

	inline void			SetLabel(const TRef& refLabel)		{ m_refSensorLabel = refLabel; }
	inline TRef			GetLabel()					const	{ return m_refSensorLabel; }
	
	inline void			SetRanges(float fLow, float fHigh)	{ m_fRangeLow = fLow; m_fRangeHigh = fHigh; }
	inline void			SetDeadZone(float fZone)			{ m_fDeadZone = fZone; }

protected:

	virtual void	ProcessMessage(TPtr<TLMessaging::TMessage>& pMessage);

	void			Process(float fValue);

public:

	TRef		m_refSensorID;			// ID of the sensor - maps to hardware device sensor index, NOTE: Not sure this is totally needed?
	TRef		m_refSensorLabel;		// Key type label - 'x', 'ctrl' 'start' 'axis1'.  Might be slow to use :(
	TSensorType	m_Type;					// Sensor type - button, axes etc
	float		m_fValue;				// Current value
	float		m_fRangeLow;			// Lowest possible value
	float		m_fRangeHigh;			// Highest possible value
	float		m_fDeadZone;			// Absolute deadzone value - values < m_fDeadZone are ignored.  0.0f ( deadzone < range low == not used)
};


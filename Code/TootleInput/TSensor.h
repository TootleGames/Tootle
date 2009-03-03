
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
	explicit TInputSensor(TRefRef SensorRef, TSensorType SensorType)	:
		m_SensorRef(SensorRef),
			m_Type(SensorType),
			m_fValue(0.0f),
			m_fRangeLow(-1.0f),
			m_fRangeHigh(1.0f),
			m_fDeadZone(0.0f)
	{}

	inline Bool			operator==(const TRef& SensorRef)				const	{	return m_SensorRef == SensorRef;	}
	inline Bool			operator==(const TInputSensor& InputSensor)		const 	{	return m_SensorRef == InputSensor.m_SensorRef;	}

	inline void			AddLabel(const TRef& refLabel)		
	{ 
		m_SensorLabels.Add(refLabel); 
#ifdef _DEBUG
		// Print out the label being set
		TString inputinfo = "Sensor label added: ";
		TString label;
		refLabel.GetString(label);
		inputinfo.Append(label);
		TLDebug::Print(inputinfo);
#endif
	}

	inline Bool			HasLabel(TRefRef LabelRef)	{ return m_SensorLabels.Exists(LabelRef); }
	
	inline void			SetRanges(float fLow, float fHigh)	{ m_fRangeLow = fLow; m_fRangeHigh = fHigh; }
	inline void			SetDeadZone(float fZone)			{ m_fDeadZone = fZone; }

protected:

	virtual void	ProcessMessage(TLMessaging::TMessage& Message);

	void			Process(float fValue);

public:

	TRef			m_SensorRef;			// ID of the sensor - maps to hardware device sensor index, NOTE: Not sure this is totally needed?
	TArray<TRef>	m_SensorLabels;			// Key type labels - 'x', 'ctrl' 'start' 'axis1'.
	TSensorType		m_Type;					// Sensor type - button, axes etc
	float			m_fValue;				// Current value
	float			m_fRangeLow;			// Lowest possible value
	float			m_fRangeHigh;			// Highest possible value
	float			m_fDeadZone;			// Absolute deadzone value - values < m_fDeadZone are ignored.  0.0f ( deadzone < range low == not used)
};


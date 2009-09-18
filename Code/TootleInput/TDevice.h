#pragma once

#include <TootleCore/TFlags.h>
#include <TootleCore/TPtrArray.h>
#include <TootleCore/TRelay.h>
#include <TootleCore/TRef.h>

#include "TSensor.h"
#include "TEffect.h"

namespace TLInput
{
	class TInputData;

	class TInputDevice;
	class TInputManager;

	namespace DeviceFlags
	{
		const u32 Attached = 0;		// Signifies whether the device is attached
		const u32 Virtual = 1;		// Specifies a virtual device i.e. one without a physical counterpart
	};

	const TRef KeyboardRef		= TRef_Static(K,e,y,b,o); // "Keyboard"
	const TRef MouseRef			= TRef_Static(M,o,u,s,e); // "Mouse"
	const TRef GamepadRef		= TRef_Static(G,a,m,e,p); // "Gamepad"
	const TRef TrackpadRef		= TRef_Static(T,r,a,c,k); // "Trackpad"

}

/*
	Structure use for the input data for a specific sensor object button/axis/POV etc
*/
class TLInput::TInputData
{
public:
	TRef	m_SensorRef;			// The ref of the input device sensor
	float	m_fData;				// Data for the sensor
};

//TLCore_DeclareIsDataType(TInputData);

/*
	Generic input device that will be created per physical device
	When created it will have 'sensor' objects attached so that we can accurately 
	represent any type of input device rather than assuming a certain setup.
	Note: Not sure whether this object should be a class factory as well??
*/
class TLInput::TInputDevice : public TLMessaging::TRelay
{
	friend class TLInput::TInputManager;
public:
	TInputDevice(TRefRef DeviceRef,TRefRef DeviceTypeRef) :
	  	m_DeviceRef		( DeviceRef ),
		m_DeviceType	( DeviceTypeRef )
	{
	}

	TRefRef				GetDeviceRef() const			{	return m_DeviceRef;	}
	TRefRef				GetHardwareDeviceID() const		{	return m_HardwareDeviceRef;	}
	TRefRef				GetDeviceType() const			{	return m_DeviceType;	}

	inline Bool			operator==(TRefRef InputRef) const					{	return GetDeviceRef() == InputRef;	}
	inline Bool			operator==(const TInputDevice& InputDevice) const 	{	return GetDeviceRef() == InputDevice.GetDeviceRef();	}

	FORCEINLINE void	SetAttached(Bool bAttached)			{	m_Flags.Set(DeviceFlags::Attached, bAttached);	}
	FORCEINLINE Bool	IsAttached()	const				{	return m_Flags.IsSet(DeviceFlags::Attached);	}

	FORCEINLINE Bool	IsVirtual()	const				{	return m_Flags.IsSet(DeviceFlags::Virtual);	}

	FORCEINLINE Bool	AssignToHardwareDevice(TRefRef HardwareDeviceRef);

	// Sensor access
	TPtr<TLInput::TInputSensor>&	AttachSensor(TRefRef SensorRef, TSensorType SensorType);
	Bool							HasSensor(TRefRef SensorRef)	{	return m_Sensors.Exists( SensorRef );	}
	TPtr<TLInput::TInputSensor>&	GetSensor(TRefRef SensorRef)	{	return m_Sensors.FindPtr( SensorRef );	}

	TPtr<TLInput::TInputSensor>&	GetSensorFromLabel(TRefRef SensorLabelRef);
	Bool							GetSensorRefFromLabel(TRef SensorLabelRef, TRef& SensorRef);

	u32								GetSensorCount(TSensorType SensorType);
	u32								GetTotalSensorCount()		const	{ return m_Sensors.GetSize(); }

	// Effects access
	TPtr<TLInput::TInputEffect>&	AttachEffect(TRefRef EffectRef, TRefRef TypeRef);

	// Data buffer access - sensor information from hardware
	TArray<TInputData>&				GetInputBuffer()				{ return m_InputBuffer; }

	void							Debug_GetSensorLabels(TArray<TRef>& LabelRefs);		//	get a list of labels availible for all the sensors on this device

private:
	void						Update();

	void						ProcessSensors();

	void						RemoveAllSensors()				{}
	void						RemoveAllEffects()				{}

private:
	TRef						m_DeviceRef;			// external input device ref - ID of the generic input device object
	TRef						m_HardwareDeviceRef;	// internal hardware ref - Reference to the physical device
	TRef						m_DeviceType;			//	device type

	TArray<TInputData>			m_InputBuffer;			// Buffer for all input from the hardware device
	TPtrArray<TInputSensor>		m_Sensors;				// List of sensors that the device has access to
	TPtrArray<TInputEffect>		m_Effects;				// List of (output) effects such as force feedback (rumble) and audio

	TFlags<u32>					m_Flags;				// Device flags
};




FORCEINLINE Bool TLInput::TInputDevice::AssignToHardwareDevice(TRefRef HardwareDeviceRef)	
{
	if ( GetHardwareDeviceID().IsValid() )
		return FALSE;

	m_HardwareDeviceRef = HardwareDeviceRef;
	return TRUE;
}



#include "TAction.h"



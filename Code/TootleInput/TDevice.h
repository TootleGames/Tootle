#pragma once

#include <TootleCore/TFlags.h>
#include <TootleCore/TPtrArray.h>
#include <TootleCore/TRelay.h>

#include "TSensor.h"
#include "TEffect.h"

namespace TLInput
{
	class TInputDevice;
	class TInputManager;

	namespace DeviceFlags
	{
		const u32 Attached = 0;		//	gr: TFlags work off indexes, not bits. bits are internal to the TFlags class
	};
}

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
		m_pDeviceDataBuffer = new TBinaryTree("Buffer");
	}

	~TInputDevice()
	{
		m_pDeviceDataBuffer = NULL;
	}

	TRefRef				GetDeviceRef() const			{	return m_DeviceRef;	}
	TRefRef				GetHardwareDeviceID() const		{	return m_HardwareDeviceRef;	}
	TRefRef				GetDeviceType() const			{	return m_DeviceType;	}

	inline Bool			operator==(const TRef& InputRef) const				{	return GetDeviceRef() == InputRef;	}
	inline Bool			operator==(const TInputDevice& InputDevice) const 	{	return GetDeviceRef() == InputDevice.GetDeviceRef();	}

	inline Bool			AssignToHardwareDevice(TRefRef HardwareDeviceRef)
	{
		if ( GetHardwareDeviceID().IsValid() )
			return FALSE;

		m_HardwareDeviceRef = HardwareDeviceRef;
		return TRUE;
	}

	inline void							SetAttached(Bool bAttached)
	{
		m_Flags.Set(DeviceFlags::Attached, bAttached);
	}

	inline Bool							IsAttached()	const
	{
		return m_Flags.IsSet(DeviceFlags::Attached);
	}

	// Sensor access
	TPtr<TLInput::TInputSensor>	AttachSensor(TRef refSensorID, TSensorType SensorType);
	Bool						HasSensor(TRef refSensorID);
	TPtr<TLInput::TInputSensor>	GetSensor(TRef refSensorID);

	s32							GetSensorIndex(TRef refSensorLabel);

	u32							GetSensorCount(TSensorType SensorType);

	// Effects access
	TPtr<TLInput::TInputEffect>		AttachEffect(TRef refEffectID);

	// Data buffer access - sensor information from hardware
	TPtr<TBinaryTree>&				GetDataBuffer()				{ return m_pDeviceDataBuffer; }

	// [24 11 08] DB - On the iPod we may process multiple 'events' in a frame so need to be able to manually
	// call the update form the ipod code.  This will need changing at some point so that all platforms work the 
	// same way and without explicit calls to the update.
#ifdef TL_TARGET_IPOD
	void							ForceUpdate()
	{
		Update();
	}
#endif
	
private:
	void						Update();

	void						ProcessSensors(TPtr<TBinaryTree>& dataBuffer);

	void						RemoveAllSensors()				{}
	void						RemoveAllEffects()				{}

private:
	TRef						m_DeviceRef;			// external input device ref - ID of the generic input device object
	TRef						m_HardwareDeviceRef;	// internal hardware ref - Reference to the physical device
	TRef						m_DeviceType;			//	device type

	TPtr<TBinaryTree>			m_pDeviceDataBuffer;	// Buffer for all input from the hardware device
	TPtrArray<TInputSensor>		m_Sensors;				// List of sensors that the device has access to
	TPtrArray<TInputEffect>		m_Effects;				// List of (output) effects such as force, feedback (rumble) and audio

	TFlags<u32>					m_Flags;				// Device flags
};

#include "TAction.h"

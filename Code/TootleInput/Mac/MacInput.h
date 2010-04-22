#pragma once

#include <TootleCore/TLCore.h>

#include "../TLInput.h"

#import <IOKit/hid/IOHIDManager.h>


namespace TLInput
{
	namespace Platform
	{
		namespace HID
		{	
			SyncBool	Init();				// Init HID input
			SyncBool	Shutdown();			// Cleanup HID input
			
			void		RemoveAllDevices();

			class	TLInputHIDDevice;
			
			Bool InitialiseDevice(TPtr<TInputDevice> pDevice, TRefRef DeviceTypeRef, Bool bVirtual);
			
			Bool InitialiseDevice(TPtr<TInputDevice> pDevice, const IOHIDDeviceRef device);
			
		}
		
		//	gr: I can't seem to inline these (not found in linker). If I don't inline, I get multiple references... 
		//	todo: sort it so they're not stripped as we want them inlined
		SyncBool	Init();
		SyncBool	Update();
		SyncBool	Shutdown();
		
		// On the Mac the enumeration of devices is done automatically by registering callbacks and 
		// scheduling the callbacks with the runloop.  We therefore don't need to explicitly re-enumerate devices
		// like on Windows.		
		SyncBool	EnumerateDevices();
		void		RemoveAllDevices();
		
		Bool			UpdateDevice(TInputDevice& Device);
		int2			GetCursorPosition(u8 uIndex);				
		
		SyncBool		CreateVirtualDevice(TRefRef InstanceRef, TRefRef DeviceTypeRef);
		SyncBool		RemoveVirtualDevice(TRefRef InstanceRef);

		// TEMP test routine
		void TestVibrateDevice();
				
	};
};

class TLInput::Platform::HID::TLInputHIDDevice : public TLMessaging::TPublisher
{
public:
	
	TLInputHIDDevice(TRefRef DeviceRef, IOHIDDeviceRef HIDDevice) :
	m_DeviceRef(DeviceRef),
	m_HIDDevice(HIDDevice),
	m_uDeviceType(0),
	m_SensorDataQueue(NULL)
	{
		// Create data buffer?
		// register call back and init runloop schedule?
	}
	
	~TLInputHIDDevice();
	
	virtual TRefRef		GetPublisherRef() const	{	static TRef Ref("Hid");	return Ref; }
	
	FORCEINLINE Bool			operator==(TRefRef HIDDeviceRef)							const	{	return GetDeviceRef() == HIDDeviceRef;	}
	FORCEINLINE Bool			operator==(const TLInputHIDDevice& HIDDevice)	const 	{	return GetDeviceRef() == HIDDevice.GetDeviceRef();	}
	
	
	FORCEINLINE const	TRef&					GetDeviceRef()														const	{ return m_DeviceRef; }
	FORCEINLINE const 	IOHIDDeviceRef			GetHIDDevice()														const	{ return m_HIDDevice; }
	
	FORCEINLINE const	u32						GetDeviceType()														const	{ return m_uDeviceType; }
	FORCEINLINE void							SetDeviceType(u32 uDeviceType)										{ m_uDeviceType = uDeviceType; }		
		
	void										PublishData(u32 uUniqueID, float fValue);
	
	FORCEINLINE void							SetProductID(u32 uProductID)			{ m_uProductID = uProductID; }
	FORCEINLINE u32								GetProductID()					const	{ return m_uProductID; }
		
	FORCEINLINE void							SetSensorQueue(IOHIDQueueRef sensorQueue)			{ m_SensorDataQueue = sensorQueue;}
	FORCEINLINE const IOHIDQueueRef&			GetSensorQueue()							const	{ return m_SensorDataQueue; }

	Bool										EnumerateObjects();
protected:	
	
	void										EnumDeviceObject(IOHIDElementRef elementRef);

private:
	TRef									m_DeviceRef;
	IOHIDDeviceRef							m_HIDDevice;
	u32										m_uDeviceType;
	u32										m_uProductID;
	
	IOHIDQueueRef							m_SensorDataQueue;
};



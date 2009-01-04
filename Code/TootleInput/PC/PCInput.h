
#pragma once

#include <TootleCore/TLCore.h>

#include "../TLInput.h"

#pragma comment( lib, "dinput8.lib" )
#pragma comment( lib, "dxguid.lib" )
#define DIRECTINPUT_VERSION 0x0800		// Need to define the version of direct input we will use
#include <dinput.h>

namespace TLInput
{
	namespace Platform
	{
		namespace DirectX
		{
			SyncBool	Init();				// Init directx input
			SyncBool	Shutdown();			// Cleanup directx input

			SyncBool	EnumerateDevices();	// Enumerate devices
			void		RemoveAllDevices();	// Remove all devices

			BOOL CALLBACK CreateDevice(const DIDEVICEINSTANCE* pdidInstance, VOID* pContext);		// Create device callback when the devices are enumerated
			BOOL CALLBACK EnumDeviceObject(const DIDEVICEOBJECTINSTANCE* pdidObjectInstance, VOID* pContext);	// Object enumeration callback - enumerates buttons, axes, pov's effects etc
			Bool InitialiseDevice(TPtr<TInputDevice> pDevice, LPDIRECTINPUTDEVICE8 lpdiDevice, u32 uDeviceType);

			Bool	UpdateDevice(TPtr<TInputDevice> pDevice);

			class TLInputDirectXDevice;

			extern LPDIRECTINPUT8									g_pTLDirectInputInterface;	// Global direct input interface pointer
			extern TPtrArray<TLInputDirectXDevice>				g_TLDirectInputDevices;		// Global array of direct device pointers

			/*
			Special update routines for devices in DirectX.  Hopefully at some point I can remove these as this shouldn't really be necessary.
			*/
			Bool UpdateDirectXDevice_Mouse(TPtr<TInputDevice> pDevice, TPtr<TLInputDirectXDevice> pTLDirectInputDevice);
			Bool UpdateDirectXDevice_Keyboard(TPtr<TInputDevice> pDevice, TPtr<TLInputDirectXDevice> pTLDirectInputDevice);
			Bool UpdateDirectXDevice_Gamepad(TPtr<TInputDevice> pDevice, TPtr<TLInputDirectXDevice> pTLDirectInputDevice);
		}


		SyncBool		Init()					{	return DirectX::Init();	}
		SyncBool		Update()				{	return SyncTrue;	}
		SyncBool		Shutdown()				{	return DirectX::Shutdown();	}

		SyncBool		EnumerateDevices()		{ return DirectX::EnumerateDevices(); }
		void			RemoveAllDevices()		{ DirectX::RemoveAllDevices(); }

		Bool			UpdateDevice(TPtr<TInputDevice> pDevice);
		int2			GetCursorPosition(u8 uIndex);
	};
};

class TLInput::Platform::DirectX::TLInputDirectXDevice : public TLMessaging::TPublisher
{
public:

	TLInputDirectXDevice(TRef refID, LPDIRECTINPUTDEVICE8 pDirectXDevice) :
		m_DeviceID(refID),
		m_pDevice(pDirectXDevice),
		m_uDeviceType(0)
	{
	}

	~TLInputDirectXDevice()
	{
			// Release the directx device
			if(m_pDevice)
			{
				m_pDevice->Unacquire(); 
				m_pDevice->Release(); 
				m_pDevice = NULL; 
			}
	}

	const	TRef&								GetDeviceID()															const	{ return m_DeviceID; }
	const 	LPDIRECTINPUTDEVICE8				GetDevice()															const	{ return m_pDevice; }

	const	u32									GetDeviceType()														const	{ return m_uDeviceType; }
	inline void									SetDeviceType(u32 uDeviceType)										{ m_uDeviceType = uDeviceType; }		

	const	DIDATAFORMAT&						GetDataFormat()													const	{ return *m_pDataFormat; }
	inline void									SetDataFormat(const DIDATAFORMAT* pDataFormat)				{ m_pDataFormat = const_cast<DIDATAFORMAT*>(pDataFormat); }

	void										PublishData(u32 uUniqueID, float fValue);

private:
	TRef									m_DeviceID;
	LPDIRECTINPUTDEVICE8					m_pDevice;
	u32										m_uDeviceType;
	DIDATAFORMAT*							m_pDataFormat;
};

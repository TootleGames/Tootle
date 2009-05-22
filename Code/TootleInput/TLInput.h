

#pragma once

#include <TootleCore/TLTypes.h>
#include <TootleCore/TClassFactory.h>
#include <TootleCore/TRelay.h>
#include <TootleCore/TManager.h>


#include "TDevice.h"

/*
DB - Don't want a UID database just yet but we may eventually need one for more verbose ID's
// UID's database
namespace UID
{
	// List of unique actions the game will use as u32 values - string based TRef's don't give enough reolution
	//const u32 Action					= 0x00010000;
	const u32 Action_Camera_Move_Up		= 0x00010001;
	const u32 Action_Camera_Move_Down	= 0x00010002;
	const u32 Action_Camera_Move_Left	= 0x00010003;
	const u32 Action_Camera_Move_Right	= 0x00010004;
}
*/

namespace TLInput
{
	class TInputManager;

	extern 	TPtr<TInputManager>	g_pInputSystem;	// The global input system

	namespace Platform
	{
		SyncBool	Init();
		SyncBool	Update();
		SyncBool	Shutdown();

		SyncBool	EnumerateDevices();
		void		RemoveAllDevices();

		Bool		UpdateDevice(TInputDevice& Device);

		Type2<s32>	GetCursorPosition(u8 uIndex);
		
		Bool			CreateVirtualKeyboard();
		Bool			DestroyVirtualKeyboard();

	};


	// Helper routines
	TRef		GetDefaultAxisRef(u32 uObjectIndex);
	TRef		GetDefaultAxisRef(u32 AxisIndex,const char AxisChar);
	TRef		GetDefaultButtonRef(u32 uObjectIndex);
	TRef		GetDefaultPOVRef(u32 uObjectIndex);

	TPtr<TLInput::TInputDevice>&	GetDevice(TRefRef DeviceRef);			//	fetch device with this ref
	TPtr<TLInput::TInputDevice>&	GetDeviceOfType(TRefRef DeviceType);	//	find the (first) device of this type
	TRef							GetFreeDeviceRef(TRef BaseRef=TRef());	//	get an unused ref for a device
	
	// Virtual device creation
	Bool			CreateVirtualKeyboard();
	Bool			DestroyVirtualKeyboard();
};


/*
	Input system/management.  Keeps track of the input devices and notifies subscribers of input based events.
*/
class TLInput::TInputManager : public TManager, public TClassFactory<TLInput::TInputDevice> 
{
public:
	TInputManager(TRef refManagerID);

	~TInputManager()
	{
	}

	Bool					GetDeviceIDs(TArray<TRef>& refArray);
	
	FORCEINLINE void		ForceUpdate()
	{
		Update(0.0f);
	}

	FORCEINLINE Bool		IsEnabled()		const		{ return m_bEnabled; }

protected:
	virtual SyncBool		Initialise();
	virtual SyncBool		Update(float fTimeStep);
	virtual SyncBool		Shutdown();

	virtual void			ProcessMessage(TLMessaging::TMessage& Message);
	virtual void			OnEventChannelAdded(TRefRef refPublisherID, TRefRef refChannelID);	

private:
	void					RemoveAllDevices();

	void					CheckForDeviceChanges();

	virtual TLInput::TInputDevice*		CreateObject(TRefRef InstanceRef,TRefRef TypeRef);

private:
	float					m_fDeviceCheckTimer;		// Interval timer between checks for devices
	Bool					m_bEnabled;
};



#pragma once

#include <TootleCore/TLTypes.h>
#include <TootleCore/TClassFactory.h>
#include <TootleCore/TRelay.h>
#include <TootleCore/TManager.h>
#include <TootleCore/TKeyArray.h>

#include "TDevice.h"


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

		SyncBool		CreateVirtualDevice(TRefRef InstanceRef, TRefRef DeviceTypeRef);
		SyncBool		RemoveVirtualDevice(TRefRef InstanceRef);
		
		//TEMP test routine
		void TestVibrateDevice();
	};


	// Helper routines
	TRef		GetDefaultAxisRef(u32 uObjectIndex);
	TRef		GetDefaultAxisRef(u32 AxisIndex,const char AxisChar);
	TRef		GetDefaultButtonRef(u32 uObjectIndex);
	TRef		GetDefaultPOVRef(u32 uObjectIndex);

	TPtr<TLInput::TInputDevice>&	GetDevice(TRefRef DeviceRef);			//	fetch device with this ref
	TPtr<TLInput::TInputDevice>&	GetDeviceOfType(TRefRef DeviceType);	//	find the (first) device of this type
	TRef							GetFreeDeviceRef(TRef BaseRef=TRef());	//	get an unused ref for a device
		
	
	// TEMP test routine
	void TempTriggerVibration();
};


/*
	Input system/management.  Keeps track of the input devices and notifies subscribers of input based events.
*/
class TLInput::TInputManager : public TLCore::TManager, public TClassFactory<TLInput::TInputDevice> 
{
private:
	class TVirtualDeviceRequest
	{
	public:
		TRef m_DeviceInstanceRef;		// Device instance ID
		TRef m_DeviceTypeRef;			// Device type, keyboard, etc
		Bool m_bCreate;					// True - Create, otherwise Remove
	};
	
public:
	TInputManager(TRefRef ManagerRef);
	~TInputManager()		{	}

	Bool					GetDeviceIDs(TArray<TRef>& refArray);
	
	FORCEINLINE void		ForceUpdate()
	{
		Update(0.0f);
	}

	FORCEINLINE Bool		IsEnabled()		const		{ return m_bEnabled; }
	
	// Input character support
	Bool			IsSupportedInputCharacter(const char& character);
	Bool			GetSupportedInputCharacter(TRefRef CharacterRef, char& character);
	Bool			GetSupportedInputCharacterRef(TRef& CharacterRef, const char& character);
	Bool			BuildArrayOfSupportInputCharacterRefs(TArray<TRef>& array);

	// Virtual device handling
	Bool			CreateVirtualDevice(TRefRef InstanceRef, TRefRef DeviceTypeRef);
	Bool			RemoveVirtualDevice(TRefRef InstanceRef);
	
	
	// [02/06/09] DB - Dirty hack to setup the initial virtual keyboard text. Could do with being able to send this data to
	// the device but not a nice way to do that yet
	void			SetVirtualKeyboardText(const TString& vktext)
	{
		m_VKText = vktext;
	}
	
	TString		GetVirtualKeyboardText() const
	{
		return m_VKText;
	}
	
protected:
	virtual SyncBool		Initialise();
	virtual SyncBool		Update(float fTimeStep);
	virtual SyncBool		Shutdown();

	virtual void			ProcessMessage(TLMessaging::TMessage& Message);
	virtual void			OnEventChannelAdded(TRefRef refPublisherID, TRefRef refChannelID);	

private:
	
	void					InitSupportedTextCharacters();
	
	void					RemoveAllDevices();

	void					CheckForDeviceChanges();

	virtual TLInput::TInputDevice*		CreateObject(TRefRef InstanceRef,TRefRef TypeRef);

private:
	// Array of supported input 'text' characters mapped to an input 'key' ref to represent the character.  Actions will in turn be mapped to the key ref.
	// NOTE: Should be Unicode? UTF8? Possibly?
	TKeyArray<TRef, char>				m_SupportedTextCharacters;
	TArray<TVirtualDeviceRequest>		m_VirtualDeviceRequests;

	TString								m_VKText;
	
	float					m_fDeviceCheckTimer;		// Interval timer between checks for devices
	Bool					m_bEnabled;
};

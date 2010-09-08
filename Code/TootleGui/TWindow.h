/*
 *  TWindow.h
 *  TootleGui
 *
 *  Created by Graham Reeves on 14/01/2010.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */
#pragma once
#include "TLGui.h"
#include "TControl.h"
#include <TootleCore/TPtrArray.h>
#include <TootleMaths/TBox.h>
#include <TootleCore/TPublisher.h>

//------------------------------------------------------
//	base window type for an OS window.
//------------------------------------------------------
class TLGui::TWindow : public TLMessaging::TPublisher
{
	friend class TLGui::TControl;
public:
	TWindow(TRefRef WindowRef);

	virtual TRefRef			GetPublisherRef() const				{	return m_WindowRef;	}

	virtual Bool			IsVisible() const=0;				//	get current visibility of the window
	virtual void			Show()=0;							//	show the window. Currently we're just exposing the ability to delay the showing, rather than exposing hiding and showing

	virtual void			SetSize(const Type2<u16>& WidthHeight)=0;	//	set the client size of the window (game doesn't care about the real size, only the client size)
	virtual Type2<u16>		GetSize()=0;								//	get the client size of the window
	virtual void			SetPosition(const Type2<u16>& xy)=0;		//	set the window's top left position
	virtual Type2<u16>		GetPosition() const=0;						//	get the window's top left position
	
	virtual TMenuHandler*	GetMenuHandler()					{	return NULL;	}	//	get the menu handler for the window.
	
#if defined(TL_TARGET_PC)
	virtual void*			GetHandle() const=0;				//	get the win32 window handle (HWND)
#endif
	DEPRECATED const Type2<u16>&	GetLastMousePos() const			{	return m_LastMousePos;	}


	inline Bool				operator==(TRefRef WindowRef) const		{	return m_WindowRef == WindowRef;	}
	
protected:	//	for derivitives
	virtual void			OnAddedChild(TControl& Control)	{} 
	void					OnSizeChanged() const;					//	when [client] size of the window changes, broadcast a message 
	bool					OnMouseInput(u32 ButtonIndex,const Type2<u16>& Position,SyncBool Down);	//	mouse input, if Down=Wait then the input is a mouse move
	bool					OnKeyInput(TRef KeySensor,bool Down);									//	key input
	
protected:	//	for friends
	void					AddChild(TControl& Control)				{	m_Controls.Add( &Control );		OnAddedChild(Control);	}
	
protected:	//	for derivitives
	TPointerArray<TControl>	m_Controls;		//	child controls. Todo: change to TPtr's when we have intrusive ref counters
	
private:
	bool					CreateInputDevices();					//	create window-orientated input devices

private:
	TRef					m_Mouse;			//	mouse device for this window
	Type2<u16>				m_LastMousePos;		//	last-known mouse pos 
	TRef					m_Keyboard;			//	keyboard device for this window
	TRef					m_WindowRef;
};




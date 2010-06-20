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
public:
	TWindow(TRefRef WindowRef);

	virtual TRefRef			GetPublisherRef() const				{	return m_WindowRef;	}

	virtual Bool			IsVisible() const=0;				//	get current visibility of the window
	virtual void			Show()=0;							//	show the window. Currently we're just exposing the ability to delay the showing, rather than exposing hiding and showing

	virtual void			SetSize(const int2& WidthHeight)=0;	//	set the client size of the window (game doesn't care about the real size, only the client size)
	virtual int2			GetSize()=0;						//	get the client size of the window
	virtual void			SetPosition(const int2& xy)=0;		//	set the window's top left position

	void					AddChild(TControl* pControl)		{	m_Controls.Add( pControl );	}
	
	inline Bool				operator==(TRefRef WindowRef) const		{	return m_WindowRef == WindowRef;	}
	
protected:
	void					OnSizeChanged() const;				//	when [client] size of the window changes, broadcast a message 
	
protected:
	TPointerArray<TControl>	m_Controls;		//	child controls. Todo: change to TPtr's when we have intrusive ref counters
	
private:
	TRef				m_WindowRef;
};




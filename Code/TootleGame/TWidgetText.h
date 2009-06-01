/*
 *  TWidgetText.h
 *  TootleGame
 *
 *  Created by Duane Bradbury on 28/05/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */

#pragma once

#include "TInputInterface.h"

namespace TLGui
{
	class TWidgetText;
}


class TLGui::TWidgetText : public TLInput::TInputInterface
{
public:
	TWidgetText(TRefRef RenderTargetRef,TRefRef RenderNodeRef,TRefRef UserRef,TRefRef ActionOutDown,TRefRef ActionOutUp=TRef());

	SyncBool					Initialise();						//	continue initialising
	void						Shutdown();							//	shutdown code - just unsubscribes from publishers - this is to release all the TPtr's so we can be destructed
	
	void						SetString(const TTempString& str) { m_Text = str; OnTextChange(); }
	
protected:
	virtual void				OnClickEnd(const TClick& Click);
	
	virtual void				ProcessMessage(TLMessaging::TMessage& Message);	

private:	
	void						BeginEditing();
	void						EndEditing();
	
	void						OnInputDeviceAdded(TRefRef DeviceRef, TRefRef DeviceTypeRef);
	void						OnInputDeviceRemoved(TRefRef DeviceRef, TRefRef DeviceTypeRef);

	void						OnTextChange();
	
private:
	
	TString			m_Text;	// The text for the widget
	
	Bool			m_bEditing;
		
};
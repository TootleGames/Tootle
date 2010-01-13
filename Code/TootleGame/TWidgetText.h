/*
 *  TWidgetText.h
 *  TootleGame
 *
 *  Created by Duane Bradbury on 28/05/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */

#pragma once

#include "TWidget.h"

namespace TLGui
{
	class TWidgetText;
}


class TLGui::TWidgetText : public TLGui::TWidget
{
	friend class TWidgetFactory;

public:
	TWidgetText(TRefRef RenderTargetRef,TRefRef RenderNodeRef,TRefRef UserRef,TRefRef ActionOutDown,TRefRef ActionOutUp=TRef());

	DEPRECATED void						SetString(const TTempString& str) { m_Text = str; OnTextChange(); }
	
protected:
	TWidgetText(TRefRef InstanceRef, TRefRef TypeRef);
	
	virtual void				SetProperty(TLMessaging::TMessage& Message);

	virtual void				OnClickEnd(const TClick& Click);
	
	virtual void				ProcessMessage(TLMessaging::TMessage& Message);	

private:	
	void						BeginEditing();
	void						EndEditing();
	
	void						OnTextChange();
	
private:
	
	TString			m_Text;	// The text for the widget
	
	Bool			m_bEditing;
		
};
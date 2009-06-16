
#pragma once

#include "TInputInterface.h"

namespace TLGui
{
	class TWidgetThumbStick;
}


class TLGui::TWidgetThumbStick : public TLInput::TInputInterface
{
public:
	TWidgetThumbStick(TRefRef RenderTargetRef,TRefRef RenderNodeRef,TRefRef UserRef,TRefRef ActionOutDown,TRefRef ActionOutUp=TRef(),float DeadZone=TLMaths_NearZero);
	
	FORCEINLINE void		SetDeadZone(const float DeadZone)					{	m_DeadZone = DeadZone;	}

protected:
	virtual SyncBool		ProcessClick(TClick& Click,TLRender::TScreen& Screen,TLRender::TRenderTarget& RenderTarget,TLRender::TRenderNode& RenderNode);	//	process a click and detect clicks on/off our render node. return SyncWait if we didnt process it and want to process again
	virtual void			OnCursorMove(const int2& NewCursorPosition)			{	QueueClick( NewCursorPosition, 1.f );	}

protected:
	float					m_DeadZone;		//	if click is within this deadzone (0..1) then ignore it
};
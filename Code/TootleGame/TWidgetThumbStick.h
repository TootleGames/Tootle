
#pragma once

#include "TWidget.h"

namespace TLGui
{
	class TWidgetThumbStick;
}


class TLGui::TWidgetThumbStick : public TLGui::TWidget
{
public:
	TWidgetThumbStick(TRefRef RenderTargetRef,TRefRef RenderNodeRef,TRefRef UserRef,TRefRef ActionOutDown,TRefRef ActionOutUp=TRef(),float DeadZone=TLMaths_NearZero);
	
	FORCEINLINE void		SetDeadZone(const float DeadZone)					{	m_DeadZone = DeadZone;	}

protected:
	virtual SyncBool		ProcessClick(TClick& Click,TLRender::TScreen& Screen,TLRender::TRenderTarget& RenderTarget,TLRender::TRenderNode& RenderNode,const TLMaths::TShapeSphere2D& BoundsDatum,const TLMaths::TShape* pClickDatum);	//	process a click and detect clicks on/off our render node. return SyncWait if we didnt process it and want to process again
	virtual void			OnCursorMove(const int2& NewCursorPosition, TRefRef ActionRef)			{	QueueClick( NewCursorPosition, 1.f, ActionRef, TLGui_WidgetActionType_Move );	}

protected:
	float					m_DeadZone;		//	if click is within this deadzone (0..1) then ignore it
};
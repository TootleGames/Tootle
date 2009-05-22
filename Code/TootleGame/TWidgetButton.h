
#pragma once

#include "TInputInterface.h"

namespace TLGui
{
	class TWidgetButton;
}


class TLGui::TWidgetButton : public TLInput::TInputInterface
{
public:
	TWidgetButton(TRefRef RenderTargetRef,TRefRef RenderNodeRef,TRefRef UserRef,TRefRef ActionOutDown,TRefRef ActionOutUp=TRef()) : 
		TLInput::TInputInterface	( RenderTargetRef, RenderNodeRef, UserRef, ActionOutDown, ActionOutUp ),
		m_bAllowClickOnCursorMove(FALSE)
	{
	}
	
	void SetAllowClickOnCursorMove(Bool bValue)		{ m_bAllowClickOnCursorMove = bValue; }
	
protected:
	virtual void				OnCursorMove(const int2& NewCursorPosition);
	
private:
	Bool						AllowClickOnCursorMove()		{ return m_bAllowClickOnCursorMove; }

private:
	Bool		m_bAllowClickOnCursorMove;

};
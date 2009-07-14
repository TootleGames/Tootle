
#pragma once

#include "TWidget.h"

namespace TLGui
{
	class TWidgetButton;
}


class TLGui::TWidgetButton : public TLGui::TWidget
{
public:
	TWidgetButton(TRefRef RenderTargetRef,TRefRef RenderNodeRef,TRefRef UserRef,TRefRef ActionOutDown,TRefRef ActionOutUp=TRef()) : 
		TLGui::TWidget				( RenderTargetRef, RenderNodeRef, UserRef, ActionOutDown, ActionOutUp ),
		m_bAllowClickOnCursorMove	( FALSE )
	{
	}
	
	TWidgetButton(TRefRef RenderTargetRef,TBinaryTree& WidgetData) : 
		TLGui::TWidget				( RenderTargetRef, WidgetData ),
		m_bAllowClickOnCursorMove	( FALSE )
	{
		WidgetData.ImportData("ClkMove", m_bAllowClickOnCursorMove );
	}
	
	void SetAllowClickOnCursorMove(Bool bValue)		{	m_bAllowClickOnCursorMove = bValue; }
	
protected:
	virtual void	OnCursorMove(const int2& NewCursorPosition, TRefRef ActionRef);
	
private:
	Bool			AllowClickOnCursorMove()		{	return m_bAllowClickOnCursorMove;	}

private:
	Bool			m_bAllowClickOnCursorMove;

};
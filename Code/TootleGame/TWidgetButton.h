
#pragma once

#include "TWidget.h"

namespace TLGui
{
	class TWidgetButton;
}


class TLGui::TWidgetButton : public TLGui::TWidget
{
	friend class TWidgetFactory;

public:

	
	DEPRECATED TWidgetButton(TRefRef RenderTargetRef,TRefRef RenderNodeRef,TRefRef UserRef,TRefRef ActionOutDown,TRefRef ActionOutUp=TRef(), TRefRef DatumRef=TLRender_TRenderNode_DatumBoundsBox2D) : 
		TLGui::TWidget				( RenderTargetRef, RenderNodeRef, UserRef, ActionOutDown, ActionOutUp, NULL, DatumRef  ),
		m_bAllowClickOnCursorMove	( FALSE )
	{
	}

	
	DEPRECATED TWidgetButton(TRefRef RenderTargetRef,TBinaryTree& WidgetData) : 
		TLGui::TWidget				( RenderTargetRef, WidgetData ),
		m_bAllowClickOnCursorMove	( FALSE )
	{
		WidgetData.ImportData("ClkMove", m_bAllowClickOnCursorMove );
	}
	
	void SetAllowClickOnCursorMove(Bool bValue)		{	m_bAllowClickOnCursorMove = bValue; }
	
protected:
	TWidgetButton(TRefRef InstanceRef, TRefRef TypeRef);
	
	virtual void	SetProperty(TLMessaging::TMessage& Message);

	virtual void	OnCursorMove(const int2& NewCursorPosition, TRefRef ActionRef);
	
private:
	Bool			AllowClickOnCursorMove()		{	return m_bAllowClickOnCursorMove;	}

private:
	Bool			m_bAllowClickOnCursorMove;

};
#include "TWidgetDrag.h"





TLGui::TWidgetDrag::TWidgetDrag(TRefRef RenderTargetRef,TRefRef RenderNodeRef,TRefRef UserRef,TRefRef ActionOutDown,TRefRef ActionOutUp,TRefRef ActionOutDrag) :
	m_ActionOutDrag		( ActionOutDrag ),
	m_Dragging			( FALSE ),
	TLInput::TInputInterface	( RenderTargetRef, RenderNodeRef, UserRef, ActionOutDown, ActionOutUp)
{
}


void TLGui::TWidgetDrag::OnClickBegin(const TLInput::TInputInterface::TClick& Click)
{
	//	start drag
	if ( !m_Dragging )
	{
		m_Dragging = TRUE;
		
		m_DragLast2 =
		m_DragFrom2 = Click.GetCursorPos();
	
		m_DragLast3 =
		m_DragFrom3 = Click.GetWorldPos(0.f);
	
		//	normal behaviour
		TLInput::TInputInterface::OnClickBegin( Click );
	}
	else
	{
		//	continue dragging

		//	get changes
		float3 Change3 = Click.GetWorldPos(0.f) - m_DragLast3;
		int2 Change2 = Click.GetCursorPos() - m_DragLast2;

		//	do drag
		OnDrag( Click, Change2, Change3 );

		//	set new last
		m_DragLast3 = Click.GetWorldPos(0.f);
		m_DragLast2 = Click.GetCursorPos();
	}

}


void TLGui::TWidgetDrag::OnClickEnd(const TLInput::TInputInterface::TClick& Click)
{
	//	if the mouse isn't actually released then we might just be outside the render node now
	//	only continue if we really have released the mouse
	if ( Click.GetActionValue() > 0.f && m_Dragging )
	{
		//	continue drag...
		OnClickBegin( Click );
		return;
	}

	//	normal ClickEnd;
	m_Dragging = FALSE;
	TLInput::TInputInterface::OnClickEnd( Click );
}


void TLGui::TWidgetDrag::OnCursorMove()
{
	TLDebug_Break("unimplemented in base code");
}


void TLGui::TWidgetDrag::OnDrag(const TClick& Click,const int2& Drag2,const float3& Drag3)
{
	//	make up fake input message
	TLMessaging::TMessage Message(TRef_Static(A,c,t,i,o));
	Message.Write( m_ActionOutDrag );
	
//	Message.ExportData("RawData", RawData );

	//	write change
	Message.ExportData("Move2", Drag2 );
	Message.ExportData("Move3", Drag3 );

	//	write real positions
	Message.ExportData("Pos2", Click.GetCursorPos() );
	Message.ExportData("Pos3", Click.GetWorldPos(0.f) );

	//	send message
	PublishMessage( Message );
}
	

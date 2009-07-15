#include "TWidgetDrag.h"





TLGui::TWidgetDrag::TWidgetDrag(TRefRef RenderTargetRef,TRefRef RenderNodeRef,TRefRef UserRef,TRefRef ActionOutDown,TRefRef ActionOutUp,TRefRef ActionOutDrag,TBinaryTree* pWidgetData) :
	m_ActionOutDrag		( ActionOutDrag ),
	m_Dragging			( SyncFalse ),
	TLGui::TWidget		( RenderTargetRef, RenderNodeRef, UserRef, ActionOutDown, ActionOutUp, pWidgetData)
{
}


TLGui::TWidgetDrag::TWidgetDrag(TRefRef RenderTargetRef,TBinaryTree& WidgetData) :
	m_Dragging			( SyncFalse ),
	TLGui::TWidget		( RenderTargetRef, WidgetData )
{
	WidgetData.ImportData("ActDrag", m_ActionOutDrag );
}


void TLGui::TWidgetDrag::OnClickBegin(const TLGui::TWidget::TClick& Click)
{
	//	start drag
	if ( m_Dragging == SyncFalse )
	{
		//	first "click", is just the mouse down.
		m_Dragging = SyncWait;
		
		m_DragLast2 =
		m_DragFrom2 = Click.GetCursorPos();
	
		m_DragLast3 =
		m_DragFrom3 = Click.GetWorldPos(0.f);
	
		//	normal behaviour
		TLGui::TWidget::OnClickBegin( Click );
	}
	else
	{
		//	continue dragging (or first drag if m_Dragging is SyncWait)
		m_Dragging = SyncTrue;

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


void TLGui::TWidgetDrag::OnClickEnd(const TLGui::TWidget::TClick& Click)
{
	//	if the mouse isn't actually released then we might just be outside the render node now
	//	only continue if we really have released the mouse
	if ( Click.GetActionType() == TLGui_WidgetActionType_Down && m_Dragging )
	{
		//	continue drag...
		OnClickBegin( Click );
		return;
	}

	//	send the normal click end message, add some extra data to say if we did a drag or not
	//TLGui::TWidget::OnClickEnd( Click );
	TBinaryTree MessageData( TRef_Invalid );
	Bool DidDrag = (m_Dragging == SyncTrue);
	MessageData.ExportData("DidDrag", DidDrag );
	SendActionMessage( Click, m_ActionOutUp, &MessageData );
		
	//	reset dragging state
	m_Dragging = SyncFalse;
}


void TLGui::TWidgetDrag::OnCursorMove(const int2& NewCursorPosition, TRefRef ActionRef)
{		
	//	gr: the Move action is dependant on the parent, so assuming mouse is down.
	//		if that changes (which would be for hover-detection, which would be windows only)
	//		then we need to work out the button's raw state from this message...
	float RawValue = 1.f;
	QueueClick( NewCursorPosition, RawValue, ActionRef, TLGui_WidgetActionType_Down );	
}


void TLGui::TWidgetDrag::OnDrag(const TClick& Click,const int2& Drag2,const float3& Drag3)
{
	//	make up fake input message
	TLMessaging::TMessage Message(TRef_Static(A,c,t,i,o));
	AppendWidgetData( Message );
	Message.Write( m_ActionOutDrag );
	
//	Message.ExportData("RawData", RawData );

	//	write change
	Message.ExportData("Move2", Drag2 );
	Message.ExportData("Move3", Drag3 );

	//	write world positions
	Message.ExportData("Pos2", Click.GetCursorPos() );
	if ( Click.IsWorldRayValid() )
		Message.ExportData("Pos3", Click.GetWorldPos(0.f) );

	//	write original action
	Message.ExportData("InpAction", Click.GetActionRef() );
	Message.ExportData("InpType", Click.GetActionType() );

	//	send message
	PublishMessage( Message );
}
	

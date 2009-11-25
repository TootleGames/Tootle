#include "TWidgetDrag.h"


//	gr: very small finger movements are large in pixels on an iphone screen, so make this value a bit higher
#define DEFAULT_DRAG_MIN	((100.f/480.f) * 5.f)	//	the effectiveness of this varies, in ortho (100/480) is one pixel. In perspective... it depends where the camera is


TLGui::TWidgetDrag::TWidgetDrag(TRefRef InstanceRef, TRefRef TypeRef)	:
	TLGui::TWidget(InstanceRef, TypeRef),
	m_Dragging				( SyncFalse ),
	m_DragMinimum			( DEFAULT_DRAG_MIN ),
	m_HoriztonalDragEnabled	( TRUE ),
	m_VerticalDragEnabled	( TRUE )
{
}


TLGui::TWidgetDrag::TWidgetDrag(TRefRef RenderTargetRef,TRefRef RenderNodeRef,TRefRef UserRef,TRefRef ActionOutDown,TRefRef ActionOutUp,TRefRef ActionOutDrag,TBinaryTree* pWidgetData) :
	m_ActionOutDrag			( ActionOutDrag ),
	m_Dragging				( SyncFalse ),
	m_DragMinimum			( DEFAULT_DRAG_MIN ),
	m_HoriztonalDragEnabled	( TRUE ),
	m_VerticalDragEnabled	( TRUE ),
	TLGui::TWidget			( RenderTargetRef, RenderNodeRef, UserRef, ActionOutDown, ActionOutUp, pWidgetData)
{
	if ( pWidgetData )
	{
		pWidgetData->ImportData("MinDrag", m_DragMinimum );
		pWidgetData->ImportData("HorzDrag", m_HoriztonalDragEnabled );
		pWidgetData->ImportData("VertDrag", m_VerticalDragEnabled );
	}
}


TLGui::TWidgetDrag::TWidgetDrag(TRefRef RenderTargetRef,TBinaryTree& WidgetData) :
	m_Dragging				( SyncFalse ),
	m_DragMinimum			( DEFAULT_DRAG_MIN ),
	m_HoriztonalDragEnabled	( TRUE ),
	m_VerticalDragEnabled	( TRUE ),
	TLGui::TWidget			( RenderTargetRef, WidgetData )
{
	WidgetData.ImportData("ActDrag", m_ActionOutDrag );
	WidgetData.ImportData("MinDrag", m_DragMinimum );
	WidgetData.ImportData("HorzDrag", m_HoriztonalDragEnabled );
	WidgetData.ImportData("VertDrag", m_VerticalDragEnabled );
}


void TLGui::TWidgetDrag::SetProperty(TLMessaging::TMessage& Message)	
{ 
	TWidget::SetProperty(Message);
	
	Message.ImportData("ActDrag", m_ActionOutDrag );
	Message.ImportData("MinDrag", m_DragMinimum );
	Message.ImportData("HorzDrag", m_HoriztonalDragEnabled );
	Message.ImportData("VertDrag", m_VerticalDragEnabled );
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
		//	get changes
		float3 Change3 = Click.GetWorldPos(0.f) - m_DragLast3;
		int2 Change2 = Click.GetCursorPos() - m_DragLast2;

		//	cancel movement on X if no horizontal dragging
		if ( !m_HoriztonalDragEnabled )
		{
			Change3.x = 0.f;
			Change2.x = 0;
		}

		//	cancel movement on Y if no vertical dragging
		if ( !m_VerticalDragEnabled )
		{
			Change3.y = 0.f;
			Change2.y = 0;
		}

		//	check the drag meets min requirements. if it doesn't we ignore this click and will come to this test again when the mouse moves again
		//	gr: only do this check if it's the first drag, let subsequent drags be smooth
		if ( m_Dragging != SyncTrue )
			if ( Change3.LengthSq() < m_DragMinimum*m_DragMinimum )
				return;

		//	continue dragging (or first drag if m_Dragging is SyncWait)
		m_Dragging = SyncTrue;

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
	if ( Click.GetActionType() == TLGui_WidgetActionType_Down && (m_Dragging != SyncFalse) )
	{
		//	continue drag...
		OnClickBegin( Click );
		return;
	}

	//	gr: if the drag status indicates we never had a mouse-down message then don't send a mouse up
	//	todo: move this into base widget code somewhere so we never have a mouse up without a mouse down
	if ( m_Dragging == SyncFalse )
		return;

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
	QueueClick( NewCursorPosition, RawValue, ActionRef, TLGui_WidgetActionType_Move );	
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
	

//------------------------------------------------------
//	widget was enabled
//------------------------------------------------------
void TLGui::TWidgetDrag::OnEnabled()
{
	//	reset drag state so we definatly start with the right command on first click/drag
	m_Dragging = SyncFalse;

	TWidget::OnEnabled();
}


//------------------------------------------------------
//	widget disabled
//------------------------------------------------------
void TLGui::TWidgetDrag::OnDisabled()
{
	//	reset drag state so we definatly start with the right command on first click/drag
	m_Dragging = SyncFalse;

	TWidget::OnDisabled();

}

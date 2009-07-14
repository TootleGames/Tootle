/*------------------------------------------------------

	Node which detects and reports a drag of a render node.

-------------------------------------------------------*/
#pragma once

#include "TWidget.h"


namespace TLGui
{
	class TWidgetDrag;
}


class TLGui::TWidgetDrag : public TLGui::TWidget
{
public:
	TWidgetDrag(TRefRef RenderTargetRef,TRefRef RenderNodeRef,TRefRef UserRef,TRefRef ActionOutDown,TRefRef ActionOutUp,TRefRef ActionOutDrag,TBinaryTree* pWidgetData=NULL);

protected:
	virtual void				OnClickBegin(const TClick& Click);
	virtual void				OnClickEnd(const TClick& Click);
	virtual void				OnCursorMove(const int2& NewCursorPosition, TRefRef ActionRef);		
	
	void						OnDrag(const TClick& Click,const int2& Drag2,const float3& Drag3);

protected:
	TRef						m_ActionOutDrag;	//	
	Bool						m_Dragging;			//	down/up state basicly
	int2						m_DragFrom2;		//	base position of the mouse down
	int2						m_DragLast2;		//	last position
	float3						m_DragFrom3;		//	base position of the mouse down
	float3						m_DragLast3;		//	last position
};



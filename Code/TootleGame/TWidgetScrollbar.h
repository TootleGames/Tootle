/*------------------------------------------------------

	Scroll bar widget;
	stores a float 0..1, deals with user interaction
	and sends out an action with the 0..1 as the raw data
	whenever it changes

-------------------------------------------------------*/
#pragma once

#include "TWidget.h"


namespace TLGui
{
	class TWidgetScrollbar;
};



//----------------------------------------------
//	scroll bar derives from the TWidget, catches clicks then updates the scroll value
//----------------------------------------------
class TLGui::TWidgetScrollbar : public TLGui::TWidget
{
public:
	TWidgetScrollbar(TRefRef RenderTargetRef,TRefRef ScrollBarRenderNode,TRefRef SliderRenderNode,TRefRef UserRef,TRefRef ActionOut,float InitialScrollValue=0.f);

	FORCEINLINE void		PublishScrollValue()								{	SendActionMessage( TRUE, m_ScrollValue );	}

protected:
	virtual Bool			Update();											//	update routine - return FALSE if we don't need updates any more
	virtual SyncBool		ProcessClick(TClick& Click,TLRender::TScreen& Screen,TLRender::TRenderTarget& RenderTarget,TLRender::TRenderNode& RenderNode,const TLMaths::TShapeSphere2D& BoundsDatum,const TLMaths::TShape* pClickDatum);	//	process a click and detect clicks on/off our render node. return SyncWait if we didnt process it and want to process again
	virtual void			GetRenderNodes(TArray<TRef>& RenderNodeArray);		//	get array of all the render nodes we're using

	virtual void				OnCursorMove(const int2& NewCursorPosition, TRefRef ActionRef);		
	
	void					SetScrollValue(float NewValue);						//	set value and send out message if it changes
	void					UpdateSliderPos();									//	update graphical position of slider

protected:
	TRef					m_ScrollBarRenderNode;
	TRef					m_SliderRenderNode;
	float					m_ScrollValue;
	Bool					m_SliderPosValid;					//	true if slider graphic is out of date 
};



/*------------------------------------------------------

	Scroll bar widget;
	stores a float 0..1, deals with user interaction
	and sends out an action with the 0..1 as the raw data
	whenever it changes

-------------------------------------------------------*/
#pragma once

#include "TGui.h"


namespace TLGui
{
	class TWidgetScrollbar;
};



//----------------------------------------------
//	scroll bar derives from the TGui, catches clicks then updates the scroll value
//----------------------------------------------
class TLGui::TWidgetScrollbar : public TLGui::TGui
{
public:
	TWidgetScrollbar(TRefRef RenderTargetRef,TRefRef ScrollBarRenderNode,TRefRef SliderRenderNode,TRefRef UserRef,TRefRef ActionOut,float InitialScrollValue=0.f);

protected:
	virtual Bool			Update();											//	update routine - return FALSE if we don't need updates any more
	virtual SyncBool		ProcessClick(const TClick& Click,TLRender::TScreen& Screen,TLRender::TRenderTarget& RenderTarget,TPtr<TLRender::TRenderNode>& pRenderNode);	//	process a click and detect clicks on/off our render node. return SyncWait if we didnt process it and want to process again
	virtual void			GetRenderNodes(TArray<TRef>& RenderNodeArray);		//	get array of all the render nodes we're using
	virtual void			OnInitialised();									//	when init has finished set the position of the slider

	void					SetScrollValue(float NewValue);						//	set value and send out message if it changes
	void					UpdateSliderPos();									//	update graphical position of slider

protected:
	TRef					m_ScrollBarRenderNode;
	TRef					m_SliderRenderNode;
	float					m_ScrollValue;
	Bool					m_SliderPosValid;					//	true if slider graphic is out of date 
};



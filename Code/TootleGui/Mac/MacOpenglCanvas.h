/*------------------------------------------------------

	

-------------------------------------------------------*/
#pragma once
#include "../TOpenglCanvas.h"


namespace TLGui
{
	namespace Platform
	{
		class OpenglCanvas;
	}
}

//	forward declarations
@class TootleOpenGLView;


class TLGui::Platform::OpenglCanvas : public TLGui::TOpenglCanvas
{
public:
	OpenglCanvas(TLGui::TWindow& Parent,TRefRef ControlRef);
	
	virtual Bool	BeginRender();
	virtual void	EndRender();
		
protected:
	TootleOpenGLView*	m_pView;	//	cocoa opengl view
};




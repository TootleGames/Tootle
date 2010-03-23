/*------------------------------------------------------

	

-------------------------------------------------------*/
#pragma once
#include "../TOpenglCanvas.h"
#include "PCWinWindow.h"	//	need to include for HDC etc


//	include opengl stuff
#pragma comment( lib, "Opengl32.lib" )
#pragma comment( lib, "glu32.lib" )

#include "gl.h"
#include "glext.h"
#include "wglext.h"


namespace TLGui
{
	namespace Platform
	{
		class OpenglCanvas;
	}
}


class TLGui::Platform::OpenglCanvas : public TLGui::TOpenglCanvas
{
public:
	OpenglCanvas(TLGui::TWindow& Parent,TRefRef ControlRef);
	~OpenglCanvas();
	
	virtual Bool	BeginRender();
	virtual void	EndRender();
		
protected:
	bool			InitContext(Win32::GWindow& Window);	//	create the device context and opengl context

protected:
	HDC				m_HDC;		//	DC we've setup for opengl
	HGLRC			m_HGLRC;	//	opengl context
	bool			m_HasArbMultiSample;	//	is antialiasing supported?
};




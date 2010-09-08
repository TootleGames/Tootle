/*------------------------------------------------------

	

-------------------------------------------------------*/
#pragma once
#include "../TWindow.h"


namespace TLGui
{
	namespace Platform
	{
		class Window;
	}
}

//	forward declarations
@class TootleWindowDelegate;
@class UIWindow;


class TLGui::Platform::Window : public TLGui::TWindow
{
public:
	Window(TRefRef WindowRef);
	
	virtual Bool			IsVisible() const;				//	get current visibility of the window
	virtual void			Show();							//	show the window. Currently we're just exposing the ability to delay the showing, rather than exposing hiding and showing
	
	virtual void			SetSize(const Type2<u16>& WidthHeight);	//	set the client size of the window (game doesn't care about the real size, only the client size)
	virtual Type2<u16>		GetSize();								//	get the client size of the window
	virtual void			SetPosition(const Type2<u16>& xy);		//	set the window's top left position
	virtual Type2<u16>		GetPosition() const;					//	get the window's top left position
		
public:	//	access for anything that explicitly casts a window to this type
	TootleWindowDelegate*	m_pDelegate;	//	our window delegate
	UIWindow*				m_pWindow;		//	actual window
};




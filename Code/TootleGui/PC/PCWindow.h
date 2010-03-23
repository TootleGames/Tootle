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

//	forward declaration
namespace Win32
{
	class GWinControl;
}

class TLGui::Platform::Window : public TLGui::TWindow
{
public:
	Window(TRefRef WindowRef);
	
	virtual Bool			IsVisible() const;
	virtual void			Show();
	
	virtual void			SetSize(const int2& WidthHeight);	//	set the CLIENT SIZE ("content" in os x) of the window
	virtual int2			GetSize();							//	get the CLIENT SIZE ("content" in os x) of the window
	virtual void			SetPosition(const int2& xy);		//	set the top-left position of the window frame
	
public:	//	access for anything that explicitly casts a window to this type
	TPtr<Win32::GWinControl>	m_pWindow;
};




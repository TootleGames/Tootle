/*------------------------------------------------------
	
	screen implementation on PC - opengl

-------------------------------------------------------*/
#pragma once

#include "MacRender.h"
#include "../TScreen.h"
#include "MacWinWindow.h"



//	external forward declarations
namespace TLRender 
{
	class TScreen;

	//	internal forward declarations
	namespace Platform	
	{
		class Screen;
		class ScreenWide;
	};
};




//----------------------------------------------------------
//	win32 screen (it's actually just an opengl window)
//----------------------------------------------------------
class TLRender::Platform::Screen : public TLRender::TScreen
{
public:
	Screen(TRefRef ScreenRef,TScreenShape ScreenShape);
	
	virtual SyncBool		Init();
	virtual SyncBool		Update();
	virtual SyncBool		Shutdown();

	virtual void			Draw();
	virtual Type4<s32>		GetSize() const;	//	get size of the screen

protected:
	void					GetDesktopSize(Type4<s32>& DesktopSize) const;	//	get the desktop dimensions
	void					GetCenteredSize(Type4<s32>& Size) const;		//	take a screen size and center it on the desktop
	virtual void			GetViewportMaxSize(Type4<s32>& MaxSize);	//	need to max-out to client-area on the window 
	Win32::GOpenglWindow*	GetOpenglWindow();

protected:
	TPtr<Win32::GWinControl>	m_pWindow;
};



//----------------------------------------------------------
//	widescreen screen
//----------------------------------------------------------
class TLRender::Platform::ScreenWide : public TLRender::Platform::Screen
{
public:
	ScreenWide(TRefRef ScreenRef) :
		TLRender::Platform::Screen	( ScreenRef, TLRender::ScreenShape_Wide )
	{
		//	swap dimensions
		TLMaths::SwapVars( m_Size.Height(), m_Size.Width() );
	}
};


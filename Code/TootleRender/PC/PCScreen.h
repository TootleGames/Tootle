/*------------------------------------------------------
	
	screen implementation on PC - opengl

-------------------------------------------------------*/
#pragma once

#include "PCRender.h"
#include "../TScreen.h"
#include "PCWinWindow.h"



//	external forward declarations
namespace TLRender 
{
	class TScreen;

	//	internal forward declarations
	namespace Platform	
	{
		class Screen;
		class ScreenWideLeft;
		class ScreenWideRight;
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
	virtual void			GetRenderTargetMaxSize(Type4<s32>& MaxSize);	//	need to max-out to client-area on the window 
	Win32::GOpenglWindow*	GetOpenglWindow();

protected:
	TPtr<Win32::GWinControl>	m_pWindow;
};



//----------------------------------------------------------
//	widescreen screen
//----------------------------------------------------------
class TLRender::Platform::ScreenWideLeft : public TLRender::Platform::Screen
{
public:
	ScreenWideLeft(TRefRef ScreenRef) :
		TLRender::Platform::Screen	( ScreenRef, TLRender::ScreenShape_WideLeft )
	{
		//	dont swap dimensions to emulate ipod
	}
};


//----------------------------------------------------------
//	widescreen screen
//----------------------------------------------------------
class TLRender::Platform::ScreenWideRight : public TLRender::Platform::Screen
{
public:
	ScreenWideRight(TRefRef ScreenRef) :
		TLRender::Platform::Screen	( ScreenRef, TLRender::ScreenShape_WideRight )
	{
		/*
		//	swap height and width
		s16 h = m_Size.Height();
		s16 w = m_Size.Width();
		m_Size.Width() = h;
		m_Size.Height() = w;
		*/
	}
};


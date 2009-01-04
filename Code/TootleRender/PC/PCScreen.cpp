#include "PCScreen.h"
#include <TootleCore/TPtr.h>
#include "PCOpenglExt.h"

namespace TLRender
{
	namespace Platform
	{
		namespace Opengl
		{
			extern TPtr<Win32::GOpenglWindow>	g_pSpareWindow;
		}
	}
}


using namespace TLRender;



Platform::Screen::Screen(const TRef& Ref) :
	TLRender::TScreen	( Ref )
{
}


//----------------------------------------------------------
//	create window
//----------------------------------------------------------
SyncBool Platform::Screen::Init()
{
	//	need to wait for win32 factory to exist
	if ( !Win32::g_pFactory )
		return SyncWait;

	//	todo: need to work out if render system has finished initialising here...

	//	if there is a spare window use that first
	//	when the opengl system is initialised we have to create a dummy window, so
	//	that one is usually spare on first screen creation
	if ( Opengl::g_pSpareWindow )
	{
		//	if the spare window doesnt support multisampling, but we now know it does, 
		//	delete this spare window and then create a new one (as if we had no spare)
		if ( !Opengl::g_pSpareWindow->HasArbMultiSample() )
		{
			if ( OpenglExtensions::IsHardwareSupported( OpenglExtensions::GHardware_ARBMultiSample ) )
			{
				Win32::g_pFactory->RemoveInstance( Opengl::g_pSpareWindow->GetRef() );
				Opengl::g_pSpareWindow = NULL;
			}
		}
	}

	if ( Opengl::g_pSpareWindow )
	{
		//	steal spare window
		m_pWindow = Opengl::g_pSpareWindow;
		Opengl::g_pSpareWindow = NULL;

		//	rename ref of the spare window
		m_pWindow->SetRef( GetRef() );
	}
	else
	{
		//	create a window if we have no spare window
		TPtr<Win32::GWinControl> pNullParent;
		m_pWindow = Win32::g_pFactory->GetInstance( GetRef(), TRUE, "OpenglWindow" );
		if ( !m_pWindow )
			return SyncFalse;

		if ( !m_pWindow->Init( pNullParent, m_pWindow->DefaultFlags() ) )
		{
			Win32::g_pFactory->RemoveInstance( m_pWindow->GetRef() );
			m_pWindow = NULL;
			return SyncFalse;
		}

		// The window has changed.  Notify to all subscribers.
		TPtr<TLMessaging::TMessage> pMessage = new TLMessaging::TMessage("SCREEN");

		if(pMessage.IsValid())
		{
			TRef change = "NEW";
			pMessage->Write(change);
			PublishMessage(pMessage);
		}
	}

	//	failed to be created
	if ( !m_pWindow )
		return SyncFalse;

	//	get our size
	Type4<s32> ScreenSize = GetSize();

	//	center it on the desktop
	GetCenteredSize( ScreenSize );

	//	set the window's size to the screen size
	m_pWindow->SetDimensions( ScreenSize );

	//	make the window visible
	m_pWindow->Show();

	return TLRender::TScreen::Init();
}


//----------------------------------------------------------
//	update window
//----------------------------------------------------------
SyncBool Platform::Screen::Update()
{
	//	lost window?
	if ( !m_pWindow )
		return SyncFalse;

	//	window has been closed
	if ( m_pWindow->IsClosed() )
		return SyncFalse;

	//	continue
	return SyncTrue;
}


void Platform::Screen::Draw()
{
	Win32::GOpenglWindow* pWindow = GetOpenglWindow();

	//	lost window?
	if ( !pWindow )
		return;

	//	do inherited draw
	TScreen::Draw();

	//	flip buffers
	SwapBuffers( pWindow->m_HDC );

	
	//	post-draw make sure the swap interval is limited/not limited
	if ( OpenglExtensions::IsHardwareEnabled( OpenglExtensions::GHardware_SwapInterval ) )
	{
		if ( GetFlag( TScreen::Flag_SyncFrameRate ) )
			OpenglExtensions::glSwapIntervalEXT()( 1 );
		else
			OpenglExtensions::glSwapIntervalEXT()( 0 );
	}

}

//----------------------------------------------------------
//	clean up
//----------------------------------------------------------
SyncBool Platform::Screen::Shutdown()
{
	m_pWindow = NULL;
	
	return TLRender::TScreen::Shutdown();
}


//----------------------------------------------------------
//	get size of the screen
//----------------------------------------------------------
Type4<s32> Platform::Screen::GetSize() const
{
	Type4<s32> Size = TScreen::GetSize();
	Type4<s32> DesktopSize;
	GetDesktopSize( DesktopSize );

	if ( Size.x == TLRender::g_MaxSize )		Size.x = DesktopSize.x;
	if ( Size.y == TLRender::g_MaxSize )		Size.y = DesktopSize.y;
	if ( Size.Width() == TLRender::g_MaxSize )	Size.Width() = DesktopSize.Width();
	if ( Size.Height() == TLRender::g_MaxSize )	Size.Height() = DesktopSize.Height();

	return Size;
}


//----------------------------------------------------------
//	get the desktop dimensions
//----------------------------------------------------------
void Platform::Screen::GetDesktopSize(Type4<s32>& DesktopSize) const
{
	DesktopSize.x = 0;
	DesktopSize.y = 0;
	DesktopSize.Width() = GetSystemMetrics(SM_CXSCREEN);
	DesktopSize.Height() = GetSystemMetrics(SM_CYSCREEN);
}


//----------------------------------------------------------
//	take a screen size and center it on the desktop
//----------------------------------------------------------
void Platform::Screen::GetCenteredSize(Type4<s32>& Size) const
{
	Type4<s32> DesktopSize;
	GetDesktopSize( DesktopSize );

	s32 DesktopCenterX = DesktopSize.x + (DesktopSize.Width() / 2);
	s32 DesktopCenterY = DesktopSize.y + (DesktopSize.Height() / 2);

	Size.x = DesktopCenterX - (Size.Width() / 2);
	Size.y = DesktopCenterY - (Size.Height() / 2);
}



//---------------------------------------------------------------
//	need to max-out to client-area on the window 
//---------------------------------------------------------------
void Platform::Screen::GetRenderTargetMaxSize(Type4<s32>& MaxSize)
{
	if ( !m_pWindow )
	{
		TScreen::GetRenderTargetMaxSize( MaxSize );
		return;
	}

	//	gr: don't know if I have to cut off x/y here?
	MaxSize.Left() = 0;
	MaxSize.Top() = 0;
	MaxSize.Width() = m_pWindow->m_ClientSize.x;
	MaxSize.Height() = m_pWindow->m_ClientSize.y;
}


//------------------------------------------------------------
//	return our window as an opengl window
//------------------------------------------------------------
Win32::GOpenglWindow* Platform::Screen::GetOpenglWindow()
{
	return m_pWindow.GetObject<Win32::GOpenglWindow>();
}

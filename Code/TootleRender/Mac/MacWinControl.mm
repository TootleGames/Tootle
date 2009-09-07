#include "MacWinControl.h"
#include <TootleCore/TLDebug.h>
#include <TootleCore/TString.h>
//#include <winuser.h>	//	updated core SDK
#include "MacWinWindow.h"


//	globals
//------------------------------------------------
namespace Win32
{
	TPtr<Win32::TWinControlFactory>	g_pFactory;

//	LRESULT CALLBACK			Win32CallBack(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam );
	u32							GWinControl::g_MouseWheelMsg = 0;

	TArray<const char*>			g_ClassCreatedList;	//	array of strings of classes we've created
};


//	Definitions
//------------------------------------------------



//------------------------------------------------
//	init win32
//------------------------------------------------
SyncBool Win32::Init()
{
	//	create factory if it doesnt exist
	if ( !g_pFactory )
	{
		g_pFactory = new Win32::TWinControlFactory;
		if ( !g_pFactory )
			return SyncFalse;
	}

	return SyncTrue;
}


//------------------------------------------------
//	shutdown win32
//------------------------------------------------
SyncBool Win32::Shutdown()
{
	//	no more factory all cleaned up
	if ( !g_pFactory )
		return SyncTrue;

	SyncBool Result = g_pFactory->ShutdownObjects();

	//	still shutting down
	if ( Result == SyncWait )
		return Result;

	//	free factory
	g_pFactory = NULL;

	//	empty global array
	g_ClassCreatedList.Empty(TRUE);

	return Result;
}




//------------------------------------------------
//	win control factory/manager
//------------------------------------------------
Win32::GWinControl* Win32::TWinControlFactory::CreateObject(TRefRef InstanceRef,TRefRef TypeRef)
{
	if ( TypeRef == "Window" )
		return new Win32::GWindow( InstanceRef );

	if ( TypeRef == "OpenglWindow" )
		return new Win32::GOpenglWindow( InstanceRef );

	return NULL;
	//return new Win32::GWinControl( InstanceRef );
}




Win32::GWinControl::GWinControl(TRefRef InstanceRef) :
	m_Ref				( InstanceRef ),
//	m_Hwnd				( NULL ),
//	m_OwnerHwnd			( NULL ),
	m_ClientPos			( int2(0,0) ),
	m_ClientSize		( int2(40,40) ),
	m_StyleFlags		( 0x0 ),
	m_Closed			( FALSE ),
	m_HasExplicitText	( FALSE )
{
}



Win32::GWinControl::~GWinControl()
{
	//	destroy control
	Destroy();

	//	null entry in children
	for ( u32 c=0;	c<m_ChildControls.GetSize();	c++ )
	{
		if ( m_ChildControls[c]->m_pOwnerControl == this )
		{
			m_ChildControls[c]->m_pOwnerControl = NULL;
		}
	}

	//	remove from parents list
	if ( m_pOwnerControl )
	{
		s32 Index = m_pOwnerControl->m_ChildControls.FindIndex(this);
		if ( Index != -1 )
			m_pOwnerControl->m_ChildControls.RemoveAt( (u32)Index );
	}
}


void Win32::GWinControl::PosToScreen(int2& ClientPos)
{
	TLDebug_Break("not implemented");
/*
 Removed from MAC build
	//	convert client pos to screen
	POINT p;
	p.x = ClientPos.x;
	p.y = ClientPos.y;
	ClientToScreen( m_Hwnd, &p );
	ClientPos.x = p.x;
	ClientPos.y = p.y;
 */
}


void Win32::GWinControl::ScreenToPos(int2& ScreenPos)
{
	TLDebug_Break("not implemented");
	
	/*
	//	convert screen pos to client pos
	POINT p;
	p.x = ScreenPos.x;
	p.y = ScreenPos.y;
	ScreenToClient( m_Hwnd, &p );
	ScreenPos.x = p.x;
	ScreenPos.y = p.y;
	 */
}



Bool Win32::GWinControl::Init(TPtr<GWinControl>& pOwner, u32 Flags)
{
	return TRUE;
	
/*
 // REMOVED FROM MAC BUILD
 
	//	check we havent already created a contorl
	if ( m_Hwnd != NULL )
	{
		TLDebug_Break("Control already created");
		return FALSE;
	}
	
	//	create control
	Flags |= AdditionalStyleFlags();
	m_StyleFlags	= Flags;

//	HMENU hMenu		= (HMENU)ControlID;
	HMENU hMenu		= (HMENU)NULL;
	HINSTANCE hInstance = TLCore::Platform::g_HInstance;
	HWND OwnerHwnd	= pOwner ? pOwner->m_Hwnd : m_OwnerHwnd;

	//	reset handle
	m_Hwnd = NULL;

	//	set owner
	m_pOwnerControl = pOwner;

	TString RefString;
	m_Ref.GetString( RefString );
	m_HasExplicitText = FALSE;

	//	get resulting hwnd, m_Hwnd is set from the WM_CREATE callback
	HWND ResultHwnd = CreateWindowEx( StyleExFlags(), ClassName(), RefString.GetData(), StyleFlags(), m_ClientPos.x, m_ClientPos.y, m_ClientSize.x, m_ClientSize.y, OwnerHwnd, hMenu, hInstance, (void*)&m_Ref );

	// [19-09-08] DB - HACK - set the global HWND
	if(TLCore::Platform::g_HWnd == NULL)
		TLCore::Platform::g_HWnd = ResultHwnd;

	//	if control doesnt get a WM_CREATE (if its a standard windows control) call it
	if ( ResultHwnd != NULL && m_Hwnd == NULL )
	{
		//	grab our ptr
		TPtr<Win32::GWinControl> pThis = g_pFactory->GetInstance( m_Ref );
		OnWindowCreate( pThis, ResultHwnd );
	}

	//	failed
	if ( m_Hwnd == NULL || ResultHwnd == NULL || m_Hwnd != ResultHwnd )
	{
		TLDebug::Platform::CheckWin32Error();
		g_pFactory->RemoveInstance( m_Ref );
		return FALSE;
	}

	//	control has been created
	OnCreate();

	return TRUE;
 */
}

/*
//-------------------------------------------------------------------------
//	callback after a window has been created
//-------------------------------------------------------------------------
void Win32::GWinControl::OnWindowCreate(TPtr<GWinControl>& pControl,HWND Hwnd)
{
	if ( !pControl )
	{
		TLDebug_Break("WM_CREATE callback with invalid control pointer");
		return;
	}

	//	set handle
	pControl->m_Hwnd = Hwnd;

	//	update styles added by windows
	pControl->GetStyleFlags();

	//	set member values now we successfully created the window
	if ( pControl->m_pOwnerControl )
	{
		pControl->m_pOwnerControl->m_ChildControls.Add( pControl );
	}
}
 */


Bool Win32::GWinControl::CreateClass()
{
	//TLDebug_Break("not implemented");
	return TRUE;
/*
	//	if class already created we dont need to create it
	if ( g_ClassCreatedList.Find( ClassName() ) )
		return TRUE;

	WNDCLASS wc;
	ZeroMemory(&wc,sizeof(wc));
	wc.style		= ClassStyle();
	wc.lpfnWndProc	= Win32::Win32CallBack; 
	wc.cbClsExtra	= 0;
	wc.cbWndExtra	= 0;
	wc.hInstance	= TLCore::Platform::g_HInstance;
	wc.hIcon		= GetIconHandle();
	wc.hCursor		= NULL;//LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = GetBackgroundBrush();
	wc.lpszMenuName	= NULL;
	wc.lpszClassName = ClassName();

	if (!RegisterClass(&wc))
	{
		TLDebug::Platform::CheckWin32Error();
		return FALSE;
	}

	//	add to list of created-classes
	g_ClassCreatedList.Add( ClassName() );

	return TRUE;
 */
}

/*static*/Bool Win32::GWinControl::DestroyClass(const char* pClassName)
{
	//TLDebug_Break("not implemented");
	return TRUE;

	/*
	if ( !UnregisterClass( pClassName, TLCore::Platform::g_HInstance ) )
	{
		TLDebug::Platform::CheckWin32Error();
		return FALSE;
	}

	//	remove class from created-class list
	s32 ClassIndex = g_ClassCreatedList.FindIndex( pClassName );
	g_ClassCreatedList.RemoveAt( ClassIndex );

	return TRUE;
	 */
}

Bool Win32::GWinControl::DestroyClass()
{
	return DestroyClass( ClassName() );
}

Bool Win32::GWinControl::ClassExists()
{
	TLDebug_Break("not implemented");

	return FALSE;
	/*
	WNDCLASS wc;
	return GetClassInfo( TLCore::Platform::g_HInstance, ClassName(), &wc ) != 0;
*/
}


void Win32::GWinControl::Destroy()
{
	OnDestroy();
	
	//TLDebug_Break("not implemented");

/*
	//	destroy the control if we still have a handle
	if ( m_Hwnd )
	{
		//	HACK - unassign global hwnd
		if(TLCore::Platform::g_HWnd == m_Hwnd)
			TLCore::Platform::g_HWnd = NULL;

		//	gr: recurses? clear hwnd then destroy
		HWND Hwnd = m_Hwnd;
		//m_Hwnd = NULL;

		if ( ! DestroyWindow( Hwnd ) )
		{
			TLDebug::Platform::CheckWin32Error();
		}

		m_Hwnd = NULL;
	}
 */

	//	remove from global control list
	if ( Win32::g_pFactory )
		Win32::g_pFactory->RemoveInstance( m_Ref );
}



void Win32::GWinControl::Show(Bool Show)
{
	TLDebug_Break("not implemented");

/*	
	if ( m_Hwnd )
	{
		ShowWindow( m_Hwnd, Show ? SW_SHOW : SW_HIDE );
		GetStyleFlags();
	}
*/
}

Bool Win32::GWinControl::SetText(const TString& Text,Bool IsExplicitText)
{
	//	if this is automated text (eg. from ref) and we've already manually
	//	set the text, then don't overwrite it
	if ( !IsExplicitText && m_HasExplicitText )
		return FALSE;

	TLDebug_Break("not implemented");

/*	
	if ( !m_Hwnd )
		return FALSE;

	SetWindowText( m_Hwnd, Text.GetData() );
 */
	m_HasExplicitText = IsExplicitText;
	return TRUE;
}

//-----------------------------------------------------
//	change ref. update text if neccessary
//-----------------------------------------------------
void Win32::GWinControl::SetRef(const TRef& Ref)				
{	
	m_Ref = Ref;

	//	update text
	if ( !m_HasExplicitText )
	{
		TString RefString;
		Ref.GetString( RefString );

		SetText( RefString.GetData(), FALSE );
	}
}


//-------------------------------------------------------------------------
//	redraw window
//-------------------------------------------------------------------------
void Win32::GWinControl::Refresh()
{
/*	
 REMOVED FORM MAC BUILD
	if ( m_Hwnd)
	{
		//	invalidate whole window for redrawing
		if ( InvalidateRgn( m_Hwnd, NULL, TRUE ) )
		{
			UpdateWindow( m_Hwnd );
		}
		else
		{
			TLDebug::Platform::CheckWin32Error();
		}
	}

	//	refresh is manually called, so update scrollbars
	UpdateScrollBars();
 */
}


//-------------------------------------------------------------------------
//	set new pos and dimensions at once
//-------------------------------------------------------------------------
void Win32::GWinControl::SetDimensions(int2 Pos, int2 Size)
{
	//	update our pos
	m_ClientPos = Pos;

	//	update our size
	m_ClientSize = Size;

	//	apply the changes
	UpdateDimensions();
}


//-------------------------------------------------------------------------
//	set new pos and dimensions at once
//-------------------------------------------------------------------------
void Win32::GWinControl::SetDimensions(const Type4<s32>& PosSizes)
{
	//	update our pos
	m_ClientPos = int2( PosSizes.x, PosSizes.y );

	//	update our size
	m_ClientSize = int2( PosSizes.Width(), PosSizes.Height() );

	//	apply the changes
	UpdateDimensions();
}


//-------------------------------------------------------------------------
//	set new position
//-------------------------------------------------------------------------
void Win32::GWinControl::Move(int2 Pos)
{
	//	update our pos
	m_ClientPos = Pos;

	//	apply the changes
	UpdateDimensions();
}


//-------------------------------------------------------------------------
//	set new width/height
//-------------------------------------------------------------------------
void Win32::GWinControl::Resize(int2 Size)
{
	//	gr: client size is not window size
	//	update our size
	m_ClientSize = Size;

	//	apply the changes
	UpdateDimensions();
}


//-------------------------------------------------------------------------
//	update window dimensions to current client size settings
//-------------------------------------------------------------------------
void Win32::GWinControl::UpdateDimensions()
{
	/*
	 REMOVED FORM MAC BUILD
	if ( m_Hwnd )
	{
		if ( !MoveWindow( m_Hwnd, m_ClientPos.x, m_ClientPos.y, m_ClientSize.x, m_ClientSize.y, TRUE ) )
		{
			TLDebug::Platform::CheckWin32Error();
		}
	}
	 */
}


/*
//-------------------------------------------------------------------------
//	calcs the window's overall size to accomadate the client size
//-------------------------------------------------------------------------
void Win32::GWinControl::ResizeClientArea(int2 ClientSize)
{
	RECT ClientRect;
	ClientRect.top		= 0;
	ClientRect.left		= 0;
	ClientRect.bottom	= ClientSize.y;
	ClientRect.right	= ClientSize.x;

	if ( !AdjustWindowRectEx( &ClientRect, StyleFlags(), HasMenu(), StyleExFlags() ) )
	{
		TLDebug::Platform::CheckWin32Error();
		return;
	}

	//	set new window size
	Resize( int2( ClientRect.right-ClientRect.left, ClientRect.bottom-ClientRect.top ) );

}
 */


void Win32::GWinControl::GetStyleFlags()
{
	/*
	 REMOVED FORM MAC BUILD

	m_StyleFlags = 0x0;

	//	use GetWindowLongPtr in newer platform SDK
	#if(WINVER >= 0x0500)
		m_StyleFlags |= GetWindowLongPtr( m_Hwnd, GWL_STYLE );
		m_StyleFlags |= GetWindowLongPtr( m_Hwnd, GWL_EXSTYLE );
	#else
		m_StyleFlags |= GetWindowLong( m_Hwnd, GWL_STYLE );
		m_StyleFlags |= GetWindowLong( m_Hwnd, GWL_EXSTYLE );
	#endif
	 */
}
	
void Win32::GWinControl::SetNewStyleFlags(u32 Flags)
{
	/*
	 REMOVED FORM MAC BUILD

	m_StyleFlags = Flags;

	if ( !SetWindowLong( m_Hwnd, GWL_STYLE, StyleFlags() ) )
		TLDebug::Platform::CheckWin32Error();

	if ( !SetWindowLong( m_Hwnd, GWL_EXSTYLE, StyleExFlags() ) )
		TLDebug::Platform::CheckWin32Error();

	HWND WindowOrder = HWND_NOTOPMOST;
	if ( StyleExFlags() & GWinControlFlags::AlwaysOnTop )
		WindowOrder = HWND_TOPMOST;

	//	update/refresh window
	if ( !SetWindowPos( m_Hwnd, WindowOrder, 0, 0, 0, 0, SWP_SHOWWINDOW|SWP_FRAMECHANGED|SWP_NOMOVE|SWP_NOSIZE ) )
		TLDebug::Platform::CheckWin32Error();
	 */
}

void Win32::GWinControl::SetStyleFlags(u32 Flags)
{
	//	add flags to our current set
	SetNewStyleFlags( m_StyleFlags|Flags );
}

void Win32::GWinControl::ClearStyleFlags(u32 Flags)
{
	//	remove flags from our current flags
	SetNewStyleFlags( m_StyleFlags&(~Flags) );
}


/*
Win32::GMenuSubMenu* Win32::GWinControl::GetChildSubMenu(HMENU HMenu,TPtr<GWinControl>& pControl)	
{
	GMenuSubMenu* pSubMenu = NULL;
	pControl = NULL;

	for ( u32 i=0;	i<m_ChildControls.GetSize();	i++ )
	{
		pSubMenu = m_ChildControls[i]->GetSubMenu( HMenu );
		if ( pSubMenu )
		{
			pControl = m_ChildControls[i];
			break;
		}
	}

	return pSubMenu;
	 
}
*/


Win32::GMenuItem* Win32::GWinControl::GetChildMenuItem(u16 ItemID, TPtr<GWinControl>& pControl)	
{
	GMenuItem* pMenuItem = NULL;
	pControl = NULL;

	for ( u32 i=0;	i<m_ChildControls.GetSize();	i++ )
	{
		pMenuItem = m_ChildControls[i]->GetMenuItem( ItemID );
		if ( pMenuItem )
		{
			pControl = m_ChildControls[i];
			break;
		}
	}

	return pMenuItem;
}


//-------------------------------------------------------------------------
//	update window's scrollbar info if applicable
//-------------------------------------------------------------------------
void Win32::GWinControl::UpdateScrollBars()
{
	TLDebug_Break("not implemented");

	/*
	//	only needed if window is setup
	if ( !Hwnd() )
		return;

	//	
	SCROLLINFO ScrollInfo;
	ScrollInfo.cbSize = sizeof(SCROLLINFO);

	//	update vert scroll bar
	if ( StyleFlags() & GWinControlFlags::VertScroll )
	{
		int Min = 0;
		int Max = 0;
		int Jump = 0;
		int Pos = 0;
		Bool Enable = GetVertScrollProperties( Min, Max, Jump, Pos );

		ScrollInfo.fMask  = SIF_RANGE | SIF_PAGE | SIF_POS; 
		if ( Enable )
		{
			ScrollInfo.nMin		= Min;
			ScrollInfo.nMax		= Max;
			ScrollInfo.nPage	= Jump;
			ScrollInfo.nPos		= Pos;
			ScrollInfo.fMask |= SIF_DISABLENOSCROLL;	//	disable if invalid/useless properties (eg. min==max)
		}
		else
		{
			//	remove scroll bar by invalidating properies and NOT setting SIF_DISABLENOSCROLL flag
			ScrollInfo.nMin		= 0;
			ScrollInfo.nMax		= 0;
			ScrollInfo.nPage	= 0;
			ScrollInfo.nPos		= 0;
			ScrollInfo.fMask &= ~SIF_DISABLENOSCROLL;
		}

		SetScrollInfo( Hwnd(), SB_VERT, &ScrollInfo, TRUE );
	}

	//	update horz scroll bar
	if ( StyleFlags() & GWinControlFlags::HorzScroll )
	{
		int Min = 0;
		int Max = 0;
		int Jump = 0;
		int Pos = 0;
		Bool Enable = GetHorzScrollProperties( Min, Max, Jump, Pos );

		ScrollInfo.fMask  = SIF_RANGE | SIF_PAGE | SIF_POS; 
		if ( Enable )
		{
			ScrollInfo.nMin		= Min;
			ScrollInfo.nMax		= Max;
			ScrollInfo.nPage	= Jump;
			ScrollInfo.nPos		= Pos;
			ScrollInfo.fMask |= SIF_DISABLENOSCROLL;	//	disable if invalid/useless properties (eg. min==max)
		}
		else
		{
			//	remove scroll bar by invalidating properies and NOT setting SIF_DISABLENOSCROLL flag
			ScrollInfo.nMin		= 0;
			ScrollInfo.nMax		= 0;
			ScrollInfo.nPage	= 0;
			ScrollInfo.nPos		= 0;
			ScrollInfo.fMask &= ~SIF_DISABLENOSCROLL;
		}

		SetScrollInfo( Hwnd(), SB_HORZ, &ScrollInfo, TRUE );
	}
	 */
}

/*
//-------------------------------------------------------------------------
//	load a resource and turn it into a brush
//-------------------------------------------------------------------------
HBRUSH Win32::GWinControl::GetBrushFromResource(int Resource)
{
	HBITMAP HBitmap = NULL;
	HBRUSH HBrush = NULL;
	
	//	didnt load from external file, try loading internal resource
	HBitmap = (HBITMAP)LoadImage( TLCore::Platform::g_HInstance, MAKEINTRESOURCE(Resource), IMAGE_BITMAP, 0, 0, 0x0 );
	if ( HBitmap ) 
	{
		HBrush = CreatePatternBrush( HBitmap );
		DeleteObject( HBitmap );
		return HBrush;
	}

	return HBrush;
}
 */
	
//-------------------------------------------------------------------------
//	setup a timer
//-------------------------------------------------------------------------
void Win32::GWinControl::StartTimer(int TimerID,int Time)
{
	TLDebug_Break("not implemented");
/*
	if ( !SetTimer( m_Hwnd, TimerID, Time, NULL ) )
	{
		TLDebug::Platform::CheckWin32Error();
	}
 */
}

//-------------------------------------------------------------------------
//	stop a registered timer
//-------------------------------------------------------------------------
void Win32::GWinControl::StopTimer(int TimerID)
{
	TLDebug_Break("not implemented");

/*
	if ( !KillTimer( m_Hwnd, TimerID ) )
	{
		TLDebug::Platform::CheckWin32Error();
	}
*/
}



/*
Bool Win32::GWinControl::HandleMessage(u32 message, WPARAM wParam, LPARAM lParam, u32& Result)
{
	Result = 0;
	return FALSE;
}


int Win32::GWinControl::HandleNotifyMessage(u32 message, NMHDR* pNotifyData)
{
	return 0;
}
 */


void Win32::GWinControl::OnActivate()
{
	TLMessaging::TMessage Message("OnWindowChanged");
	Message.ExportData("State", TRef("Activate"));
	PublishMessage(Message);
}


void Win32::GWinControl::OnDeactivate()
{
	TLMessaging::TMessage Message("OnWindowChanged");
	Message.ExportData("State", TRef("Deactivate"));
	PublishMessage(Message);
}








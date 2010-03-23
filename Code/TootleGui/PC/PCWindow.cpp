#include "PCWindow.h"


#if defined(TL_ENABLE_WX)
#include "wxWidgets/Window.h"
#include "wxWidgets/OpenglCanvas.h"
#include "wxWidgets/Tree.h"
#else
#include "PCWinWindow.h"
#endif


//	temp whilst the factory functions are in this file
#include "../TTree.h"


//------------------------------------------------------
//	
//------------------------------------------------------
TPtr<TLGui::TWindow> TLGui::CreateGuiWindow(TRefRef Ref)
{
#if defined(TL_ENABLE_WX)
	TPtr<TLGui::TWindow> pWindow = new wx::Window( Ref );
#else
	TPtr<TLGui::TWindow> pWindow = new TLGui::Platform::Window( Ref );
#endif // TL_ENABLE_WX
	
	return pWindow;
}



TPtr<TLGui::TTree> TLGui::CreateTree(TLGui::TWindow& Parent,TRefRef Ref,TPtr<TLGui::TTreeItem>& pRootItem,const TArray<TRef>& Columns)
{
#if defined(TL_ENABLE_WX)
	TPtr<TLGui::TTree> pControl = new wx::Tree( Parent, Ref, pRootItem, Columns );
#else
	TPtr<TLGui::TTree> pControl;
#endif
	return pControl;
}



TLGui::Platform::Window::Window(TRefRef WindowRef) : 
	TLGui::TWindow		( WindowRef )
{
	//	need to wait for win32 factory to exist
	if ( !Win32::g_pFactory )
	{
		TLDebug_Break("Win32 factory expected");
		return;
	}

	//	create a window
	TPtr<Win32::GWinControl> pNullParent;
	m_pWindow = Win32::g_pFactory->GetInstance( WindowRef, TRUE, "Window" );
	if ( !m_pWindow )
	{
		TLDebug_Break("Failed to allocate window from win32");
		return;
	}

	if ( !m_pWindow->Init( pNullParent, m_pWindow->DefaultFlags() ) )
	{
		TLDebug_Break("Failed to init win32 window");
		Win32::g_pFactory->RemoveInstance( m_pWindow->GetRef() );
		m_pWindow = NULL;
		return;
	}
}


Bool TLGui::Platform::Window::IsVisible() const
{
	return m_pWindow ? !m_pWindow->IsClosed() : false;
}

void TLGui::Platform::Window::Show()
{
	if ( !m_pWindow )
		return;

	m_pWindow->Show();
}


//---------------------------------------------------------
//	set the CLIENT SIZE ("content" in os x) of the window
//---------------------------------------------------------
void TLGui::Platform::Window::SetSize(const int2& WidthHeight)
{
	if ( !m_pWindow )
		return;

	//	gr: not sure this is setting client size?
	m_pWindow->Resize( WidthHeight );
}


//---------------------------------------------------------
//	set the CLIENT SIZE ("content" in os x) of the window
//---------------------------------------------------------	 
int2 TLGui::Platform::Window::GetSize()
{
	if ( !m_pWindow )
		return int2(0,0);

	return m_pWindow->m_ClientSize;
}

//---------------------------------------------------
//	set the top-left position of the window frame
//---------------------------------------------------
void TLGui::Platform::Window::SetPosition(const int2& xy)
{
	if ( !m_pWindow )
		return;

	m_pWindow->Move( xy );
}


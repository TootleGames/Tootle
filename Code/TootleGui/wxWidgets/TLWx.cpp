#if !defined(TL_ENABLE_WX)
#error Should only be built in wx only build
#endif // TL_ENABLE_WX

#include "TLWx.h"
#include <TootleRender/TScreenManager.h>
#include "../TWindow.h"
#include "Window.h"




//------------------------------------------------------
//	
//------------------------------------------------------
TPtr<TLGui::TWindow> TLGui::CreateGuiWindow(TRefRef Ref)
{
	TPtr<TLGui::TWindow> pWindow = new wx::Window( Ref );
	return pWindow;
}



TPtr<TLGui::TTree> TLGui::CreateTree(TLGui::TWindow& Parent,TRefRef Ref,TPtr<TLGui::TTreeItem>& pRootItem,const TArray<TRef>& Columns)
{
	TPtr<TLGui::TTree> pControl = new wx::Tree( Parent, Ref, pRootItem, Columns );
	return pControl;
}



//------------------------------------------------------
//	
//------------------------------------------------------
TPtr<TLGui::TOpenglCanvas> TLGui::CreateOpenglCanvas(TWindow& Parent,TRefRef Ref)
{
	TPtr<TLGui::TOpenglCanvas> pControl = new wx::OpenglCanvas( Parent, Ref );
	return pControl;
}

//---------------------------------------------------------------
//	get the cursor position in the default screen's client space
//---------------------------------------------------------------
int2 TLGui::Platform::GetScreenMousePosition(TLGui::TWindow& Window,u8 MouseIndex)
{
	//	convert mouse screen pos to client space
	wxMouseState MouseState = wxGetMouseState();
	wx::Window& wxWindow = static_cast<wx::Window&>( Window );
	int2 MousePos( MouseState.GetX(), MouseState.GetY() );
	wxWindow.ScreenToClient( &MousePos.x, &MousePos.y );
	
	return MousePos;
}


//------------------------------------------------------
//	convert binary data to a wxvariant
//------------------------------------------------------
bool wx::GetVariant(wxVariant& Variant,const TBinary& Data)
{
	//	todo!
	TTempString Debug;
	Debug << Data.GetDataTypeHint();
	Variant = GetString( Debug );
	return true;
}


//------------------------------------------------------
//	put wxVariant data into binary data
//------------------------------------------------------
bool wx::SetData(const TBinary& Data,const wxVariant& Variant)
{
	//	todo!
	return false;
}


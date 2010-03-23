#if defined(TL_ENABLE_WX)

#include "TLWx.h"
#include <TootleRender/TScreenManager.h>
#include "../TWindow.h"
#include "Window.h"

//----------------------------------------------------------
//	get mouse position relative to this window	
//----------------------------------------------------------
int2 wx::GetScreenMousePosition(TLGui::TWindow& Window)
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

#endif // TL_ENABLE_WX

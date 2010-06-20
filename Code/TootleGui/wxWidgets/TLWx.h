/*------------------------------------------------------
 
	wrapper to include wx
 
 -------------------------------------------------------*/
#pragma once
#if !defined(TL_ENABLE_WX)
#error This file should not be compiled in non-wx builds
#endif // TL_ENABLE_WX

#include "wx/wx.h"
#include <TootleCore/TString.h>
#include <TootleCore/TRef.h>
#include <TootleCore/TBinaryTree.h>
#include "../TWindow.h"


namespace TLGui
{
	namespace Platform
	{
		int2			GetScreenMousePosition(TLGui::TWindow& Window,u8 MouseIndex);
		void			GetDesktopSize(Type4<s32>& DesktopSize);	//	get the desktop dimensions. note: need a window so we can decide which desktop?	
	}
	
}

namespace wx
{
	FORCEINLINE wxWindowID GetID(TRefRef Ref)
	{
		//	todo: test this
		/*
		if ( Ref.IsValid() )
			return Ref.GetData();
		else
		*/	return wxID_ANY;
	}

	//------------------------------------------------------
	//	convert a TString to a wxString
	//------------------------------------------------------
	FORCEINLINE wxString GetString(const TString& String)
	{
		//	wxString compatible buffer
		TFixedArray<wchar_t,1000> CharBuffer;
		const TArray<TChar>& StringArray = String.GetStringArray();
		for ( u32 i=0;	i<StringArray.GetSize();	i++ )
		{
			wchar_t wchar = (wchar_t)StringArray[i];
			CharBuffer.Add( wchar );
		}
		CharBuffer.Add( (wchar_t)0 );
	
		return wxString( CharBuffer.GetData() );
	}

	//------------------------------------------------------
	//	quick function to convert a ref to a wxString
	//------------------------------------------------------
	DEPRECATED FORCEINLINE wxString GetString(TRefRef Ref)
	{
		TTempString RefString;
		Ref.GetString( RefString );
	
		return GetString( RefString );
	}
	
	bool	GetVariant(wxVariant& Variant,const TBinary& Data);	//	convert binary data to a wxvariant
	bool	SetData(const TBinary& Data,const wxVariant& Variant);	//	put wxVariant data into binary data

	int2	GetScreenMousePosition(TLGui::TWindow& Window);	//	get mouse position relative to this window
}





/*------------------------------------------------------

	The GUI library is a generic interface to the OS's windows
	and controls. For the Mac and PC this goes through wxWidgets
	(via our own wrapper)

-------------------------------------------------------*/
#pragma once

#include <TootleCore/TLTypes.h>
#include <TootleCore/TRef.h>
#include <TootleCore/TPtr.h>

namespace TLGui
{
	//	generic types
	class TControl;			//	an OS control inside a window
	class TWindow;			//	a OS window. 
	
	//	control types
	class TOpenglCanvas;	//	opengl area - maybe rename this to "RenderCanvas" to cope with directx's requirements etc
	class TTree;			//	Multi-columned tree
	class TTreeItem;		//	item in a tree (not a control)

	SyncBool				Init();		//	lib init
	SyncBool				Shutdown();	//	lib init

	const TString&			GetAppExe();
	void					SetAppExe(const TString& NewExePath);
	bool					OnCommandLine(const TString& CommandLine,int& Result);	//	handle command line params - if true is returned, return Result from main

	//	factory style wrappers. Temporary implementation until I decide how data/factory driven I want this to be	
	TPtr<TWindow>			CreateGuiWindow(TRefRef Ref);
	TPtr<TOpenglCanvas>		CreateOpenglCanvas(TWindow& Parent,TRefRef Ref);
	TPtr<TTree>				CreateTree(TWindow& Parent,TRefRef Ref,TPtr<TTreeItem>& pRootItem,const TArray<TRef>& Columns);	//	render[0] = ItemData[ column[0] ]
	
	//	some hacky global functions
	int2					GetDefaultScreenMousePosition(u8 MouseIndex);	//	get the cursor position in the default screen's client space

	namespace Platform
	{
		class Window;	//	os-specific window (non-wx!)

		SyncBool		Init();
		SyncBool		Shutdown();
		int2			GetScreenMousePosition(TLGui::TWindow& Window,u8 MouseIndex);
		void			GetDesktopSize(Type4<s32>& DesktopSize);	//	get the desktop dimensions. note: need a window so we can decide which desktop?	
	}
};



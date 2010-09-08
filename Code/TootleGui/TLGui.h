/*------------------------------------------------------

	The GUI library is a generic interface to the OS's windows
	and controls. For the Mac and PC this goes through wxWidgets
	(via our own wrapper)

-------------------------------------------------------*/
#pragma once

#include <TootleCore/TLTypes.h>
#include <TootleCore/TRef.h>
#include <TootleCore/TPtr.h>


namespace TLMessaging
{
	class TPublisher;
}

namespace TLGui
{
	//	generic types
	class TControl;			//	an OS control inside a window
	class TWindow;			//	a OS window. 
	class TMenuWrapper;		//	manages link between a menu controller and a menu attached to a window (not OS specialised)
	class TMenuHandler;		//	OS menu handler - try and merge menu wrapper and handler?
	
	//	control types
	class TOpenglCanvas;	//	opengl area - maybe rename this to "RenderCanvas" to cope with directx's requirements etc
	class TTree;			//	Multi-columned tree
	class TTreeItem;		//	item in a tree (not a control)

	SyncBool				Init();		//	lib init
	SyncBool				Shutdown();	//	lib init

	DEPRECATED TString		GetAppExe();
	const TString&			GetAppPath();
	void					SetAppPath(const TString& Path);
	bool					OnCommandLine(const TString& CommandLine,int& Result);	//	handle command line params - if true is returned, return Result from main

	//	factory style wrappers. Temporary implementation until I decide how data/factory driven I want this to be	
	TPtr<TWindow>			CreateGuiWindow(TRefRef Ref);
	TPtr<TOpenglCanvas>		CreateOpenglCanvas(TWindow& Parent,TRefRef Ref);
	TPtr<TTree>				CreateTree(TWindow& Parent,TRefRef Ref,TPtr<TTreeItem>& pRootItem,const TArray<TRef>& Columns);	//	render[0] = ItemData[ column[0] ]
	
	//	some hacky global functions
	DEPRECATED int2			GetDefaultScreenMousePosition();	//	get the cursor position in the default screen's client space

	namespace Platform
	{
		SyncBool		Init();
		SyncBool		Shutdown();
		void			GetDesktopSize(Type4<s32>& DesktopSize);	//	get the desktop dimensions. note: need a window so we can decide which desktop?	

#if defined(TL_TARGET_PC)
		extern void*	g_HInstance;	//	HINSTANCE
#endif
	}
};


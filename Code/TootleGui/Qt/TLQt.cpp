#include "TLQt.h"
#include <TootleRender/TScreenManager.h>
#include "../TWindow.h"
#include "Window.h"
#include "OpenglCanvas.h"
#include "Tree.h"
#include "App.h"



//------------------------------------------------------
//	
//------------------------------------------------------
SyncBool TLGui::Platform::Init()
{
	return SyncTrue;
}

//------------------------------------------------------
//	
//------------------------------------------------------
SyncBool TLGui::Platform::Shutdown()
{
	return SyncTrue;
}


//------------------------------------------------------
//	
//------------------------------------------------------
TPtr<TLGui::TWindow> TLGui::CreateGuiWindow(TRefRef Ref)
{
	//	TPtr<TLGui::TWindow> pWindow = new Qt::Window( Ref );
	TPtr<TLGui::TWindow> pWindow;
	return pWindow;
}



TPtr<TLGui::TTree> TLGui::CreateTree(TLGui::TWindow& Parent,TRefRef Ref,TPtr<TLGui::TTreeItem>& pRootItem,const TArray<TRef>& Columns)
{
	//TPtr<TLGui::TTree> pControl = new Qt::Tree( Parent, Ref, pRootItem, Columns );
	TPtr<TLGui::TTree> pControl;
	return pControl;
}



//------------------------------------------------------
//	
//------------------------------------------------------
TPtr<TLGui::TOpenglCanvas> TLGui::CreateOpenglCanvas(TWindow& Parent,TRefRef Ref)
{
//	TPtr<TLGui::TOpenglCanvas> pControl = new Qt::OpenglCanvas( Parent, Ref );
	TPtr<TLGui::TOpenglCanvas> pControl;
	return pControl;
}


//---------------------------------------------------------------
//	get the cursor position in the default screen's client space
//---------------------------------------------------------------
int2 TLGui::Platform::GetScreenMousePosition(TLGui::TWindow& Window,u8 MouseIndex)
{
	return int2(0,0);
}


//----------------------------------------------------------
//	get the desktop dimensions. note: need a window so we can decide which desktop?
//----------------------------------------------------------
void TLGui::Platform::GetDesktopSize(Type4<s32>& DesktopSize)
{
	DesktopSize.x = 0;
	DesktopSize.y = 0;
	
	DesktopSize.Width() = 800;
	DesktopSize.Height() = 800;
}






int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
	MainWindow w;
	w.show();
    return a.exec();
}



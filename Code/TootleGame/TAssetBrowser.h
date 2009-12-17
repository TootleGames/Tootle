/*------------------------------------------------------

	Asset browser

-------------------------------------------------------*/
#pragma once


#include "TMenuController.h"


namespace TLGame
{
	class TAssetBrowser;
}

namespace TLRender
{
	class TRenderNode;
	class TRenderTarget;
}


//----------------------------------------------
//	this is the menu renderer AND menu controller in one class
//----------------------------------------------
class TLGame::TAssetBrowser : public TLMenu::TMenuController
{
public:
	TAssetBrowser(TRefRef ScreenRef,TRefRef FontRef);
	~TAssetBrowser()				{	Shutdown();	}

	SyncBool						Initialise();		//	async initialise routine
	void							Shutdown();			//	cleanup all the stuff we created

protected:
	TPtr<TLRender::TRenderTarget>	GetRenderTarget();	//	find our render target

	virtual TPtr<TLMenu::TMenu>		CreateMenu(TRefRef MenuRef);
	virtual Bool					ExecuteCommand(TRefRef MenuCommand);
	virtual void					ProcessMessage(TLMessaging::TMessage& Message);

	void							CreateAssetRenderMenu();
	void							CloseAssetRenderMenu();
	void							CreateRootRenderMenu();
	void							CloseRootRenderMenu();
	void							UpdateAssetMenuAnim();

protected:
	TRef							m_FontRef;
	TRef							m_RenderTargetRef;
	TRef							m_ScreenRef;
	TPtr<TLRender::TRenderNode>		m_pMenuRenderNode;	//	root render object for the menu
	TRef							m_RootMenuNode;
	TRef							m_AssetMenuNode;
	TArray<TRef>					m_ClosingAssetMenuNodes;
};



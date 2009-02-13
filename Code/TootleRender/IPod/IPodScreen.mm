/*
 *  IPodScreen.cpp
 *  TootleRender
 *
 *  Created by Graham Reeves on 01/09/2008.
 *  Copyright 2008 Tootle. All rights reserved.
 *
 */

#include "IPodScreen.h"

#include <TootleCore/TCoreManager.h>
#import <TootleCore/IPod/IPodApp.h>


#define USE_TEST_EMAIL

namespace TLCore
{
	extern TPtr<TCoreManager>		g_pCoreManager;
}





TLRender::Platform::Screen::Screen(TRefRef Ref,TLRender::TScreenShape ScreenShape) :
	TLRender::TScreen	( Ref, ScreenShape )
{
}


//----------------------------------------------------------
//	create window
//----------------------------------------------------------
SyncBool TLRender::Platform::Screen::Init()
{
	//	no gl view? fail!
	EAGLView *glView = TLCore::Platform::g_pIPodApp.glView;
	if ( !glView )
		return SyncFalse;

/*
	// Create window
	g_pWindow = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];

	// mode: window with status bar
	CGRect ScreenSize = [[UIScreen mainScreen] applicationFrame];
	// mode: full screen
	//CGRect ScreenSize = [[UIScreen mainScreen] bounds];
    
    // Set up content view
    UIView* glview = [[eglView alloc] initWithFrame:ScreenSize];
	[g_pWindow addSubview:glview];
    
	// Show window
	[g_pWindow makeKeyAndVisible];

	m_Size.Left() = ScreenSize.origin.x;
	m_Size.Top() = ScreenSize.origin.y;
	m_Size.Width() = ScreenSize.size.width;
	m_Size.Height() = ScreenSize.size.height;
*/

	//	save off dimensions for our engine
	m_Size.Left() = 0;
	m_Size.Top() = 0;
	m_Size.Width() = glView.backingWidth;
	m_Size.Height() = glView.backingHeight;
	
	return TScreen::Init();
}



//----------------------------------------------------------
//	update window
//----------------------------------------------------------
SyncBool TLRender::Platform::Screen::Update()
{
	return SyncTrue;
}



void TLRender::Platform::Screen::Draw()
{
	EAGLView *glView = TLCore::Platform::g_pIPodApp.glView;

	[EAGLContext setCurrentContext:glView.context];
	glBindFramebufferOES(GL_FRAMEBUFFER_OES, glView.viewFramebuffer);
	
	//	do inherited draw
	TScreen::Draw();

	//	unbind data
	//	gr: not needed?
	Opengl::Unbind();

	//	flip buffers
    glBindRenderbufferOES(GL_RENDERBUFFER_OES, glView.viewRenderbuffer);
    [glView.context presentRenderbuffer:GL_RENDERBUFFER_OES];
    
    
	// Take a screenshot if flagged to do so.  
	// I'm not sure this is the right place but...
	if(GetFlag(Flag_TakeScreenshot))
	{
		[glView saveViewToPhotoLibrary];
		//[glView saveViewToPhotoLibraryAndSetupEmail];
		m_Flags.Set(Flag_TakeScreenshot, FALSE);
	}
}


//----------------------------------------------------------
//	clean up
//----------------------------------------------------------
SyncBool TLRender::Platform::Screen::Shutdown()
{
	return TScreen::Shutdown();
}


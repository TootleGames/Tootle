/*------------------------------------------------------
	A screen is the owner for render targets. In windows the screen
	is the Win32 window control. Win could have multiple screens
	and they're flexible.
	Other platforms have just 1 screen, fixed resolution etc

-------------------------------------------------------*/
#pragma once


#include <TootleCore/TLTypes.h>
#include <TootleCore/TPtrArray.h>
#include <TootleCore/TKeyArray.h>
#include <TootleCore/TRelay.h>
#include <TootleCore/TFlags.h>
#include "TRenderTarget.h"		//	gr: needed to include the array type because of templated destructor... would be good to find a way to avoid that
//#include "TLRender.h"		//	gr: needed to include the array type because of templated destructor... would be good to find a way to avoid that
#include <TootleGui/TWindow.h>
#include <TootleGui/TOpenglCanvas.h>

#include "TScreenShape.h"

namespace TLRender
{
	class TScreen;
	class TRenderTarget;
	class TRenderNodeText;

	//	gr: these are no longer platform specific... move them out of the Platform namespace
	namespace Platform
	{
		class Screen;
		class ScreenWide;
		class ScreenWideLeft;	//	rotated-left display for ipod (or ipod emulation)
		class ScreenWideRight;	//	rotated-right display for ipod (or ipod emulation)
	}

	static const s32		g_MaxSize		= -1;	//	for screen & render target sizes -1 indicates max extents
}

//	forward declaration
namespace TLGui
{
	class TWindow;
}

//---------------------------------------------------------
//	base screen type
//---------------------------------------------------------
class TLRender::TScreen : public TLMessaging::TRelay
{
public:
	enum Flags
	{
		Flag_SyncFrameRate=0,	//	synchronise frames to refresh rate
		Flag_TakeScreenshot,	//	Take a screenshot
	};

public:
	TScreen(TRefRef ScreenRef,TScreenShape ScreenShape);
	~TScreen();

	virtual TRefRef						GetSubscriberRef() const			{	return GetRef();	}
	FORCEINLINE TRefRef					GetRef() const						{	return m_Ref;	}
	virtual Type4<s32>					GetSize() const						{	return m_Size;	}
	FORCEINLINE TFlags<Flags>&			GetFlags()							{	return m_Flags;	}
	FORCEINLINE Bool					GetFlag(TScreen::Flags Flag) const	{	return m_Flags(Flag);	}
	FORCEINLINE TScreenShape			GetScreenShape() const				{	return m_ScreenShape;	}
	FORCEINLINE const TLMaths::TAngle&	GetScreenAngle() const				{	return TLRender::GetScreenAngle( GetScreenShape() );	}

	virtual SyncBool				Init();
	virtual SyncBool				Update();
	virtual SyncBool				Shutdown();

	virtual void					Draw();			//	render our render targets

	virtual TLGui::TWindow*			GetWindow()		{	return NULL;	}	//	get the GUI window for this screen
	
	TPtr<TLRender::TRenderTarget>	CreateRenderTarget(TRefRef TargetRef);					// Creates a new render target
	SyncBool						DeleteRenderTarget(TRefRef TargetRef);					//	shutdown a render target
	void							OnRenderTargetZChanged(const TRenderTarget& RenderTarget);	//	z has changed on render target - resorts render targets

	TPtr<TRenderTarget>&			GetRenderTarget(TRefRef TargetRef);						//	fetch render target

	void							GetRenderTargetMaxSize(Type4<s32>& MaxSize);			//	get the render target max size (in "render target space") - this is the viewport size, but rotated
	virtual void					GetViewportMaxSize(Type4<s32>& MaxSize)					{	MaxSize.Set( 0, 0, GetSize().Width(), GetSize().Height() );	}
	Bool							GetRenderTargetSize(Type4<s32>& Size,const TRenderTarget& RenderTarget);	//	get the dimensions of a render target
	Bool							GetRenderTargetSize(Type4<s32>& Size,TRefRef TargetRef);

	Bool							GetWorldRayFromScreenPos(const TRenderTarget& RenderTarget,TLMaths::TLine& WorldRay,const Type2<s32>& ScreenPos);
	Bool							GetWorldPosFromScreenPos(const TRenderTarget& RenderTarget,float3& WorldPos,float WorldDepth,const Type2<s32>& ScreenPos);
	Bool							GetScreenPosFromWorldPos(const TRenderTarget& RenderTarget,const float3& WorldPos, Type2<s32>& ScreenPos);

	FORCEINLINE void				RequestScreenshot()							{	m_Flags.Set(Flag_TakeScreenshot);	}	// Screenshot request

	TLRender::TRenderNodeText*		Debug_GetRenderNodeText(TRefRef DebugTextRef);	//	return text render node for this debug text

	FORCEINLINE Bool				operator==(const TRef& ScreenRef)	const	{	return GetRef() == ScreenRef;	}
	FORCEINLINE Bool				operator==(const TScreen& Screen)	const 	{	return GetRef() == Screen.GetRef();	}

protected:
	Bool							GetRenderTargetPosFromScreenPos(const TRenderTarget& RenderTarget,Type2<s32>& RenderTargetPos,Type4<s32>& RenderTargetSize,const Type2<s32>& ScreenPos);	//	Get a render target-relative cursor position from a screen pos - fails if outside render target box
	Bool							GetScreenPosFromRenderTargetPos(Type2<s32>& ScreenPos, const TRenderTarget& RenderTarget,const Type2<s32>& RenderTargetPos, Type4<s32>& RenderTargetSize);	//	Get a screen pos render target-relative cursor position- fails if outside render target box
	void							CreateDebugRenderTarget(TRefRef FontRef=TRef("FDebug"));

protected:
	TPtrArray<TRenderTarget,4,TSortPolicyPtrSorted<TRenderTarget,u8> >		m_RenderTargets;	//	render targets sorted by depth
	TPtrArray<TRenderTarget>		m_ShutdownRenderTargets;	//	list of render targets we're destroying
	Bool							m_HasShutdown;				//	
	TRef							m_Ref;						//	reference to screen
	Type4<s32>						m_Size;						//	pos + w + h. Note viewport maybe smaller (ie. because of window borders in windows)
	TFlags<Flags>					m_Flags;					//	screen flags
	TScreenShape					m_ScreenShape;				//	screen orientation
	
	TRef							m_DebugRenderTarget;		//	debug render target
	TKeyArray<TRef,TRef>			m_DebugRenderText;			//	keyed list of debug strings -> render nodes
};



//----------------------------------------------------------
//	win32 screen (it's actually just an opengl window)
//----------------------------------------------------------
class TLRender::Platform::Screen : public TLRender::TScreen
{
public:
	Screen(TRefRef ScreenRef,TScreenShape ScreenShape);
	
	virtual SyncBool		Init();
	virtual SyncBool		Update();
	virtual SyncBool		Shutdown();
	
	virtual void			Draw();
	virtual Type4<s32>		GetSize() const;	//	get size of the screen
	virtual TLGui::TWindow*	GetWindow()			{	return m_pWindow;	}	//	get the GUI window for this screen
	
protected:
	void					GetDesktopSize(Type4<s32>& DesktopSize) const;	//	get the desktop dimensions
	void					GetCenteredSize(Type4<s32>& Size) const;		//	take a screen size and center it on the desktop
	virtual void			GetViewportMaxSize(Type4<s32>& MaxSize);	//	need to max-out to client-area on the window 
	
protected:
	TPtr<TLGui::TWindow>		m_pWindow;
	TPtr<TLGui::TOpenglCanvas>	m_pCanvas;
};



//----------------------------------------------------------
//	widescreen screen
//----------------------------------------------------------
class TLRender::Platform::ScreenWide : public TLRender::Platform::Screen
{
public:
	ScreenWide(TRefRef ScreenRef) :
	TLRender::Platform::Screen	( ScreenRef, TLRender::ScreenShape_Wide )
	{
		//	swap dimensions
		TLMaths::SwapVars( m_Size.Height(), m_Size.Width() );
	}
};


//----------------------------------------------------------
//	widescreen screen
//----------------------------------------------------------
class TLRender::Platform::ScreenWideLeft : public TLRender::Platform::Screen
{
public:
	ScreenWideLeft(TRefRef ScreenRef) :
		TLRender::Platform::Screen	( ScreenRef, TLRender::ScreenShape_WideLeft )
	{
	}
};


//----------------------------------------------------------
//	widescreen screen
//----------------------------------------------------------
class TLRender::Platform::ScreenWideRight : public TLRender::Platform::Screen
{
public:
	ScreenWideRight(TRefRef ScreenRef) :
		TLRender::Platform::Screen	( ScreenRef, TLRender::ScreenShape_WideRight )
	{
	}
};


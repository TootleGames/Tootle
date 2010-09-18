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
#include "TRasteriser.h"		//	gr: needed to include the array type because of templated destructor... would be good to find a way to avoid that
#include <TootleGui/TWindow.h>
#include <TootleGui/TOpenglCanvas.h>

#include "TScreenShape.h"

//	the default size is the iphone size
#define TLScreen_DefaultSize	Type2<u16>( 320, 480 )

namespace TLRender
{
	class TScreen;
	class TRenderTarget;
	class TRenderNodeText;

	class TScreenWide;		//	should be deprecated now...
	class TScreenWideLeft;	//	rotated-left display for ipod (or ipod emulation)
	class TScreenWideRight;	//	rotated-right display for ipod (or ipod emulation)

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
	TScreen(TRefRef ScreenRef,const Type2<u16>& Size=TLScreen_DefaultSize,TScreenShape ScreenShape=ScreenShape_Portrait);
	~TScreen();

	virtual TRefRef						GetSubscriberRef() const			{	return GetRef();	}
	FORCEINLINE TRefRef					GetRef() const						{	return m_Ref;	}
	Type2<u16>							GetSize() const						{	return m_pCanvas ? m_pCanvas->GetSize() : Type2<u16>(0,0);	}
	FORCEINLINE TFlags<Flags>&			GetFlags()							{	return m_Flags;	}
	FORCEINLINE Bool					GetFlag(TScreen::Flags Flag) const	{	return m_Flags(Flag);	}
	FORCEINLINE TScreenShape			GetScreenShape() const				{	return m_ScreenShape;	}
	FORCEINLINE const TLMaths::TAngle&	GetScreenAngle() const				{	return TLRender::GetScreenAngle( GetScreenShape() );	}
			
	virtual SyncBool				Init();
	virtual SyncBool				Update();
	virtual SyncBool				Shutdown();

	virtual void					Draw();			//	render our render targets
	TLGui::TWindow*					GetWindow()			{	return m_pWindow;	}	//	get the GUI window for this screen
	
	TPtr<TLRender::TRenderTarget>	CreateRenderTarget(TRefRef TargetRef);					// Creates a new render target
	SyncBool						DeleteRenderTarget(TRefRef TargetRef);					//	shutdown a render target
	void							OnRenderTargetZChanged(const TRenderTarget& RenderTarget);	//	z has changed on render target - resorts render targets

	TPtr<TRenderTarget>&			GetRenderTarget(TRefRef TargetRef);						//	fetch render target

	Type4<s32>						GetRenderTargetMaxSize();								//	get the render target max size (in "render target space") - this is the viewport size, but rotated
	Type4<s32>						GetViewportSize() const									{	return Type4<s32>( 0, 0, GetSize().x, GetSize().y );	}
	DEPRECATED Type4<s32>			GetViewportMaxSize() const								{	return GetViewportSize();	}
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
	void							GetDesktopSize(Type4<s32>& DesktopSize) const;	//	get the desktop dimensions
	void							GetCenteredSize(Type4<s32>& Size) const;		//	take a screen size and center it on the desktop
	Bool							GetRenderTargetPosFromScreenPos(const TRenderTarget& RenderTarget,Type2<s32>& RenderTargetPos,Type4<s32>& RenderTargetSize,const Type2<s32>& ScreenPos);	//	Get a render target-relative cursor position from a screen pos - fails if outside render target box
	Bool							GetScreenPosFromRenderTargetPos(Type2<s32>& ScreenPos, const TRenderTarget& RenderTarget,const Type2<s32>& RenderTargetPos, Type4<s32>& RenderTargetSize);	//	Get a screen pos render target-relative cursor position- fails if outside render target box
	void							CreateDebugRenderTarget(TRefRef FontRef=TRef("FDebug"));

protected:
	TPtrArray<TRenderTarget,4,TSortPolicyPtrSorted<TRenderTarget,u8> >		m_RenderTargets;	//	render targets sorted by depth
	TPtrArray<TRenderTarget>		m_ShutdownRenderTargets;	//	list of render targets we're destroying
	Bool							m_HasShutdown;				//	
	TRef							m_Ref;						//	reference to screen
	TFlags<Flags>					m_Flags;					//	screen flags
	TScreenShape					m_ScreenShape;				//	screen orientation
	Type2<u16>						m_InitialSize;				//	desired size from constructor
	
	TRef							m_DebugRenderTarget;		//	debug render target
	TKeyArray<TRef,TRef>			m_DebugRenderText;			//	keyed list of debug strings -> render nodes

	TPtr<TLRaster::TRasteriser>		m_pRasteriser;				//	rasteriser (tied to the canvas - maybe it should be owned and created by the canvas?)
	TPtr<TLGui::TWindow>			m_pWindow;					//	window for the screen which contains the canvas (todo; remove this and just have the screen work from a canvas - move window construction external)
	TPtr<TLGui::TOpenglCanvas>		m_pCanvas;					//	canvas we render to. todo; when rasterisation goes in, the rasteriser will instance a base RasterCanvas type
};



//----------------------------------------------------------
//	widescreen screen
//----------------------------------------------------------
class TLRender::TScreenWide : public TLRender::TScreen
{
public:
	TScreenWide(TRefRef ScreenRef,const Type2<u16>& Size=TLScreen_DefaultSize) :
		TScreen	( ScreenRef, Type2<u16>( Size.y, Size.x ), TLRender::ScreenShape_Wide )
	{
	}
};


//----------------------------------------------------------
//	widescreen screen
//----------------------------------------------------------
class TLRender::TScreenWideLeft : public TLRender::TScreen
{
public:
	TScreenWideLeft(TRefRef ScreenRef,const Type2<u16>& Size=TLScreen_DefaultSize) :
		TScreen	( ScreenRef, Size, TLRender::ScreenShape_WideLeft )
	{
	}
};


//----------------------------------------------------------
//	widescreen screen
//----------------------------------------------------------
class TLRender::TScreenWideRight : public TLRender::TScreen
{
public:
	TScreenWideRight(TRefRef ScreenRef,const Type2<u16>& Size=TLScreen_DefaultSize) :
		TScreen	( ScreenRef, Size, TLRender::ScreenShape_WideRight )
	{
	}
};


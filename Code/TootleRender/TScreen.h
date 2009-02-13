/*------------------------------------------------------
	A screen is the owner for render targets. In windows the screen
	is the Win32 window control. Win could have multiple screens
	and they're flexible.
	Other platforms have just 1 screen, fixed resolution etc

-------------------------------------------------------*/
#pragma once


#include <TootleCore/TLTypes.h>
#include <TootleCore/TPtr.h>
#include <TootleCore/TPtrArray.h>
#include "TRenderTarget.h"		//	gr: needed to include the array type because of templated destructor... would be good to find a way to avoid that
#include "TLRender.h"		//	gr: needed to include the array type because of templated destructor... would be good to find a way to avoid that


namespace TLRender
{
	class TScreen;
	class TRenderTarget;

	namespace Platform
	{
		class Screen;
	}

	static const s32		g_MaxSize		= -1;	//	for screen & render target sizes -1 indicates max extents
}


//---------------------------------------------------------
//	base screen type
//---------------------------------------------------------
class TLRender::TScreen : public TLMessaging::TPublisher
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

	TPtr<TLRender::TRenderTarget>	CreateRenderTarget(TRefRef TargetRef);					// Creates a new render target
	SyncBool						DeleteRenderTarget(TRefRef TargetRef);					//	shutdown a render target

	TPtr<TRenderTarget>&			GetRenderTarget(TRefRef TargetRef);						//	fetch render target

	virtual void					GetRenderTargetMaxSize(Type4<s32>& MaxSize)				{	MaxSize.Set( 0, 0, GetSize().Width(), GetSize().Height() );	}
	Bool							GetRenderTargetSize(Type4<s32>& Size,const TPtr<TRenderTarget>& pRenderTarget);	//	get the dimensions of a render target
	FORCEINLINE Bool				GetRenderTargetSize(Type4<s32>& Size,TRefRef TargetRef) {	return GetRenderTargetSize( Size, GetRenderTarget( TargetRef ) );	}

	Bool							GetWorldRayFromScreenPos(const TPtr<TRenderTarget>& pRenderTarget,TLMaths::TLine& WorldRay,const Type2<s32>& ScreenPos);

	FORCEINLINE void				RequestScreenshot()							{	m_Flags.Set(Flag_TakeScreenshot);	}	// Screenshot request

	FORCEINLINE Bool				operator==(const TRef& ScreenRef)	const	{	return GetRef() == ScreenRef;	}
	FORCEINLINE Bool				operator==(const TScreen& Screen)	const 	{	return GetRef() == Screen.GetRef();	}


protected:
	TPtrArray<TRenderTarget>		m_RenderTargets;			//	list of active render targets
	TPtrArray<TRenderTarget>		m_ShutdownRenderTargets;	//	list of render targets we're destroying
	Bool							m_HasShutdown;				//	
	TRef							m_Ref;						//	reference to screen
	Type4<s32>						m_Size;						//	pos + w + h
	TFlags<Flags>					m_Flags;					//	screen flags
	TScreenShape					m_ScreenShape;				//	screen orientation
};



#include "TScreen.h"
#include "TRenderTarget.h"

#if defined(TL_TARGET_PC)
	#include "PC/PCRenderTarget.h"
#elif defined(TL_TARGET_IPOD)
	#include "IPod/IPodRenderTarget.h"
#elif defined(TL_TARGET_MAC)
	#include "IPod/IPodRenderTarget.h"
#endif



//---------------------------------------------------------
//	
//---------------------------------------------------------
TLRender::TScreen::TScreen(TRefRef Ref,TScreenShape ScreenShape) :
	m_HasShutdown	( FALSE ),
	m_Ref			( Ref ),
	m_Size			( g_MaxSize,g_MaxSize,g_MaxSize,g_MaxSize ),
	m_ScreenShape	( ScreenShape )
{
	//	gr: disabled for now, core manager limits frame rate instead of using hardware sync
	//m_Flags.Set( Flag_SyncFrameRate );
}


//---------------------------------------------------------
//	
//---------------------------------------------------------
TLRender::TScreen::~TScreen()
{
	//	if we haven't had a shutdown invoke an emergency shutdown
	if ( !m_HasShutdown )
		Shutdown();
}


//---------------------------------------------------------
//	do screen initialisation
//---------------------------------------------------------
SyncBool TLRender::TScreen::Init()
{
	return SyncTrue;
}



//---------------------------------------------------------
//	do screen update
//---------------------------------------------------------
SyncBool TLRender::TScreen::Update()
{
	return SyncTrue;
}



//---------------------------------------------------------
//	clean up
//---------------------------------------------------------
SyncBool TLRender::TScreen::Shutdown()
{
	//	already done a successfull shutdown
	if ( m_HasShutdown )
		return SyncTrue;

	SyncBool ShutdownResult = SyncTrue;

	//	clean up render targets
	if ( m_RenderTargets.GetSize() )
	{
		for ( s32 r=m_RenderTargets.GetSize()-1;	r>=0;	r-- )
		{
			/*
			SyncBool Result = m_RenderTargets[r]->Shutdown();
			
			//	error
			if ( Result == SyncFalse )
				return SyncFalse;

			//	wait
			if ( Result == SyncWait )
			{
				ShutdownResult = SyncWait;
				continue;
			}
			*/

			//	shutdown okay, release
			m_RenderTargets[r] = NULL;
			m_RenderTargets.RemoveAt( r );
		}
	}

	//	mark as shutdown if everything has succeeded
	if ( ShutdownResult == SyncTrue )
		m_HasShutdown = TRUE;

	return ShutdownResult;
}

	
//---------------------------------------------------------
//	render the render targets
//---------------------------------------------------------
void TLRender::TScreen::Draw()
{
	Type4<s32> RenderTargetMaxSize;
	GetRenderTargetMaxSize( RenderTargetMaxSize );
	Type4<s32> ViewportMaxSize;
	GetViewportMaxSize( ViewportMaxSize );

	//	render each render target
	for ( u32 r=0;	r<m_RenderTargets.GetSize();	r++ )
	{
		//	get render target
		TPtr<TRenderTarget>& pRenderTarget = m_RenderTargets[r];
		if ( !pRenderTarget.IsValid() )
			continue;

		//	begin draw of render target
		if ( !pRenderTarget->BeginDraw( RenderTargetMaxSize, ViewportMaxSize, *this ) )
			continue;

		//	draw
		pRenderTarget->Draw();

		//	cleanup draw
		pRenderTarget->EndDraw();
	}	

}

TPtr<TLRender::TRenderTarget> TLRender::TScreen::CreateRenderTarget(TRefRef TargetRef)
{
	// Check to make sure a render target with the specified name doesn;t already exist
	if ( m_RenderTargets.Exists( TargetRef ) )
		return NULL;

	// Create a new render target and add it to the list
	TPtr<TLRender::TRenderTarget> pRenderTarget = new TLRender::Platform::RenderTarget(TargetRef);

	if(!pRenderTarget)
		return NULL;

	//	add render target to list
	m_RenderTargets.Add( pRenderTarget );
	return pRenderTarget;
}


//---------------------------------------------------------
//	fetch render target
//---------------------------------------------------------
TPtr<TLRender::TRenderTarget>& TLRender::TScreen::GetRenderTarget(const TRef& TargetRef)
{
	return m_RenderTargets.FindPtr( TargetRef );
}


//---------------------------------------------------------
//	shutdown a render target
//---------------------------------------------------------
SyncBool TLRender::TScreen::DeleteRenderTarget(const TRef& TargetRef)
{
	//	find the active render target index 
	s32 Index = m_RenderTargets.FindIndex(TargetRef);

	//	doesnt exist
	if ( Index == -1 )
	{
		//	if it's in the shutdown list, return wait
		if ( m_ShutdownRenderTargets.Exists( TargetRef ) )
			return SyncWait;

		//	non-existant target ref
		return SyncFalse;
	}

	//	grab pointer to the render target
	TPtr<TRenderTarget> pRenderTarget = m_RenderTargets[Index];

	//	remove from render target list
	m_RenderTargets.RemoveAt( (u32)Index );

	pRenderTarget = NULL;
	/*
	//	shutdown render target
	SyncBool Result = pRenderTarget->Shutdown();

	//	instant shutdown, so destroy
	if ( Result != SyncWait )
	{
		pRenderTarget = NULL;
		return Result;
	}

	//	is shutting down, move to shutdown list
	m_ShutdownRenderTargets.Add( pRenderTarget );
	*/

	return SyncWait;
}



//---------------------------------------------------------
//	get the dimensions of a render target
//---------------------------------------------------------
Bool TLRender::TScreen::GetRenderTargetSize(Type4<s32>& Size,const TPtr<TRenderTarget>& pRenderTarget)
{
	if ( !pRenderTarget )
		return FALSE;

	Type4<s32> RenderTargetMaxSize;
	GetRenderTargetMaxSize( RenderTargetMaxSize );

	pRenderTarget->GetSize( Size, RenderTargetMaxSize );

	return TRUE;
}


//---------------------------------------------------------
//	get a world position from this screen posiiton
//---------------------------------------------------------
Bool TLRender::TScreen::GetWorldRayFromScreenPos(const TPtr<TRenderTarget>& pRenderTarget,TLMaths::TLine& WorldRay,const Type2<s32>& ScreenPos)
{
	if ( !pRenderTarget )
	{
		TLDebug_Break("RenderTarget expected");
		return FALSE;
	}

	return GetWorldRayFromScreenPos( *pRenderTarget.GetObject(), WorldRay, ScreenPos );
}



//---------------------------------------------------------
//	Get a render target-relative cursor position from a screen pos - fails if outside render target box
//---------------------------------------------------------
Bool TLRender::TScreen::GetRenderTargetPosFromScreenPos(const TRenderTarget& RenderTarget,Type2<s32>& RenderTargetPos,Type4<s32>& RenderTargetSize,const Type2<s32>& ScreenPos)
{
	//	check the point is inside the screen viewport
	Type4<s32> ViewportMaxSize;
	GetViewportMaxSize( ViewportMaxSize );

	if ( !ViewportMaxSize.GetIsInside( ScreenPos ) )
		return FALSE;

	//	convert screen(viewport) pos to render target pos by rotating it inside the viewport
	RenderTargetPos = ScreenPos;

	Type4<s32> MaxRenderTargetSize;
	GetRenderTargetMaxSize( MaxRenderTargetSize );

	//	rotate screen pos to be in "render target" space
	if ( GetScreenShape() == TLRender::ScreenShape_WideLeft )
	{
		//	rotate RIGHT
		RenderTargetPos.Left() = MaxRenderTargetSize.Right()  - ScreenPos.Top();
	//	RenderTargetPos.Left() = ViewportMaxSize.Right()  - ScreenPos.Top();
		RenderTargetPos.Top() = ScreenPos.Left();
	}
	else if ( GetScreenShape() == TLRender::ScreenShape_WideRight )
	{
		//	rotate LEFT
		RenderTargetPos.Left() = ScreenPos.Top();
	//	RenderTargetPos.Top() = ViewportMaxSize.Bottom() - ScreenPos.Left();
		RenderTargetPos.Top() = MaxRenderTargetSize.Bottom() - ScreenPos.Left();
	}

	//	make relative to render target
	RenderTarget.GetSize( RenderTargetSize, MaxRenderTargetSize );

	RenderTargetPos.Left() -= RenderTargetSize.Left();
	RenderTargetPos.Top() -= RenderTargetSize.Top();

	//	outside render target, fail
	if ( !RenderTargetSize.GetIsInside( RenderTargetPos ) )
		return FALSE;

	return TRUE;
}

//---------------------------------------------------------
//	get a world position from this screen posiiton
//---------------------------------------------------------
Bool TLRender::TScreen::GetWorldRayFromScreenPos(const TRenderTarget& RenderTarget,TLMaths::TLine& WorldRay,const Type2<s32>& ScreenPos)
{
	Type2<s32> RenderTargetPos;
	Type4<s32> RenderTargetSize;
	if ( !GetRenderTargetPosFromScreenPos( RenderTarget, RenderTargetPos, RenderTargetSize, ScreenPos ) )
		return FALSE;

	//	let render target do it's own conversions what with fancy cameras n that
	return RenderTarget.GetWorldRay( WorldRay, RenderTargetPos, RenderTargetSize, GetScreenShape() );
}


//---------------------------------------------------------
//	
//---------------------------------------------------------
Bool TLRender::TScreen::GetWorldPosFromScreenPos(const TRenderTarget& RenderTarget,float3& WorldPos,float WorldDepth,const Type2<s32>& ScreenPos)
{
	Type2<s32> RenderTargetPos;
	Type4<s32> RenderTargetSize;
	if ( !GetRenderTargetPosFromScreenPos( RenderTarget, RenderTargetPos, RenderTargetSize, ScreenPos ) )
		return FALSE;

	//	let render target do it's own conversions what with fancy cameras n that
	return RenderTarget.GetWorldPos( WorldPos, WorldDepth, RenderTargetPos, RenderTargetSize, GetScreenShape() );
}


//---------------------------------------------------------
//	
//---------------------------------------------------------
Bool TLRender::TScreen::GetWorldPosFromScreenPos(const TPtr<TRenderTarget>& pRenderTarget,float3& WorldPos,float WorldDepth,const Type2<s32>& ScreenPos)
{
	if ( !pRenderTarget )
	{
		TLDebug_Break("RenderTarget expected");
		return FALSE;
	}

	return GetWorldPosFromScreenPos( *pRenderTarget.GetObject(), WorldPos, WorldDepth, ScreenPos );
}


//---------------------------------------------------------
//	get the render target max size (in "render target space") - this is the viewport size, but rotated
//---------------------------------------------------------
void TLRender::TScreen::GetRenderTargetMaxSize(Type4<s32>& MaxSize)
{
	GetViewportMaxSize( MaxSize );

	//	rotate render target so it's in "render target" space
	if ( GetScreenShape() == TLRender::ScreenShape_WideLeft )
	{
		Type4<s32> ViewportMaxSize = MaxSize;

		//	rotate left
	//	topleft = bottomleft
	//	topright = topleft
	//	bottomright = topright
	//	bottomleft = bottomright
		MaxSize.x = ViewportMaxSize.Height() - ViewportMaxSize.Bottom();
		MaxSize.y = ViewportMaxSize.Left();
		MaxSize.Width() = ViewportMaxSize.Height();
		MaxSize.Height() = ViewportMaxSize.Width();
	}
	else if ( GetScreenShape() == TLRender::ScreenShape_WideRight )
	{
		Type4<s32> ViewportMaxSize = MaxSize;

		//	rotate right
		MaxSize.x = ViewportMaxSize.Top();
		MaxSize.y = ViewportMaxSize.Width() - ViewportMaxSize.Right();
		MaxSize.Width() = ViewportMaxSize.Height();
		MaxSize.Height() = ViewportMaxSize.Width();
	}

}


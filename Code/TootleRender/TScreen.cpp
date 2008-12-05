#include "TScreen.h"
#include "TRenderTarget.h"

#if defined(TL_TARGET_PC)
#include "PC/PCRenderTarget.h"
#endif

#if defined(TL_TARGET_IPOD)
#include "IPod/IPodRenderTarget.h"
#endif



//---------------------------------------------------------
//	
//---------------------------------------------------------
TLRender::TScreen::TScreen(TRefRef Ref) :
	m_HasShutdown	( FALSE ),
	m_Ref			( Ref ),
	m_Size			( g_MaxSize,g_MaxSize,g_MaxSize,g_MaxSize )
{
	//	gr: default to double iphone resolution for now
	m_Size.Width() = (s16)( 320.f * 1.5f );
	m_Size.Height() = (s16)( 480.f * 1.5f );

	//	gr: disabled for now, core manager limits frame rate...
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

	//	render each render target
	for ( u32 r=0;	r<m_RenderTargets.GetSize();	r++ )
	{
		//	get render target
		TPtr<TRenderTarget>& pRenderTarget = m_RenderTargets[r];
		if ( !pRenderTarget.IsValid() )
			continue;

		//	begin draw of render target
		if ( !pRenderTarget->BeginDraw(RenderTargetMaxSize) )
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
TPtr<TLRender::TRenderTarget> TLRender::TScreen::GetRenderTarget(const TRef& TargetRef)
{
	//	find the render target with this ref. returns pointer to the correct one so we need to turn
	//	that into a plain ptr
	TPtr<TLRender::TRenderTarget>* pResult = m_RenderTargets.Find( TargetRef );
	if ( !pResult )
		return NULL;

	return *pResult;
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

	//	convert screen pos to render target pos
	Type4<s32> MaxRenderTargetSize;
	GetRenderTargetMaxSize( MaxRenderTargetSize );
	Type4<s32> RenderTargetSize;
	pRenderTarget->GetSize( RenderTargetSize, MaxRenderTargetSize );

	Type2<s32> RenderTargetPos( ScreenPos );
	RenderTargetPos.x -= RenderTargetSize.Left();
	RenderTargetPos.y -= RenderTargetSize.Top();

	//	outside render target, aint gonna find nowt
	if ( !RenderTargetSize.GetIsInside( RenderTargetPos ) )
		return FALSE;

	//	let render target do it's own conversions what with fancy cameras n that
	return pRenderTarget->GetWorldRay( WorldRay, RenderTargetPos, RenderTargetSize );
}



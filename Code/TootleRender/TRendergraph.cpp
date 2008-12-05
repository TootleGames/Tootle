#include "TRendergraph.h"


namespace TLRender
{
	TPtr<TRendergraph> g_pRendergraph;
};



SyncBool TLRender::TRendergraph::Initialise()
{
	if ( TLGraph::TGraph<TLRender::TRenderNode>::Initialise() == SyncFalse )
		return SyncFalse;

	return TLRender::Platform::Init();
}

SyncBool TLRender::TRendergraph::Update(float fTimeStep)
{
	if ( TLGraph::TGraph<TLRender::TRenderNode>::Update(fTimeStep) == SyncFalse )
		return SyncFalse;

	return TLRender::Platform::Update();
}

SyncBool TLRender::TRendergraph::Shutdown()
{
	SyncBool ShutdownResult = TLGraph::TGraph<TLRender::TRenderNode>::Shutdown();
	if ( ShutdownResult == SyncWait )
		return SyncWait;

	return TLRender::Platform::Shutdown();
}

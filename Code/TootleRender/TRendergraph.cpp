#include "TRendergraph.h"
#include "TRenderNodeTile.h"
#include "TRenderNodeText.h"
#include "TRenderNodeDebugMesh.h"



namespace TLRender
{
	TPtr<TRendergraph> g_pRendergraph;
};


	

SyncBool TLRender::TRendergraph::Initialise()
{
	if ( TLGraph::TGraph<TLRender::TRenderNode>::Initialise() == SyncFalse )
		return SyncFalse;

	//	create generic render node factory
	TPtr<TClassFactory<TRenderNode,FALSE> > pFactory = new TRenderNodeFactory();
	AddFactory(pFactory);

	return TLRender::Platform::Init();
}


SyncBool TLRender::TRendergraph::Shutdown()
{
	SyncBool ShutdownResult = TLGraph::TGraph<TLRender::TRenderNode>::Shutdown();
	if ( ShutdownResult == SyncWait )
		return SyncWait;

	return TLRender::Platform::Shutdown();
}









TLRender::TRenderNode* TLRender::TRenderNodeFactory::CreateObject(TRefRef InstanceRef,TRefRef TypeRef)
{
	if ( TypeRef == "Tile" )
		return new TLRender::TRenderNodeTile(InstanceRef,TypeRef);

	if ( TypeRef == "Text" )
		return new TLRender::TRenderNodeText(InstanceRef,TypeRef);

	if ( TypeRef == "DbgMesh" )
		return new TLRender::TRenderNodeDebugMesh(InstanceRef,TypeRef);

	return NULL;
}


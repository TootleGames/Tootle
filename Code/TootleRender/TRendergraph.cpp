#include "TRendergraph.h"
#include "TRenderNodeTile.h"
#include "TRenderNodeText.h"
#include "TRenderNodeDebugMesh.h"
#include "TRenderNodePathNetwork.h"
#include "TRenderNodePhysicsNode.h"



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

// [06/03/09] DB - The glyph render node needs changing to be publicly creatable.  
// This is the only render node that has this issue
//	if ( TypeRef == "Glyph" )
//		return new TLRender::TRenderNodeGlyph(InstanceRef,TypeRef);

	if ( TypeRef == "DbgMesh" )
		return new TLRender::TRenderNodeDebugMesh(InstanceRef,TypeRef);

	if ( TypeRef == "DbgPath" )
		return new TLRender::TRenderNodePathNetwork(InstanceRef,TypeRef);

	if ( TypeRef == "DbgPhys" )
		return new TLRender::TRenderNodePhysicsNode(InstanceRef,TypeRef);

	return NULL;
}


#include "TRendergraph.h"
#include "TRenderNodeTile.h"
#include "TRenderNodeText.h"
#include "TRenderNodeDebugMesh.h"
#include "TRenderNodePathNetwork.h"
#include "TRenderNodePhysicsNode.h"
#include "TRenderNodeQuadTree.h"
#include "TRenderNodeSwoosh.h"
#include "TRenderNodeScrollableView.h"
#include "TRenderNodeTimeline.h"
#include "TRenderNodeParticle.h"
#include <TootleCore/TLTime.h>
#include "TLRender.h"


namespace TLRender
{
	TPtr<TRendergraph> g_pRendergraph;
};



TLGraph::TNodeMessage::TNodeMessage(TRefRef NodeRef,TLMessaging::TMessage& Message) :
	m_NodeRef	( NodeRef ),
	m_Message	( Message )
{
}



SyncBool TLRender::TRendergraph::Initialise()
{
	if ( TLGraph::TGraph<TLRender::TRenderNode>::Initialise() == SyncFalse )
		return SyncFalse;

	SyncBool Result = TLRender::Init();

	if(Result != SyncTrue)
		return Result;
	
	//	create generic render node factory
	TPtr<TClassFactory<TRenderNode,FALSE> > pFactory = new TRenderNodeFactory();
	AddFactory(pFactory);
	return SyncTrue;
}


SyncBool TLRender::TRendergraph::Shutdown()
{
	TLDebug_Print("Rendergraph shutdown");
	SyncBool ShutdownResult = TLGraph::TGraph<TLRender::TRenderNode>::Shutdown();
	if ( ShutdownResult == SyncWait )
		return SyncWait;

	//	gr; this looks odd but the render graph
	return TLRender::Shutdown();
}


//------------------------------------------------
//	alternative graph update
//------------------------------------------------
SyncBool TLRender::TRendergraph::Update(float fTimeStep)
{
	TLTime::TScopeTimer Timer( TRef_Static(R,e,n,U,p) );
	//	gr: NO NODE UPDATES FOR RENDER GRAPH!

	//	process graph messages
	ProcessMessageQueue();
	
	//	gr; update structure first, this will remove nodes first and 
	//	mean any node messages wont do uncessacary processing
	UpdateGraphStructure();
	TPtrArray<TLGraph::TNodeMessage> LostMessages;

	//	process node messages. fifo
	u32 MessageCount = m_NodeMessages.GetSize();
	for ( u32 m=0;	m<MessageCount;	m++ )
	{
		TLGraph::TNodeMessage& Message = *(m_NodeMessages[m]);
		TRenderNode* pRenderNode = FindNode( Message.GetNodeRef() );
		if ( pRenderNode )
		{
			Message.GetMessage().ResetReadPos();
			pRenderNode->ProcessMessage( Message.GetMessage() );
		}
		else
		{
			//	missing render node... not yet in tree? keep the message
			LostMessages.Add( m_NodeMessages[m] );
		}
	}

	//	clear messages (dont remove all in case some kinda processing added another message to the queue
	m_NodeMessages.RemoveAt( 0, MessageCount );

	//	warning break if we have a lot of them
	if ( LostMessages.GetSize() > 200 )
	{
		if ( !TLDebug_Break("Lots of lost messages... trying to send messages to something that will never exist?") )
			LostMessages.Empty();
	}

	//	insert the unhandled messages back to the start of the queue
	m_NodeMessages.InsertAt( 0, LostMessages );


	return SyncTrue;
}

//------------------------------------------------
//	send message to node
//------------------------------------------------
Bool TLRender::TRendergraph::SendMessageToNode(TRefRef NodeRef,TLMessaging::TMessage& Message)
{
	//	add message to the node message queue
	TPtr<TLGraph::TNodeMessage> pNodeMessage = new TLGraph::TNodeMessage( NodeRef, Message );
	m_NodeMessages.Add( pNodeMessage );
	//m_NodeMessages.AddNewPtr( new TLGraph::TNodeMessage( NodeRef, Message ) );
	
	//	assume okay - could fetch node, but thats a bit expensive
	return TRUE;
}







TLRender::TRenderNode* TLRender::TRenderNodeFactory::CreateObject(TRefRef InstanceRef,TRefRef TypeRef)
{
	if ( TypeRef == "Tile" )
		return new TLRender::TRenderNodeTile(InstanceRef,TypeRef);

	if ( TypeRef == "Text" )
	{
		TLDebug_Warning("Text type of render node is deprecated, please use VText for vector text now");
		return new TLRender::TRenderNodeVectorText(InstanceRef,"VText");
	}

	if ( TypeRef == "VText" )
		return new TLRender::TRenderNodeVectorText(InstanceRef,TypeRef);

	if ( TypeRef == "TxText" )
		return new TLRender::TRenderNodeTextureText(InstanceRef,TypeRef);

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

	if ( TypeRef == "DbgQuad" )
		return new TLRender::TRenderNodeQuadTreeZone(InstanceRef,TypeRef);

	if ( TypeRef == "Swoosh" )
		return new TLRender::TRenderNodeSwoosh(InstanceRef,TypeRef);

	if ( TypeRef == "Scroll" )
		return new TLRender::TRenderNodeScrollableView(InstanceRef,TypeRef);

	if ( TypeRef == "Timeline" )
		return new TLRender::TRenderNodeTimeline(InstanceRef,TypeRef);

	if ( TypeRef == "Particle" )
		return new TLRender::TRenderNodeParticle(InstanceRef,TypeRef);

	return NULL;
}


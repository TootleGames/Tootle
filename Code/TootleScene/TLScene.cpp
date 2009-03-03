#include "TLScene.h"
#include "TSceneGraph.h"

/*

//-------------------------------------------------------------
//	create a new node on the scene graph and initialise it - returns 
//	the new node's ref if successfully created and added
//-------------------------------------------------------------
TRef TLScene::CreateNode(TRefRef InstanceRef,TRefRef TypeRef,TPtr<TLScene::TSceneNode> pParentNode,TLMessaging::TMessage& pInitMessage)
{
	//	Create a new node via the scenegraph
	TPtr<TSceneNode> pNode = TLScene::g_pScenegraph->CreateInstance( InstanceRef, TypeRef );
	if ( !pNode )
		return TRef();

	//	make up initialise message if we havent provided one
	if ( !pInitMessage )
	{
		pInitMessage = new TLMessaging::TMessage Message( TLCore::InitialiseRef );
	}

	//	send init message
	pNode->QueueMessage( pInitMessage );

	//	add to graph
	if ( !TLScene::g_pScenegraph->AddNode( pNode, pParentNode ) )
	{
		//	failed
		return TRef();
	}

	//	success, return the ref of the created node
	return pNode->GetNodeRef();
}

*/

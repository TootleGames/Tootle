#include "TSceneNode.h"


using namespace TLScene;

TSceneNode::TSceneNode(TRefRef NodeRef,TRefRef TypeRef) :
	TLGraph::TGraphNode<TLScene::TSceneNode>	( NodeRef ),
	m_NodeTypeRef								( TypeRef )
{
}


void TSceneNode::Update(float fTimeStep)
{
	// Do pre-update handling
	ProcessMessageQueue();

	// Call object virtual update
	DoUpdate(fTimeStep);
}


void TSceneNode::ProcessMessage(TPtr<TLMessaging::TMessage>& pMessage)
{

	// Super class process message
	TLGraph::TGraphNode<TSceneNode>::ProcessMessage(pMessage);
}

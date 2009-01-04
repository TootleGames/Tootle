#include "TSceneNode.h"


using namespace TLScene;

TSceneNode::TSceneNode(TRefRef NodeRef,TRefRef TypeRef) :
	TLGraph::TGraphNode<TLScene::TSceneNode>	( NodeRef, TypeRef ),
	m_NodeTypeRef								( TypeRef )
{
}


void TSceneNode::Update(float fTimeStep)
{
	//	do inherited update
	TLGraph::TGraphNode<TLScene::TSceneNode>::Update( fTimeStep );
}


void TSceneNode::ProcessMessage(TPtr<TLMessaging::TMessage>& pMessage)
{

	// Super class process message
	TLGraph::TGraphNode<TSceneNode>::ProcessMessage(pMessage);
}

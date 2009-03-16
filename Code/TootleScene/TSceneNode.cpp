#include "TSceneNode.h"



TLScene::TSceneNode::TSceneNode(TRefRef NodeRef,TRefRef TypeRef) :
	TLGraph::TGraphNode<TLScene::TSceneNode>	( NodeRef, TypeRef )
{
}

//-------------------------------------------------------
//
//-------------------------------------------------------
void TLScene::TSceneNode::UpdateAll(float Timestep)
{
	if ( !IsEnabled() )
	{
		ProcessMessageQueue();
		return;
	}

	// Update this
	Update( Timestep );
	PostUpdate( Timestep );

	//	update tree
	TPtrArray<TLScene::TSceneNode>& Children = GetChildren();
	for ( u32 c=0;	c<Children.GetSize();	c++ )
	{
		TPtr<TLScene::TSceneNode>& pChild = Children[c];
		pChild->UpdateAll( Timestep );
	}

}



void TLScene::TSceneNode::ProcessMessage(TLMessaging::TMessage& Message)
{

	// Super class process message
	TLGraph::TGraphNode<TSceneNode>::ProcessMessage(Message);
}



void TLScene::TSceneNode::Update(float fTimeStep)
{
	//	do inherited update
	TLGraph::TGraphNode<TLScene::TSceneNode>::Update( fTimeStep );
}


void TLScene::TSceneNode::PostUpdate(float fTimestep)
{
}



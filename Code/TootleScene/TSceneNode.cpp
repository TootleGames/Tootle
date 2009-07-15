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


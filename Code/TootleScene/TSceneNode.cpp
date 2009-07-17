#include "TSceneNode.h"



TLScene::TSceneNode::TSceneNode(TRefRef NodeRef,TRefRef TypeRef) :
	TLGraph::TGraphNode<TLScene::TSceneNode>	( NodeRef, TypeRef ),
	m_bEnabled(TRUE)
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


void TLScene::TSceneNode::SetEnable(const Bool& bEnabled)
{
	if(bEnabled != m_bEnabled)
	{
		m_bEnabled = bEnabled;

		if(m_bEnabled)
			OnEnable();
		else
			OnDisable();
	}
}



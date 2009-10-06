#include "TSceneNode.h"

#include "TScenegraph.h"

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


void TLScene::TSceneNode::Initialise(TLMessaging::TMessage& Message)
{
	//	create children during init
	TPtrArray<TBinaryTree> InitChildDatas;
	if ( Message.GetChildren("Child", InitChildDatas ) )
		for ( u32 c=0;	c<InitChildDatas.GetSize();	c++ )
			CreateChildNode( *InitChildDatas[c] );

	//	do inherited init
	TLGraph::TGraphNode<TLScene::TSceneNode>::Initialise( Message );
}

//---------------------------------------------------------
//	create a child node from plain data
//---------------------------------------------------------
Bool TLScene::TSceneNode::CreateChildNode(TBinaryTree& ChildInitData)
{
	/*
	TTempString Debug_String("Creating child node from SceneNode ");
	this->GetNodeRef().GetString( Debug_String );
	TLDebug_Print( Debug_String );
	ChildInitData.Debug_PrintTree();
	*/

	//	import bits of optional data
	TRef Type;
	ChildInitData.ImportData("Type", Type);

	TRef Parent = this->GetNodeRef();
	if ( ChildInitData.ImportData("Parent", Parent) )
	{
		if ( Parent != this->GetNodeRef() )
		{
			TTempString Debug_String("Import \"child\" data for scene node ");
			this->GetNodeRef().GetString( Debug_String );
			Debug_String.Append(", and child's parent is ");
			Parent.GetString( Debug_String );
			TLDebug_Break(Debug_String);
		}
	}

	TRef ChildRef;
	ChildInitData.ImportData("NodeRef", ChildRef);

	//	make up an initialise message and use the data provided
	TLMessaging::TMessage InitMessage(TLCore::InitialiseRef);
	InitMessage.ReferenceDataTree( ChildInitData );

	ChildRef = TLScene::g_pScenegraph->CreateNode( ChildRef, Type, Parent, &InitMessage );
	
	return ChildRef.IsValid();
}


void TLScene::TSceneNode::SetProperty(TLMessaging::TMessage& Message)
{
	Bool bEnable = TRUE;
	if(Message.ImportData("Enable", bEnable))
		SetEnable(bEnable);

	// Call super setproperty routine
	TLGraph::TGraphNode<TLScene::TSceneNode>::SetProperty(Message);
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



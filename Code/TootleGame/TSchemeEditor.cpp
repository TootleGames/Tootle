#include "TSchemeEditor.h"
#include <TootleRender/TScreenManager.h>




TLGame::TSchemeEditor::TSchemeEditor()
{
}

TLGame::TSchemeEditor::~TSchemeEditor()
{
	m_pGraph = NULL;

	//	unselect all the nodes - need to do this to send out the end-edit message for node cleanups
	UnselectAllNodes();
}



//----------------------------------------------------------
//	
//----------------------------------------------------------
Bool TLGame::TSchemeEditor::Initialise(TRefRef EditorScheme,TRefRef GraphRef,TRefRef SchemeRootNode,TRefRef GameRenderTarget)
{
	//	check params
	if ( !SchemeRootNode.IsValid() )
	{
		TLDebug_Break("Valid root node for the editor expected");
		return FALSE;
	}

	//	fetch graph 
	//	gr: no type checking atm, have to assume this is a graph...
	m_pGraph = TLCore::g_pCoreManager->GetManager<TLGraph::TGraphBase>( GraphRef );
	if ( !m_pGraph )
	{
		TLDebug_Break("Initialised scheme editor with graph ref that doesnt exist");
		return FALSE;
	}

	m_SchemeRootNode = SchemeRootNode;
	m_GameRenderTarget = GameRenderTarget;

	TLGraph::TGraphNodeBase* pRootNode = m_pGraph->FindNodeBase( m_SchemeRootNode );
	if ( !pRootNode )
	{
		TLDebug_Break("Missing root scheme node for editor - need to add async functionality?");
		return FALSE;
	}

	//	create node widgets so we can interact with the current scene
	CreateNodeWidgets( *pRootNode );

	//	create editor UI to add new nodes, stats etc
	if ( EditorScheme.IsValid() )
	{
		if ( !CreateEditorGui( EditorScheme ) )
			return FALSE;
	}

	return TRUE;
}


//----------------------------------------------------------
//	
//----------------------------------------------------------
void TLGame::TSchemeEditor::ProcessMessage(TLMessaging::TMessage& Message)
{
	if(Message.GetMessageRef() == TRef_Static(A,c,t,i,o))
	{
		TRef ActionRef;
		TRef NodeRef;
		Message.ResetReadPos();
		if ( Message.Read(ActionRef) && Message.ImportData("Node", NodeRef) )
		{
			if ( ActionRef == "NDrag" )
			{
				//	get world change
				float3 WorldChange;
				if ( Message.ImportData("Move3",WorldChange ) )
				{
					OnNodeDrag( NodeRef, WorldChange );
				}
			}
			else if ( ActionRef == "NDown" )
			{
				OnNodeSelected( NodeRef );
			}
			else if ( ActionRef == "NUp" )
			{
				OnNodeUnselected( NodeRef );
			}
		}
	}
	

}


//----------------------------------------------------------
//	create a node widget to allow us to drag around in-game nodes
//	recurses down the tree
//----------------------------------------------------------
void TLGame::TSchemeEditor::CreateNodeWidgets(TLGraph::TGraphNodeBase& Node)
{
	//	get the render node of the scene node
	TRefRef RenderNodeRef = Node.GetRenderNodeRef();
	if ( RenderNodeRef.IsValid() )
	{
		TBinaryTree WidgetData( TRef_Invalid );	//	ref irrelavant
		WidgetData.ExportData("Node", Node.GetNodeRef() );

		TPtr<TLGui::TWidgetDrag> pWidget = new TLGui::TWidgetDrag( m_GameRenderTarget, RenderNodeRef, "global", "NDown", "NUp", "NDrag", &WidgetData );
		m_NodeWidgets.Add( pWidget );

		//	subscribe to widget to get the actions
		this->SubscribeTo( pWidget );
	}

	//	subscribe to this node to get changes to the render node
	this->SubscribeTo( &Node );

	//	create widgets for children
	TArray<TLGraph::TGraphNodeBase*> ChildNodes;
	Node.GetChildrenBase( ChildNodes );
	for ( u32 c=0;	c<ChildNodes.GetSize();	c++ )
	{
		TLGraph::TGraphNodeBase* pChild = ChildNodes.ElementAt(c);
		CreateNodeWidgets( *pChild );
	}

}


//----------------------------------------------------------
//	create render target, widgets, icons etc
//----------------------------------------------------------
Bool TLGame::TSchemeEditor::CreateEditorGui(TRefRef EditorScheme)
{
	//	without a scheme, we don't create an interface
	if ( !EditorScheme.IsValid() )
		return FALSE;

	//	create render target
	if ( !m_EditorRenderTarget.IsValid() )
	{
		TPtr<TLRender::TScreen>& pScreen = TLRender::g_pScreenManager->GetDefaultScreen();
		TPtr<TLRender::TRenderTarget> pRenderTarget = pScreen->CreateRenderTarget( EditorScheme.IsValid() ? EditorScheme : TRef("Editor") );
		if ( !pRenderTarget )
			return FALSE;
		m_EditorRenderTarget = pRenderTarget->GetRef();

		pRenderTarget->SetClearColour( TColour( 0.f, 0.f, 0.f, 0.0f ) );
		pRenderTarget->SetEnabled( TRUE );

		//	setup camera
		TPtr<TLRender::TCamera> pCamera = new TLRender::TOrthoCamera();
		pCamera->SetPosition( float3( 0.f, 0.f, -20.f ) );
		pRenderTarget->SetCamera( pCamera );

		//	create root render node
		if ( !m_EditorRenderNodeRef.IsValid() )
		{
			TLMessaging::TMessage Message( TLCore::InitialiseRef );
			Message.ExportData("RFClear", TLRender::TRenderNode::RenderFlags::EnableCull );
			m_EditorRenderNodeRef = TLRender::g_pRendergraph->CreateNode("EditorRoot", TRef(), TRef(), &Message );
			pRenderTarget->SetRootRenderNode( m_EditorRenderNodeRef );
		}
	}

	//	instance the scheme
	TLDebug_Break("todo");
	return FALSE;
}


//----------------------------------------------------------
//	node has been selected
//----------------------------------------------------------
void TLGame::TSchemeEditor::OnNodeSelected(TRefRef SceneNode)
{
	m_SelectedNodes.AddUnique( SceneNode );
	
	TLMessaging::TMessage EditMessage("EdtStart");
	m_pGraph->SendMessageToNode( SceneNode, EditMessage );
}



//----------------------------------------------------------
//	node has been selected
//----------------------------------------------------------
void TLGame::TSchemeEditor::OnNodeUnselected(TRefRef SceneNode)
{
	m_SelectedNodes.Remove( SceneNode );
		
	TLMessaging::TMessage EditMessage("EdtEnd");
	m_pGraph->SendMessageToNode( SceneNode, EditMessage );
}




//----------------------------------------------------------
//	node has been dragged
//----------------------------------------------------------
void TLGame::TSchemeEditor::OnNodeDrag(TRefRef SceneNode,const float3& DragAmount)
{
	TLMessaging::TMessage TransformMessage("DoTransform");
	TransformMessage.ExportData("Translate", DragAmount );

	TLDebug_Print( TString("Node drag; %2.2f, %2.2f, %2.2f", DragAmount.x, DragAmount.y, DragAmount.z ) );
	
	m_pGraph->SendMessageToNode( SceneNode, TransformMessage );
}

	
//----------------------------------------------------------
//	unselect all nodes
//----------------------------------------------------------
void TLGame::TSchemeEditor::UnselectAllNodes()
{
	//	send editor-end message to all selected nodes
	TLMessaging::TMessage EditMessage("EdtEnd");

	for ( u32 i=0;	i<m_SelectedNodes.GetSize();	i++ )
		m_pGraph->SendMessageToNode( m_SelectedNodes[i], EditMessage );

	//	now unselect all those nodes
	m_SelectedNodes.Empty();
}


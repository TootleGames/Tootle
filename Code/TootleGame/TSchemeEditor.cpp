#include "TSchemeEditor.h"
#include <TootleRender/TScreenManager.h>
#include <TootleGame/TWidgetButton.h>
#include <TootleGame/TWidgetManager.h>
#include <TootleInput/TUser.h>




TLGame::TSchemeEditor::TSchemeEditor() :
	m_CommonNodeData	( TRef() )
{
}

TLGame::TSchemeEditor::~TSchemeEditor()
{
	//	unselect all the nodes - need to do this to send out the end-edit message for node cleanups
	UnselectAllNodes();

	//	delete render target
	if ( m_EditorRenderTarget.IsValid() )
	{
		TLRender::g_pScreenManager->DeleteRenderTarget( m_EditorRenderTarget );
		m_EditorRenderTarget.SetInvalid();
	}

	//	delete our nodes
	TLRender::g_pRendergraph->RemoveNode( m_EditorRenderNodeRef );

	//	release ptrs
	m_pGraph = NULL;
}



//----------------------------------------------------------
//	
//----------------------------------------------------------
Bool TLGame::TSchemeEditor::Initialise(TRefRef EditorScheme,TRefRef GraphRef,TRefRef SchemeRootNode,TRefRef GameRenderTarget,TBinaryTree* pCommonNodeData)
{
	//	copy node params
	if ( pCommonNodeData )
		m_CommonNodeData.ReferenceDataTree( *pCommonNodeData, FALSE );

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
		Message.ResetReadPos();
		if ( !Message.Read(ActionRef) )
			return;
		
		TRef Ref;
		if ( Message.ImportData("Node", Ref) )
		{
			ProcessNodeMessage( Ref, ActionRef, Message );
		}
		else if ( Message.ImportData("Icon", Ref ) )
		{
			ProcessIconMessage( Ref, ActionRef, Message );
		}
		else 
		{
			if ( ActionRef == m_NewSceneNodeDragAction && m_NewSceneNodeDragAction.IsValid() )
			{
				ProcessMouseMessage( m_NewSceneNodeDragAction, Message );
				return;
			}
			else if ( ActionRef == m_NewSceneNodeClickAction && m_NewSceneNodeClickAction.IsValid() )
			{
				ProcessMouseMessage( m_NewSceneNodeClickAction, Message );
				return;
			}
			else
			{
				//	gui command for editor from widget
				switch ( ActionRef.GetData() )
				{
					case TRef_Static(C,l,o,s,e):
					{
						//	todo: do some shutdown
						//	send close message
						TLMessaging::TMessage Message( ActionRef, "Editor" );
						PublishMessage( Message );
					}
					break;

					case TRef_Static4(S,a,v,e):
					{
						//	send save message - currently handled externally...
						TLMessaging::TMessage Message( ActionRef, "Editor" );
						PublishMessage( Message );
					}
					break;

					case TRef_Static(C,l,e,a,r):
						ClearScheme();
						break;

					default:
					{
						TTempString Debug_String("Unknown editor command ");
						ActionRef.GetString( Debug_String );
						TLDebug_Print( Debug_String );
					}
					break;
				}
			}
		}
	}

	if ( Message.GetMessageRef() == "NodeAdded" && Message.GetSenderRef() == TLRender::g_pRendergraph->GetGraphRef() )
	{
		TRef NodeRef;
		Message.ResetReadPos();
		if ( Message.Read( NodeRef ) )
		{
			if ( NodeRef == m_EditorRenderNodeRef )
				OnEditorRenderNodeAdded();
		}

		return;
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

	TLAsset::TScheme* pEditorScheme = TLAsset::LoadAsset( EditorScheme, TRUE, "Scheme" ).GetObject<TLAsset::TScheme>();
	if ( !pEditorScheme )
	{
		TLDebug_Break("failed to load editor scheme");
		return FALSE;
	}

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

			//	catch when the editor render node is created so we can create icons etc
			this->SubscribeTo( TLRender::g_pRendergraph );
		}

		//	subscribe to user's (mouse) actions
		TPtr<TLUser::TUser>& pUser = TLUser::g_pUserManager->GetUser("global");
		if ( pUser )
		{
			this->SubscribeTo( pUser );
		}
		else
		{
			TLDebug_Break("missing user to subscribe to actions");
		}
	}

	//	instance the scheme
	if ( !TLRender::g_pRendergraph->ImportScheme( pEditorScheme, m_EditorRenderNodeRef ) )
		return FALSE;

	//	read in data from the scheme
	u32 i;

	//	setup widgets
	TPtrArray<TBinaryTree> WidgetDatas;
	pEditorScheme->GetData().GetChildren("Widget",WidgetDatas);
	for ( i=0;	i<WidgetDatas.GetSize();	i++ )
		CreateEditorWidget( *WidgetDatas[i] );

	//	setup list of node types we can create
	pEditorScheme->GetData().GetChildren("NewNode", m_NewNodeData );

	return TRUE;
}


//----------------------------------------------------------
//	create a widget from scheme XML
//----------------------------------------------------------
void TLGame::TSchemeEditor::CreateEditorWidget(TBinaryTree& WidgetData)
{
	//	todo: need a "widget is valid" function
	TPtr<TLGui::TWidget> pWidget = new TLGui::TWidgetButton( m_EditorRenderTarget, WidgetData );
	
	this->SubscribeTo( pWidget );
	m_EditorWidgets.Add( pWidget );
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



//----------------------------------------------------------
//	remove all nodes
//----------------------------------------------------------
void TLGame::TSchemeEditor::ClearScheme()
{
	UnselectAllNodes();

	m_pGraph->RemoveChildren( m_SchemeRootNode );
}



//----------------------------------------------------------
//	handle a [widget]message from a game node
//----------------------------------------------------------
void TLGame::TSchemeEditor::ProcessNodeMessage(TRefRef NodeRef,TRefRef ActionRef,TLMessaging::TMessage& Message)
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


//----------------------------------------------------------
//	handle a [widget]message from a editor icon
//----------------------------------------------------------
void TLGame::TSchemeEditor::ProcessIconMessage(TRefRef IconRef,TRefRef ActionRef,TLMessaging::TMessage& Message)
{
	if ( ActionRef == "IcoDrag" && !m_NewSceneNode.IsValid() )
	{
		//	create a new node at the mouse position - convert screen pos to game render target world pos
		float2 ScreenPos;
		Message.ImportData("Pos2", ScreenPos);

		float3 MouseWorldPos( 10.f, 10.f, 0.f );

		TRef NodeType;
		Message.ImportData("Type", NodeType);

		TLMessaging::TMessage InitMessage(TLCore::InitialiseRef);
		InitMessage.ReferenceDataTree( m_CommonNodeData, FALSE );
		InitMessage.ExportData("Translate", MouseWorldPos );

		//	set this new node as the "new scene node" that we're dropping into the game.
		//	the editor's mouse controls take over now
		m_NewSceneNode = m_pGraph->CreateNode("Node", NodeType, m_SchemeRootNode, &InitMessage );
		OnNodeSelected( m_NewSceneNode );

		//	store off the action to listen for, for the mouse
		if ( !Message.ImportData("InpAction", m_NewSceneNodeDragAction ) )
		{
			TLDebug_Break("Don't know what to look out for when dragging new scene node, handle me gracefully!");
			//UnselectNode( m_NewSceneNode );
			return;
		}

		//	get the equivelent mouse action
		m_NewSceneNodeClickAction = TLGui::g_pWidgetManager->GetClickActionFromMoveAction( m_NewSceneNodeDragAction );

		//	create the in-game widget for this
		//	gr: can't do until initialised...
		//CreateNodeWidget( m_NewSceneNode );
	}

}


//----------------------------------------------------------
//	create an icon for the editor
//----------------------------------------------------------
void TLGame::TSchemeEditor::CreateEditorIcons()
{
	float3 IconPosition( 0.f, 0.f, 5.f );
	float3 IconScale( 5.f, 5.f, 1.f );

	for ( u32 i=0;	i<m_NewNodeData.GetSize();	i++ )
	{
		TBinaryTree& IconData = *m_NewNodeData[i];
		TRef TypeRef;
		if ( !IconData.ImportData("Type", TypeRef) )
		{
			TLDebug_Break("Icon data requires a type - otherwise we don't know what scene node to create");
			continue;
		}

		//	create icon render node
		TLMessaging::TMessage InitMessage(TLCore::InitialiseRef);

		//	temporary text rnder node setup
		TTempString String;
		if ( !IconData.ImportDataString("name", String ) )
		{
			TRef TypeRef("????");
			IconData.ImportData("Type", TypeRef );
			TypeRef.GetString( String, TRUE );
		}
		InitMessage.ExportDataString("string", String );
		InitMessage.ExportData("scale", IconScale );
		InitMessage.ExportData("FontRef", TRef("fdebug") );
		InitMessage.ExportData("translate", IconPosition );
		InitMessage.ExportData("boxdatum", TRef("Icons") );
		InitMessage.ExportData("boxnode", TRef("e_gui") );

		TRef IconRenderNodeRef = TLRender::g_pRendergraph->CreateNode("Icon", "TxText", m_EditorRenderNodeRef, &InitMessage );

		//	create draggable widget on this icon
		TBinaryTree WidgetData("Widget");
		WidgetData.ExportData("Node", IconRenderNodeRef );
		WidgetData.ExportData("ActDown", TRef("IcoDown") );
		WidgetData.ExportData("ActDrag", TRef("IcoDrag") );

		//	custom data
		WidgetData.ExportData("Icon", TypeRef );
		WidgetData.ExportData("Type", TypeRef );
		TPtr<TLGui::TWidgetDrag> pWidget = new TLGui::TWidgetDrag( m_EditorRenderNodeRef, WidgetData );
		m_EditorWidgets.Add( pWidget );
		this->SubscribeTo( pWidget );

		IconPosition.y += IconScale.y * 1.5f;
	}

}


//----------------------------------------------------------
//	editor render node is ready to be used
//----------------------------------------------------------
void TLGame::TSchemeEditor::OnEditorRenderNodeAdded()
{
	CreateEditorIcons();
}

//----------------------------------------------------------
//	handle mouse messages 
//----------------------------------------------------------
void TLGame::TSchemeEditor::ProcessMouseMessage(TRefRef ActionRef,TLMessaging::TMessage& Message)
{
	//	dragging new scene node around
	if ( ActionRef == m_NewSceneNodeDragAction && m_NewSceneNode.IsValid() )
	{
		//	get the screen cursor pos
		int2 ScreenPos;
		if ( !Message.ImportData("Cursor", ScreenPos ) )
		{
			TLDebug_Break("Mouse move message missing cursor pos");
			return;
		}

		//	convert to world pos in game [render target]

		//	get game's render target
		TPtr<TLRender::TScreen> pScreen;
		TLRender::TRenderTarget* pRenderTarget = TLRender::g_pScreenManager->GetRenderTarget( m_GameRenderTarget, pScreen );
		if ( !pRenderTarget )
		{
			TLDebug_Break("Game's rendertarget expected");
			return;
		}

		float3 WorldPos;
		if ( !pScreen->GetWorldPosFromScreenPos( *pRenderTarget, WorldPos, 0.f, ScreenPos ) )
		{
			//	failed - probably dragged outside the game's window, drop
			TLDebug_Break("todo: drop node");
			return;
		}

		//	move the node to the cursor's world pos
		TLMessaging::TMessage SetMessage(TLCore::SetPropertyRef);
		SetMessage.ExportData("translate", WorldPos);
		m_pGraph->SendMessageToNode( m_NewSceneNode, SetMessage );
		return;
	}

	//	drop new scene node into game (let go)
	if ( ActionRef == m_NewSceneNodeClickAction && m_NewSceneNode.IsValid() )
	{
		//	create widget for this node
		TLGraph::TGraphNodeBase* pNode = m_pGraph->FindNodeBase( m_NewSceneNode );
		if ( pNode )
		{
			CreateNodeWidgets( *pNode );
		}
		else
		{
			TLDebug_Break("Node expected");
		}

		OnNodeUnselected( m_NewSceneNode );
		m_NewSceneNode.SetInvalid();
	}
}



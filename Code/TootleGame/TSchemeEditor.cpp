#include "TSchemeEditor.h"
#include <TootleRender/TScreenManager.h>
#include <TootleGame/TWidgetButton.h>
#include <TootleGame/TWidgetManager.h>
#include <TootleInput/TUser.h>
#include <TootleScene/TSceneGraph.h>




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
		m_CommonNodeData.ReferenceDataTree( *pCommonNodeData );

	//	check params
	if ( !SchemeRootNode.IsValid() )
	{
		TLDebug_Break("Valid root node for the editor expected");
		return FALSE;
	}

	//	fetch graph 
	//	gr: no type checking atm, have to assume this is a graph...
	m_pGraph = TLCore::g_pCoreManager->GetManagerPtr<TLGraph::TGraphBase>( GraphRef );
	if ( !m_pGraph )
	{
		TLDebug_Break("Initialised scheme editor with graph ref that doesnt exist");
		return FALSE;
	}

	//	set vars
	m_SchemeRootNode = SchemeRootNode;
	m_GameRenderTarget = GameRenderTarget;

	//	pre-fetch render target
	m_pGameRenderTarget = TLRender::g_pScreenManager->GetRenderTarget( m_GameRenderTarget, m_pGameScreen );
	if ( !m_pGameRenderTarget )
	{
		TLDebug_Break("game's render target expected");
		return FALSE;
	}

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

	//	init modes, first one added will be active one, this allows overloaded editors to dictate where to start
	AddStateModes();

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
			return;
		}

		TPtr<TBinaryTree>& pIconData = Message.GetChild("Icon");
		if ( pIconData )
		{
			ProcessIconMessage( pIconData, ActionRef, Message );
			return;
		}
		
		if ( ActionRef == m_NewSceneNodeDragAction && m_NewSceneNodeDragAction.IsValid() )
		{
			ProcessMouseMessage( ActionRef, Message, FALSE );
			return;
		}
		else if ( ActionRef == m_NewSceneNodeClickAction && m_NewSceneNodeClickAction.IsValid() )
		{
			ProcessMouseMessage( ActionRef, Message, TRUE );
			return;
		}

		//	catch mouse actions
		if ( TLGui::g_pWidgetManager->IsClickActionRef( ActionRef ) )
		{
			ProcessMouseMessage( ActionRef, Message, TRUE );
			return;
		}
		else if ( TLGui::g_pWidgetManager->IsMoveActionRef( ActionRef ) )
		{
			ProcessMouseMessage( ActionRef, Message, FALSE );
			return;
		}
		else
		{
			ProcessCommandMessage( ActionRef, Message );
			return;
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

	//	what node should we put icons under?
	if ( !pEditorScheme->GetData().ImportData("IconRoot", m_EditorIconRootNodeRef ) )
		m_EditorIconRootNodeRef = m_EditorRenderNodeRef;

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
	//	dont do anything if already selected
	if ( m_SelectedNodes.Exists( SceneNode ) )
		return;
	
	//	add to list
	m_SelectedNodes.Add( SceneNode );
	
	//	notify
	TLMessaging::TMessage EditMessage("EdtStart");
	m_pGraph->SendMessageToNode( SceneNode, EditMessage );
}



//----------------------------------------------------------
//	node has been selected
//----------------------------------------------------------
void TLGame::TSchemeEditor::OnNodeUnselected(TRefRef SceneNode)
{
	if ( m_SelectedNodes.Remove( SceneNode ) )
	{
		TLMessaging::TMessage EditMessage("EdtEnd");
		m_pGraph->SendMessageToNode( SceneNode, EditMessage );
	}

	//	unset our new scene node
	if ( SceneNode == m_NewSceneNode )
	{
		m_NewSceneNode.SetInvalid();
		m_NewSceneNodeDragAction.SetInvalid();
		m_NewSceneNodeClickAction.SetInvalid();
	}
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
	m_NewSceneNode.SetInvalid();
	m_NewSceneNodeDragAction.SetInvalid();
	m_NewSceneNodeClickAction.SetInvalid();
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
//	get world pos in-game from cursor
//----------------------------------------------------------
Bool TLGame::TSchemeEditor::GetGameWorldPosFromScreenPos(float3& WorldPos,const int2& ScreenPos,float ViewDepth)
{
	if ( !m_pGameScreen || !m_pGameRenderTarget )
		return FALSE;

	return m_pGameScreen->GetWorldPosFromScreenPos( *m_pGameRenderTarget, WorldPos, ViewDepth, ScreenPos );
}


//----------------------------------------------------------
//	handle a [widget]message from a editor icon
//----------------------------------------------------------
void TLGame::TSchemeEditor::ProcessIconMessage(TPtr<TBinaryTree>& pIconData,TRefRef ActionRef,TLMessaging::TMessage& Message)
{
	if ( ActionRef == "IcoDrag" && !m_NewSceneNode.IsValid() )
	{
		//	create a new node at the mouse position - convert screen pos to game render target world pos
		int2 ScreenPos;
		if ( !Message.ImportData("Pos2", ScreenPos) )
			return;
		
		//	error, mouse is off screen or something
		float3 WorldPos;
		if ( !GetGameWorldPosFromScreenPos( WorldPos, ScreenPos ) )
			return;

		//	make up init data for the node (or root node in scheme case)
		TLMessaging::TMessage InitMessage(TLCore::InitialiseRef);
		InitMessage.ExportData("Translate", WorldPos );
		
		//	add common data
		InitMessage.ReferenceDataTree( m_CommonNodeData );
	
		//	add data specified in the editor scheme (xml)
		TPtr<TBinaryTree>& pInitData = pIconData->GetChild("Init");
		if ( pInitData )
			InitMessage.ReferenceDataTree( pInitData );

		//	create node either from type or importing scheme under it
		TRef Type;
		if ( pIconData->ImportData("Type", Type) )
		{
			//	set this new node as the "new scene node" that we're dropping into the game.
			//	the editor's mouse controls take over now
			m_NewSceneNode = m_pGraph->CreateNode("EdNode", Type, m_SchemeRootNode, &InitMessage );
		}
		else if ( pIconData->ImportData("Scheme", Type) )
		{
			TRefRef SchemeRef = Type;
			TLAsset::TScheme* pScheme = TLAsset::LoadAsset(SchemeRef, TRUE, "Scheme" ).GetObject<TLAsset::TScheme>();
			if ( !pScheme )
			{
				TTempString Debug_String("Failed to load scheme ");
				SchemeRef.GetString( Debug_String );
				Debug_String.Append(" for new icon-node");
				TLDebug_Break( Debug_String );
				return;
			}

			//	make a base node - still trying to decide if this is the best method
			m_NewSceneNode = m_pGraph->CreateNode("EdNode", "object", m_SchemeRootNode, &InitMessage );

			//	import the scheme under neath it
			//	we do NOT use strict refs as the scheme is to be re-instanced...
			//	maybe move this option INTO the scheme XML itself?
			TLScene::g_pScenegraph->ImportScheme( *pScheme, m_NewSceneNode, FALSE, &m_CommonNodeData );
		}
		else 
		{
			TLDebug_Break("type or scheme expected");
			return;
		}		

		//	should have set this new scene node
		if ( !m_NewSceneNode.IsValid() )
		{
			TLDebug_Break("Failed to create new scene node");
			return;
		}

		//	set as selected - this also sends the EdtStart message to disable physics etc
		//	getting in the way of the drag
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
		TPtr<TBinaryTree>& pIconData = m_NewNodeData[i];
		TBinaryTree& IconData = *pIconData;
		TRef TypeOrSchemeRef;
		if ( !IconData.ImportData("Type", TypeOrSchemeRef) && !IconData.ImportData("Scheme", TypeOrSchemeRef) )
		{
			TLDebug_Break("Icon data requires a SceneNode Type or a Scheme - otherwise we don't know what to create");
			continue;
		}

		//	create icon render node
		TLMessaging::TMessage InitMessage(TLCore::InitialiseRef);

		//	temporary text rnder node setup
		TTempString String;
		if ( !IconData.ImportDataString("name", String ) )
			TypeOrSchemeRef.GetString( String, TRUE );
	
		InitMessage.ExportDataString("string", String );
		InitMessage.ExportData("scale", IconScale );
		InitMessage.ExportData("FontRef", TRef("fdebug") );
		InitMessage.ExportData("translate", IconPosition );
		InitMessage.ExportData("boxdatum", TRef("Icons") );
		InitMessage.ExportData("boxnode", TRef("e_gui") );

		TRef IconRenderNodeRef = TLRender::g_pRendergraph->CreateNode("Icon", "TxText", m_EditorIconRootNodeRef, &InitMessage );

		//	create draggable widget on this icon
		TBinaryTree WidgetData("Widget");
		WidgetData.ExportData("Node", IconRenderNodeRef );
		WidgetData.ExportData("ActDown", TRef("IcoDown") );
		WidgetData.ExportData("ActDrag", TRef("IcoDrag") );

		//	custom data
		TPtr<TBinaryTree>& pWidgetIconData = WidgetData.AddChild("Icon");
		if ( pWidgetIconData )
		{
			//	mark all the icon data as unread so it will be added to the widget
			pIconData->SetTreeUnread();

			//	add all the data specified in the XML including "init" data for the node when it's created
			//	this will include the "Type" or "Scheme" specification
			pWidgetIconData->ReferenceDataTree( pIconData );
		}

		//	create widget
		TPtr<TLGui::TWidgetDrag> pWidget = new TLGui::TWidgetDrag( m_EditorRenderNodeRef, WidgetData );
		m_EditorWidgets.Add( pWidget );
		this->SubscribeTo( pWidget );

		IconPosition.y += IconScale.y * 1.0f;
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
void TLGame::TSchemeEditor::ProcessMouseMessage(TRefRef ActionRef,TLMessaging::TMessage& Message,Bool IsClickAction)
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
		float3 WorldPos;
		if ( !GetGameWorldPosFromScreenPos( WorldPos, ScreenPos ) )
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
	}
}


//----------------------------------------------------------
//	handle other messages (assume are commands)
//----------------------------------------------------------
void TLGame::TSchemeEditor::ProcessCommandMessage(TRefRef CommandRef,TLMessaging::TMessage& Message)
{
	//	gui command for editor from widget
	switch ( CommandRef.GetData() )
	{
		case TRef_Static(C,l,o,s,e):
		{
			//	todo: do some shutdown
			//	send close message
			TLMessaging::TMessage Message( CommandRef, "Editor" );
			PublishMessage( Message );
		}
		break;

		case TRef_Static4(S,a,v,e):
		{
			//	send save message - currently handled externally...
			TLMessaging::TMessage Message( CommandRef, "Editor" );
			PublishMessage( Message );
		}
		break;

		case TRef_Static(C,l,e,a,r):
			ClearScheme();
			break;

		default:
		{
#ifdef _DEBUG
			TTempString Debug_String("Unknown editor command ");
			CommandRef.GetString( Debug_String );
			TLDebug_Print( Debug_String );
#endif	
		}
		break;
	}
}


//----------------------------------------------------------
//	add state machine modes here. overload to add custom modes
//----------------------------------------------------------
void TLGame::TSchemeEditor::AddStateModes()
{
	AddMode<Mode_NodeEditor>("NodeEditor");
}


//----------------------------------------------------------
//	enable/disable node widgets
//----------------------------------------------------------
void TLGame::TSchemeEditor::EnableNodeWidgets(Bool Enable)
{
	for ( u32 i=0;	i<m_NodeWidgets.GetSize();	i++ )
	{
		TPtr<TLGui::TWidgetDrag>& pWidget = m_NodeWidgets[i];
		pWidget->SetEnabled( Enable );
	}
}

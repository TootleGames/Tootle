
#include "TSceneNode_Timeline.h"
#include "TScenegraph.h"

using namespace TLScene;


void TSceneNode_Timeline::Initialise(TLMessaging::TMessage& Message)
{

	TSceneNode_Object::Initialise(Message);
}

void TSceneNode_Timeline::Update(float fTimestep)
{
	// If active update the timeline instance 
	if(m_pTimelineInstance)
	{
		if(m_pTimelineInstance->Update(fTimestep) == SyncFalse)
		{
			// timeline complete
			if(m_bAutoDelete)
			{
				TLScene::g_pScenegraph->RemoveNode(GetNodeRef());
			}
		}
	}

	TSceneNode_Object::Update(fTimestep);
}

void TSceneNode_Timeline::Shutdown()
{
	DeleteTimelineInstance();

	TSceneNode_Object::Shutdown();
}


void TSceneNode_Timeline::ProcessMessage(TLMessaging::TMessage& Message)
{
	TRef MessageRef = Message.GetMessageRef();

	if(MessageRef == "Activate")
	{
		// Start the timeline instance
		CreateTimelineInstance();
	}
	else if(MessageRef == "Deactivate")
	{
		DeleteTimelineInstance();
	}
	else if(MessageRef == "OnEvent")
	{
		// Event triggered by the timeline instance
		//TODO: Handle various cases and add ability to
		// pause the timeline.  Will need to add various conditional
		// checks to allow starting of the timeline again?

		// WaitFor XYZ - Message, other conditions
		// Distance checks?
		// Or should these be handled in the timeline instance class by default?
	}
	else if(MessageRef == "TimeJump")
	{
		// Timejump event
		OnTimelineJump();
	}
	if(MessageRef == "OnComplete")
	{
		// Timeline complete
		OnTimelineComplete( *m_pTimelineInstance );
	}

	// Super process message
	TSceneNode_Object::ProcessMessage(Message);
}


void TSceneNode_Timeline::SetProperty(TLMessaging::TMessage& Message)
{
	// Import the timeline asset ref
	if(!m_TimelineAssetRef.IsValid())
		Message.ImportData("Timeline", m_TimelineAssetRef);
	else
	{
		TRef CurrentTimelineRef = m_TimelineAssetRef;

		Message.ImportData("Timeline", m_TimelineAssetRef);

		if(m_pTimelineInstance && (CurrentTimelineRef != m_TimelineAssetRef))
		{
			// Change timeline
			DeleteTimelineInstance();

			if(m_TimelineAssetRef.IsValid())
				CreateTimelineInstance();
		}
	}

	Message.ImportData("AutoDelete", m_bAutoDelete);

	TSceneNode_Object::SetProperty(Message);
}


void TSceneNode_Timeline::CreateTimelineInstance()
{
	// Timeline instance already created?
	if(m_pTimelineInstance != NULL)
		return;

	if(m_TimelineAssetRef.IsValid())
	{
		// Create the timeline instance
		m_pTimelineInstance = new TLAnimation::TTimelineInstance(m_TimelineAssetRef);

		// Bind the timeline instance to the render node and init
		if(m_pTimelineInstance)
		{
			//	get messages from timeline
			this->SubscribeTo( m_pTimelineInstance );

			//	map special refs to specific nodes
			m_pTimelineInstance->MapNodeRef("This", GetNodeRef() );
			m_pTimelineInstance->MapNodeRef("RNode", GetRenderNodeRef());
			m_pTimelineInstance->MapNodeRef("PNode", GetPhysicsNodeRef());

			//	initialse with time
			TLMessaging::TMessage Message(TLCore::InitialiseRef);
			Message.ExportData("Time", 0.0f);
			m_pTimelineInstance->Initialise(Message);
		}
	}
	else
		TLDebug_Print("No timeline specified for timeline node");
}

// Delete the timeline instance
void TSceneNode_Timeline::DeleteTimelineInstance()
{
	if(m_pTimelineInstance != NULL)
	{
		delete m_pTimelineInstance;
		m_pTimelineInstance = NULL;
	}
}


void TSceneNode_Timeline::UpdateNodeData()
{
	GetNodeData().RemoveChild("Timeline");
	GetNodeData().ExportData("Timeline", m_TimelineAssetRef);

	GetNodeData().RemoveChild("AutoDelete");
	GetNodeData().ExportData("AutoDelete", m_bAutoDelete);

	TSceneNode_Object::UpdateNodeData();
}



void TSceneNode_Timeline::OnRenderNodeAdded(TPtr<TLRender::TRenderNode>& pRenderNode)
{
	//TODO: May want to have some control flags to determine if the timeline 
	// starts as soon as the render node is added or not
	if(m_TimelineAssetRef.IsValid())
	{
		CreateTimelineInstance();
	}

	//	update binding
	if ( m_pTimelineInstance )
		m_pTimelineInstance->MapNodeRef("RNode", GetRenderNodeRef());

	TSceneNode_Object::OnRenderNodeAdded(pRenderNode);
}


//-------------------------------------------------
//	timeline has finished (this won't occur if the timeline loops)
//-------------------------------------------------
void TSceneNode_Timeline::OnTimelineComplete(TLAnimation::TTimelineInstance& Timeline)
{
	//	gr: this onimous "deactivate" deletes the timeline
	// Queue up a deactivate message so we can remove it a t a safe time
	// deleting the object now will cause issues as the call will be from the object
	TLMessaging::TMessage DeactivateMessage("Deactivate");
	QueueMessage(DeactivateMessage);
}


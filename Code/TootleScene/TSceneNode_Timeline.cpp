
#include "TSceneNode_Timeline.h"

using namespace TLScene;


void TSceneNode_Timeline::Initialise(TLMessaging::TMessage& Message)
{
	TSceneNode_Object::Initialise(Message);
}

void TSceneNode_Timeline::Update(float fTimestep)
{
	// If active update the timeline instance 
	if(m_pTimelineInstance)
		m_pTimelineInstance->Update(fTimestep);

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
	}
	if(MessageRef == "OnComplete")
	{
		// Timeline complete
		// Queue up a deactivate message so we can remove it a t a safe time
		// deleting the object now will cause issues as the call will be from the object

		TLMessaging::TMessage DeactivateMessage("Deactivate");
		QueueMessage(DeactivateMessage);
	}

	// Super process message
	TSceneNode_Object::ProcessMessage(Message);
}


void TSceneNode_Timeline::CreateTimelineInstance()
{
	// Timeline instance already created?
	if(m_pTimelineInstance != NULL)
		return;

	// Create the timeline instance
	TRef m_TimelineAssetRef;
	m_pTimelineInstance = new TLAnimation::TTimelineInstance(m_TimelineAssetRef);

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

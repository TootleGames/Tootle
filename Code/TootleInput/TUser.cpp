#include "TUser.h"

#include <TootleCore/TEventChannel.h>

#include "TLInput.h"

namespace TLUser
{
	TPtr<TUserManager> g_pUserManager = NULL;
}

using namespace TLUser;



SyncBool TUserManager::Initialise() 
{	
	TLMessaging::g_pEventChannelManager->RegisterEventChannel(this, GetManagerID(), "USER");
	TLMessaging::g_pEventChannelManager->RegisterEventChannel(this, GetManagerID(), "ACTION");

	// Register a global user by default
	RegisterUser("Global");	

	return SyncTrue; 
}

void TUserManager::OnEventChannelAdded(TRefRef refPublisherID, TRefRef refChannelID)
{
	if(refPublisherID == "CORE")
	{
		// Subscribe to the update messages
		if(refChannelID == TLCore::UpdateRef)
			TLMessaging::g_pEventChannelManager->SubscribeTo(this, refPublisherID, refChannelID); 
	}
	else if(refPublisherID == "INPUT")
	{
		// Subscribe to the update messages
		if(refChannelID == "DEVICE")
			TLMessaging::g_pEventChannelManager->SubscribeTo(this, refPublisherID, refChannelID); 
	}
	
	// Super event channel routine
	TManager::OnEventChannelAdded(refPublisherID, refChannelID);
}



Bool TUserManager::RegisterUser(TRef refUserID)
{
	if(HasUser(refUserID))
		return FALSE;

	TPtr<TUser> pUser = new TUser(refUserID);

	if(pUser.IsValid())
	{	
		s32 sIndex = m_Users.Add(pUser);

		// Set the user index
		if(sIndex > 0)
		{
			pUser->SetUserIndex( (u8)(sIndex-1) );
		}

		if(SubscribeTo(pUser))
		{
			// Broadcast message to say a new user has been added to the system
			TPtr<TLMessaging::TMessage> pMessage = new TLMessaging::TMessage("USER");

			if(pMessage )
			{
				pMessage->AddChannelID("USER");			// User change channel
				pMessage->Write("ADDED");				// User added message
				pMessage->Write(pUser.GetObject());		// User being added

				PublishMessage(pMessage);
			}

			return TRUE;
		}
	}

	return FALSE;
}

Bool TUserManager::UnregisterUser(TRef refUserID)
{
	s32 sUserIndex = FindUserIndex(refUserID);

	if(sUserIndex == -1)
		return FALSE;

	TPtr<TUser> pUser = m_Users.ElementAt(sUserIndex);

	// Broadcast message to say a user is being removed from the system
	TPtr<TLMessaging::TMessage> pMessage = new TLMessaging::TMessage("USER");

	if(pMessage )
	{
		pMessage->AddChannelID("USER");			// User change channel
		pMessage->Write("REMOVED");				// User removed message
		pMessage->Write(pUser.GetObject());		// User being removed

		PublishMessage(pMessage);
	}

	// Remove the user
	return m_Users.RemoveAt(sUserIndex);
}

void TUserManager::UnregisterAllUsers()
{
	// Go through the users and send a message saying the user is being removed
	for(u32 uIndex = 0; uIndex < m_Users.GetSize(); uIndex++)
	{
		TPtr<TUser> pUser = m_Users.ElementAt((s32)uIndex);

		// Broadcast message to say a user is being removed from the system
		TPtr<TLMessaging::TMessage> pMessage = new TLMessaging::TMessage("USER");

		if(pMessage )
		{
			pMessage->AddChannelID("USER");			// User change channel
			pMessage->Write("REMOVED");				// User removed message
			pMessage->Write(pUser.GetObject());		// User being removed

			PublishMessage(pMessage);
		}
	}

	// Remove all user objects
	m_Users.Empty();
}


s32 TUserManager::FindUserIndex(TRef refUserID)
{
	for(u32 uIndex = 0; uIndex < m_Users.GetSize(); uIndex++)
	{
		TPtr<TUser> pUser = m_Users.ElementAt(uIndex);

		if(pUser->GetUserID() == refUserID)
			return (s32)(uIndex);
	}

	// Not found
	return -1;
}

TPtr<TUser> TUserManager::FindUser(TRef refUserID)
{
	for(u32 uIndex = 0; uIndex < m_Users.GetSize(); uIndex++)
	{
		TPtr<TUser> pUser = m_Users.ElementAt(uIndex);

		if(pUser->GetUserID() == refUserID)
			return pUser;
	}

	// Not found
	return TPtr<TUser>(NULL);
}


void TUserManager::ProcessMessage(TPtr<TLMessaging::TMessage>& pMessage)
{

	TRefRef MessageRef = pMessage->GetMessageRef();

	if(MessageRef == "INPUT")
	{
		if(pMessage->HasChannelID("DEVICE"))
		{
			// Device message form the input system
			// Check for if the device has been added or removed
			TRef refState;
			if(pMessage->ImportData("STATE", refState))
			{
				if(refState == TRef("ADDED"))
				{
					// New device
					

					// return - no need to pass this message on
					return;
				}
				else if(refState == TRef("REMOVED"))
				{
					// Device removed

					// return - no need to pass this message on
					return;
				}
			}
		}
	}


	// Do manager based message checking
	TManager::ProcessMessage(pMessage);

	// Relay the message on
	TRelay::ProcessMessage(pMessage);
}


SyncBool TUserManager::Update(float /*fTimeStep*/)		
{
	// Update the cursor position for all users
	for(u32 uIndex = 0; uIndex < m_Users.GetSize(); uIndex++)
	{
		m_Users.ElementAt(uIndex)->UpdateCursorPosition();
	}

	return SyncTrue;
}

SyncBool TUserManager::Shutdown()
{
	// Remove all users
	UnregisterAllUsers();

	return SyncTrue;
}





TUser::TUser(TRef refUserID) : 
  m_refUserID(refUserID),
  m_strUserName("Unknown"),
  m_uLocalUserIndex(0)
{
	m_CursorPosition.Set(0,0);
}


void TUser::ProcessMessage(TPtr<TLMessaging::TMessage>& pMessage)
{
	// Add the user ID to the message so things know 'who' it came from
	pMessage->AddChildAndData("USERID", m_refUserID);

	// Update the cursor position to ensure it's the latest one before being passed on
	UpdateCursorPosition();
	
	// Add the users cursor information to make it quicker to access
	pMessage->AddChildAndData("CURSOR", m_CursorPosition);

	// Relay the message on
	TRelay::ProcessMessage(pMessage);
}


void TUser::UpdateCursorPosition()
{
	int2 sPos = TLInput::Platform::GetCursorPosition(GetUserIndex());

	m_CursorPosition = sPos;
}


/*
	Adds an action to the action map
*/
Bool TUser::AddAction(TRef refActionType, TRef refActionID)
{
	if(FindActionIndex(refActionID) == -1)
	{
		TPtr<TLInput::TAction> pNewAction = NULL;

		// Gesture vaiety of action?  Only two types atm and want to merge the two into 
		// one so I've not added a factory for the action classes
		if(refActionType == "Gesture")
		{
			pNewAction = new TLInput::TAction_Gesture(refActionID);
		}
		else
		{
			pNewAction = new TLInput::TAction(refActionID);
		}

		if(pNewAction)
		{
			m_ActionMap.Add(pNewAction);

			// Subscribe to the action - the user will relay the action onto other subscribers
			return SubscribeTo(pNewAction);
		}
	}

	// Failed
	return FALSE;
}


Bool TUser::AddAction(TRef refActionType, TRef refActionID, TRef refParentActionID)
{
	if(AddAction(refActionType, refActionID))
	{
		// Now set the action to have a parent action
		return MapActionParent(refActionID, refParentActionID);
	}

	return FALSE;
}

Bool TUser::AddAction(TRef refActionType, TRef refActionID, TArray<TRef>& refParentActionIDs)
{
	if(AddAction(refActionType, refActionID))
	{
		// Now set the action to have some parent actions

		// Get the new action
		TPtr<TLInput::TAction> pAction = GetAction(refActionID);

		if(!pAction.IsValid())
			return FALSE;

		// Build a list of action objects
		// If we fail to find any then return false
		TPtrArray<TLInput::TAction> pParentActions;
		for(u32 uIndex = 0; uIndex < refParentActionIDs.GetSize(); uIndex++)
		{
			TPtr<TLInput::TAction> pParentAction = GetAction(refParentActionIDs.ElementAt(uIndex));

			if(!pParentAction.IsValid())
				return FALSE;

			pParentActions.Add(pParentAction);
		}

		// Now subscribe to the actions and add the parent action ID
		for(u32 uIndex = 0; uIndex < pParentActions.GetSize(); uIndex++)
		{
			TPtr<TLInput::TAction> pParentAction = pParentActions.ElementAt(uIndex);
			
			if(!MapActionParentPtr(pAction, pParentAction))
				return FALSE;
		}

		return TRUE;
	}

	return FALSE;
}

Bool TUser::MapActionParent(TRef refActionID, TRef refParentActionID, Bool bCondition)
{
		// Get the new action
	TPtr<TLInput::TAction> pAction = GetAction(refActionID);

	if(!pAction.IsValid())
		return FALSE;

	// Get the parent action
	TPtr<TLInput::TAction> pParentAction = GetAction(refParentActionID);

	if(!pParentAction.IsValid())
		return FALSE;

	return MapActionParentPtr(pAction, pParentAction, bCondition);
}


Bool TUser::MapActionParentPtr(TPtr<TLInput::TAction> pAction, TPtr<TLInput::TAction> pParentAction, Bool bCondition)
{
	TRef refParentActionID = pParentAction->GetActionID();

	if(!pAction->HasParentAction(refParentActionID))
	{
		// Subscribe the action to the parent action
		if(pAction->SubscribeTo(pParentAction))
		{
			// Add the parent action to the actions list for checking state
			pAction->AddParentAction(refParentActionID, bCondition);

			return TRUE;
		}
	}

	return FALSE;
}


Bool TUser::MapAction(TRef refActionID, TRef refDeviceID, TRef refSensorID)
{
	TPtr<TLInput::TAction> pAction = GetAction(refActionID);

	if(!pAction.IsValid())
		return FALSE;

	TPtr<TLInput::TInputDevice> pDevice = TLInput::g_pInputSystem->GetInstance(refDeviceID);

	if(!pDevice.IsValid())
		return FALSE;

	TPtr<TLInput::TInputSensor> pSensor = pDevice->GetSensor(refSensorID);

	if(!pSensor.IsValid())
		return FALSE;

	// Assign the sensor to the action
	return pAction->SubscribeTo(pSensor);
}


Bool TUser::MapActionLabel(TRef refActionID, TRef refDeviceID, TRef refSensorLabel)
{
	TPtr<TLInput::TAction> pAction = GetAction(refActionID);

	if(!pAction.IsValid())
		return FALSE;

	TPtr<TLInput::TInputDevice> pDevice = TLInput::g_pInputSystem->GetInstance(refDeviceID);

	if(!pDevice.IsValid())
		return FALSE;

	s32 sSensorIndex = pDevice->GetSensorIndex(refSensorLabel);

	if(sSensorIndex == -1)
		return FALSE;

	TRef refSensorID = (u32)sSensorIndex;
	TPtr<TLInput::TInputSensor> pSensor = pDevice->GetSensor(refSensorID);

	if(!pSensor.IsValid())
		return FALSE;

	// Assign the sensor to the action
	return pAction->SubscribeTo(pSensor);
}



Bool TUser::MapActionCondition(TRef refActionID, TLInput::TActionCondition uCondition, float fThreshold)
{
	TPtr<TLInput::TAction> pAction = GetAction(refActionID);

	if(!pAction.IsValid())
		return FALSE;

	pAction->SetCondition(uCondition, fThreshold);
	return TRUE;
}



/*
	Removes an action from the action map
*/
Bool TUser::RemoveAction(TRef refActionID)
{
	s32 iIndex = FindActionIndex(refActionID);

	if(iIndex != -1)
			return m_ActionMap.RemoveAt(iIndex);

	return FALSE;
}

/*
	Gets an input action with the specified ID
*/
TPtr<TLInput::TAction> TUser::GetAction(TRef refActionID)
{
	s32 iIndex = FindActionIndex(refActionID);

	if(iIndex != -1)
			return m_ActionMap.ElementAt(iIndex);

	// Not found
	return TPtr<TLInput::TAction>(NULL);
}


/*
	Finds an action within the action map with the specified action ID
*/
s32 TUser::FindActionIndex(TRef refActionID)
{
	for(u32 uIndex = 0; uIndex < m_ActionMap.GetSize(); uIndex++)
	{
		TPtr<TLInput::TAction> pAction = m_ActionMap.ElementAt(uIndex);

		if(pAction->GetActionID() == refActionID)
			return (s32) uIndex;

	}

	// Not found
	return -1;
}

#include "TWidgetManager.h"

#include <TootleCore/TEventChannel.h>
#include <TootleInput/TLInput.h>




namespace TLGui
{
	TPtr<TWidgetManager>	g_pWidgetManager = NULL;
}


using namespace TLGui;


void TWidgetManager::OnEventChannelAdded(TRefRef refPublisherID, TRefRef refChannelID)
{
	if(refPublisherID == "INPUT")
	{
		// Subscribe to the input device messages
		if(refChannelID == "DeviceChanged")
			TLMessaging::g_pEventChannelManager->SubscribeTo(this, refPublisherID, refChannelID); 
		
	}
	
	// Super event channel routine
	TLCore::TManager::OnEventChannelAdded(refPublisherID, refChannelID);
}


SyncBool TWidgetManager::Initialise()
{
	SyncBool Result = TManager::Initialise();
	
	if(Result != SyncTrue)
		return Result;
	
	// Attach the base widget factory by default
	TPtr<TClassFactory<TWidget,FALSE> > pFactory = new TWidgetFactory();
	
	if(pFactory)
		AddFactory(pFactory);
	
	return SyncTrue;
}


SyncBool TWidgetManager::Shutdown()
{ 	
	TLDebug_Print("Widgetmanager shutdown");
	
	m_WidgetCache.Clear();
	
	TLMessaging::g_pEventChannelManager->UnsubscribeFrom(this, "INPUT", "DeviceChanged"); 
	
	return TManager::Shutdown();
}


//-----------------------------------------------------------
//	process messages
//-----------------------------------------------------------
void TWidgetManager::ProcessMessage(TLMessaging::TMessage& Message)
{
	TRefRef MessageRef = Message.GetMessageRef();

	if(MessageRef == "DeviceChanged")
	{
		// Device message form the input system
		// Check for if the device has been added or removed
		TRef refState;
		if(Message.ImportData("State", refState))
		{
			TRef DeviceRef, DeviceTypeRef;
			if(!Message.ImportData("DEVID", DeviceRef))
				return;

			if(!Message.ImportData("TYPE", DeviceTypeRef))
				return;

			if(refState == STRef(A,D,D,E,D))
			{
				// New device
				OnInputDeviceAdded( DeviceRef, DeviceTypeRef );
			}
			else if(refState == STRef(R,E,M,O,V))
			{
				// Device removed
				OnInputDeviceRemoved(DeviceRef, DeviceTypeRef);
			}

			// return - no need to pass this message on
			return;
		}
	}
	
	// Action messages

	TLCore::TManager::ProcessMessage(Message);
}

void TWidgetManager::OnInputDeviceAdded(TRefRef DeviceRef, TRefRef DeviceTypeRef)
{
	TPtr<TLUser::TUser>& pGlobalUser = TLUser::g_pUserManager->GetUser("Global");
	
	if(!pGlobalUser.IsValid())
	{
		TLDebug_Break("Failed to find global user");
		return;
	}

	if(DeviceTypeRef == STRef(k,e,y,b,o))
	{
		MapDeviceActions_Keyboard(DeviceRef, pGlobalUser);
	}
	else if(DeviceTypeRef == STRef(M,O,U,S,E))
	{

#ifdef TL_TARGET_IPOD
		MapDeviceActions_TouchPad(DeviceRef, pGlobalUser);
#else
		MapDeviceActions_Mouse(DeviceRef, pGlobalUser);
#endif
	}
}

#ifdef TL_TARGET_IPOD

void TWidgetManager::MapDeviceActions_TouchPad(TRefRef DeviceRef, TPtr<TLUser::TUser> pUser)
{
	// Get the device.  This should *always* be valid if the ID is valid.
	TPtr<TLInput::TInputDevice> pDevice = TLInput::g_pInputSystem->FindPtr(DeviceRef);

	if(!pDevice)
	{
		TLDebug_Break("Invalid device ref");
		return;
	}

	
	/////////////////////////////////////////////////////////////////////////
	// GUI SYSTEM ACTIONS
	/////////////////////////////////////////////////////////////////////////
	
	// Create a generic 'click' action for all buttons
	
	u32 NumberOfButtons = pDevice->GetSensorCount(TLInput::Button);
	
	for ( u32 s=0; s < NumberOfButtons ;s++ )
	{
		TRef ButtonRef = TLInput::GetDefaultButtonRef(s);
		TRef ClickActionRef = pUser->GetUnusedActionRef("click");
		
		if(pUser->AddAction("SIMPLE", ClickActionRef))	
		{
			TActionRefData ActionRefData;
			
			ActionRefData.m_ClickActionRef = ClickActionRef;
			
			// Map the click to this particular button
			pUser->MapAction(ClickActionRef, DeviceRef, ButtonRef);
			
			// Now add a generic 'move' action that is dependant on this particular click
			TRef MoveActionRef = pUser->GetUnusedActionRef("move");
			
			if(pUser->AddAction("SIMPLE", MoveActionRef))	
			{
				// Map the click to this particular button
				pUser->MapActionParent(MoveActionRef, ClickActionRef);
				
				TRef AxisRef_x = TLInput::GetDefaultAxisRef( s, 'x' );
				TRef AxisRef_y = TLInput::GetDefaultAxisRef( s, 'y' );
				
				pUser->MapAction( MoveActionRef, DeviceRef, AxisRef_x );
				pUser->MapAction( MoveActionRef, DeviceRef, AxisRef_y );
				
				ActionRefData.m_MoveActionRef = MoveActionRef;
			}

			// Add the action ref data to the array
			m_WidgetActionRefs.Add(ActionRefData);

		}
	}	
}

#else

void TWidgetManager::MapDeviceActions_Mouse(TRefRef DeviceRef, TPtr<TLUser::TUser> pUser)
{
	// Get the device.  This should *always* be valid if the ID is valid.
	TPtr<TLInput::TInputDevice> pDevice = TLInput::g_pInputSystem->FindPtr(DeviceRef);

	if(!pDevice)
	{
		TLDebug_Break("Invalid device ref");
		return;
	}


	/////////////////////////////////////////////////////////////////////////
	// GUI SYSTEM ACTIONS
	/////////////////////////////////////////////////////////////////////////

	// Create a generic 'click' action for all buttons
	
	u32 NumberOfButtons = pDevice->GetSensorCount(TLInput::Button);
	
	for ( u32 s=0; s < NumberOfButtons ;s++ )
	{
		TRef ButtonRef = TLInput::GetDefaultButtonRef(s);
		TRef ClickActionRef = pUser->GetUnusedActionRef("click");
		
		if(pUser->AddAction("SIMPLE", ClickActionRef))	
		{
			TActionRefData ActionRefData;
			
			ActionRefData.m_ClickActionRef = ClickActionRef;
			
			// Map the click to this particular button
			pUser->MapAction(ClickActionRef, DeviceRef, ButtonRef);
			
			// Now add a generic 'move' action that is dependant on this particular click
			TRef MoveActionRef = pUser->GetUnusedActionRef("move");
			
			if(pUser->AddAction("SIMPLE", MoveActionRef))	
			{
				// Map the click to this particular button
				pUser->MapActionParent(MoveActionRef, ClickActionRef);
				
				TRef AxisRef_x = TLInput::GetDefaultAxisRef( s, 'x' );
				TRef AxisRef_y = TLInput::GetDefaultAxisRef( s, 'y' );
				
				pUser->MapAction( MoveActionRef, DeviceRef, AxisRef_x );
				pUser->MapAction( MoveActionRef, DeviceRef, AxisRef_y );
				
				ActionRefData.m_MoveActionRef = MoveActionRef;
			}
			
			// Add the action ref data to the array
			m_WidgetActionRefs.Add(ActionRefData);
			
		}
	}	

}


#endif


void TWidgetManager::MapDeviceActions_Keyboard(TRefRef DeviceRef, TPtr<TLUser::TUser> pUser)
{
	/*
	// Subscribe to the keyboard buttons
	// Get the device.  This should *always* be valid if the ID is valid.
	TPtr<TLInput::TInputDevice> pDevice = TLInput::g_pInputSystem->FindPtr(DeviceRef);
	
	if(!pDevice)
	{
		TLDebug_Break("Invalid device ref");
		return;
	}
	
	///////////////////////////////////////////////////////
	// Needs moving into the device class so we can subscribe 
	// something to all sensors of a specific class
	///////////////////////////////////////////////////////
	u32 NumberOfButtons = pDevice->GetSensorCount(TLInput::Button);
	
	for(u32 uIndex = 0; uIndex < NumberOfButtons; uIndex++)
	{
		TRef ButtonRef = TLInput::GetDefaultButtonRef(uIndex);

		TPtr<TLInput::TInputSensor>& pSensor = pDevice->GetSensor(ButtonRef);
		
		if(pSensor.IsValid())
			SubscribeTo(pSensor);
	}
	///////////////////////////////////////////////////////
	*/
			
	// Get a list of the supported characters 
	TArray<TRef> RefArray;
	
	if(TLInput::g_pInputSystem->BuildArrayOfSupportInputCharacterRefs(RefArray))
	{
		for(u32 uIndex = 0; uIndex < RefArray.GetSize(); uIndex++)
		{
			TRef KeyRef = RefArray.ElementAt(uIndex);
			
			// Map actions for each key of the keyboard we will support use of
			if(pUser->AddAction("SIMPLE", KeyRef))
			{
				// The keyref is the same as one of the sensor labels so we can use the same ref. 
				pUser->MapAction(KeyRef, DeviceRef, KeyRef);
				pUser->MapActionCondition(KeyRef, TLInput::LessThan, 1.0f); // Release
			}
		}			
	}
	/*
	 // VERY SPECIAL CHARACTERS. PROBABLY NEED A LOOKUP FOR THE CHARS FOR THESE TO WORK CORRECTLY
	pGlobalUser->AddAction("SIMPLE", "-");
	pGlobalUser->MapAction("-", DeviceRef, "k_minus");
	pGlobalUser->MapActionCondition("-", TLInput::LessThan, 1.0f); // Release
	
	pGlobalUser->AddAction("SIMPLE", "/");
	pGlobalUser->MapAction("/", DeviceRef, "k_forwardslash");
	pGlobalUser->MapActionCondition("/", TLInput::LessThan, 1.0f); // Release
	
	pGlobalUser->AddAction("SIMPLE", ":");
	pGlobalUser->MapAction(":", DeviceRef, "k_colon");
	pGlobalUser->MapActionCondition(":", TLInput::LessThan, 1.0f); // Release
	
	pGlobalUser->AddAction("SIMPLE", ";");
	pGlobalUser->MapAction(";", DeviceRef, "k_semicolon");
	pGlobalUser->MapActionCondition(";", TLInput::LessThan, 1.0f); // Release
	
	pGlobalUser->AddAction("SIMPLE", "(");
	pGlobalUser->MapAction("(", DeviceRef, "k_lbracket");
	pGlobalUser->MapActionCondition("(", TLInput::LessThan, 1.0f); // Release
	
	pGlobalUser->AddAction("SIMPLE", ")");
	pGlobalUser->MapAction(")", DeviceRef, "k_rbracket");
	pGlobalUser->MapActionCondition(")", TLInput::LessThan, 1.0f); // Release
	
	pGlobalUser->AddAction("SIMPLE", "£");
	pGlobalUser->MapAction("£", DeviceRef, "k_sterling");
	pGlobalUser->MapActionCondition("£", TLInput::LessThan, 1.0f); // Release
	
	pGlobalUser->AddAction("SIMPLE", "&");
	pGlobalUser->MapAction("&", DeviceRef, "k_ampersand");
	pGlobalUser->MapActionCondition("&", TLInput::LessThan, 1.0f); // Release
	
	pGlobalUser->AddAction("SIMPLE", "@");
	pGlobalUser->MapAction("@", DeviceRef, "k_at");
	pGlobalUser->MapActionCondition("@", TLInput::LessThan, 1.0f); // Release
	
	pGlobalUser->AddAction("SIMPLE", "\"");
	pGlobalUser->MapAction("\"", DeviceRef, "k_quote");
	pGlobalUser->MapActionCondition("\"", TLInput::LessThan, 1.0f); // Release
	
	pGlobalUser->AddAction("SIMPLE", ".");
	pGlobalUser->MapAction(".", DeviceRef, "k_period");
	pGlobalUser->MapActionCondition(".", TLInput::LessThan, 1.0f); // Release
	
	pGlobalUser->AddAction("SIMPLE", ",");
	pGlobalUser->MapAction(",", DeviceRef, "k_comma");
	pGlobalUser->MapActionCondition(",", TLInput::LessThan, 1.0f); // Release
	
	pGlobalUser->AddAction("SIMPLE", "?");
	pGlobalUser->MapAction("?", DeviceRef, "k_question");
	pGlobalUser->MapActionCondition("?", TLInput::LessThan, 1.0f); // Release
	
	pGlobalUser->AddAction("SIMPLE", "!");
	pGlobalUser->MapAction("!", DeviceRef, "k_exclamation");
	pGlobalUser->MapActionCondition("!", TLInput::LessThan, 1.0f); // Release
	
	pGlobalUser->AddAction("SIMPLE", "'");
	pGlobalUser->MapAction("'", DeviceRef, "k_apostrophe");
	pGlobalUser->MapActionCondition("'", TLInput::LessThan, 1.0f); // Release
	
	pGlobalUser->AddAction("SIMPLE", "+");
	pGlobalUser->MapAction("+", DeviceRef, "k_plus");
	pGlobalUser->MapActionCondition("+", TLInput::LessThan, 1.0f); // Release
	
	pGlobalUser->AddAction("SIMPLE", "*");
	pGlobalUser->MapAction("*", DeviceRef, "k_multiply");
	pGlobalUser->MapActionCondition("*", TLInput::LessThan, 1.0f); // Release
	
	pGlobalUser->AddAction("SIMPLE", "=");
	pGlobalUser->MapAction("=", DeviceRef, "k_equals");
	pGlobalUser->MapActionCondition("=", TLInput::LessThan, 1.0f); // Release
*/			
	
	// Special character actions
	if(pUser->AddAction("SIMPLE", "k_backspace"))
	{
		pUser->MapAction("k_backspace", DeviceRef, "k_backspace");
		pUser->MapActionCondition("k_backspace", TLInput::LessThan, 1.0f); // Release
	}

	if(pUser->AddAction("SIMPLE", "k_return"))
	{
		pUser->MapAction("k_return", DeviceRef, "k_return");
		pUser->MapActionCondition("k_return", TLInput::LessThan, 1.0f); // Release
	}
	///////////////////////////////////////////////////////
	
}


void TWidgetManager::OnInputDeviceRemoved(TRefRef DeviceRef, TRefRef DeviceTypeRef)
{
	// Remove any added actions from the global user.
	
	TPtr<TLUser::TUser>& pGlobalUser = TLUser::g_pUserManager->GetUser("Global");
	
	if(!pGlobalUser.IsValid())
	{
		TLDebug_Break("Failed to find global user");
		return;
	}
	
	if(DeviceTypeRef == STRef(k,e,y,b,o))
	{
		// Get a list of the supported characters 
		TArray<TRef> RefArray;
		
		if(TLInput::g_pInputSystem->BuildArrayOfSupportInputCharacterRefs(RefArray))
		{
			for(u32 uIndex = 0; uIndex < RefArray.GetSize(); uIndex++)
			{
				TRef KeyRef = RefArray.ElementAt(uIndex);
				
				// Map actions for each key of the keyboard we will support use of
				if(!pGlobalUser->RemoveAction(KeyRef))
				{
					TLDebug_Break("Action being removed from user that doesn't exist.");
				}
			}			
		}
		
		// Remove special actions
		if(!pGlobalUser->RemoveAction("k_backspace"))
		{
			TLDebug_Break("Action being removed from user that doesn't exist.");
		}
		
		if(!pGlobalUser->RemoveAction("k_return"))
		{
			TLDebug_Break("Action being removed from user that doesn't exist.");
		}
		
	}		
}


TRef TWidgetManager::CreateWidget(TRefRef WidgetGroupRef, TRefRef InstanceRef, TRefRef TypeRef, Bool bStrict)
{
#ifdef _DEBUG
	TTempString str("Attempting to create widget ");
	InstanceRef.GetString(str);
	str.Append(" with group ");
	WidgetGroupRef.GetString(str);
	TLDebug_Print(str);	
#endif
	
	// Find group to add the widget ref to and if not found create a new one
	Bool bGroupCreated = FALSE;
	
	TPtrArray<TWidget>* pGroupArray;
	
	if(m_WidgetCache.GetWidgetGroupRef() == WidgetGroupRef)
		pGroupArray = m_WidgetCache.GetWidgetGroup();
	else 
	{
		// Different group so clear the cache
		m_WidgetCache.Clear();
		
		pGroupArray = m_WidgetRefs.Find(WidgetGroupRef);
	}
	

	
	if(!pGroupArray)
	{
		pGroupArray = m_WidgetRefs.AddNew(WidgetGroupRef);
		
		// Failed?
		if(!pGroupArray)
		{
			TLDebug_Break("Failed to create new widget group");
			return TRef();
		}
		
		bGroupCreated = TRUE;
		
		TLDebug_Print("Created new widget group");
	}
	
	
	// Now create the actual widget object
	TPtr<TWidget> pWidget;
	
	TRef FinalWidgetRef = InstanceRef;

	if(bStrict)
	{
		// does the widget need to be an exact ref?  (usually only when created from schemes)
		if(pGroupArray->FindPtr(FinalWidgetRef).IsValid())
		{
			if(bGroupCreated)
			{
				pGroupArray = NULL;
				// Remove the group again if no widget was created
				m_WidgetRefs.Remove(WidgetGroupRef);
				TLDebug_Print("Removing newly created group");
			}				

			TLDebug_Break("Widget with ref already exists with strict ref creation");

			// Failed
			return TRef();
		}
	}
	else 
	{
		// While the instance ref is not unique within the group, increment the ref
		while(pGroupArray->FindPtr(FinalWidgetRef).IsValid())
		{
			FinalWidgetRef.Increment();
		}
	}
	
	
	// Create new widget via the factories
	for(u32 uIndex=0; uIndex < m_WidgetFactories.GetSize(); uIndex++)
	{
		m_WidgetFactories[uIndex]->CreateInstance( pWidget, FinalWidgetRef, TypeRef );
		
		// If a widget was created then carry on otherwise try the next factory
		if ( pWidget )
			break;

	}
	
	if(!pWidget)	
	{
		TLDebug_Break("Failed to create widget");

		if(bGroupCreated)
		{
			pGroupArray = NULL;
			// Remove the group again if no widget was created
			m_WidgetRefs.Remove(WidgetGroupRef);
			TLDebug_Print("Removing newly created group");

		}
		
		return TRef();
	}
	
	pGroupArray->Add(pWidget);
	
	// Update the cache
	m_WidgetCache.Set(WidgetGroupRef, pGroupArray, pWidget);
		
	return FinalWidgetRef;
}

Bool TWidgetManager::DoRemoveWidget(TRefRef WidgetGroupRef, TRefRef InstanceRef)
{
#ifdef _DEBUG
	TTempString str("Attempting to remove widget ");
	InstanceRef.GetString(str);
	str.Append(" from group ");
	WidgetGroupRef.GetString(str);
	TLDebug_Print(str);
	
#endif
	
	TPtrArray<TWidget>* pGroupArray;
	
	// Check the cache first, otherwise do a find of the group
	if(m_WidgetCache.GetWidgetGroupRef() == WidgetGroupRef)
		pGroupArray = m_WidgetCache.GetWidgetGroup();
	else
	{
		// Different group so clear the cache
		m_WidgetCache.Clear();
		
		pGroupArray = m_WidgetRefs.Find(WidgetGroupRef);
		
		if(pGroupArray)
		{
			// Going to be removing the widget so set the caches group data only - subsequent removals can then use
			// the group in the cache rather than looking it up
			m_WidgetCache.Set(WidgetGroupRef, pGroupArray, NULL);
		}
	}
	
	TLDebug_Assert(pGroupArray,"Failed to find group for widget");
	
	TPtr<TWidget> pWidget = pGroupArray->FindPtr(InstanceRef);
	
	if(!pWidget.IsValid())
	{
		TLDebug_Break("Failed to find widget for removal");

		return FALSE;
	}
	
	// Check the widget cache and reset the widget pointer if it is this widget
	if(pWidget == m_WidgetCache.GetWidget())
	{
		m_WidgetCache.SetWidget(NULL);
	}
	
	// Now remove the widget
	if(!pGroupArray->Remove(pWidget))
	{
		TLDebug_Break("Failed to remove widget");
		return FALSE;
	}
	
	// At this point the only reference to the widget object should be our temporary pWidget TPtr.
	// Check the TPtr counter to ensure nothing is referencing it outside of the widget system and then null the pointer 
	// to delete the object
#ifdef _DEBUG
	if(pWidget.GetRefCount() > 1)
	{
		TLDebug_Break("Widget is still being reference outside of the widget system");
	}
#endif
	pWidget = NULL;

	// No more items in the array?  Remove from the key array item/group for the widgets
	if(pGroupArray->GetSize() == 0)
	{
		TLDebug_Print("Widget group empty - removing");
		//m_WidgetRefs.RemoveAt(uIndex);
		
		// Find and remove group
		s32 sIndex = m_WidgetRefs.FindIndex(WidgetGroupRef);
		
		if(sIndex > -1)
		{
			m_WidgetRefs.RemoveAt(sIndex);
			
			// Removed the group so clear the cache
			m_WidgetCache.Clear();
		}
		else 
			TLDebug_Break("Invalid index for widget group removal");
	}
	else 
	{		
		// Clear the widget pointer in the cache.  The group can remain the same for further removal requests.
		m_WidgetCache.SetWidget(TPtr<TWidget>(NULL));
	}

	
	return TRUE;
}


void TWidgetManager::SendMessageToWidget(TRefRef WidgetGroupRef, TRefRef WidgetRef, TLMessaging::TMessage& Message)
{
	//TODO: Queue up the message for sending to the widget
	
	//TEMP: Instant process of message
	TPtr<TWidget> pWidget = FindWidget(WidgetGroupRef, WidgetRef);

	if(pWidget)
		pWidget->ProcessMessage(Message);
}


Bool TWidgetManager::SubscribeToWidget(TRefRef WidgetGroupRef, TRefRef WidgetRef, TSubscriber* pSubscriber)
{
	TPtr<TWidget> pWidget = FindWidget(WidgetGroupRef, WidgetRef);
		
	if(pWidget)
		return pSubscriber->SubscribeTo(pWidget);

	return FALSE;
}

Bool TWidgetManager::SubscribeWidgetTo(TRefRef WidgetGroupRef, TRefRef WidgetRef, TPublisher* pPublisher)
{
	TPtr<TWidget> pWidget = FindWidget(WidgetGroupRef, WidgetRef);
	
	if(pWidget)
		return pWidget->SubscribeTo(pPublisher);
	
	return FALSE;
}



TPtr<TWidget> TWidgetManager::FindWidget(TRefRef WidgetGroupRef, TRefRef WidgetRef)
{
	
	TPtrArray<TWidget>* pGroupArray;
	
	if(m_WidgetCache.GetWidgetGroupRef() == WidgetGroupRef)
		pGroupArray = m_WidgetCache.GetWidgetGroup();
	else 
	{
		// Different group so clear the cache
		m_WidgetCache.Clear();

		pGroupArray = m_WidgetRefs.Find(WidgetGroupRef);
		
		if(pGroupArray)
		{
			// When using a new group set the group data in the cache and reset the widget pointer
			m_WidgetCache.Set(WidgetGroupRef, pGroupArray, NULL);
		}
	}


	if(!pGroupArray)
		return TPtr<TWidget>(NULL);

	
	// Check the widget in the cache first and return that if it's the same ref.
	if(m_WidgetCache.GetWidget() && (m_WidgetCache.GetWidget()->GetWidgetRef() == WidgetRef))
		return m_WidgetCache.GetWidget();
	else 
	{
		TPtr<TWidget> pWidget = pGroupArray->FindPtr(WidgetRef);
		
		// Set the widget in the cache
		m_WidgetCache.SetWidget(pWidget);

		return pWidget;
	}
	
	/*
	TPtr<TWidget> pWidget;

	// Find the widget via the factories
	for(u32 uIndex=0; uIndex < pGroupArray.GetSize(); uIndex++)
	{
		pWidget = pGroupArray->ElementAt(uIndex);
		
		// If a widget was created then carry on otherwise try the next factory
		if ( pWidget )
			break;		
	}
	
	return pWidget;
	 */
}


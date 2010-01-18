#include "TAction.h"
#include "TLInput.h"

using namespace TLInput;

#ifdef _DEBUG
	//#define ENABLE_INPUTACTION_TRACE
#endif

TAction::TAction(TRefRef refActionID)	:
	 m_refActionID		( refActionID ),
	 m_uActionCondition	( None ),
	 m_fThreshold		( 0.0f )
{
}


void TAction::AddParentAction(TRefRef ParentActionRef, Bool bCondition)
{
	// Check to see if the Action Parent already exists
	for(u32 uIndex = 0; uIndex < m_refParentActions.GetSize(); uIndex++)
	{
		if(m_refParentActions.ElementAt(uIndex) == ParentActionRef)
		{
			// Parent action exists already - check the condition
			TParentActionState& PAS = m_bParentActionStates.ElementAt(uIndex);

			// parent action alreayd exist with the same condition check - return TRUE
			if(PAS.m_bCondition != bCondition)
			{
				// Change the condition
				PAS.m_bCondition = bCondition;
			}

			return;
		}
	}

	// Add a new action and associated state
	m_refParentActions.Add(ParentActionRef);

	TParentActionState PAS;
	PAS.m_bState = FALSE;
	PAS.m_bCondition = bCondition;

	m_bParentActionStates.Add(PAS);
}


void TAction::ProcessMessage(TLMessaging::TMessage& Message)
{
	if ( Message.GetMessageRef() == TRef_Static(A,c,t,i,o) )
	{
		// Check to see if the message coming in is for a parent action
		if ( m_refParentActions.GetSize() )
			ProcessParentActionMessage( Message );
		else
			ProcessSensorMessage( Message );			
	}
	else if ( Message.GetMessageRef() == TRef_Static(O,n,I,n,p) )
	{
		ProcessSensorMessage( Message );
	}
}

void TAction::ProcessParentActionMessage(TLMessaging::TMessage& Message)
{
	// Get the action ID
	TRef refActionID;
	if(!Message.Read(refActionID))
		return;

	//	gr: use find index here? better to just merge m_refParentActions & m_bParentActionStates and then Find...
	for(u32 uIndex = 0; uIndex < m_refParentActions.GetSize(); uIndex++)
	{
		if(m_refParentActions.ElementAt(uIndex) == refActionID)
		{
			TParentActionState& PAS = m_bParentActionStates.ElementAt(uIndex);

			// Toggle the state to say the parent action is currently active/inactive
			//	gr: this system doesn't work; I create a new condition (mouse is down) in a previous "mouse is down" message.
			//		the ParentActionState is initialised to OFF... (even though the mouse is down)
			//		mouse button goes up, this state is then TOGGLED to ON (even though mouse is up)
			//		the action with this parent is then triggered at the opposite expected time. (State is ON when it should be off and vice versa)

			//	to fix this.... implement a proper condition system for the parent state?
			//					or interpret the raw data as on/off?
			//	toggling isn't practical!
			
			//	gr: temp version using raw data. this works for mouse clicks. Needs changing for more complicated usage.
			float RawData;
			if ( !Message.ImportData("RawData", RawData ) )
				continue;

			//PAS.m_bState = !PAS.m_bState;
			PAS.m_bState = (RawData > 0.f);
			
#ifdef ENABLE_INPUTACTION_TRACE
			TTempString Debug_String("Parent action state [of ");
			m_refActionID.GetString(Debug_String);
			Debug_String.Appendf("] state set to %d", PAS.m_bState  );
			TLDebug_Print(Debug_String);
#endif
		}
	}

}



void TAction::ProcessSensorMessage(TLMessaging::TMessage& Message)
{
	// Check the parent action states
	// If any are FALSE then this action will be oppressed
	for(u32 uIndex = 0; uIndex < m_bParentActionStates.GetSize(); uIndex++)
	{
		TParentActionState& PAS = m_bParentActionStates.ElementAt(uIndex);
		if(PAS.m_bState != PAS.m_bCondition)
		{
#ifdef ENABLE_INPUTACTION_TRACE
			TTempString Debug_String("Parent action state [of ");
			m_refActionID.GetString(Debug_String);
			Debug_String.Append("] doesn't meet condition");
			TLDebug_Print(Debug_String);
#endif				
			return;
		}
	}

	Bool bActionOccured = TRUE;

	// Check to see if the condition has been met
	if(m_uActionCondition != None)
	{
		float fRawValue = 1.0f;

		if(Message.ImportData("RAWDATA", fRawValue))
		{
			// Assume failed
			bActionOccured = FALSE;

			switch(m_uActionCondition)
			{
				case GreaterThan:
					if(fRawValue > m_fThreshold)
							bActionOccured = TRUE;
					break;
				case LessThan:
					if(fRawValue < m_fThreshold)
						bActionOccured = TRUE;
					break;
				case GreaterThanEqual:
					if(fRawValue >= m_fThreshold)
							bActionOccured = TRUE;
					break;
				case LessThanEqual:
					if(fRawValue < m_fThreshold)
						bActionOccured = TRUE;
					break;
			}
		}
	}

	//	action aborted due to conditions not being met
	if(!bActionOccured)
		return;

#ifdef ENABLE_INPUTACTION_TRACE
	TTempString Debug_String("Action ");
	m_refActionID.GetString( Debug_String );
	Debug_String.Append(" triggered");
	TLDebug_Print(Debug_String);
#endif
	// Send out a new message specifying the action ID to say this action has happened.
	TLMessaging::TMessage NewMessage(TRef_Static(A,c,t,i,o));
	NewMessage.Write(m_refActionID);			// The action performed

	TPtr<TBinaryTree>& pChild = Message.GetChild("RAWDATA");

	// Copy the raw data value to the new message if it exists
	if(pChild.IsValid())
		NewMessage.AddChild(pChild);
	
#ifdef TL_TARGET_IPOD		
	TPtr<TBinaryTree>& pChild2 = Message.GetChild("CIDX");
	
	// Copy the cursor index value to the new message if it exists
	if(pChild2.IsValid())
		NewMessage.AddChild(pChild2);
#endif

	PublishMessage(NewMessage);
}




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



TAction_Gesture::TAction_Gesture(TRef refActionID) :
	TAction		(refActionID)
{
	// Create a new neural network by default
	m_pNeuralNetwork = new TLNeuralNetwork::TNeuralNetwork("ACTIONNETWORK");
}

/*
	Adds a sensor as an input to a neural network
*/
Bool TAction_Gesture::AddNeuralNetworkInput(TRef refDeviceID, TRef refSensorID)
{
	TPtr<TInputDevice> pDevice = TLInput::g_pInputSystem->GetInstance(refDeviceID);

	if(!pDevice.IsValid())
		return FALSE;

	// Get the sensor we are going to add as an input
	TPtr<TInputSensor> pSensor = pDevice->GetSensor(refSensorID);

	// Sensor doesn't exist
	if(!pSensor.IsValid())
		return FALSE;

	// Add the sensor as an input to the network
	TPtr<TLNeuralNetwork::TNeuron> pNeuron = m_pNeuralNetwork->AddInputNeuron();

	// Failed to add neuron?
	if(!pNeuron.IsValid())
		return FALSE;

	// subscribe the neuron to the sensor
	return pNeuron->SubscribeTo(pSensor);
}


/*
	Removes a nerual network sensor input
*/
Bool TAction_Gesture::RemoveNeuralNetworkInput(TRef refDeviceID, TRef refSensorID)
{
	TPtr<TInputDevice> pDevice = TLInput::g_pInputSystem->GetInstance(refDeviceID);

	if(!pDevice.IsValid())
		return FALSE;

	// Get the sensor we are going to add as an input
	TPtr<TInputSensor> pSensor = pDevice->GetSensor(refSensorID);

	// Sensor doesn't exist
	if(!pSensor.IsValid())
		return FALSE;

	// Remove the sensor as an input

	return FALSE;
}


/*
	Creates a default neural network for a particular button detection along with setting up the sensor input and action output
*/
Bool TAction_Gesture::CreateDefaultButtonNeuralNetwork(TRef refDeviceID, TRef refSensorID)
{
	TPtr<TInputDevice> pDevice = TLInput::g_pInputSystem->GetInstance(refDeviceID);

	if(!pDevice.IsValid())
		return FALSE;

	TPtr<TInputSensor> pSensor = pDevice->GetSensor(refSensorID);

	// Does sensor exist?  We don't create this so if not then we can't proceed
	if(!pSensor.IsValid())
		return FALSE;


	// Add an input neuron
	TPtr<TLNeuralNetwork::TNeuron> pInputNeuron = m_pNeuralNetwork->AddInputNeuron();

	if(!pInputNeuron.IsValid())
		return FALSE;

	// Add an output neuron
	TPtr<TLNeuralNetwork::TNeuron> pOutputNeuron = m_pNeuralNetwork->AddNeuron(pInputNeuron);

	if(!pOutputNeuron.IsValid())
		return FALSE;		

	// Link the nodes together
	TPtr<TLNeuralNetwork::TAxon> pAxon = m_pNeuralNetwork->Connect(pInputNeuron, pOutputNeuron);

	if(!pAxon.IsValid())
		return FALSE;

	// Set the threshold
	pInputNeuron->SetThreshold(0.9f);

	// Link the input neuron to the input sensor
	pInputNeuron->SubscribeTo(pSensor);

	// Subscribe the action to the output neuron
	SubscribeTo(pOutputNeuron);

	// All done
	return TRUE;
}

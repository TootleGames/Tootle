#include "TAction.h"
#include "TLInput.h"

using namespace TLInput;

#ifdef _DEBUG
//#define ENABLE_INPUTACTION_TRACE
#endif

TAction::TAction(TRef refActionID)	:
	 m_refActionID(refActionID),
	 m_uActionCondition(None),
	 m_fThreshold(0.0f)
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


void TAction::ProcessMessage(TPtr<TLMessaging::TMessage>& pMessage)
{
	// Does this action have any parent actions?  If so handle those first

	if(m_refParentActions.GetSize())
	{
		TRef refChannelID;
		if(pMessage->ImportData("CHANNELID", refChannelID))
		{
			// Check to see if the message coming in is for a parent action
			if(refChannelID == "ACTION")
			{
				// Get the action ID
				TRef refActionID;

				if(pMessage->Read(refActionID))
				{
					for(u32 uIndex = 0; uIndex < m_refParentActions.GetSize(); uIndex++)
					{
						if(m_refParentActions.ElementAt(uIndex) == refActionID)
						{
							TParentActionState& PAS = m_bParentActionStates.ElementAt(uIndex);

							// Toggle the state to say the parent action is currently active/inactive
							PAS.m_bState = (PAS.m_bState ? 0 : 1);
							
#ifdef ENABLE_INPUTACTION_TRACE
							TString straction;
							m_refActionID.GetString(straction);
							TLDebug_Print(straction);
							
							TString strstate;
							strstate.Appendf("Parent Action state set to %d", PAS.m_bState);
							TLDebug_Print(strstate);
#endif
							
						}
					}
				}

				// We don't need to process htis message any further
				return;
			}
		}

		// Check the parent action states
		// If any are FALSE then this action will be oppressed
		for(u32 uIndex = 0; uIndex < m_bParentActionStates.GetSize(); uIndex++)
		{
			TParentActionState& PAS = m_bParentActionStates.ElementAt(uIndex);
			if(PAS.m_bState != PAS.m_bCondition)
			{
#ifdef ENABLE_INPUTACTION_TRACE
				TLDebug_Print("Parent Action not set");
				TString straction;
				m_refActionID.GetString(straction);
				TLDebug_Print(straction);
#endif
				
				return;
			}
		}
	}

	Bool bActionOccured = TRUE;

	// Check to see if the condition has been met
	if(m_uActionCondition != None)
	{
		float fRawValue = 1.0f;

		if(pMessage->ImportData("RAWDATA", fRawValue))
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

	if(bActionOccured)
	{
#ifdef ENABLE_INPUTACTION_TRACE
		TLDebug_Print("Action triggered");
		TString straction;
		m_refActionID.GetString(straction);
		TLDebug_Print(straction);
#endif
		// Send out a new message specifying the action ID to say this action has happened.
		TPtr<TLMessaging::TMessage> pNewMessage = new TLMessaging::TMessage("Input");
		if ( pNewMessage.IsValid() )
		{
			pNewMessage->AddChannelID("ACTION");
			pNewMessage->Write(m_refActionID);			// The action performed

			TPtr<TBinaryTree> pChild = pMessage->GetChild("RAWDATA");

			// Copy the raw data value to the new message if it exists
			if(pChild.IsValid())
				pNewMessage->AddChild(pChild);

			PublishMessage(pNewMessage);
		}
	}
}



TAction_Gesture::TAction_Gesture(TRef refActionID) :
  TAction(refActionID)
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

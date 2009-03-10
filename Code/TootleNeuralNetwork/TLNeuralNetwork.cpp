#include "TLNeuralNetwork.h"


using namespace TLNeuralNetwork;


TNeuron::TNeuron(void)
{
}

void	TNeuron::Update()
{
	// Sum all of the inputs.  
	float fSum = 0.0f;
	for(u32 uIndex = 0; uIndex < m_fInputs.GetSize(); uIndex++)
	{
		fSum += m_fInputs.ElementAt(uIndex);
	}

	fSum /= m_fInputs.GetSize();
	
	//If greater than the threashold then perform activation

	if(fSum > m_fThreshold)
	{
		// Perform action by sending message to allsubscribers
		TLMessaging::TMessage Message("PULSE");

		// TODO: Cater for prohibitive neurons here
		//Message.SetSenderRef(GetNeuronID());
		Message.Write(1.0f);
		PublishMessage(Message);
	}

	/*
	// Reduce/clear the input data values
	for(u32 uIndex = 0; uIndex < m_fInputs.GetSize(); uIndex++)
	{
		m_fInputs.ElementAt(uIndex) = 0.0f;
	}
	*/

	// Remove the inputs - not needed anymore
	m_fInputs.Empty();
}

/*
*/
void TNeuron::ProcessMessage(TLMessaging::TMessage& Message)
{
	// Check the message coming in and if it is a pulse then add the pulse to our inputs
	if(Message.GetMessageRef() == "PULSE")
	{
		float fInput = 0.0f;

		if(Message.Read(fInput))
			m_fInputs.Add(fInput);
	}
}


Bool	TNeuron::Activate()
{
	return FALSE;
}

/*
	Neural network class
*/
TNeuralNetwork::TNeuralNetwork(TRef refNeuralNetworkID) : 
	m_refNeuralNetworkID(refNeuralNetworkID)
{
}

TNeuralNetwork::~TNeuralNetwork(void)
{
	RemoveAllNeurons();
}

/*
	Removes all neurons from the neural netowrk and clears the input buffer
*/
void TNeuralNetwork::RemoveAllNeurons()
{
	// Remove all of the input neurons first (these will be in the main neuron list as well)
	for(u32 uIndex = 0; uIndex < m_InputNeurons.GetSize(); uIndex++)
		m_InputNeurons.ElementAt(uIndex) = NULL;
	m_InputNeurons.Empty();

	// Clean up the input buffer
	m_fInputBuffer.Empty();

	// Now remove all of the neurons themselves
	for(u32 uIndex = 0; uIndex < m_Neurons.GetSize(); uIndex++)
		m_Neurons.ElementAt(uIndex) = NULL;

	m_Neurons.Empty();
}



void TNeuralNetwork::Update()
{
	for(u32 uIndex = 0; uIndex < m_Neurons.GetSize(); uIndex++)
	{
		TPtr<TNeuron> pNeuron = m_Neurons.ElementAt(uIndex);

		if(pNeuron.IsValid() && pNeuron->HasInput())
			pNeuron->Update();
	}
}


/*
	Adds a new neuron and automatically links it to the specified neuron	
*/
TPtr<TNeuron> TNeuralNetwork::AddInputNeuron()
{
	TPtr<TNeuron> pNeuron = AddNeuron();
	
	if(pNeuron.IsValid())
	{
		// add a new data member to the input buffer
		m_fInputBuffer.Add(0.0f);

		// Add to the list of input neurons
		m_InputNeurons.Add(pNeuron);
	}

	return pNeuron;
}

/*
	Adds n number of input neurons and returns them in a list
*/
Bool TNeuralNetwork::AddInputNeurons(u32 uNumberOfNeurons, TPtrArray<TNeuron>& NeuronList)
{
	for(u32 uIndex = 0; uIndex < uNumberOfNeurons; uIndex++)
	{
		TPtr<TNeuron> pNeuron = AddInputNeuron();

		// Failed to add a neuron??
		if(!pNeuron.IsValid())
			return FALSE;

		NeuronList.Add(pNeuron);
	}

	// All done
	return TRUE;
}


/*
	Adds a new neuron and automatically links it to the specified neuron	
*/
TPtr<TNeuron> TNeuralNetwork::AddNeuron(TPtr<TNeuron> pNeuronTo)
{
	TPtr<TNeuron> pNeuron = new TNeuron();

	if(pNeuron.IsValid())
	{
		m_Neurons.Add(pNeuron);

		// Connect the neurons with an axon
		if(pNeuronTo.IsValid())
			Connect(pNeuron, pNeuronTo);
	}

	return pNeuron;
}

/*
	Adds a new list of neurons and automatically links them to the specified neuron
*/
Bool TNeuralNetwork::AddNeurons(u32 uNumberOfNeurons, TPtrArray<TNeuron>& NeuronList, TPtr<TNeuron> pNeuronTo)
{
	for(u32 uIndex = 0; uIndex < uNumberOfNeurons; uIndex++)
	{
		TPtr<TNeuron> pNeuron = AddNeuron();

		// Failed to add a neuron??
		if(!pNeuron.IsValid())
			return FALSE;

		NeuronList.Add(pNeuron);

		// Connect the neurons with an axon
		if(pNeuronTo.IsValid())
		{
			TPtr<TAxon> pAxon = Connect(pNeuron, pNeuronTo);

			// Failed to connect the neurons??
			if(!pAxon.IsValid())
				return FALSE;
		}
	}

	return TRUE;
}


/*
	Removes a neuron removing all links to and from the neuron
*/
void TNeuralNetwork::RemoveNeuron(TPtr<TNeuron> pNeuron)
{
	// Check for if the neuron is an input neuron
	for(u32 uIndex = 0; uIndex < m_InputNeurons.GetSize(); uIndex++)
	{
		TPtr<TNeuron> pTemp = m_InputNeurons.ElementAt(uIndex);

		if(pTemp == pNeuron)
		{
			// Remove the node fromt he list
			m_InputNeurons.RemoveAt(uIndex);

			// remove the last input buffer value - preserve the order of the buffer by not removing the one at the same index
			m_fInputBuffer.RemoveLast();
			break;
		}
	}

	// Now find the neuron in the main list and remove it
	for(u32 uIndex = 0; uIndex < m_Neurons.GetSize(); uIndex++)
	{
		TPtr<TNeuron> pTemp = m_Neurons.ElementAt(uIndex);

		if(pTemp == pNeuron)
		{
			// Remove the node fromt he list
			m_Neurons.RemoveAt(uIndex);
			break;
		}

	}
}

/*
	Adds a new neuron and connects it to the list of neurons
*/
TPtr<TNeuron> TNeuralNetwork::AddNeuron(TPtrArray<TNeuron>& pNeuronsTo)
{
	TPtr<TNeuron> pNeuron = AddNeuron();

	if(pNeuron.IsValid())
	{
		// Connect the neuron to the specified list of neurons
		for(u32 uIndex = 0; uIndex < pNeuronsTo.GetSize(); uIndex++)
		{
			TPtr<TNeuron> pTemp = pNeuronsTo.ElementAt(uIndex);

			if(pTemp.IsValid())
			{
				Connect(pNeuron, pTemp);
			}
		}
	}

	return pNeuron;
}

/*
	Adds a new list of neurons and connects them to the list of neurons
*/
Bool TNeuralNetwork::AddNeurons(u32 uNumberOfNeurons, TPtrArray<TNeuron>& NeuronList, TPtrArray<TNeuron>& pNeuronsTo)
{
	for(u32 uIndex = 0; uIndex < uNumberOfNeurons; uIndex++)
	{
		TPtr<TNeuron> pNeuron = AddNeuron();

		// Failed to add a neuron??
		if(!pNeuron.IsValid())
			return FALSE;

		NeuronList.Add(pNeuron);

		// Connect the neurons with an axon to all of the neurons in the list
		for(u32 uIndex2 = 0; uIndex2 < pNeuronsTo.GetSize(); uIndex2++)
		{
			TPtr<TNeuron> pNeuronTo = pNeuronsTo.ElementAt(uIndex2);
				
			if(!pNeuronTo.IsValid())
				return FALSE;

			TPtr<TAxon> pAxon = Connect(pNeuron, pNeuronTo);

			// Failed to connect the neurons??
			if(!pAxon.IsValid())
				return FALSE;
		}
	}

	return TRUE;
}

/*
	Connect neuron A to neuron B.  This is done via an axon which stores a weight for when processing input values
	before data is passed onto neuron b
*/
TPtr<TAxon> TNeuralNetwork::Connect(TPtr<TNeuron> pNeuronA, TPtr<TNeuron> pNeuronB)
{
	TPtr<TAxon> pAxon = new TAxon(1.0f);

	if(pAxon.IsValid())
	{
		// Subscribe the axon to neuron A
		pAxon->SubscribeTo(pNeuronA);

		// Subscribe neuron B to the axon
		pNeuronB->SubscribeTo(pAxon);
	}

	return pAxon;
}

/*
	Disconnects the specified neuron from all links to and from the neuron
*/
void TNeuralNetwork::Disconnect(TPtr<TNeuron> pNeuron)
{
}


/*
	Disconnect neuron A from neuron B
*/
void TNeuralNetwork::Disconnect(TPtr<TNeuron> pNeuronA, TPtr<TNeuron> pNeuronB)
{
}


/*
	Gets the axon connection between two neurons
*/
TPtr<TAxon> TNeuralNetwork::GetConnection(TPtr<TNeuron> pNeuronA, TPtr<TNeuron> pNeuronB)
{
	TPtr<TAxon> pAxon = NULL;

	return pAxon;
}


/*
	Sets the weight between two neurons A and B
*/
void TNeuralNetwork::SetConnectionWeight(TPtr<TNeuron> pNeuronA, TPtr<TNeuron> pNeuronB, float fWeight)
{
}


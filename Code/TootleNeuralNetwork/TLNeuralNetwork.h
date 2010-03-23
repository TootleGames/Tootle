#pragma once

#include <TootleCore/TLTypes.h>
#include <TootleCore/TPtrArray.h>
#include <TootleCore/TRelay.h>

namespace TLNeuralNetwork
{
	class TNeuron;
	class TAxon;

	class TNeuralNetwork;
}


/*
	Neuron class
*/
class TLNeuralNetwork::TNeuron : public TLMessaging::TRelay
{
public:
	TNeuron();

	virtual TRefRef		GetSubscriberRef() const		{	static TRef Ref("Neuron");	return Ref;	} 
	void				Update();
	Bool				Activate();

//	TRef				GetNeuronID()	{ return m_refNeuronID; }

	inline void			AddInput(float fInput)			{ m_fInputs.Add(fInput); }
	inline Bool			HasInput()				const	{ return (m_fInputs.GetSize() > 0); }

	inline void			SetThreshold(float fValue)		{ m_fThreshold = fValue; }
	inline float		GetThreshold()			const	{ return m_fThreshold; }

protected:
	virtual void	ProcessMessage(TLMessaging::TMessage& Message);

private:
//	TRef					m_refNeuronID;
	float					m_fThreshold;

	TArray<float>		m_fInputs;
};


/*
	TAxon class - the connection between two neurons
*/
class TLNeuralNetwork::TAxon : public TLMessaging::TRelay
{
public:
	explicit TAxon(float fWeight) :
	  m_fWeight(fWeight)
	  {
	  }

	virtual TRefRef		GetSubscriberRef() const		{	static TRef Ref("Axon");	return Ref;	} 

	// Weight access
	inline float		GetWeight()					const		{ return m_fWeight; }
	inline void		SetWeight(float fWeight)					{ m_fWeight = fWeight; }

protected:
	virtual void	ProcessMessage(TLMessaging::TMessage& Message)
	{
		// If we receive a pulse multiply the pulse by our weighting and send a new message
		if(Message.GetMessageRef() == "PULSE")
		{
			float fInput = 0.0f;

			if(Message.Read(fInput))
			{
				
				// NOTE: This may not want to be done immediately.  If there are any circular links we cold end up 
				// getting stuck in a BIG loop and overflow the stack.  Assuming all Axons end up getting
				// linked to neurons we should be OK as the neurons do not process the data immediately
				TLMessaging::TMessage Message("PULSE");
				
				float fWeightedPulse = fInput * GetWeight();
				Message.Write(fWeightedPulse);

				PublishMessage(Message);
			}
		}
	}

private:
	float					m_fWeight;
};


/*
	TNeural Network implementation
*/
class TLNeuralNetwork::TNeuralNetwork : public TLMessaging::TRelay
{
public:
	explicit TNeuralNetwork(TRef refNeuralNetworkID);
	virtual ~TNeuralNetwork(void);

	virtual const TRefRef		GetSubscriberRef() const	{	return m_refNeuralNetworkID;	}
	void						Update();		

	TPtr<TNeuron>		AddInputNeuron();
	Bool						AddInputNeurons(u32 uNumberOfNeurons, TPtrArray<TNeuron>& NeuronList);


	// Neuron manipulation
	TPtr<TNeuron>		AddNeuron(TPtr<TNeuron> pNeuronTo = NULL);																								// Adds a new neuron and automatically links it to the specified neuron				
	Bool						AddNeurons(u32 uNumberOfNeurons, TPtrArray<TNeuron>& NeuronList, TPtr<TNeuron> pNeuronTo = NULL);		// Adds a new list of neurons and automatically links them to the specified neuron

	void						RemoveNeuron(TPtr<TNeuron> pNeuron);					// Removes a neuron removing all links to and from the neuron

	TPtr<TNeuron>		AddNeuron(TPtrArray<TNeuron>& pNeuronsTo);																								// Adds a new neuron and connects it to the list of neurons
	Bool						AddNeurons(u32 uNumberOfNeurons, TPtrArray<TNeuron>& NeuronList, TPtrArray<TNeuron>& pNeuronsTo);		// Adds a new list of neurons and connects them to the list of neurons

	// Connection manipulation
	TPtr<TAxon>			Connect(TPtr<TNeuron> pNeuronA, TPtr<TNeuron> pNeuronB);			// Connect neuron A to neuron B

	void						Disconnect(TPtr<TNeuron> pNeuron);												// Disconnects the specified neuron from all links to and from the neuron
	void						Disconnect(TPtr<TNeuron> pNeuronA, TPtr<TNeuron> pNeuronB);		// Disconnect neuron A from neuron B

	TPtr<TAxon>			GetConnection(TPtr<TNeuron> pNeuronA, TPtr<TNeuron> pNeuronB);									// Gets the axon connection between two neurons
	void						SetConnectionWeight(TPtr<TNeuron> pNeuronA, TPtr<TNeuron> pNeuronB, float fWeight);		// Sets the weight between two neurons A and B

	TRefRef					GetNeuralNetworkID()					{ return m_refNeuralNetworkID; }

private:

	void						RemoveAllNeurons();

private:
	TRef								m_refNeuralNetworkID;

	TArray<float>					m_fInputBuffer;
	TPtrArray<TNeuron>		m_InputNeurons;

	TPtrArray<TNeuron>		m_Neurons;
};


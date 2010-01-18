/*
 *  TAction_Gesture.cpp
 *  TootleNeuralNetwork
 *
 *  Created by Duane Bradbury on 18/01/2010.
 *  Copyright 2010 Tootle. All rights reserved.
 *
 */

#include "TAction_Gesture.h"


#include <TootleInput/TLInput.h>

using namespace TLInput;

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

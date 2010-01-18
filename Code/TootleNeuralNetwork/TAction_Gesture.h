/*
 *  TAction_Gesture.h
 *  TootleNeuralNetwork
 *
 *  Created by Duane Bradbury on 18/01/2010.
 *  Copyright 2010 Tootle. All rights reserved.
 *
 */

#pragma once

#include <TootleInput/TAction.h>
#include "TLNeuralNetwork.h"

namespace TLInput
{
	class TAction_Gesture;
}


class TLInput::TAction_Gesture : public TLInput::TAction
{
public:
	TAction_Gesture(TRef refActionID);
	
	// Neural Network manipulation
	Bool						AddNeuralNetworkInput(TRef refDeviceID, TRef refSensorID);
	Bool						RemoveNeuralNetworkInput(TRef refDeviceID, TRef refSensorID);
	
	Bool						CreateDefaultButtonNeuralNetwork(TRef refDeviceID, TRef refSensorID);
	
private:
	// Internal Neural Network manipulation
	TPtr<TLNeuralNetwork::TNeuron>				AddNeuralNetworkInput(TPtr<TInputSensor> pSensor);
	
private:
	TPtr<TLNeuralNetwork::TNeuralNetwork>		m_pNeuralNetwork;
};


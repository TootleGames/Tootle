#pragma once

#include <TootleCore/TLTypes.h>
#include <TootleCore/TClassFactory.h>
#include <TootleNeuralNetwork/TLNeuralNetwork.h>

#include "TSensor.h"

namespace TLInput
{
	class TAction;
	class TAction_Gesture;

	typedef enum TActionCondition
	{
		None = 0,
		GreaterThan,
		LessThan,
		GreaterThanEqual,
		LessThanEqual,
	};
}

class TLInput::TAction : public TLMessaging::TRelay
{
public:
	explicit TAction(TRef refActionID);

//		inline Bool	operator<(const TAction&) const { return FALSE; }
//		inline Bool	operator==(const TAction& Action) const { return (Action.GetActionID() == GetActionID()); }

	inline TRefRef		GetActionID()		const	{ return m_refActionID; }

	void				SetCondition(TActionCondition uActionCondition, float fThreshold)
	{
		m_uActionCondition = uActionCondition;
		m_fThreshold = fThreshold; 
	}

	Bool				HasParentAction(TRef& refParentActionID);
	void				AddParentAction(TRef& refParentActionID, Bool bCondition = TRUE);

protected:

	virtual void	ProcessMessage(TPtr<TLMessaging::TMessage>& pMessage);

	// Internal parent action state info
	class TParentActionState
	{
	public:
		inline Bool			operator<(const TParentActionState& Ref) const		{	return TRUE;	}
	public:
		Bool		m_bState;				// Current action state - true if the action has been sent
		Bool		m_bCondition;			// State condition we want to check for
	};

private:
	TRef				m_refActionID;

	TArray<TRef>					m_refParentActions;
	TArray<TParentActionState>		m_bParentActionStates;

	TActionCondition	m_uActionCondition;
	float				m_fThreshold;
};


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
	TPtr<TLNeuralNetwork::TNeuron>						AddNeuralNetworkInput(TPtr<TInputSensor> pSensor);

private:
	TPtr<TLNeuralNetwork::TNeuralNetwork>		m_pNeuralNetwork;
};

#pragma once

#include <TootleCore/TLTypes.h>
#include <TootleCore/TClassFactory.h>

#include "TSensor.h"

namespace TLInput
{
	class TAction;

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
	explicit TAction(TRefRef refActionID);

	FORCEINLINE TRefRef		GetActionID() const								{	return m_refActionID; }

	FORCEINLINE void		SetCondition(TActionCondition uActionCondition, float fThreshold)	{	m_uActionCondition = uActionCondition;	m_fThreshold = fThreshold; 	}

	FORCEINLINE Bool		HasParentAction(TRefRef ParentActionRef) const	{	return m_refParentActions.Exists(ParentActionRef); }
	void					AddParentAction(TRefRef ParentActionRef, Bool bCondition = TRUE);

	FORCEINLINE Bool	operator==(TRefRef ActionRef) const					{	return GetActionID() == ActionRef;	}

protected:
	virtual void	ProcessMessage(TLMessaging::TMessage& Message);
	void			ProcessParentActionMessage(TLMessaging::TMessage& Message);
	void			ProcessSensorMessage(TLMessaging::TMessage& Message);

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
	TArray<TParentActionState>		m_bParentActionStates;	//	gr: merge these two arrays together?

	TActionCondition	m_uActionCondition;
	float				m_fThreshold;
};



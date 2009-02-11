/*------------------------------------------------------
	
	State machine class - shouldn't normally need to overload
	the state machine, just the state modes.
	
-------------------------------------------------------*/
#pragma once
#include "TPtrArray.h"
#include "TRef.h"


class TStateMachine;
class TStateMode;



class TStateMachine
{
public:
	TStateMachine()			{}
	
	void					Update();						//	update current mode
	
	template<class TYPE> 
	TPtr<TYPE>				GetMode(TRefRef ModeRef)		{	return m_Modes.FindPtr( ModeRef );	}
	TPtr<TStateMode>		GetMode(TRefRef ModeRef)		{	return m_Modes.FindPtr( ModeRef );	}
	
	inline Bool				HasMode(TRefRef ModeRef)		{	return (GetMode(ModeRef).IsValid()); }
	
	template<class TYPE> 
	TPtr<TYPE>				GetCurrentMode()				{	return m_pCurrentMode;	}
	TPtr<TStateMode>		GetCurrentMode()				{	return m_pCurrentMode;	}
	TRef					GetCurrentModeRef() const;		
	
	template<class TYPE>
	TRef					AddMode(TRefRef ModeRef)		{	TPtr<TStateMode> pMode = new TYPE;	return AddMode( ModeRef, pMode );	}
	template<class TYPE>
	TRef					AddMode()						{	TPtr<TStateMode> pMode = new TYPE;	return AddMode( GetUnusedModeRef(), pMode );	}
	TRef					AddMode(TRefRef ModeRef,TPtr<TStateMode>& pMode);	//	add mode to list of modes - returns new mode's ref. Invalid if not created

	void					Debug_ModeLog(const TString& StateMachineName="TStateMachine") const;			//	debugprint the mode log				

private:
	TRef					GetUnusedModeRef() const;		//	generate a ref that isn't currently used
	Bool					SetMode(TRefRef NextModeRef);	//	change current mode

protected:
	TPtrArray<TStateMode>	m_Modes;
	TPtr<TStateMode>		m_pCurrentMode;
	TArray<TRef>			m_Debug_ModeLog;								//	keep a log of what mode's we've been in for debugging
};




class TStateMode
{
	friend class TStateMachine;
public:
	TStateMode() : m_pStateMachine ( NULL )						{	}
	virtual ~TStateMode()										{	}
	
	TRefRef					GetModeRef() const					{	return m_ModeRef;	}
	inline Bool				operator==(TRefRef ModeRef) const	{	return GetModeRef() == ModeRef;	}

protected:
	Bool					Init(TRefRef ModeRef,TStateMachine* pStateMachine);

	virtual Bool			OnBegin(TRefRef PreviousMode)		{	return GetModeRef().IsValid() ? TRUE : FALSE;	}	//	return FALSE to stop going into this mode
	virtual TRef			Update()							{	return TRef();	}	//	update current mode - return specifies the mode to go onto - returning TRef() will just keep in the current mode
	virtual void			OnEnd(TRefRef NextMode)				{	}	

	template<class TYPE> 
	TYPE*					GetStateMachine()					{	return static_cast<TYPE*>( m_pStateMachine );	}
	TStateMachine*			GetStateMachine()					{	return m_pStateMachine;	}

protected:
	TRef					m_ModeRef;
	TStateMachine*			m_pStateMachine;					//	gr: should be a TPtr but too hard to get one... can do when smart pointers are intrusive
};






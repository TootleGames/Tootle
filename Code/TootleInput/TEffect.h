#pragma once

#include <TootleCore/TRelay.h>

namespace TLInput
{
	class TInputEffect;
	
	class TInputEffect_ForceFeedback;
}

class TLInput::TInputEffect : public TLMessaging::TPublisherSubscriber
{
public:
	TInputEffect(TRefRef InstanceRef, TRefRef TypeRef) :
		m_InstanceRef(InstanceRef),
		m_TypeRef(TypeRef)
	{}
	virtual ~TInputEffect()			{}

protected:
	virtual SyncBool				Initialise(TLMessaging::TMessage& Message)			{ return SyncTrue; }
	virtual SyncBool				Update(float fTimestep)								{ return SyncTrue; }
	virtual SyncBool				Shutdown()											{ return SyncTrue; }
	
	virtual TRefRef					GetSubscriberRef() const							{	return m_InstanceRef;	}
	virtual void					ProcessMessage(TLMessaging::TMessage& Message)		{}

	
private:
	TRef		m_InstanceRef;
	TRef		m_TypeRef;
};


// Force feedback (rumble) effect - will produce a force feedback response
class TLInput::TInputEffect_ForceFeedback : public TLInput::TInputEffect
{
public:
	TInputEffect_ForceFeedback(TRefRef InstanceRef) :
		TInputEffect(InstanceRef, "ForceFeedback")
	{}
	
	virtual ~TInputEffect_ForceFeedback()			{ OnStopVibration(); }

		
protected:
	virtual SyncBool				Initialise(TLMessaging::TMessage& Message);
	virtual SyncBool					Update(float fTimestep);
	virtual SyncBool				Shutdown();
	
private:
	void				OnStartVibration();
	void				OnUpdateVibration();
	void				OnStopVibration();
		
private:	
	float m_fIntensity;			// Intensity of the effect
	float m_fDuration;			// Length of time the effect occurs for
};
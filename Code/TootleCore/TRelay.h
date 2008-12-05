
#pragma once

#include "TSubscriber.h"


namespace TLMessaging
{
	class TRelay;
};

/*
	Message relay class - is both a publisher and subscriber and simply relays messages onto subscribers
*/
class TLMessaging::TRelay : public TPublisher, public TSubscriber
{
protected:
	virtual void				ProcessMessage(TPtr<TLMessaging::TMessage>& pMessage)
	{
		// Relay message to all subscribers
		PublishMessage(pMessage);
	}
};

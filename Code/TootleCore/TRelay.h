
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
	virtual void				ProcessMessage(TLMessaging::TMessage& Message)
	{
		// Relay message to all subscribers
		PublishMessage(Message);
	}
};

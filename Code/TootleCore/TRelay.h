
#pragma once

#include "TSubscriber.h"


namespace TLMessaging
{
	class TPublisherSubscriber;
	class TRelay;
};

/*
	Publisher and subscriber class in one
*/
class TLMessaging::TPublisherSubscriber : public TPublisher, public TSubscriber
{
};


/*
	Message relay class - is both a publisher and subscriber and simply relays messages onto subscribers
*/
class TLMessaging::TRelay : public TPublisherSubscriber
{
protected:
	virtual void				ProcessMessage(TLMessaging::TMessage& Message)
	{
		// Relay message to all subscribers
		PublishMessage(Message);
	}
};

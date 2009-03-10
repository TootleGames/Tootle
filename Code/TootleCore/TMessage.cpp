
#include "TMessage.h"
#include "TLTime.h"

using namespace TLMessaging;



/*
	Adds a new timestamp to the message
*/
TPtr<TBinaryTree>& TMessage::AddTimeStamp(TRefRef TimeStampRef)			
{
	TLTime::TTimestamp tTimeStamp(TRUE);

	return AddChildAndData(TimeStampRef, tTimeStamp);
}

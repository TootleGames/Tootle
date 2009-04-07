
#include "TMessage.h"
#include "TLTime.h"

using namespace TLMessaging;



/*
	Adds a new timestamp to the message
*/
void TMessage::AddTimeStamp(TRefRef TimeStampRef)			
{
	TLTime::TTimestamp tTimeStamp(TRUE);

	ExportData(TimeStampRef, tTimeStamp);
}

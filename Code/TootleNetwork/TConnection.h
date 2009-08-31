/*------------------------------------------------------
	
	Network connection type. This can be thought of as a socket 
	which whatever function puts data into or through

-------------------------------------------------------*/
#pragma once
#include <TootleCore/TBinary.h>


namespace TLNetwork
{
	class TConnection;
}



class TLNetwork::TConnection
{
public:
	TConnection()			{	}
	virtual ~TConnection()	{	Shutdown();	}

	virtual SyncBool		Initialise(TRef& ErrorRef)					{	return SyncTrue;	}	//	async function to initialise/test connection
	virtual SyncBool		Shutdown()									{	return SyncTrue;	}	//	async function to close connection

	virtual SyncBool		GetData(const TString& Url,TBinary& Data,TRef& ErrorRef)	{	return SyncFalse;	}
};


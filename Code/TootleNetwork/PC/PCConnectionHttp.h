/*------------------------------------------------------
	
	Network connection type. This can be thought of as a socket 
	which whatever function puts data into or through

-------------------------------------------------------*/
#pragma once
#include "../TConnection.h"


//	link to lib curl on the pc
#pragma comment(lib,"../../../Tootle/Code/Lib/libcurl.lib")
//#pragma comment(lib,"../../../Tootle/Code/Lib/curllib.lib")
#pragma comment( lib, "ws2_32.lib" )
#pragma comment( lib, "wldap32.lib" )


typedef void CURL;

namespace TLNetwork
{
	namespace Platform
	{
		class TConnectionHttp;
	}
}



class TLNetwork::Platform::TConnectionHttp : public TLNetwork::TConnection
{
public:
	TConnectionHttp();

	virtual SyncBool	Initialise(TRef& ErrorRef);
	virtual SyncBool	Shutdown();

	virtual SyncBool	GetData(const TString& Url,TBinary& Data,TRef& ErrorRef);

	Bool				OnRecieveData(const u8* pData,u32 Size);	//	recieved some data from curl

private:
	TBinary*			m_pRecieveData;	//	current binary data to recieve into
	CURL*				m_pCurl;		//	curl handler
};


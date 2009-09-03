/*------------------------------------------------------
	
	Network connection type. This can be thought of as a socket 
	which whatever function puts data into or through

-------------------------------------------------------*/
#pragma once
#include "../TConnection.h"
#include <TootleCore/TKeyArray.h>


//	link to lib curl on the pc
#pragma comment(lib,"../../../Tootle/Code/Lib/libcurl.lib")
//#pragma comment(lib,"../../../Tootle/Code/Lib/curllib.lib")
#pragma comment( lib, "ws2_32.lib" )
#pragma comment( lib, "wldap32.lib" )


typedef void CURL;
typedef void CURLM;


namespace TLNetwork
{
	namespace Platform
	{
		class TConnectionHttp;
		class TCurlTask;
	}
}


//-------------------------------------------------------------
//	Task which also contains a curl handle for mutli (thread?) handles
//-------------------------------------------------------------
class TLNetwork::Platform::TCurlTask : public TLNetwork::TTask
{
public:
	TCurlTask(const TTypedRef& TaskRef,TBinaryTree& TaskData) :
		TTask			( TaskRef, TaskData ),
		m_pHandle		( NULL ),
		m_pMultiHandle	( NULL )
	{
	}
	~TCurlTask();

	CURL*			InitHandle();		//	create curl handle

public:
	CURL*			m_pHandle;
	CURLM*			m_pMultiHandle;
};



class TLNetwork::Platform::TConnectionHttp : public TLNetwork::TConnection
{
public:
	TConnectionHttp();

	virtual SyncBool	Initialise(TRef& ErrorRef);
	virtual SyncBool	Shutdown();

protected:
	virtual TPtr<TTask>	CreateTask(const TTypedRef& TaskRef,TBinaryTree& TaskData)	{	return new TCurlTask( TaskRef, TaskData );	}

	virtual void		StartDownloadTask(TTask& Task);				//	start a simple GET task
	virtual void		StartUploadTask(TTask& Task);				//	start a POST task
};


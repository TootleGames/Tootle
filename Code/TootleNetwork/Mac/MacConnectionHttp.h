/*------------------------------------------------------
	
	Network connection type. This can be thought of as a socket 
	which whatever function puts data into or through

-------------------------------------------------------*/
#pragma once
#include "../TConnection.h"
#import <Foundation/NSUrl.h>
#import <Foundation/NSUrlConnection.h>
#import <Foundation/NSURLRequest.h>
#import <Foundation/NSString.h>
#import <TootleCore/TKeyArray.h>


namespace TLNetwork
{
	namespace Platform
	{
		class TConnectionHttp;
		class TNSUrlConnectionTask;
	}
}




//-------------------------------------------------------------
//	Task which also contains a curl handle for mutli (thread?) handles
//-------------------------------------------------------------
class TLNetwork::Platform::TNSUrlConnectionTask : public TLNetwork::TTask
{
public:
	TNSUrlConnectionTask(const TTypedRef& TaskRef,TBinaryTree& TaskData) :
		TTask			( TaskRef, TaskData ),
		m_pUrlString	( NULL ),
		m_pUrl			( NULL ),
		m_pUrlRequest	( NULL ),
		m_pConnection	( NULL )
	{
	}
	~TNSUrlConnectionTask();

	FORCEINLINE Bool	operator==(TRefRef TaskRef) const				{	return GetTaskRef() == TaskRef;	}
	FORCEINLINE Bool	operator==(NSURLConnection* pConnection) const	{	return m_pConnection == pConnection;	}

public:
	NSString*				m_pUrlString;
	NSURL*					m_pUrl;
	NSMutableURLRequest*	m_pUrlRequest;
	NSURLConnection*		m_pConnection;
};




@interface TConnectionDelegate : NSObject 
{
	TLNetwork::Platform::TConnectionHttp* m_pConnection;
}

@property TLNetwork::Platform::TConnectionHttp* m_pConnection;

- (id) Initialise:(TLNetwork::Platform::TConnectionHttp*)pConnection;

@end








class TLNetwork::Platform::TConnectionHttp : public TLNetwork::TConnection
{
public:
	TConnectionHttp();

	virtual SyncBool		Initialise(TRef& ErrorRef);
	virtual SyncBool		Shutdown();

	TTask*					GetTask(NSURLConnection* pConnection);		//	get a task from a connection

protected:
	virtual TPtr<TTask>		CreateTask(const TTypedRef& TaskRef,TBinaryTree& TaskData)	{	return new TNSUrlConnectionTask( TaskRef, TaskData );	}	//	allocate connection-specific task type
	virtual void			StartDownloadTask(TTask& Task);				//	start a simple GET task
	virtual void			StartUploadTask(TTask& Task);				//	start a POST task

private:
	TConnectionDelegate*	m_pDelegate;
};


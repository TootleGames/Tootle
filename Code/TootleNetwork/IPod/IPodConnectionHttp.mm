#include "IPodConnectionHttp.h"

namespace TLNetwork
{
	namespace Platform
	{
	}
}



TLNetwork::Platform::TConnectionHttp::TConnectionHttp() :
	m_pDelegate				( NULL )
{
}


//---------------------------------------------------------
//	initialise connection delegate
//---------------------------------------------------------
SyncBool TLNetwork::Platform::TConnectionHttp::Initialise(TRef& ErrorRef)
{
	//	create connection delegate
	m_pDelegate = [[TConnectionDelegate alloc] Initialise:this ];
	if ( !m_pDelegate )
		return SyncFalse;

	return SyncTrue;
}



//---------------------------------------------------------
//	start a task.
//---------------------------------------------------------
void TLNetwork::Platform::TConnectionHttp::StartTask(TTask& Task)
{
	//	start type of task
	if ( Task.GetTaskType() == "Get" )
	{
		StartGetTask( Task );
		return;
	}

	//	unknown type
	Task.SetStatusFailed("NoType");
}



//---------------------------------------------------------
//	start a GET task. Returns error ref. 
//---------------------------------------------------------
void TLNetwork::Platform::TConnectionHttp::StartGetTask(TTask& Task)
{
	if ( !m_pDelegate )
	{
		Task.SetStatusFailed("NoInit");
		return;
	}

	//	get the url string
	TTempString Url;
	if ( !Task.GetData().ImportDataString("url", Url ) )
	{
		Task.SetStatusFailed("NoUrl");
		return;
	}

	//	create request
//	NSURL* pUrl = [[NSURL alloc] initWithString:@"http://www.google.com/"];
	NSURL* pUrl = [[NSURL alloc] initWithUTF8String:Url.GetData()];

	NSURLRequest *pRequest = [NSURLRequest	requestWithURL:pURL
											cachePolicy:NSURLRequestReloadIgnoringLocalCacheData 
											timeoutInterval:60];
	if ( !pRequest )
	{
		[pUrl release];
		Task.SetStatusFailed("NoRequest");
		return;
	}

	//	create connection - 
	//	gr: DO NOT START IT YET!
	//	to avoid race condition problems, we wait until we've created a connection <-> task link
	//	before starting in case we recieve data before they;re linked and would have no where to 
	//	put the downloaded data	
	NSURLConnection *pConnection = [[NSURLConnection alloc]	initWithRequest:pRequest 
															delegate:m_pDelegate
															startImmediately:NO];

	[pUrl release];
	[pRequest release];
	
	//	failed to create/init connection with request
	if ( !pConnection ) 
	{
		Task.SetStatusFailed("NoConnection");
		return;
	}

	//	created connection, associate it with a task
	m_ConnectionTasks.Add( pConnection, Task.GetTaskRef() );
	
	//	now start connection
	[pConnection start];

	//	now we just wait for it to do it's thing!...	
}


//---------------------------------------------------------
//	
//---------------------------------------------------------
SyncBool TLNetwork::Platform::TConnectionHttp::Shutdown()
{
	if ( m_pDelegate )
	{
		[m_pDelegate release];
		m_pDelegate = NULL;
	}
	
	return TConnection::Shutdown();
}

//---------------------------------------------------------
//	get a task from a connection
//---------------------------------------------------------
TPtr<TLNetwork::TTask>& TLNetwork::Platform::TConnectionHttp::GetTask(NSURLConnection* pConnection)
{
	TRef* pTaskRef = m_ConnectionTasks.Find( pConnection );
	if ( !pTaskRef )
		return TLPtr::GetNullPtr<TLNetwork::TTask>();

	TPtr<TLNetwork::TTask>& pTask = TConnection::GetTask( *pTaskRef );
	return pTask;
}

//---------------------------------------------------------
//	clean up task
//---------------------------------------------------------
void TLNetwork::Platform::TConnectionHttp::OnTaskRemoved(TRef TaskRef)
{
	//	find connection for this task
	const NSURLConnection** ppConnection = m_ConnectionTasks.FindKey( TaskRef );
	
	//	didnt have a connection entry
	if ( !ppConnection )
		return;

	//	get index and proper key
	s32 Index = m_ConnectionTasks.FindIndex( *ppConnection );
	if ( Index == -1 )
	{
		TLDebug_Break("Index expected");
		return;
	}
	
	//	get proper key
	NSURLConnection*& pConnection = m_ConnectionTasks.GetKeyAt( Index );
	[pConnection release];
	pConnection = NULL;
	
	//	remove entry
	m_ConnectionTasks.RemoveAt( Index );
}



@implementation TConnectionDelegate

@synthesize m_pConnection;


/* This method initiates the load request. The connection is asynchronous, 
 and we implement a set of delegate methods that act as callbacks during 
 the load. */

- (id) Initialise:(TLNetwork::Platform::TConnectionHttp*)pConnection;
{
	if ( self = [super init] )
	{
		//	store connection pointer
		m_pConnection = pConnection;
	}
	
	return self;
}



- (void) connection:(NSURLConnection *)connection didReceiveResponse:(NSURLResponse *)response
{
	/* This method is called when the server has determined that it has
	enough information to create the NSURLResponse. It can be called
	multiple times, for example in the case of a redirect, so each time
	we reset the data. */
	TLNetwork::TTask* pTask = m_pConnection->GetTask( connection );
	if ( !pTask )
	{
		TLDebug_Break("Missing task on connection recv start");
		return;
	}

	pTask->ResetData();
}


- (void) connection:(NSURLConnection *)connection didReceiveData:(NSData *)data
{
	TLNetwork::TTask* pTask = m_pConnection->GetTask( connection );
	if ( !pTask )
	{
		TLDebug_Break("Missing task on connection recv");
		return;
	}

	const u8* pBytes = (const u8*)data.bytes;
	u32 Size = data.length;
	pTask->RecieveData( pBytes, Size );
}


- (void) connection:(NSURLConnection *)connection didFailWithError:(NSError *)error
{
	TLNetwork::TTask* pTask = m_pConnection->GetTask( connection );
	if ( !pTask )
	{
		TLDebug_Break("Missing task on connection error");
		return;
	}

	//	fail task
	pTask->SetStatusFailed("Fail");
}


- (NSCachedURLResponse *) connection:(NSURLConnection *)connection 
				   willCacheResponse:(NSCachedURLResponse *)cachedResponse
{
	// this application does not use a NSURLCache disk or memory cache
    return nil;
}


- (void) connectionDidFinishLoading:(NSURLConnection *)connection
{
	TLNetwork::TTask* pTask = m_pConnection->GetTask( connection );
	if ( !pTask )
	{
		TLDebug_Break("Missing task on finished connection");
		return;
	}
		
	pTask->SetStatusSuccess();
}


@end

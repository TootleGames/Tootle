#include "IPodConnectionHttp.h"

namespace TLNetwork
{
	namespace Platform
	{
	}
}




TLNetwork::Platform::TNSUrlConnectionTask::~TNSUrlConnectionTask()
{
	if ( m_pConnection )
		[m_pConnection release];
/*
	if ( m_pUrlRequest )
		[m_pUrlRequest release];
		
	if ( m_pUrl )
		[m_pUrl release];

	if ( m_pUrlString )
		[m_pUrlString release];
 */
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
//	start a download task
//---------------------------------------------------------
void TLNetwork::Platform::TConnectionHttp::StartDownloadTask(TTask& Task)
{
	TNSUrlConnectionTask& ConnectionTask = static_cast<TNSUrlConnectionTask&>( Task );
	if ( !m_pDelegate )
	{
		Task.SetStatusFailed("NoInit");
		return;
	}

	//	get the url string
	TTempString UrlString;
	if ( !Task.GetData().ImportDataString("url", UrlString ) )
	{
		Task.SetStatusFailed("NoUrl");
		return;
	}

	//	create request
	ConnectionTask.m_pUrlString = [[NSString alloc] initWithUTF8String:UrlString.GetData() ];
	ConnectionTask.m_pUrl = [[NSURL alloc] initWithString:ConnectionTask.m_pUrlString ];
	ConnectionTask.m_pUrlRequest = [NSURLRequest	requestWithURL:ConnectionTask.m_pUrl
								cachePolicy:NSURLRequestReloadIgnoringLocalCacheData 
								timeoutInterval:60];
	if ( !ConnectionTask.m_pUrlRequest )
	{
		Task.SetStatusFailed("NoRequest");
		return;
	}

	//	create connection - 
	//	gr: DO NOT START IT YET!
	//	to avoid race condition problems, we wait until we've created a connection <-> task link
	//	before starting in case we recieve data before they;re linked and would have no where to 
	//	put the downloaded data	
	ConnectionTask.m_pConnection = [[NSURLConnection alloc]	initWithRequest:ConnectionTask.m_pUrlRequest 
												delegate:m_pDelegate
												startImmediately:YES];
	
	//	failed to create/init connection with request
	if ( !ConnectionTask.m_pConnection ) 
	{
		Task.SetStatusFailed("NoConnection");
		return;
	}

	//	now start connection
	[ConnectionTask.m_pConnection start];

	//	now we just wait for it to do it's thing!...	
}


//---------------------------------------------------------
//	start a upload task
//---------------------------------------------------------
void TLNetwork::Platform::TConnectionHttp::StartUploadTask(TTask& Task)
{
	Task.SetStatusFailed("Todo");
	TLDebug_Break("Todo");
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
TLNetwork::TTask* TLNetwork::Platform::TConnectionHttp::GetTask(NSURLConnection* pConnection)
{
	TLNetwork::TTask* pTask = NULL;
	for ( u32 i=0;	i<m_Tasks.GetSize();	i++ )
	{
		TNSUrlConnectionTask* pNsTask = m_Tasks[i].GetObjectPointer<TNSUrlConnectionTask>();
		if ( *pNsTask == pConnection )
		{
			pTask = pNsTask;
			break;
		}
	}
	
	return pTask;
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

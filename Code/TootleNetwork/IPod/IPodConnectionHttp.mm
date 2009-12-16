#include "IPodConnectionHttp.h"

#import <TootleCore/IPod/IPodString.h>

namespace TLNetwork
{
	namespace Platform
	{
	}
}




TLNetwork::Platform::TNSUrlConnectionTask::~TNSUrlConnectionTask()
{
/*
	if ( m_pConnection )
		[m_pConnection release];

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
	ConnectionTask.m_pUrlString = TLString::ConvertToUnicharString(UrlString);
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
	ConnectionTask.m_pUrlString = TLString::ConvertToUnicharString(UrlString);
	ConnectionTask.m_pUrl = [[NSURL alloc] initWithString:ConnectionTask.m_pUrlString ];
	ConnectionTask.m_pUrlRequest = [NSMutableURLRequest	requestWithURL:ConnectionTask.m_pUrl];

    //	POST, not default GET
    //	http://www.w3.org/Protocols/rfc1341/7_2_Multipart.html
    [ConnectionTask.m_pUrlRequest setHTTPMethod:@"POST"];
    
    //	post boundry is a VERY UNIQUE identifier to split up the form parts. this must never ever been in the body!
    NSString *stringBoundary = [NSString stringWithString:@"0xKhTmLbOuNdArY"];
    NSString *contentType = [NSString stringWithFormat:@"multipart/form-data; boundary=%@",stringBoundary];
    [ConnectionTask.m_pUrlRequest addValue:contentType forHTTPHeaderField: @"Content-Type"];
 

    //	setup the post body
    NSMutableData *postBody = [NSMutableData data];
	
	//	add form data for each bit of upload data
	TPtr<TBinaryTree>& pUploadData = ConnectionTask.GetData().GetChild("Upload");
	if ( !pUploadData )
	{
		Task.SetStatusFailed("NoData");
		return;
	}
	
	TPtrArray<TBinaryTree>& UploadDatas = pUploadData->GetChildren();
	for ( u32 i=0;	i<UploadDatas.GetSize();	i++ )
	{
		TBinaryTree& UploadData = *UploadDatas[i];
		
		//	the data's ref represents the field name
		TTempString DataRefString;
		UploadData.GetDataRef().GetUrlString( DataRefString );
		
		//	append the data type if we have it. Parenthesis are safe to use as theyre not in the ref alphabet, and are safe url-enc characters
		if ( UploadData.GetDataTypeHint().IsValid() )
		{
			//	this produces "MYINT(U16)=00FF" instead of just "MYINT=00FF"
			DataRefString.Append('(');
			UploadData.GetDataTypeHint().GetUrlString( DataRefString );
			DataRefString.Append(')');
		}
		
		//	convert the raw data to a hex string(UTF8 compatible)
		//	we're doing this to match the IPod code which I can't see a way to send raw data
		//	without converting it to a string
		TString DataString;
		UploadData.GetData().GetDataHexString( DataString, TRUE, TRUE );
		
		TString NameString = "Content-Disposition: form-data; name=\"";
		NameString.Append( DataRefString );
		NameString.Append( "\"\r\n\r\n" );
		
		
		NSString* pNameString = TLString::ConvertToUnicharString(NameString);
		NSString* pDataString = TLString::ConvertToUnicharString(DataString);
		
		[postBody appendData:[[NSString stringWithFormat:@"\r\n\r\n--%@\r\n",stringBoundary] dataUsingEncoding:NSUTF8StringEncoding]];
		
		[postBody appendData:[pNameString dataUsingEncoding:NSUTF8StringEncoding]];
		[postBody appendData:[pDataString dataUsingEncoding:NSUTF8StringEncoding]];
		
		[pNameString release];
		[pDataString release];
	}
 
	/*
    [postBody appendData:[[NSString stringWithFormat:@"\r\n--%@\r\n",stringBoundary] dataUsingEncoding:NSUTF8StringEncoding]];
    [postBody appendData:[[NSString stringWithString:@"Content-Disposition: form-data; name=\"description\"\r\n\r\n"] dataUsingEncoding:NSUTF8StringEncoding]];
    [postBody appendData:[[NSString stringWithString:@"some description"] dataUsingEncoding:NSUTF8StringEncoding]];
 
 
    [postBody appendData:[[NSString stringWithFormat:@"\r\n--%@\r\n",stringBoundary] dataUsingEncoding:NSUTF8StringEncoding]];
    [postBody appendData:[[NSString stringWithString:@"Content-Disposition: form-data; name=\"username\"\r\n\r\n"] dataUsingEncoding:NSUTF8StringEncoding]];
    [postBody appendData:[[NSString stringWithString:@"some username"] dataUsingEncoding:NSUTF8StringEncoding]];
 
 
    [postBody appendData:[[NSString stringWithFormat:@"\r\n--%@\r\n",stringBoundary] dataUsingEncoding:NSUTF8StringEncoding]];
    [postBody appendData:[[NSString stringWithString:@"Content-Disposition: form-data; name=\"password\"\r\n\r\n"] dataUsingEncoding:NSUTF8StringEncoding]];
    [postBody appendData:[[NSString stringWithString:@"some password"] dataUsingEncoding:NSUTF8StringEncoding]];
 
 /*
    [postBody appendData:[[NSString stringWithFormat:@"\r\n--%@\r\n",stringBoundary] dataUsingEncoding:NSUTF8StringEncoding]];
    [postBody appendData:[[NSString stringWithFormat:@"Content-Disposition: form-data; name=\"videoFile\"; filename=\"@\"\r\n", [[edFileName stringValue] lastPathComponent]] dataUsingEncoding:NSUTF8StringEncoding]];
    [postBody appendData:[[NSString stringWithString:@"Content-Type: application/octet-stream\r\n\r\n"] dataUsingEncoding:NSUTF8StringEncoding]];
    [postBody appendData:[[NSString stringWithString:@"Content-Transfer-Encoding: binary/video\r\n\r\n"] dataUsingEncoding:NSUTF8StringEncoding]];

    [postBody appendData:[NSData dataWithContentsOfFile:[edFileName stringValue]]];
*/ 
    [postBody appendData:[[NSString stringWithFormat:@"\r\n--%@--\r\n",stringBoundary] dataUsingEncoding:NSUTF8StringEncoding]];
    [ConnectionTask.m_pUrlRequest setHTTPBody:postBody];
 
 
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

#include "IPodConnectionHttp.h"

namespace TLNetwork
{
	namespace Platform
	{
	}
}



TLNetwork::Platform::TConnectionHttp::TConnectionHttp() :
	TLNetwork::TConnection	( ),
	m_pDelegate				( NULL ),
	m_pRecvData				( NULL )
{
}


//---------------------------------------------------------
//	initialise connection delegate
//---------------------------------------------------------
SyncBool TLNetwork::Platform::TConnectionHttp::Initialise(TRef& ErrorRef)
{
	return SyncTrue;
}


//---------------------------------------------------------
//	
//---------------------------------------------------------
SyncBool TLNetwork::Platform::TConnectionHttp::GetData(const TString& Url,TBinary& Data,TRef& ErrorRef)
{
	m_pRecvData = &Data;
	
	//	create url
	NSURL* pUrl = [[NSURL alloc] initWithString:@"http://www.google.com/"];
	
	//	create connection delegate
	m_pDelegate = [[TConnectionDelegate alloc] initWithURL:pUrl Connection:this ];

	//	delete url
	[pUrl release];

	m_pRecvData = NULL;
	
	return SyncTrue;
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
	
	return SyncTrue;
}





@implementation TConnectionDelegate

@synthesize receivedData;
@synthesize m_pConnection;


/* This method initiates the load request. The connection is asynchronous, 
 and we implement a set of delegate methods that act as callbacks during 
 the load. */

- (id) initWithURL:(NSURL *)pURL Connection:(TLNetwork::Platform::TConnectionHttp*)pConnection;
{
	if (self = [super init]) 
	{
		//	store connection pointer
		m_pConnection = pConnection;
		
		/* Create the request. This application does not use a NSURLCache 
		 disk or memory cache, so our cache policy is to satisfy the request
		 by loading the data from its source. */
		
		NSURLRequest *pRequest = [NSURLRequest requestWithURL:pURL
													cachePolicy:NSURLRequestReloadIgnoringLocalCacheData 
												timeoutInterval:60];
		
		/* create the NSMutableData instance that will hold the received data */
		receivedData = [[NSMutableData alloc] initWithLength:0];

		/* Create the connection with the request and start loading the
		 data. The connection object is owned both by the creator and the
		 loading system. */
			
		NSURLConnection *connection = [[NSURLConnection alloc] initWithRequest:pRequest 
																	  delegate:self 
															  startImmediately:YES];
		if (connection == nil) 
		{
			//	failed to init with request
		}
	}

	return self;
}


- (void)dealloc
{
	[receivedData release];
	[super dealloc];
}


#pragma mark NSURLConnection delegate methods

- (void) connection:(NSURLConnection *)connection didReceiveResponse:(NSURLResponse *)response
{
    /* This method is called when the server has determined that it has
	 enough information to create the NSURLResponse. It can be called
	 multiple times, for example in the case of a redirect, so each time
	 we reset the data. */
	self.m_pConnection->OnResetData();
    [self.receivedData setLength:0];
}


- (void) connection:(NSURLConnection *)connection didReceiveData:(NSData *)data
{
	const u8* pBytes = (const u8*)data.bytes;
	u32 Size = data.length;
	m_pConnection->OnRecieveData( pBytes, Size );
    
	/* Append the new data to the received data. */
    [self.receivedData appendData:data];
}


- (void) connection:(NSURLConnection *)connection didFailWithError:(NSError *)error
{
	//	todo: set error
	[connection release];
}


- (NSCachedURLResponse *) connection:(NSURLConnection *)connection 
				   willCacheResponse:(NSCachedURLResponse *)cachedResponse
{
	/* this application does not use a NSURLCache disk or memory cache */
    return nil;
}


- (void) connectionDidFinishLoading:(NSURLConnection *)connection
{
	[connection release];
}


@end

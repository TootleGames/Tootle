/*------------------------------------------------------
	
	Network connection type. This can be thought of as a socket 
	which whatever function puts data into or through

-------------------------------------------------------*/
#pragma once
#include "../TConnection.h"
#import <UIKit/UIKit.h>


namespace TLNetwork
{
	namespace Platform
	{
		class TConnectionHttp;
	}
}




@interface TConnectionDelegate : NSObject {
	NSMutableData *receivedData;
}

@property (nonatomic, retain) NSMutableData *receivedData;
- (id) initWithURL:(NSURL *)theURL ;
@end








class TLNetwork::Platform::TConnectionHttp : public TLNetwork::TConnection
{
public:
	TConnectionHttp();

	virtual SyncBool	Initialise(TRef& ErrorRef);
	virtual SyncBool	Shutdown();

	virtual SyncBool	GetData(const TString& Url,TBinary& Data,TRef& ErrorRef);

private:
	TConnectionDelegate*	m_pDelegate;
};


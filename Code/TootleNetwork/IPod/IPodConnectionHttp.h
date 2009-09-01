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




@interface TConnectionDelegate : NSObject 
{
	NSMutableData *receivedData;
	TLNetwork::Platform::TConnectionHttp* m_pConnection;
}

@property (nonatomic, retain) NSMutableData *receivedData;
@property TLNetwork::Platform::TConnectionHttp* m_pConnection;
- (id) initWithURL:(NSURL *)pURL Connection:(TLNetwork::Platform::TConnectionHttp*)pConnection ;
- (void) dealloc;
@end








class TLNetwork::Platform::TConnectionHttp : public TLNetwork::TConnection
{
public:
	TConnectionHttp();

	virtual SyncBool	Initialise(TRef& ErrorRef);
	virtual SyncBool	Shutdown();

	virtual SyncBool	GetData(const TString& Url,TBinary& Data,TRef& ErrorRef);

	void				OnResetData()								{	m_pRecvData->Empty();	}
	void				OnRecieveData(const u8* pData,u32 Size)		{	m_pRecvData->WriteData( pData, Size );		}
	
private:
	TBinary*				m_pRecvData;
	TConnectionDelegate*	m_pDelegate;
};


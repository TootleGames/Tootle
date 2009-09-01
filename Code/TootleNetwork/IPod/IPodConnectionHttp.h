/*------------------------------------------------------
	
	Network connection type. This can be thought of as a socket 
	which whatever function puts data into or through

-------------------------------------------------------*/
#pragma once
#include "../TConnection.h"
#import <UIKit/UIKit.h>
#import <TootleCore/TKeyArray.h>


namespace TLNetwork
{
	namespace Platform
	{
		class TConnectionHttp;
	}
}




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

	virtual SyncBool	Initialise(TRef& ErrorRef);
	virtual SyncBool	Shutdown();

	virtual void		StartTask(TTask& Task);						//	start a task.

	TPtr<TTask>&		GetTask(NSURLConnection* pConnection);		//	get a task from a connection

protected:
	void				StartGetTask(TTask& Task);					//	start a GET task
	virtual void		OnTaskRemoved(TRef TaskRef);				//	clean up task

private:
	TKeyArray<NSURLConnection*,TRef>	m_ConnectionTasks;		//	this associates a connection with a task ref
	TConnectionDelegate*				m_pDelegate;
};


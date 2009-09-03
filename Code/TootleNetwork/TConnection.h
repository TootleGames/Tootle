/*------------------------------------------------------
	
	Network connection type. This can be thought of as a socket 
	which whatever function puts data into or through

-------------------------------------------------------*/
#pragma once
#include <TootleCore/TBinaryTree.h>


namespace TLNetwork
{
	class TConnection;	//	some kind of connection interface
	class TTask;		//	task for the connection
}



//-------------------------------------------------------------
//	task for a network connection to perform and store data to
//-------------------------------------------------------------
class TLNetwork::TTask
{
public:
	TTask(const TTypedRef& TaskRef,TBinaryTree& TaskData);
	virtual ~TTask()	{}

	TRefRef				GetTaskRef() const	{	return m_TaskRef.GetRef();	}
	TRefRef				GetTaskType() const	{	return m_TaskRef.GetTypeRef();	}
	SyncBool			GetStatus() const	{	return m_Status;	}	//	get the status of this task. SyncFalse failed, SyncWait still going, SyncTrue success
	TBinaryTree&		GetData()			{	return m_Data;	}	//	get the status of this task. SyncFalse failed, SyncWait still going, SyncTrue success

	void				SetStatusSuccess()						{	m_Status = SyncTrue;	}	//	set task as succeeded and finished
	void				SetStatusFailed(TRefRef ErrorRef);		//	set task as failed
	void				ResetData()								{	m_Data.GetData().Empty();	}
	Bool				RecieveData(const u8* pData,u32 Size)	{	m_Data.WriteData( pData, Size );	return TRUE;	}

	FORCEINLINE Bool	operator==(TRefRef TaskRef) const		{	return (GetTaskRef() == TaskRef);	}

protected:
	SyncBool		m_Status;			//	current status
	TTypedRef		m_TaskRef;			//	unique task ref and type of task, eg. Get
	TBinaryTree		m_Data;				//	task data, root data should be recieved data. Will have other relevent data, eg, "url", "Error", "Stage" etc
};


//-------------------------------------------------------------
//	interface to a network connection of some kind
//-------------------------------------------------------------
class TLNetwork::TConnection
{
public:
	TConnection()			{	}
	virtual ~TConnection()	{	Shutdown();	}

	virtual SyncBool		Initialise(TRef& ErrorRef)					{	return SyncTrue;	}	//	async function to initialise/test connection
	virtual SyncBool		Shutdown()									{	return SyncTrue;	}	//	async function to close connection

	DEPRECATED TRef			GetData(const TString& Url)					{	return Download( Url );	}
	TRef					Download(const TString& Url);							//	Get data from url. creates a get-data task and return the task ref
	TRef					Upload(const TString& Url,TBinaryTree& UploadData);		//	Upload data to an url, each bit of data is named (the root data is unused). The refs are converted to strings and used as HTTP params, maybe for packets they wont be strings. creates a get-data task and return the task ref

	TBinaryTree*			GetTask(TRefRef TaskRef,SyncBool& TaskStatus);	//	get the current state and data of a task. returns NULL if task doesn't exist. 
	Bool					RemoveTask(TRef TaskRef);					//	delete a task when we're done with it

protected:
	virtual TPtr<TTask>		CreateTask(const TTypedRef& TaskRef,TBinaryTree& TaskData)	{	return new TTask( TaskRef, TaskData );	}	//	allocate connection-specific task type
	virtual void			StartDownloadTask(TTask& Task)=0;			//	start a download task
	virtual void			StartUploadTask(TTask& Task)=0;				//	start an upload task

	TPtr<TTask>&			GetTask(TRefRef TaskRef)					{	return m_Tasks.FindPtr( TaskRef );	}

private:
	virtual void			StartTask(TTask& Task);						//	start a task. Base code sets it to fail instantly

protected:
	TPtrArray<TTask>		m_Tasks;				//	current tasks
};



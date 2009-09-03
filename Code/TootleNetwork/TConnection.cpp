#include "TConnection.h"


namespace TLNetwork
{
	TRefRef	GetNextNetworkTaskRef();	//	get next network task ref

	TRef	g_CurrentNetworkTaskRef;
}


//---------------------------------------------
//	get next network task ref
//---------------------------------------------
TRefRef TLNetwork::GetNextNetworkTaskRef()
{
	g_CurrentNetworkTaskRef.Increment();

	return g_CurrentNetworkTaskRef;
}


TLNetwork::TTask::TTask(const TTypedRef& TaskRef,TBinaryTree& TaskData) :
	m_Status		( SyncWait ),
	m_TaskRef		( TaskRef ),
	m_Data			( "Task" )
{
	//	copy data out of the data provided
	m_Data.ReferenceDataTree( TaskData );
}



//-------------------------------------------------------------
//	set task as failed
//-------------------------------------------------------------
void TLNetwork::TTask::SetStatusFailed(TRefRef ErrorRef)
{
	//	set as failed
	m_Status = SyncFalse;

	//	store error ref
	if ( ErrorRef.IsValid() )
		m_Data.ExportData("Error", ErrorRef );

	//	print out fail
	TTempString Debug_String("Network task ");
	m_TaskRef.GetString( Debug_String );
	Debug_String.Append(" failed: ");
	ErrorRef.GetString( Debug_String );
	TLDebug_Print( Debug_String );
}


//---------------------------------------------------------
//	start a task.
//---------------------------------------------------------
void TLNetwork::TConnection::StartTask(TTask& Task)
{
	//	start type of task
	if ( Task.GetTaskType() == "Download" )
	{
		StartDownloadTask( Task );
		return;
	}

	if ( Task.GetTaskType() == "Upload" )
	{
		StartUploadTask( Task );
		return;
	}

	//	unknown type
	Task.SetStatusFailed("NoType");
}


//-------------------------------------------------------------
//	Get data from url. creates a get-data task and return the task ref
//-------------------------------------------------------------
TRef TLNetwork::TConnection::Download(const TString& Url)
{
	//	new task ref
	TRefRef TaskRef = GetNextNetworkTaskRef();

	//	setup data for the task
	TBinaryTree TaskData( TRef_Invalid );
	TaskData.ExportDataString("url", Url );

	//	create task and add to list
	TPtr<TTask> pTask = CreateTask( TTypedRef( TaskRef, "Download" ), TaskData );
	m_Tasks.Add( pTask );

	//	start the task - note: may block and complete here...
	StartTask( *pTask );

	return pTask->GetTaskRef();
}


//-------------------------------------------------------------
//	Upload data to an url, each bit of data is named (the root data is unused). 
//	The refs are converted to strings and used as HTTP params, maybe for packets they wont be strings. creates a get-data task and return the task ref
//-------------------------------------------------------------
TRef TLNetwork::TConnection::Upload(const TString& Url,TBinaryTree& UploadData)
{
	//	new task ref
	TRefRef TaskRef = GetNextNetworkTaskRef();

	//	setup data for the task
	TBinaryTree TaskData( TRef_Invalid );
	TaskData.ExportDataString("url", Url );

	//	put upload data in the tasks data
	TPtr<TBinaryTree>& pUploadData = TaskData.AddChild("Upload");
	pUploadData->ReferenceDataTree( UploadData );

	//	create task and add to list
	TPtr<TTask> pTask = CreateTask( TTypedRef( TaskRef, "Upload" ), TaskData );
	m_Tasks.Add( pTask );

	//	start the task - note: may block and complete here...
	StartTask( *pTask );

	return pTask->GetTaskRef();
}


//-------------------------------------------------------------
//	get the current state and data of a task. returns NULL if task doesn't exist. 
//-------------------------------------------------------------
TBinaryTree* TLNetwork::TConnection::GetTask(TRefRef TaskRef,SyncBool& TaskStatus)				
{	
	TTask* pTask = m_Tasks.FindPtr(TaskRef);

	//	task doesnt exist
	if ( !pTask )
	{
		TTempString Debug_String("Task ");
		TaskRef.GetString( Debug_String );
		Debug_String.Append(" doesn't exist");
		TaskStatus = SyncFalse;
		return NULL;
	}

	//	get latest state
	TaskStatus = pTask->GetStatus();

	//	return data
	return &pTask->GetData();
}

//-------------------------------------------------------------
//	delete a task when we're done with it
//-------------------------------------------------------------
Bool TLNetwork::TConnection::RemoveTask(TRef TaskRef)	
{
	//	todo: insert shutdown here as required

	if ( !m_Tasks.Remove( TaskRef ) )
		return FALSE;

	return TRUE;
}	


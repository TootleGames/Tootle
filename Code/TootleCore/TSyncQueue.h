/*------------------------------------------------------
	
	Queue of sync jobs (functions)

	If a job is blocking all the jobs BEFORE it must be completed
	before it will be processed, and other jobs aren't processed
	until that one is complete

	future enhancements:
	*	Threading (specify threads for different jobs or allow
		jobs to go onto any threads)

-------------------------------------------------------*/
#pragma once

#include "TLTypes.h"
#include "TArray.h"
#include "TString.h"
#include "TLTime.h"


class TSyncQueue
{
public:
	typedef SyncBool (*JobFunc)();

protected:
	class SyncJob;

public:
	TSyncQueue()			{}

	SyncBool				Update(Bool BreakOnAnyNonWait=FALSE);		//	returns Wait if jobs are still going, FALSE if any fail, TRUE when all done

	void					Add(JobFunc pFunc,const TTempString& Name,Bool Blocking)	{	Add( SyncJob( pFunc, Name, Blocking ) );	}
	void					Add(const SyncJob& Job)										{	m_SyncJobs.Add( Job );	}
	void					Empty(Bool Dealloc)											{	m_SyncJobs.Empty(Dealloc);	}

protected:
	class SyncJob
	{
	public:
		SyncJob(JobFunc pFunction=NULL,const TTempString& JobName="NULL",Bool Blocking=TRUE) :
			m_pFunction	( pFunction ),
			m_Name		( JobName ),
			m_Blocking	( Blocking )
		{
		}

		SyncBool					Update()				{	return m_pFunction ? (*m_pFunction)() : SyncFalse;	}
		Bool						IsBlocking() const		{	return m_Blocking;	}
		const TString&				GetName() const			{	return m_Name;	}
		const TLTime::TTimestamp&	GetTimestamp() const	{	return m_FirstTimestamp;	}
		void						InitTimestamp() 		{	m_FirstTimestamp.SetTimestampNow();	}

		FORCEINLINE Bool			operator==(const SyncJob& Job) const	{	return (m_pFunction == Job.m_pFunction) && (m_Blocking == Job.m_Blocking);	}
		FORCEINLINE Bool			operator<(const SyncJob& Job) const		{	return FALSE;	}

	protected:
		JobFunc				m_pFunction;
		TString				m_Name;
		Bool				m_Blocking;
		TLTime::TTimestamp	m_FirstTimestamp;	//	timestamp of when we did our first Update() - can use later to determine what takes ages to update/init etc
	};

	THeapArray<SyncJob>	m_SyncJobs;			//	array of jobs

};


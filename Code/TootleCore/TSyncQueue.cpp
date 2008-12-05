#include "TSyncQueue.h"


//--------------------------------------------------------------
//	returns Wait if jobs are still going, FALSE if any fail, TRUE when all done
//--------------------------------------------------------------
SyncBool TSyncQueue::Update(Bool BreakOnAnyNonWait)
{
	//	no jobs so queue is complete :)
	if ( !m_SyncJobs.GetSize() )
		return SyncTrue;

	//	process the next job in the queue
	for ( s32 j=0;	j<(s32)m_SyncJobs.GetSize();	j++ )
	{
		SyncJob& Job = m_SyncJobs[j];

		//	if the job is blocking and it's not the first job, bail out of the queue update
		if ( j!=0 && Job.IsBlocking() )
			return SyncWait;

		//	if first update then init the timestamp
		if ( !Job.GetTimestamp().IsValid() )
			Job.InitTimestamp();

		//	update the job
		SyncBool Result = Job.Update();

		//	job still going
		if ( Result == SyncWait )
		{
			//	if blocking job, dont do any more jobs after this
			if ( Job.IsBlocking() )
				return SyncWait;

			//	next job!
			continue;
		}

		//	track timing of job once completed
		TLTime::TTimestamp Now(TRUE);
		s32 TimeDiff = Job.GetTimestamp().GetMilliSecondsDiff(Now);
		s32 OneFrame = (s32)((1.f/60.f)*1000.f);
		//	report if this update is greater than half a frame
		if ( TimeDiff > OneFrame*2 )
		{
			TTempString TimerString;
			TimerString.Appendf("SyncJob %s took %d ms (%d frames)", Job.GetName().GetData(), TimeDiff, TimeDiff/OneFrame );
			TLDebug_Print(TimerString);
		}

		//	job has finished - remove
		m_SyncJobs.RemoveAt( j );

		//	failed? bail out of the queue update and fail
		if ( Result == SyncFalse )
			return SyncFalse;

		//	re-process this index as we've just removed it!
		j--;

		//	if any job gives a non-wait result we need to break
		if ( BreakOnAnyNonWait )
			return Result;
	}

	//	still some jobs to do
	if ( m_SyncJobs.GetSize() )
		return SyncWait;

	//	all the jobs have been processed and removed... so queue is done!
	return SyncTrue;
}




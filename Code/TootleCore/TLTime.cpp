#include "TLTime.h"



namespace TLTime
{
	TTimestamp		g_CurrentTime;
	TTimestampMicro	g_CurrentMicroTime;

	TKeyArray<TRef,float>	g_TimerCounters;		//	current elapsed time for timers
	TKeyArray<TRef,float>	g_TimerAverages;		//	timer time elapsed in the last second in millisecs
}

namespace TLCounter
{
	TKeyArray<TRef,u16>		g_Counters;
	TKeyArray<TRef,float>	g_Averages;
}


void TLTime::Shutdown()
{
	g_TimerCounters.Empty(TRUE);
	g_TimerAverages.Empty(TRUE);
}

void TLCounter::Shutdown()
{
	g_Counters.Empty(TRUE);
	g_Averages.Empty(TRUE);
}



//---------------------------------------------------------
//	get our application udpate rate
//---------------------------------------------------------
float TLTime::GetUpdatesPerSecondf()
{	
	return 40.f;
}


//---------------------------------------------------------
//	
//---------------------------------------------------------
void TLTime::Debug_PrintTimestamp(const TTimestampMicro& Timestamp)			
{	
	Debug_PrintTimestamp( Timestamp, Timestamp.GetMicroSeconds() );	
}

//---------------------------------------------------------
//	return timestamp as of right now
//---------------------------------------------------------
const TLTime::TTimestamp& TLTime::GetTimeNow()
{
	//	update our global timestamp
	Platform::GetTimeNow( g_CurrentTime );

	//	and return it
	return g_CurrentTime;
}


//---------------------------------------------------------
//	return timestamp as of right now
//---------------------------------------------------------
const TLTime::TTimestampMicro& TLTime::GetMicroTimeNow()
{
	//	update our global timestamp
	Platform::GetMicroTimeNow( g_CurrentMicroTime );

	//	and return it
	return g_CurrentMicroTime;
}



//---------------------------------------------------------
//	
//---------------------------------------------------------
TLTime::TTimestamp::TTimestamp(Bool InitToNow) :
	m_Date			( 0 ),
	m_MilliSeconds	( 0 )
{
	//	initialise time to now
	if ( InitToNow )
	{
		*this = GetTimeNow();
	}
}



TLTime::TTimestamp::TTimestamp(const TTimestamp& Timestamp) :
	m_Date			( Timestamp.GetDate() ),
	m_MilliSeconds	( Timestamp.GetMilliSeconds() )
{
}


Bool TLTime::TTimestamp::operator<(const TTimestamp& Timestamp) const
{
	if ( GetDate() < Timestamp.GetDate() )
		return TRUE;

	if ( GetDate() > Timestamp.GetDate() )
		return FALSE;

	//	date is the same, compare millisecs
	return GetMilliSeconds() < Timestamp.GetMilliSeconds();
}


Bool TLTime::TTimestamp::operator>(const TTimestamp& Timestamp) const
{
	if ( GetDate() > Timestamp.GetDate() )
		return TRUE;

	if ( GetDate() < Timestamp.GetDate() )
		return FALSE;

	//	date is the same, compare millisecs
	return GetMilliSeconds() > Timestamp.GetMilliSeconds();
}



//---------------------------------------------------------
//	timer has finished, update global counters
//---------------------------------------------------------
void TLTime::TScopeTimer::OnTimerEnd()
{
	//	get time
	float Time = GetTimeMillisecs();

	//	update global counter
	TLTime::AddTimerTime( m_TimerRef, Time );
}


//---------------------------------------------------------
//	return the time spent in the timer in millisecs (.micro)
//---------------------------------------------------------
float TLTime::TScopeTimer::GetTimeMillisecs(const TTimestampMicro& EndTime) const
{
	//	record time now (before any string operations)
	s32 DiffMilliSeconds = m_StartTimestamp.GetMilliSecondsDiff( EndTime );
	s32 DiffMicroSeconds = m_StartTimestamp.GetMicroSecondsDiff( EndTime );
	
	m_StartTimestamp.GetTimeDiff( EndTime, DiffMilliSeconds, DiffMicroSeconds );

	//	make microsecs into a fraction
	float MicroFraction = (float)DiffMicroSeconds / (float)MilliSecsToMicro(1);
	float MilliFloat = (float)DiffMilliSeconds + MicroFraction;
	
	return MilliFloat;
}





TLTime::TTimestampMicro::TTimestampMicro(Bool InitToNow) :
	TTimestamp		( FALSE ),
	m_MicroSeconds	( 0 )
{
	//	initialise time to now
	if ( InitToNow )
	{
		*this = GetMicroTimeNow();
	}
}



TLTime::TTimestampMicro::TTimestampMicro(const TTimestampMicro& Timestamp) :
	TTimestamp		( Timestamp ),
	m_MicroSeconds	( Timestamp.GetMicroSeconds() )
{
}


TLTime::TTimestampMicro::TTimestampMicro(const TTimestamp& Timestamp) :
	TTimestamp		( Timestamp ),
	m_MicroSeconds	( 0 )
{
}


//---------------------------------------------------------
//	get difference between timestamp in parts
//---------------------------------------------------------
void TLTime::TTimestampMicro::GetTimeDiff(const TTimestampMicro& Timestamp,s32& Secs,s32& MilliSecs,s32& MicroSecs) const
{
	s32 MicroDiff = this->GetMicroSecondsDiff( Timestamp );
	Bool Negate = (MicroDiff < 0);
	if ( Negate )
		MicroDiff = -MicroDiff;

	//	get micro secs difference 
	MicroSecs = MicroDiff % 1000;

	//	get milli secs difference
	s32 MilliDiff = TLTime::MicroToMilliSecs( MicroDiff - MicroSecs );
	MilliSecs = MilliDiff % 1000;

	//	get secs difference
	Secs = TLTime::MilliSecsToSecs( MilliDiff - MilliSecs );

	if ( Negate )
	{
		MicroSecs = -MicroSecs;
		MilliSecs = -MilliSecs;
		Secs = -Secs;
	}
}



//---------------------------------------------------------
//	get difference between timestamp in parts
//---------------------------------------------------------
void TLTime::TTimestampMicro::GetTimeDiff(const TTimestampMicro& Timestamp,s32& MilliSecs,s32& MicroSecs) const
{
	s32 MicroDiff = this->GetMicroSecondsDiff( Timestamp );
	Bool Negate = (MicroDiff < 0);
	if ( Negate )
		MicroDiff = -MicroDiff;

	//	get micro secs difference 
	MicroSecs = MicroDiff % 1000;

	//	get milli secs difference
	s32 MilliDiff = TLTime::MicroToMilliSecs( MicroDiff - MicroSecs );
	//MilliSecs = MilliDiff % 1000;

	if ( Negate )
	{
		MicroSecs = -MicroSecs;
		MilliSecs = -MilliSecs;
	}
}



//---------------------------------------------------------
//	get per-second averages for timers when a second elapses
//---------------------------------------------------------
void TLTime::OnSecondElapsed(u32 FrameCount)
{
	float FrameCountf = (FrameCount==0) ? 1.f : (float)FrameCount;

	//	update the update-counters to get per second values
	for ( u32 c=0;	c<g_TimerCounters.GetSize();	c++ )
	{
		TRefRef TimerRef = g_TimerCounters.GetKeyAt(c);

		//	divide the MS counters by number of frames to get an average per call. if we don't do this
		//	the MS is going to vary wildly when frame rate goes up and down (even by 1)
		float& Counter = g_TimerCounters.ElementAt(c);
		Counter /= FrameCountf;

		//	save to per-seond key array
		g_TimerAverages.Add( TimerRef, Counter );

		//	reset counter
		Counter = 0.f;
	}
}




//---------------------------------------------------------
//	get per-second averages for counters when a second elapses
//---------------------------------------------------------
void TLCounter::OnSecondElapsed(u32 FrameCount)
{
	float FrameCountf = (FrameCount==0) ? 1.f : (float)FrameCount;

	//	update the update-counters to get per second values
	for ( u32 c=0;	c<g_Counters.GetSize();	c++ )
	{
		TRefRef TimerRef = g_Counters.GetKeyAt(c);

		//	divide the MS counters by number of frames to get an average per call. if we don't do this
		//	the MS is going to vary wildly when frame rate goes up and down (even by 1)
		u16& Counter = g_Counters.ElementAt(c);
		float Counterf = (float)Counter / FrameCountf;

		//	save to per-seond key array
		g_Averages.Add( TimerRef, Counter );

		//	reset counter
		Counter = 0;
	}
}

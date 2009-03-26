#include "TLTime.h"


namespace TLTime
{
	TTimestamp		g_CurrentTime;
	TTimestampMicro	g_CurrentMicroTime;
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
//	note: I've ordered these so string is constructed/copied (hopefully)
//	BEFORE the time is recorded
//---------------------------------------------------------
TLTime::TScopeTimer::TScopeTimer(const TTempString& TimerName,Bool Verbose) : 
	m_TimerName			( TimerName ),
	m_StartTimestamp	( TRUE ),
	m_Verbose			( Verbose )
{
}


//---------------------------------------------------------
//	print out timer result (time between start and now)
//---------------------------------------------------------
void TLTime::TScopeTimer::Debug_PrintTimerResult()
{
#ifndef _DEBUG
	return;
#endif

	//	record time now (before any string operations)
	TLTime::TTimestampMicro Now(TRUE);

	s32 DiffSeconds = m_StartTimestamp.GetSecondsDiff( Now );
	s32 DiffMilliSeconds = m_StartTimestamp.GetMilliSecondsDiff( Now );
	s32 DiffMicroSeconds = m_StartTimestamp.GetMicroSecondsDiff( Now );
	m_StartTimestamp.GetTimeDiff( Now, DiffSeconds, DiffMilliSeconds, DiffMicroSeconds );

	//	use a local string for speed :)
	TTempString String( m_TimerName );
	String.Append(" took ");

	//	show seconds if it was more than 1000 millisecs :)
	if ( DiffSeconds != 0 )
		String.Appendf("%d secs ", DiffSeconds );

	if ( DiffMilliSeconds != 0 )
		String.Appendf("%d ms ", DiffMilliSeconds );
	
	String.Appendf("%d us", DiffMicroSeconds );

	TLDebug_Print( String );
}

//---------------------------------------------------------
//	return the time spent in the timer in millisecs (.micro)
//---------------------------------------------------------
float TLTime::TScopeTimer::GetTimeMillisecs() const
{
	//	record time now (before any string operations)
	TLTime::TTimestampMicro Now(TRUE);

	s32 DiffMilliSeconds = m_StartTimestamp.GetMilliSecondsDiff( Now );
	s32 DiffMicroSeconds = m_StartTimestamp.GetMicroSecondsDiff( Now );
	
	m_StartTimestamp.GetTimeDiff( Now, DiffMilliSeconds, DiffMicroSeconds );

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





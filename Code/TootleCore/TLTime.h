/*------------------------------------------------------
	
	Generic time class. 
	Hopefully I'll find a way for this to deal with 
	date/timestamps (for files) and milliseconds so
	we can use it for system updates too...

-------------------------------------------------------*/
#pragma once
#include "TLTypes.h"
#include "TLDebug.h"
#include "TManager.h"
#include "TKeyArray.h"






namespace TLTime
{
	class TTimestamp;
	class TTimestampMicro;	//	timestamp but with micro second resolution
	class TScopeTimer;
	class TScopeTimerDummy;

	//	in release builds replace Debug_TScopeTimers with dummy classes that do nothing
#ifdef _DEBUG
	typedef TScopeTimer Debug_TScopeTimer;
#else
	typedef TScopeTimer Debug_TScopeTimer;
#endif


	extern TKeyArray<TRef,float>	g_TimerCounters;		//	current elapsed time for timers
	extern TKeyArray<TRef,float>	g_TimerAverages;		//	timer time elapsed in the last second in millisecs

	const TTimestamp&			GetTimeNow();							//	return timestamp as of right now
	const TTimestampMicro&		GetMicroTimeNow();						//	return timestamp as of right now

	float						GetUpdatesPerSecondf();				//	get desired frame rate (eg. 60fps)
	FORCEINLINE float			GetUpdateTimeMilliSecsf()			{	return 1000.f/GetUpdatesPerSecondf();	}
	FORCEINLINE float			GetUpdateTimeSecondsf()				{	return 1.f/GetUpdatesPerSecondf();	}	//	update time as a fraction of a second

	FORCEINLINE s32				MilliSecsToMicro(s32 Milliseconds)	{	return Milliseconds * 1000;	}
	FORCEINLINE s32				MicroToMilliSecs(s32 Microseconds)	{	return Microseconds / 1000;	}
	FORCEINLINE float			MicroToMilliSecsf(s32 Microseconds)	{	return Microseconds / 1000.f;	}
	FORCEINLINE s32				SecsToMilliSecs(s32 Seconds)		{	return Seconds * 1000;	}
	FORCEINLINE s32				MilliSecsToSecs(s32 Milliseconds)	{	return Milliseconds / 1000;	}
	FORCEINLINE float			MilliSecsToSecsf(s32 Milliseconds)	{	return Milliseconds / 1000.f;	}
	FORCEINLINE s32				SecsToMicroSecs(s32 Seconds)		{	return MilliSecsToMicro( SecsToMilliSecs( Seconds ) );	}

	namespace Platform
	{
		SyncBool			Init();									//	init time system (records time/date at startup)
		void				GetTimeNow(TTimestamp& Timestamp);
		void				GetMicroTimeNow(TTimestampMicro& Timestamp);
		void				Debug_PrintTimestamp(const TTimestamp& Timestamp,s32 Micro);
	}

	FORCEINLINE void							Debug_PrintTimestamp(const TTimestamp& Timestamp,s32 Micro=-1)		{	Platform::Debug_PrintTimestamp( Timestamp, Micro );	}
	void										Debug_PrintTimestamp(const TTimestampMicro& Timestamp);

	FORCEINLINE void							AddTimerTime(TRefRef TimerRef,float MillisecTime);	//	increment a timer counter
	void										OnSecondElapsed(u32 FrameCount);					//	get per-second averages for timers when a second elapses
	FORCEINLINE const TKeyArray<TRef,float>&	GetTimers()											{	return g_TimerAverages;	}	//	current elapsed time for timers

	class TTimeManager;

	void	Shutdown();
}


//-----------------------------------------
//	gr: debug counter system - better off out of the time system, but not sure where to put it
//		assuming just for debug usage at the moment - will come up with somewhere more appropriate soon
//		but it cannot go in the debug files, or the core...
//		maybe a CounterManager and just make all these funcs static
//-----------------------------------------
namespace TLCounter
{
//private:
	extern TKeyArray<TRef,u16>		g_Counters;		//	current counters, counters
	extern TKeyArray<TRef,float>	g_Averages;		//	average counter-per-second values for counters

//public:
	FORCEINLINE void							Increment(TRefRef CounterRef,u16 Amount=1);		//	increment a counter
	FORCEINLINE void							Debug_Increment(TRefRef CounterRef,u16 Amount=1);

	void										OnSecondElapsed(u32 FrameCount);				//	calcualate averages

	FORCEINLINE const TKeyArray<TRef,float>&	GetCounters()		{	return g_Averages;	}	//	current elapsed time for timers

	void	Shutdown();
};	




//---------------------------------------------------------
//	time stamp with date (stored in seconds since epoch) and milliseconds
//---------------------------------------------------------
class TLTime::TTimestamp
{
public:
	TTimestamp(Bool InitToNow=FALSE);
	TTimestamp(const TTimestamp& Timestamp);

	u32				GetDateSeconds() const								{	return m_Date % 60;	}
	u32				GetEpochSeconds() const								{	return m_Date;	}
	u16				GetMilliSeconds() const								{	return m_MilliSeconds;	}
	u32				GetTotalMilliSeconds() const						{	return SecsToMilliSecs( GetEpochSeconds() ) + GetMilliSeconds();	}
	u32&			GetDate()											{	return m_Date;	}
	const u32&		GetDate() const										{	return m_Date;	}

	void			SetInvalid()										{	m_Date = 0;	m_MilliSeconds = 0;	}
	void			SetEpochSeconds(u32 DateSeconds)					{	m_Date = DateSeconds;	}
	void			SetMilliSeconds(u32 MilliSeconds)					{	TLDebug_CheckInRange( MilliSeconds, 0, SecsToMilliSecs(1) );	m_MilliSeconds = MilliSeconds;	}
	void			SetTimestampNow()									{	TLTime::Platform::GetTimeNow( *this );	}

	s32				GetSecondsDiff(const TTimestamp& Timestamp) const		{	return Timestamp.GetEpochSeconds() - GetEpochSeconds();	}
	s32				GetMilliSecondsDiff(const TTimestamp& Timestamp) const	{	return SecsToMilliSecs(GetSecondsDiff(Timestamp)) + (Timestamp.GetMilliSeconds() - GetMilliSeconds());	}
	void			GetTimeDiff(const TTimestamp& Timestamp,s32& Secs,s32& MilliSecs);		//	get difference between timestamp in parts
	
	Bool			IsValid() const										{	return (m_Date != 0) || (m_MilliSeconds != 0);	}

	Bool			operator<(const TTimestamp& Timestamp) const;
	Bool			operator>(const TTimestamp& Timestamp) const;
	FORCEINLINE Bool		operator==(const TTimestamp& Timestamp) const		{	return (GetEpochSeconds() == Timestamp.GetEpochSeconds()) && (GetMilliSeconds() == Timestamp.GetMilliSeconds());	}
	FORCEINLINE Bool		operator!=(const TTimestamp& Timestamp) const		{	return (GetEpochSeconds() != Timestamp.GetEpochSeconds()) || (GetMilliSeconds() != Timestamp.GetMilliSeconds());	}
	FORCEINLINE void		operator=(const TTimestamp& Timestamp)				{	SetEpochSeconds( Timestamp.GetEpochSeconds() );	SetMilliSeconds( Timestamp.GetMilliSeconds() );	}

protected:
	u32				m_Date;				//	date in seconds (since epoch)
	u16				m_MilliSeconds;		//	milliseconds - should max out at 1000
};





//---------------------------------------------------------
//	time stamp with Micro second resolution
//---------------------------------------------------------
class TLTime::TTimestampMicro : public TLTime::TTimestamp
{
public:
	TTimestampMicro(Bool InitToNow=FALSE);
	TTimestampMicro(const TTimestamp& Timestamp);
	TTimestampMicro(const TTimestampMicro& Timestamp);

	u16				GetMicroSeconds() const								{	return m_MicroSeconds;	}
	float			GetMilliSecondsf() const							{	return GetMilliSeconds() + MicroToMilliSecsf( GetMicroSeconds() );	}
	u32				GetTotalMicroSeconds() const						{	return MilliSecsToMicro( GetTotalMilliSeconds() ) + GetMicroSeconds();	}

	void			SetInvalid()										{	TTimestamp::SetInvalid();	m_MicroSeconds = 0;	}
	void			SetMicroSeconds(u32 MicroSeconds)					{	TLDebug_CheckInRange( MicroSeconds, 0, MilliSecsToMicro(1) );	m_MicroSeconds = MicroSeconds;	}
	void			SetTimestampNow()									{	TLTime::Platform::GetMicroTimeNow( *this );	}

	s32				GetMicroSecondsDiff(const TTimestampMicro& Timestamp) const	{	return MilliSecsToMicro( GetMilliSecondsDiff(Timestamp) ) + (Timestamp.GetMicroSeconds() - GetMicroSeconds());	}
	void			GetTimeDiff(const TTimestampMicro& Timestamp,s32& Secs,s32& MilliSecs,s32& MicroSecs) const;	//	get difference between timestamp in parts
	void			GetTimeDiff(const TTimestampMicro& Timestamp,s32& MilliSecs,s32& MicroSecs) const;				//	get difference between timestamp in parts

protected:
	u32				m_MicroSeconds;		//	1,000 milliseconds
};





//---------------------------------------------------------
//	scope timer - records time when created, then when destroyed
//	records the time in the time manager's timers list
//---------------------------------------------------------
class TLTime::TScopeTimer
{
public:
	TScopeTimer(TRefRef TimerRef) :
		m_TimerRef			( TimerRef ),
		m_StartTimestamp	( TRUE )
	{
	}
	~TScopeTimer()								
	{	
		OnTimerEnd();	
	}

protected:
	void			OnTimerEnd();				//	timer has finished, update global counters
	float			GetTimeMillisecs(const TTimestampMicro& EndTime=TTimestampMicro(TRUE)) const;	//	return the time spent in the timer in millisecs (.micro)

protected:
	TRef			m_TimerRef;					//	name of timer
	TTimestampMicro	m_StartTimestamp;			//	timestamp set at start
};



//---------------------------------------------------------
//	dummy scope timer for release builds where Debug_ScopeTimer is replaced with this type
//---------------------------------------------------------
class TLTime::TScopeTimerDummy
{
};




class TLTime::TTimeManager : public TLCore::TManager
{
public:
	TTimeManager(TRefRef ManagerRef) :
		TLCore::TManager	( ManagerRef )
	{
	}

protected:
	SyncBool		Initialise()
	{
		return Platform::Init();
	}

	SyncBool		Shutdown()
	{
		TLTime::Shutdown();
		TLCounter::Shutdown();
	
		return TManager::Shutdown();
	}
};



//------------------------------------------
//	increment a timer counter
//------------------------------------------
FORCEINLINE void TLTime::AddTimerTime(TRefRef TimerRef,float MillisecTime)
{
	//	update existing counter
	float* pCounter = g_TimerCounters.Find( TimerRef );	
	if ( pCounter )	
		*pCounter += MillisecTime;
	else
		g_TimerCounters.Add( TimerRef, MillisecTime );
}




//------------------------------------------
//	increment a counter
//------------------------------------------
FORCEINLINE void TLCounter::Increment(TRefRef CounterRef,u16 Amount)
{
	//	update existing counter
	u16* pCounter = g_Counters.Find( CounterRef );	
	if ( pCounter )	
		*pCounter += Amount;
	else
		g_Counters.Add( CounterRef, Amount );
}


//------------------------------------------
//	increment a counter
//------------------------------------------
FORCEINLINE void TLCounter::Debug_Increment(TRefRef CounterRef,u16 Amount)
{
#ifdef _DEBUG
	Increment( CounterRef, Amount );
#endif
}


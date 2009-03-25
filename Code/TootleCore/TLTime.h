/*------------------------------------------------------
	
	Generic time class. 
	Hopefully I'll find a way for this to deal with 
	date/timestamps (for files) and milliseconds so
	we can use it for system updates too...

-------------------------------------------------------*/
#pragma once
#include "TLTypes.h"
#include "TLDebug.h"
#include "TString.h"
#include "TManager.h"






namespace TLTime
{
	class TTimestamp;
	class TTimestampMicro;	//	timestamp but with micro second resolution
	class TScopeTimer;

	const TTimestamp&		GetTimeNow();							//	return timestamp as of right now
	const TTimestampMicro&	GetMicroTimeNow();						//	return timestamp as of right now

	FORCEINLINE float		GetUpdatesPerSecondf();
	FORCEINLINE float		GetRendersPerSecondf();
	inline float			GetUpdateTimeMilliSecsf()			{	return 1000.f/GetUpdatesPerSecondf();	}
	inline float			GetUpdateTimeSecondsf()				{	return 1.f/GetUpdatesPerSecondf();	}	//	update time as a fraction of a second

	inline s32				MilliSecsToMicro(s32 Milliseconds)	{	return Milliseconds * 1000;	}
	inline s32				MicroToMilliSecs(s32 Microseconds)	{	return Microseconds / 1000;	}
	inline float			MicroToMilliSecsf(s32 Microseconds)	{	return Microseconds / 1000.f;	}
	inline s32				SecsToMilliSecs(s32 Seconds)		{	return Seconds * 1000;	}
	inline s32				MilliSecsToSecs(s32 Milliseconds)	{	return Milliseconds / 1000;	}
	inline float			MilliSecsToSecsf(s32 Milliseconds)	{	return Milliseconds / 1000.f;	}
	inline s32				SecsToMicroSecs(s32 Seconds)		{	return MilliSecsToMicro( SecsToMilliSecs( Seconds ) );	}

	namespace Platform
	{
		SyncBool		Init();									//	init time system (records time/date at startup)
		void			GetTimeNow(TTimestamp& Timestamp);
		void			GetMicroTimeNow(TTimestampMicro& Timestamp);
		void			Debug_PrintTimestamp(const TTimestamp& Timestamp,s32 Micro);
	}

	inline void				Debug_PrintTimestamp(const TTimestamp& Timestamp,s32 Micro=-1)		{	Platform::Debug_PrintTimestamp( Timestamp, Micro );	}
	void					Debug_PrintTimestamp(const TTimestampMicro& Timestamp);

	class TTimeManager;
}


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
	
	Bool			IsValid() const										{	return (m_Date != 0) && (m_MilliSeconds != 0);	}

	Bool			operator<(const TTimestamp& Timestamp) const;
	Bool			operator>(const TTimestamp& Timestamp) const;
	inline Bool		operator==(const TTimestamp& Timestamp) const		{	return (GetEpochSeconds() == Timestamp.GetEpochSeconds()) && (GetMilliSeconds() == Timestamp.GetMilliSeconds());	}
	inline Bool		operator!=(const TTimestamp& Timestamp) const		{	return (GetEpochSeconds() != Timestamp.GetEpochSeconds()) || (GetMilliSeconds() != Timestamp.GetMilliSeconds());	}
	inline void		operator=(const TTimestamp& Timestamp)				{	SetEpochSeconds( Timestamp.GetEpochSeconds() );	SetMilliSeconds( Timestamp.GetMilliSeconds() );	}

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
	void			GetTimeDiff(const TTimestampMicro& Timestamp,s32& Secs,s32& MilliSecs,s32& MicroSecs);	//	get difference between timestamp in parts

protected:
	u32				m_MicroSeconds;		//	1,000 milliseconds
};





//---------------------------------------------------------
//	scope timer - records time when created, prints out time when 
//	destroyed
//	simple use of this is TLTime::TScopeTimer(__FUNCTION__)
//---------------------------------------------------------
class TLTime::TScopeTimer
{
public:
	TScopeTimer(const TTempString& TimerName);
	~TScopeTimer()								{	Debug_PrintTimerResult();	}

	void			Debug_PrintTimerResult();	//	print out timer result (time between start and now)

protected:
	TTimestampMicro	m_StartTimestamp;			//	timestamp
	TTempString		m_TimerName;				//	name of timer
};


class TLTime::TTimeManager : public TManager
{
public:
	TTimeManager(TRef refManagerID) :
	  TManager(refManagerID)
	{
	}
protected:
	SyncBool		Initialise()
	{
		return Platform::Init();
	}
};






//---------------------------------------------------------
//	get our application udpate rate
//---------------------------------------------------------
FORCEINLINE float TLTime::GetUpdatesPerSecondf()
{	
	return 60.f;
}



//---------------------------------------------------------
//	get our application render rate
//---------------------------------------------------------
FORCEINLINE float TLTime::GetRendersPerSecondf()
{	
#if defined(TL_TARGET_PC)
	return 60.f;
#else
	//return GetUpdatesPerSecondf();
	return 30.f;	
#endif
}

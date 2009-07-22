/*------------------------------------------------------
	
	Type-safe bit-field/flags type

-------------------------------------------------------*/
#pragma once
#include <TootleCore/TLTypes.h>
#include <TootleCore/TLDebug.h>


//	gr: macro is faster
#define TFlags_GetFlag(a)			(1<<(a))
#define TFlags_GetFlag2(a,b)		(TFlags_GetFlag(a)|TFlags_GetFlag(b))
#define TFlags_GetFlag3(a,b,c)		(TFlags_GetFlag(a)|TFlags_GetFlag(b)|TFlags_GetFlag(c))
#define TFlags_GetFlag4(a,b,c,d)	(TFlags_GetFlag(a)|TFlags_GetFlag(b)|TFlags_GetFlag(c)|TFlags_GetFlag(d))
#define TFlags_GetFlag5(a,b,c,d,e)	(TFlags_GetFlag(a)|TFlags_GetFlag(b)|TFlags_GetFlag(c)|TFlags_GetFlag(d)|TFlags_GetFlag(e))



template<typename FLAGTYPE,typename STORAGETYPE=u32>
class TFlags
{
private:
	typedef const FLAGTYPE&	FT;

public:
	FORCEINLINE TFlags() :							m_Flags (0)						{}
	FORCEINLINE TFlags(const TFlags& Flags) :		m_Flags ( Flags.GetData() )		{}

	FORCEINLINE TFlags(FT a) :						m_Flags ( TFlags_GetFlag(a) )			{}
	FORCEINLINE TFlags(FT a,FT b) :					m_Flags ( TFlags_GetFlag2(a,b) )		{}
	FORCEINLINE TFlags(FT a,FT b,FT c) :			m_Flags ( TFlags_GetFlag3(a,b,c) )		{}
	FORCEINLINE TFlags(FT a,FT b,FT c,FT d) :		m_Flags ( TFlags_GetFlag4(a,b,c,d) )	{}
	FORCEINLINE TFlags(FT a,FT b,FT c,FT d,FT e) :	m_Flags ( TFlags_GetFlag5(a,b,c,d,e) )	{}

	FORCEINLINE Bool				IsSet(const FLAGTYPE& FlagIndex) const			{	return (m_Flags & TFlags_GetFlag(FlagIndex)) == TFlags_GetFlag(FlagIndex);	}
	FORCEINLINE void				Set(const FLAGTYPE& FlagIndex,Bool SetFlag)		{	SetFlag ? Set(FlagIndex) : Clear(FlagIndex);	}
	FORCEINLINE void				Set(const FLAGTYPE& FlagIndex)					{	TLDebug_CheckIndex(FlagIndex, sizeof(STORAGETYPE) * 8 );	m_Flags |= TFlags_GetFlag(FlagIndex);	}
	FORCEINLINE void				Set(const TFlags<FLAGTYPE>& Flags)				{	m_Flags |= Flags.GetData();	}
	FORCEINLINE void				Clear(const FLAGTYPE& FlagIndex)				{	m_Flags &= ~TFlags_GetFlag(FlagIndex);	}
	FORCEINLINE void				Clear(const TFlags<FLAGTYPE>& Flags)			{	m_Flags &= ~Flags.GetData();	}
	FORCEINLINE void				Clear()											{	m_Flags = 0x0;	}

	FORCEINLINE const STORAGETYPE&	GetData() const									{	return m_Flags;	}
	FORCEINLINE void				SetData(STORAGETYPE& Flags)						{ m_Flags = Flags; } // raw Set flags

	FORCEINLINE TFlags&				operator=(const TFlags& Flags)					{	m_Flags = Flags.GetData();	return *this;	}

	//	accessor - m_Flags(FlagIndex) == Bool
	FORCEINLINE Bool				operator()(const FLAGTYPE& FlagIndex) const		{	return IsSet( FlagIndex );	}


protected:
	STORAGETYPE			m_Flags;
};



/*------------------------------------------------------
	
	Maths funcs

-------------------------------------------------------*/
#pragma once


#include "TLTypes.h"


//	include the platform specific header
#if defined(_MSC_EXTENSIONS) && defined(TL_TARGET_PC)

#include "PC/PCMaths.h"

#elif defined(TL_TARGET_IPOD)

#include "IPod/IPodMaths.h"

#elif defined(TL_TARGET_MAC)

#include "Mac/MacMaths.h"

#elif

#pragma message("Invalid platform for maths library")

#endif


//	gr: this value of PI is what the cmath libs return
#define	PI				((float)3.1415927f )
#define TWO_PI			((float)6.2831853f)
#define HALF_PI			((float)1.5707963f)




namespace TLMaths
{
	void				Init();		//	maths system init, just seeds random number

	//	math types
	class TAngle;		//	simple class to stop ambiguity between radians and degrees
//	class TQuaternion;	//	quaternion type
//	class TMatrix;		//	matrix type

	class TEuler;		// Simple convenience class to represent a set of 3 Euler angles
	class TAxisAngle;	// Simple convenience class to represent an axis and angle

	//	external types
	class TBox;
	class TBox2D;
	class TLine;
	class TLine2D;
	class TSphere;
	class TSphere2D;
	class TPlane;		//	3D plane
	class TFrustum;		//	Camera frustum, box of 6 planes really

#define TLMATHS_LOOKUP_TO_RAD(i)	TAngle::DegreesToRadians( (float)i )	//	scale DOWN if table gets larger
//#define TLMATHS_RAD_TO_LOOKUP(r)	((s32)TAngle::RadiansToDegrees( r ))		//	scale UP if table gets larger
	FORCEINLINE s32					GetCosineLookupFromRad(float Radians);	


	// Various non-platform specific maths routines
	FORCEINLINE u32				GetFixedf(float f)						{	return (u32)(f * 65536.0f);	}
	
	template<typename TYPE>
	FORCEINLINE TYPE			Interp(const TYPE& From,const TYPE& To,float Interp);	//	get a value between two values
	template<typename TYPE>
	FORCEINLINE void			InterpThis(TYPE& From,const TYPE& To,float Interp);		//	get a value between two values
	template<typename TYPE>
	FORCEINLINE void			Limit(TYPE& Value,const TYPE& Min,const TYPE& Max);		//	limit to Min and Max
	template<typename TYPE>
	FORCEINLINE void			Wrap(TYPE& Value,const TYPE& Min,const TYPE& Max);		//	wrap a number around. 361,0,360 becomes 1

	//	gr: all random functions are max INCLUSIVE. Want a random element in an array? use GetGetLastIndex()!
	FORCEINLINE float			Randf(float Max=1.f);									//	fraction between 0.f and X inclusive (float has 4 decimal places)
	FORCEINLINE float			Randf(float Min,float Max);								//	fraction between X and Y inclusive (float has 4 decimal places)
	FORCEINLINE s32				Rand(s32 Min,s32 Max);									//	Min <= N <= Max (inclusive!)

	template<typename TYPE>
	FORCEINLINE void			SwapVars(TYPE& a,TYPE& b);								//	swap variables using a temporary



	//---------------------------------------------------
	//	Limit value within bounds, NOT inclusive of max. 
	//	Min <= Value < Max
	//---------------------------------------------------
	template <typename TYPE,typename MINMAXTYPE>
	void Limit(TYPE& Value,const MINMAXTYPE& Min,const MINMAXTYPE& Max)
	{
		if ( Value < Min )	
			Value = Min;

		if ( Value > Max )
			Value = Max;
	}


	//---------------------------------------------------
	//	Limit value within bounds, NOT inclusive of max. Used for arrays
	//	Min <= Value < Max
	//---------------------------------------------------
	template <typename TYPE,typename MINMAXTYPE>
	void LimitIndex(TYPE& Value,const MINMAXTYPE& Min,const MINMAXTYPE& Max)
	{
		if ( Value < Min )	
			Value = Min;

		if ( Value >= Max )
			Value = Max - 1;
	}

	#define TLMaths_TransformBitNone		(0x0)
	#define TLMaths_TransformBitTranslate	(1<<0)
	#define TLMaths_TransformBitRotation	(1<<1)
	#define TLMaths_TransformBitScale		(1<<2)
	#define TLMaths_TransformBitAll			(TLMaths_TransformBitTranslate|TLMaths_TransformBitRotation|TLMaths_TransformBitScale)
	#define TLMaths_NearZero				0.0001f
	#define TLMaths_NearOne					(1.f - TLMaths_NearZero)		//	value just less than one - used to check stuff is almost at 1 when normally between 0..1
	
		
	// Power of 2 routines

	// Check to see if a value is a power of 2
	template <class T>
	bool IsPowerOf2(T k)
	{
		if(k == 0)
			return false;
		
		return ((k & (k - 1)) == 0);
	}
	
	// Get the nearest power of 2 value to v
	// i.e. 240 -> 256, 257 -> 256
	template <class T>
	T GetNearestPowerOf2(T v)
	{
        int k;
        if (v == 0)
			return 1;
        for (k = sizeof(T) * 8 - 1; ((static_cast<T>(1U) << k) & v) == 0; k--);
        if (((static_cast<T>(1U) << (k - 1)) & v) == 0)
			return static_cast<T>(1U) << k;
        return static_cast<T>(1U) << (k + 1);
	}
	
	// Get the next power of 2 value from k
	// i.e. 240 -> 256, 257 -> 512
	template <class T>
	T GetNextPowerOf2(T k) 
	{
        k--;
        for (int i=1; i<sizeof(T)*CHAR_BIT; i<<=1)
			k = k | k >> i;
        return k+1;
	}
	
	/*
	 // Unsigned integer version
	template <class T>
	T GetNextPowerOf2(T k) 
	 {
        if (k == 0)
			return 1;
        k--;
        for (int i=1; i<sizeof(T)*CHAR_BIT; i<<=1)
			k = k | k >> i;
        return k+1;
	 }
	 */


}


//---------------------------------------------------------
//	simple class to stop ambiguity between radians and degrees
//---------------------------------------------------------
class TLMaths::TAngle
{
public:
	TAngle(float AngDegrees=0.f) : m_AngleDegrees (AngDegrees)	{}
	
	FORCEINLINE float	GetDegrees() const						{	return m_AngleDegrees;	}
	FORCEINLINE float	GetInverseDegrees() const				{	return -m_AngleDegrees;	}
	FORCEINLINE void	SetDegrees(float AngDegrees)			{	m_AngleDegrees = AngDegrees;	}
	FORCEINLINE void	AddDegrees(float AngDegrees)			{	m_AngleDegrees += AngDegrees;	SetLimit180();	}
	
	FORCEINLINE float	GetRadians() const						{	return DegreesToRadians( m_AngleDegrees );	}
	FORCEINLINE float	GetInverseRadians() const				{	return DegreesToRadians( -m_AngleDegrees );	}
	FORCEINLINE void	SetRadians(float AngRadians)			{	m_AngleDegrees = RadiansToDegrees( AngRadians );	}
	FORCEINLINE void	AddRadians(float AngRadians)			{	m_AngleDegrees += RadiansToDegrees( AngRadians );	SetLimit180();	}
	
	FORCEINLINE void	SetLimit360()							{	TLMaths::Wrap( m_AngleDegrees, 0.f, 360.f );	}	//	limit angle to 0...360
	FORCEINLINE void	SetLimit180()							{	TLMaths::Wrap( m_AngleDegrees, -179.999f, 180.f );	}	//	limit angle to -180...180
	FORCEINLINE void	Invert()								{	m_AngleDegrees = -m_AngleDegrees;	}	//	different to adding 180
	
	FORCEINLINE void	SetAngle(const float2& From,const float2& To)	{	SetAngle( To - From );	}
	void				SetAngle(const float2& Direction);				//	get angle from a vector
	FORCEINLINE TAngle	GetAngleDiff(const TAngle& Angle) const;	//	get the difference of angles FROM this (base angle) TO Angle (new angle). So if this is zero, we will just return Angle
	FORCEINLINE TAngle	GetAngleAbsDiff(const TAngle& Angle) const;	//	get the difference of angles FROM this (base angle) TO Angle (new angle). So if this is zero, we will just return Angle
	
	static float		DegreesToRadians(float AngDegrees)		{	return AngDegrees * (PI/180.f);	}
	static float		RadiansToDegrees(float AngRadians)		{	return AngRadians * (180.f/PI);	}
	
	FORCEINLINE void	operator=(const TLMaths::TAngle& Angle)		{	SetDegrees( Angle.GetDegrees() );	}
	FORCEINLINE void	operator+=(const TLMaths::TAngle& Angle)	{	AddDegrees( Angle.GetDegrees() );	}
	FORCEINLINE void	operator-=(const TLMaths::TAngle& Angle)	{	AddDegrees( -Angle.GetDegrees() );	}
	
private:
	float				m_AngleDegrees;		//	angle stored in degrees
};




//----------------------------------------------------------
//	get the difference of angles FROM this (base angle) TO Angle (new angle). So if this is zero, we will just return Angle
//----------------------------------------------------------
FORCEINLINE TLMaths::TAngle TLMaths::TAngle::GetAngleDiff(const TLMaths::TAngle& Angle) const	
{	
	float Diff = Angle.GetDegrees() - m_AngleDegrees;
	TLMaths::Wrap( Diff, -180.f, 180.f );	
	return TAngle( Diff );
}


//----------------------------------------------------------
//	get the asbsolute difference of angles FROM this (base angle) TO Angle (new angle). So if this is zero, we will just return Angle
//----------------------------------------------------------
FORCEINLINE TLMaths::TAngle TLMaths::TAngle::GetAngleAbsDiff(const TLMaths::TAngle& Angle) const	
{	
	float Diff = Angle.GetDegrees() - m_AngleDegrees;
	TLMaths::Wrap( Diff, -180.f, 180.f );
	
	return TAngle( Diff < 0.f ? -Diff : Diff );
}




//-----------------------------------------------
//	
//-----------------------------------------------
template<typename TYPE>
FORCEINLINE TYPE TLMaths::Interp(const TYPE& From,const TYPE& To,float Interp)
{
	TYPE NewFrom = From;
	InterpThis( NewFrom, To, Interp );
	return NewFrom;
}


//-----------------------------------------------
//	
//-----------------------------------------------
template<typename TYPE>
FORCEINLINE void TLMaths::InterpThis(TYPE& From,const TYPE& To,float Interp)
{
	TYPE Diff( To - From );
	Diff *= Interp;
	From += Diff;
}



//-----------------------------------------------
//	swap variables using a temporary
//-----------------------------------------------
template<typename TYPE>
FORCEINLINE void TLMaths::SwapVars(TYPE& a,TYPE& b)
{
	TYPE tmp = a;
	a = b;
	b = tmp;
}



//-----------------------------------------------
//	fraction between 0.f and 1.f inclusive
//-----------------------------------------------
FORCEINLINE float TLMaths::Randf(float Max)
{
	//return (rand() / RAND_MAX) * Max;
	
	u32 RandInt = Rand( 0, 10000 );

	return ((float)RandInt * Max) / 10000.f;
}


//-----------------------------------------------
//	fraction between X and Y inclusive
//-----------------------------------------------
FORCEINLINE float TLMaths::Randf(float Min,float Max)
{
	return Randf( Max - Min ) + Min;
}

//-----------------------------------------------
//	get random number inclusive Min <= N <= Max
//-----------------------------------------------
FORCEINLINE s32 TLMaths::Rand(s32 Min,s32 Max)
{
	s32 Random = Rand( Max - Min + 1 );
	Random += Min;
	return Random;
}



//-----------------------------------------------
//	
//-----------------------------------------------
template<typename TYPE>
FORCEINLINE void TLMaths::Limit(TYPE& Value,const TYPE& Min,const TYPE& Max)
{
	if ( Value > Max )
		Value = Max;

	if ( Value < Min )
		Value = Min;
}

//-----------------------------------------------
//	wrap a number around. 361,0,360 becomes 1
//-----------------------------------------------
template<typename TYPE>
FORCEINLINE void TLMaths::Wrap(TYPE& Value,const TYPE& Min,const TYPE& Max)
{
	TYPE Diff( Max - Min );

	while ( Value >= Max )
		Value -= Diff;

	while ( Value < Min )
		Value += Diff;
}

FORCEINLINE s32 TLMaths::GetCosineLookupFromRad(float Radians)
{
	if ( Radians == 0.f )
		return 0;

	float Degrees = TLMaths::TAngle::RadiansToDegrees( Radians );

	if ( Degrees < 0.f )
	{
		Degrees -= 0.999f;
		return (s32)Degrees;
	}
	else
	{
		Degrees += 0.999f;
		return (s32)Degrees;
	}
}


	

/*
//	gr: add these back in when we need them
#include "TArray.h"
DECLARE_TARRAY_DATATYPE( TLMaths::TMatrix );
DECLARE_TARRAY_DATATYPE( TLMaths::TQuaternion );
DECLARE_TARRAY_DATATYPE( TLMaths::TAngle );
DECLARE_TARRAY_DATATYPE( TLMaths::TBox );
DECLARE_TARRAY_DATATYPE( TLMaths::TSphere );
DECLARE_TARRAY_DATATYPE( TLMaths::TCapsule );

TLCore_DeclareIsDataType( XYZ )
*/

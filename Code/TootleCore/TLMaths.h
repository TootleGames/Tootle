/*------------------------------------------------------
	
	Maths funcs

-------------------------------------------------------*/
#pragma once


#include <math.h>		//	gr: not sure if this is platform specific?
#include <stdlib.h>		//	gr: platform specific? needed for rand() and srand()
#include "TLTypes.h"
#include "TLDebug.h"

//	gr: this value of PI is what the cmath libs return
#define	PI				((float)3.1415927f )
#define TWO_PI			((float)6.2831853f)
#define HALF_PI			((float)1.5707963f)


//	forward declarations
class TBinaryTree;
template<typename TYPE>
class TArray;


namespace TLMaths
{
	void				Init();		//	maths system init, just seeds random number

	//	math types
	class TAngle;		//	simple class to stop ambiguity between radians and degrees
	class TQuaternion;	//	quaternion type
	class TMatrix;		//	matrix type
	class TLine;		//	3D line/ray
	class TTransform;	//	Transform class - encapsulates common usage of position, rotation and scale

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
//	extern TFixedArray<float,360>	g_SineLookupTable;
	extern float					g_SineLookupTable[360];

	//	various maths stuff
	FORCEINLINE float			Sqrtf(float SquaredValue)				{	return sqrtf( SquaredValue );	}
	FORCEINLINE float			Cosf(float RadAngle);
	FORCEINLINE float			Sinf(float RadAngle);
	FORCEINLINE float			Tanf(float RadAngle)					{	return tanf( RadAngle );	}
	FORCEINLINE float			Absf(float Value)						{	return fabsf( Value );	}
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
	FORCEINLINE u32				Rand(u32 Max);											//	number between 0 and Max inclusive
	FORCEINLINE s32				Rand(s32 Min,s32 Max);									//	Min <= N <= Max (inclusive!)

	template<typename TYPE>
	FORCEINLINE void			SwapVars(TYPE& a,TYPE& b);								//	swap variables using a temporary

	//	some conversions - should be members really
	void			QuaternionToMatrix(const TQuaternion& Quaternion,TMatrix& Matrix);
	void			MatrixToQuaternion(const TMatrix& Matrix,TQuaternion& Quaternion);


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

	#define TLMaths_TransformBitTranslate	(1<<0)
	#define TLMaths_TransformBitRotation	(1<<1)
	#define TLMaths_TransformBitScale		(1<<2)
	#define TLMaths_TransformBitAll			(TLMaths_TransformBitTranslate|TLMaths_TransformBitRotation|TLMaths_TransformBitScale)
	#define TLMaths_NearZero				0.0001f
	#define TLMaths_NearOne					(1.f - TLMaths_NearZero)		//	value just less than one - used to check stuff is almost at 1 when normally between 0..1

	//	gr: these macros are just here to keep the compiled ref usage, they're not NEEDED but saves us using TRef_Static(B,o,x,2) etc everywhere
	#define TLMaths_ShapeRef_TBox			TRef_Static3(B,o,x)
	#define TLMaths_ShapeRef_TBox2D			TRef_Static4(B,o,x,TWO)
	#define TLMaths_ShapeRef_TCapsule		TRef_Static3(C,a,p)
	#define TLMaths_ShapeRef_TCapsule2D		TRef_Static4(C,a,p,TWO)
	#define TLMaths_ShapeRef_TLine			TRef_Static4(L,i,n,e)
	#define TLMaths_ShapeRef_TLine2D		TRef_Static(L,i,n,e,TWO)
	#define TLMaths_ShapeRef_TOblong		TRef_Static3(O,b,l)
	#define TLMaths_ShapeRef_TOblong2D		TRef_Static4(O,b,l,TWO)
	#define TLMaths_ShapeRef_TPolygon		TRef_Static4(P,o,l,y)
	#define TLMaths_ShapeRef_TPolygon2D		TRef_Static5(P,o,l,y,TWO)
	#define TLMaths_ShapeRef_TSphere		TRef_Static3(S,p,h)
	#define TLMaths_ShapeRef_TSphere2D		TRef_Static4(S,p,h,TWO)
	#define TLMaths_ShapeRef_Polygon		TRef_Static4(P,o,l,y)
	#define TLMaths_ShapeRef_Polygon2D		TRef_Static5(P,o,l,y,TWO)

	#define TLMaths_ShapeRef(ShapeType)		TLMaths_ShapeRef_##ShapeType
}


//---------------------------------------------------------
//	simple class to stop ambiguity between radians and degrees
//---------------------------------------------------------
class TLMaths::TAngle
{
public:
	TAngle(float AngDegrees=0.f) : m_AngleDegrees (AngDegrees)	{}

	FORCEINLINE float	GetDegrees() const						{	return m_AngleDegrees;	}
	FORCEINLINE void	SetDegrees(float AngDegrees)			{	m_AngleDegrees = AngDegrees;	}
	FORCEINLINE void	AddDegrees(float AngDegrees)			{	m_AngleDegrees += AngDegrees;	SetLimit180();	}

	FORCEINLINE float	GetRadians() const						{	return DegreesToRadians( m_AngleDegrees );	}
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


//-------------------------------------------------------------------
//	matrix type
//-------------------------------------------------------------------
class TLMaths::TMatrix
{
public:
	TMatrix()											{	};
	TMatrix(const float Value);
	TMatrix(const float* pValues)						{	Copy( pValues );	};
	TMatrix(const TMatrix& Other)						{	Copy( Other );	};
	TMatrix(const float4& col0,const float4& col1,const float4& col2,const float4& col3);

	FORCEINLINE u32			GetSize() const					{	return 16;	}
//	static FORCEINLINE int	DataSize()		 				{	return ( sizeof(float) * 16);	};
	FORCEINLINE				operator float*()				{	return GetData();	};
	FORCEINLINE				operator const float*() const	{	return GetData();	};
	FORCEINLINE float*		GetData()						{	return m_Column[0].GetData();	};
	FORCEINLINE const float*	GetData() const					{	return m_Column[0].GetData();	};
	
//	float4				&operator [] (unsigned int Index);
//	const float4		&operator [] (unsigned int Index) const;
	void				operator =  (const TMatrix& Other)	{	Copy( Other.GetData() );	};
	void				operator =  (const float* pFloats)	{	Copy( pFloats );	};
	void				operator += (const TMatrix &Other);
	void				operator -= (const TMatrix &Other);
	void				operator *= (const TMatrix &Other);
	void				operator *= (float Float);
	void				operator /= (float Float);
/*
	friend FORCEINLINE Bool	operator == (const TMatrix &V1,	const TMatrix &V2)	{	return V1.Compare(V2);	};
	friend FORCEINLINE Bool	operator != (const TMatrix &V1,	const TMatrix &V2)	{	return !V1.Compare(V2);	};
	friend TMatrix		operator + (const TMatrix &V1,	const TMatrix &V2);
	friend TMatrix		operator - (const TMatrix &V1,	const TMatrix &V2);
	friend TMatrix		operator * (const TMatrix &V1,	const TMatrix &V2);
	friend float3		operator * (const TMatrix &Other, const float3 &V1);
	friend float3		operator * (const float3 &V1,	const TMatrix &Other);
	friend float4		operator * (const TMatrix &Other, const float4 &V1);
	friend float4		operator * (const float4 &V1,	const TMatrix &Other);
	friend TMatrix		operator * (const TMatrix &Other, const float Float);
	friend TMatrix		operator * (const float Float,	const TMatrix &Other);
*/
	FORCEINLINE float&			operator()(u32 Row,u32 Col)							{	return Get( Row, Col );	}
	FORCEINLINE void				Set(int r, int c, const float& f)					{	TLDebug_CheckIndex(r,4);	GetCol(c)[r] = f;	};
	FORCEINLINE void				SetCol(int c, const float4& f)						{	TLDebug_CheckIndex(c,4);	m_Column[c] = f;	};
	FORCEINLINE void				SetRow(int r, const float4& f)						{	TLDebug_CheckIndex(r,4);	m_Column[0][r] = f.x;	m_Column[1][r] = f.y;	m_Column[2][r] = f.z;	m_Column[3][r] = f.w;	};
	FORCEINLINE void				SetCol(int c, const float3& f, const float w)		{	TLDebug_CheckIndex(c,4);	m_Column[c].x = f.x;	m_Column[c].y = f.y;	m_Column[c].z = f.z;	m_Column[c].w = w;	};
	FORCEINLINE void				SetRow(int r, const float3& f, const float w)		{	TLDebug_CheckIndex(r,4);	m_Column[0][r] = f.x;	m_Column[1][r] = f.y;	m_Column[2][r] = f.z;	m_Column[3][r] = w;	};
	FORCEINLINE float&			Get(int r, int c)				{	TLDebug_CheckIndex(c,4);	TLDebug_CheckIndex(r,4);	return m_Column[c][r];	};
	FORCEINLINE float4&			GetCol(int c)					{	TLDebug_CheckIndex(c,4);	return m_Column[c];	};
	FORCEINLINE const float&		Get(int r, int c) const	{	TLDebug_CheckIndex(c,4);	TLDebug_CheckIndex(r,4);	return m_Column[c][r];	};
	FORCEINLINE float&			Get(int i)				{	return GetData()[i];	}	//	gr: quick fix replacement for use of m_Matrix (which was an array of all the floats): todo: replace with proper column[x].y usage
	FORCEINLINE const float&		Get(int i) const		{	return GetData()[i];	}	//	gr: quick fix replacement for use of m_Matrix (which was an array of all the floats): todo: replace with proper column[x].y usage
	FORCEINLINE const float4&	GetCol(int c) const		{	TLDebug_CheckIndex(c,4);	return m_Column[c];	};
	
	void				SetIdentity();
	Bool				IsIdentity() const;

	void				Transpose();
	void				Invert();
	void				Rotate(const TQuaternion& q);			//	rotate by quaternion
	void				SetRotation(const TQuaternion& q);	//	set rotation from quaternion
	void				SetTranslate(const float3& xyz);
	float3				GetTranslate() const				{	return float3( GetCol(3)[0], GetCol(3)[1], GetCol(3)[2] );	}

	void				SetRotationRadians(const float3& Angles);
	void				SetOrientation(float3& Foward,float3& Up,float3& Left);	//	construct orientation matrix

	void				InverseTranslateVect(float3& v);
	void				InverseRotateVect(float3& v);
	void				TransformVector(float3& v) const;
	void				TransformVector(float2& v) const;

private:
	void				Copy(const float* pFloats);
	void				Copy(const TMatrix& Matrix);
	Bool				Compare(const TMatrix& Matrix) const;

protected:
	float4				m_Column[4];	
};


//-------------------------------------------------------------------
//	quaternion type
//-------------------------------------------------------------------
class TLMaths::TQuaternion
{
public:
	TQuaternion()												{	SetIdentity();	};
	TQuaternion(float NewX,float NewY,float NewZ,float NewW) : xyzw ( NewX, NewY, NewZ, NewW )	{	}	//	set raw values
	TQuaternion(const float4& Newxyzw) : xyzw ( Newxyzw )		{	}									//	set raw values
	TQuaternion(const float3& Axis,float Angle)					{	Set( Axis, Angle );	}

	const float4&			GetData() const						{	return xyzw;	}

	void					operator = (const TQuaternion &Other)					{	SetValues( Other.xyzw );	}
	//void					operator ~ ();// Conjugado	//	x=-x, y=-y, z=-z
	void					SetValues(float NewX,float NewY,float NewZ,float NewW)	{	SetValues( float4( NewX, NewY, NewZ, NewW ) );	}
	void					SetValues(const float4& Newxyzw)						{	xyzw = Newxyzw;	}

	Bool					operator == (const TQuaternion &Other) const			{	return (xyzw == Other.xyzw);	}
	Bool					operator != (const TQuaternion &Other) const			{	return (xyzw != Other.xyzw);	}

	TQuaternion				operator - () const										{	return TLMaths::TQuaternion( xyzw * -1.f );	}
	TQuaternion				operator + (const TQuaternion &Other) const				{	return TLMaths::TQuaternion( xyzw + Other.xyzw );	}
	TQuaternion				operator - (const TQuaternion &Other) const				{	return TLMaths::TQuaternion( xyzw - Other.xyzw );	}
	TQuaternion				operator * (const TQuaternion &Other) const;// MultiplicaÁ„o

	void					operator += (const TQuaternion &Other);// Soma com afectaÁ„o
	void					operator -= (const TQuaternion &Other);// SubtracÁ„o com afectaÁ„o
	void					operator *= (const TQuaternion &Other);// MultiplicaÁ„o com afectaÁ„o

	void					operator /= (const float &Scalar);// Divis„o com afectaÁ„o
	void					operator *= (const float &Scalar);// MultiplicaÁ„o com afectaÁ„o

	// Euler angles
	void					SetEuler(const float& Pitch, const float& Yaw, const float& Roll);	//	radians
	FORCEINLINE void		SetEuler(const float3& vEuler)	{ SetEuler(vEuler.x, vEuler.y, vEuler.z); }
	float3					GetEuler() const;

	// Axis and angle
	void					Set(const float3& Axis,const float RadAngle);

	void					Normalise()		{ xyzw.Normalise();	}
	float					DotProduct(const TQuaternion& quat)		const { return xyzw.DotProduct(quat.GetData()); }

	float					GetLength() const	{	return xyzw.Length();	}
//	float3&					GetAxis()			{	return xyz;	};
//	float&					GetAngle()			{	return w;	}
	float3					GetTempAxis() const {	return float3( xyzw.x, xyzw.y, xyzw.z );	}	//	gr: called this "temp" to make sure I can tell if anything is using "getaxis"
	FORCEINLINE Bool				IsValid() const		{	return (xyzw.w!=0.f) && (GetTempAxis().LengthSq()!=0.f);	};
	TLMaths::TAngle			GetAngle2D() const;	//	extract a eular angle in degrees from the quaternion. Is is based on an axis of 0,0,1. probably better ways to do it to get 3D angles...

	FORCEINLINE void				SetIdentity()								{	SetValues( 0,0,0,1 );	};
	
	
	FORCEINLINE void				Invert()									{	xyzw.Invert();	};
	void					LookAt(const float3& Dir,const float3& WorldUp=float3(0,1,0));
	void					RotateVector(float3& Vector) const;
	void					RotateVector(float2& Vector) const;
	void					UnRotateVector(float3& Vector) const;

	// Interpolation
	void					Slerp(const TQuaternion& From, const TQuaternion& To, const float& t);					// Spherical linear interpolation
	void					Nlerp(const TQuaternion& From, const TQuaternion& To, const float& t);		// Normalised linear interpolation

	static float4			QuatMult(const float4& First,const float4& Second);	//	xyzw multiply for quaternions

public:
	float4					xyzw;
};

// Convenience class for using Euler angles
class TLMaths::TEuler
{
public:
	TEuler()												{};
	TEuler(float3 Angles) : 
		m_Pitch(Angles.x),
		m_Yaw(Angles.y),
		m_Roll(Angles.z)
	{};

	FORCEINLINE float GetPitch(Bool bRadians = TRUE)		const { return (bRadians ? m_Pitch.GetRadians() : m_Pitch.GetDegrees()) ; }
	FORCEINLINE float GetYaw(Bool bRadians = TRUE)			const { return (bRadians ? m_Yaw.GetRadians() : m_Yaw.GetDegrees()); }
	FORCEINLINE float GetRoll(Bool bRadians = TRUE)			const { return (bRadians ? m_Roll.GetRadians() : m_Roll.GetDegrees()); }

	FORCEINLINE	float3	GetAngles()		{ return float3(GetPitch(), GetYaw(), GetRoll()); }
private:
	TAngle					m_Pitch;
	TAngle					m_Yaw;
	TAngle					m_Roll;
};

// Convenience class for using an Axis and Angle
class TLMaths::TAxisAngle
{
public:
	TAxisAngle()												{};
	TAxisAngle(float4 AxisAngle) : 
		m_Axis(AxisAngle.x, AxisAngle.y, AxisAngle.z),
		m_Angle(AxisAngle.w)
	{};

	FORCEINLINE float3	GetAxis()					const { return m_Axis; }
	FORCEINLINE float	GetAngle(Bool bRadians = TRUE)		const { return (bRadians ? m_Angle.GetRadians() : m_Angle.GetDegrees()); }

	FORCEINLINE	float4	GetAxisAngle()		{ return float4(m_Axis.x, m_Axis.y, m_Axis.z, m_Angle.GetRadians()); }

private:
	float3					m_Axis;
	TAngle					m_Angle;
};




class TLMaths::TTransform
{
public:
	TTransform() : m_Valid ( 0x0 )			{	}

	FORCEINLINE void				SetScale(const float3& Scale)							{	m_Scale = Scale;			SetScaleValid();	}
	FORCEINLINE void				SetTranslate(const float3& Translate)					{	m_Translate = Translate;	SetTranslateValid();	}
	FORCEINLINE void				SetRotation(const TQuaternion& Rotation)				{	m_Rotation = Rotation;		SetRotationValid();	}

	FORCEINLINE u8					SetHasChanged(const TLMaths::TTransform& NewTransform);					//	set all elements and return what bits have changed
	FORCEINLINE u8					SetScaleHasChanged(const float3& Scale);								//	set scale, return's the Transform bit that has changed
	FORCEINLINE u8					SetTranslateHasChanged(const float3& Translate);						//	set translation, return's the Transform bit that has changed
	FORCEINLINE u8					SetTranslateHasChanged(const float3& Translate,float MinChange);		//	set translation, return's the Transform bit that has changed
	FORCEINLINE u8					SetRotationHasChanged(const TQuaternion& Rotation);						//	set rotation, return's the Transform bit that has changed
	FORCEINLINE u8					SetRotationHasChanged(const TQuaternion& Rotation,float MinChange);		//	set rotation, return's the Transform bit that has changed

	FORCEINLINE float3&				GetTranslate() 			{	return m_Translate;	}	//	only use if HasTranslate()
	FORCEINLINE float3&				GetScale()				{	return m_Scale;	}		//	only use if HasScale()
	FORCEINLINE TQuaternion&		GetRotation() 			{	return m_Rotation;	}	//	only use if HasRotation()
	FORCEINLINE const float3&		GetTranslate() const	{	Debug_Assert( HasTranslate(), "Translate accessed but is invalid");	return m_Translate;	}
	FORCEINLINE const float3&		GetScale() const		{	Debug_Assert( HasScale(), "Scale accessed but is invalid");			return m_Scale;	}
	FORCEINLINE const TQuaternion&	GetRotation() const		{	Debug_Assert( HasRotation(), "Rotation accessed but is invalid");	return m_Rotation;	}

	FORCEINLINE void				SetInvalid()			{	m_Valid = 0x0;	}
	FORCEINLINE void				SetTranslateInvalid()	{	m_Valid &= ~TLMaths_TransformBitTranslate;	/*m_Translate.Set( 0.f, 0.f, 0.f );*/	}
	FORCEINLINE void				SetScaleInvalid()		{	m_Valid &= ~TLMaths_TransformBitScale;		/*m_Scale.Set( 1.f, 1.f, 1.f );*/	}
	FORCEINLINE void				SetRotationInvalid()	{	m_Valid &= ~TLMaths_TransformBitRotation;		/*m_Rotation.SetIdentity();*/	}

	FORCEINLINE void				SetTranslateValid()				{	m_Valid |= TLMaths_TransformBitTranslate;	}
	FORCEINLINE void				SetTranslateValid(Bool Valid)	{	if ( Valid )	SetTranslateValid();	else SetTranslateInvalid();	}
	FORCEINLINE void				SetScaleValid()					{	m_Valid |= TLMaths_TransformBitScale;		}
	FORCEINLINE void				SetRotationValid()				{	m_Valid |= TLMaths_TransformBitRotation;	}

	FORCEINLINE Bool				HasAnyTransform() const			{	return (m_Valid != 0x0);	}
	FORCEINLINE Bool				HasTranslate() const			{	return (m_Valid & TLMaths_TransformBitTranslate) != 0x0;	}
	FORCEINLINE Bool				HasScale() const				{	return (m_Valid & TLMaths_TransformBitScale) != 0x0;	}
	FORCEINLINE Bool				HasRotation() const				{	return (m_Valid & TLMaths_TransformBitRotation) != 0x0;	}
	FORCEINLINE u8					GetHasTransformBits() const		{	return m_Valid;	}

	//	these Transform()'s are like matrix multiplies
	void				Transform(const TLMaths::TTransform& Trans);			//	transform this by another transform, this is like a local tranform, if Trans says "move right", it will move right, relative to the rotation. dumb faster method which doesn't do checks
	u8					Transform_HasChanged(const TLMaths::TTransform& Trans);	//	transform this by another transform, this is like a local tranform, if Trans says "move right", it will move right, relative to the rotation. returns elements that have changed (slightly slower, but if your caller does much LESS work if nothing changed then use this)
	void				Transform(float3& Vector) const;				//	transform vector
	void				Transform(float2& Vector) const;				//	transform vector
	void				Transform(TArray<float3>& VectorArray) const;
	void				Transform(TArray<float2>& VectorArray) const;
	void				Untransform(float3& Vector) const;				//	untransform vector
	void				Untransform(float2& Vector) const;				//	untransform vector

	//	matrix "adds"
	void				AddTransform(const TLMaths::TTransform& Trans);				//	Modify the transform values by another transform, Translates the translate, scales the scale, rotates the rotation. Doesn't multiply and rotate the translate etc
	u8					AddTransform_HasChanged(const TLMaths::TTransform& Trans);	//	Modify the transform values by another transform, Translates the translate, scales the scale, rotates the rotation. Doesn't multiply and rotate the translate etc. returns elements that have changed (slightly slower, but if your caller does much LESS work if nothing changed then use this)

	u8					ImportData(TBinaryTree& Data);				//	import transform data from binary data/message/etc- returns bitmask of the attributes that have changed
	u8					ExportData(TBinaryTree& Data,u8 TransformBits=TLMaths_TransformBitAll);	//	export all our valid data to this binary data- can optionally make it only write certain attributes. Returns bits of the attributes written.

private:
#ifdef _DEBUG
	FORCEINLINE void	Debug_Assert(Bool Condition,const char* pString) const		{	if ( !Condition )	Debug_Assert( pString );	}
#else
	FORCEINLINE void	Debug_Assert(Bool Condition,const char* pString) const		{	}
#endif
	
	void				Debug_Assert(const char* pString) const;

protected:
	float3				m_Translate;		//	simple translation
	float3				m_Scale;			//	scale
	TQuaternion			m_Rotation;			//	human-usable rotation
	u8					m_Valid;			//	bit mask of validity.	TRANSFORM_BIT_XXX - last for byte alignment
};





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
//	get random u32 up to max (NOT inclusive... 0 <= rand < Max)
//-----------------------------------------------
FORCEINLINE u32 TLMaths::Rand(u32 Max)
{
	u32 RandInt = rand() % Max;
	return RandInt;
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

FORCEINLINE float TLMaths::Cosf(float RadAngle)
{
	return ::cosf( RadAngle );
	/*
	s32 Index = GetCosineLookupFromRad( RadAngle );
	TLDebug_CheckIndex( Index, 360 );
	if ( Index < 0 )
		Index = -Index;
	return g_SineLookupTable[ (Index + 90 ) % 360 ];
	*/
}

FORCEINLINE float TLMaths::Sinf(float RadAngle)
{
	return ::sinf( RadAngle );
	/*
	s32 Index = GetCosineLookupFromRad( RadAngle );
	TLDebug_CheckIndex( Index, 360 );
	if ( Index < 0 )
		Index = -Index;
	return g_SineLookupTable[ Index % 360 ];
	*/
}

	

//--------------------------------------------------
//	set all elements and return what bits have changed
//--------------------------------------------------
FORCEINLINE u8 TLMaths::TTransform::SetHasChanged(const TLMaths::TTransform& NewTransform)
{
	u8 Changes = 0x0;

	if ( NewTransform.HasScale() )
		Changes |= SetScaleHasChanged( NewTransform.GetScale() );

	if ( NewTransform.HasRotation() )
		Changes |= SetRotationHasChanged( NewTransform.GetRotation() );

	if ( NewTransform.HasTranslate() )
		Changes |= SetTranslateHasChanged( NewTransform.GetTranslate() );

	return Changes;
}


//--------------------------------------------------
//	set scale, return's the Transform bit that has changed
//--------------------------------------------------
FORCEINLINE u8 TLMaths::TTransform::SetScaleHasChanged(const float3& Scale)
{
	if ( !HasScale() || Scale != m_Scale )
	{
		SetScale( Scale );
		return TLMaths_TransformBitScale;
	}
	else
		return 0x0;
}


//--------------------------------------------------
//	set translation, return's the Transform bit that has changed
//--------------------------------------------------
FORCEINLINE u8 TLMaths::TTransform::SetTranslateHasChanged(const float3& Translate)
{
	if ( !HasTranslate() || Translate != m_Translate )
	{
		SetTranslate( Translate );
		return TLMaths_TransformBitTranslate;
	}
	else
		return 0x0;
}



//--------------------------------------------------
//	set translation, return's the Transform bit that has changed
//--------------------------------------------------
FORCEINLINE u8 TLMaths::TTransform::SetTranslateHasChanged(const float3& Translate,float MinChange)
{
	if ( !HasTranslate() )
	{
		SetTranslate( Translate );
		return TLMaths_TransformBitTranslate;
	}
	else
	{
		if ( m_Translate.HasDifferenceMin( Translate, MinChange ) )
		{
			SetTranslate( Translate );
			return TLMaths_TransformBitTranslate;
		}
		else
		{
			//	very minor change - don't apply
			return 0x0;
		}
	}
}




//--------------------------------------------------
//	set rotation, return's the Transform bit that has changed
//--------------------------------------------------
FORCEINLINE u8 TLMaths::TTransform::SetRotationHasChanged(const TLMaths::TQuaternion& Rotation,float MinChange)
{
	if ( !HasRotation() )
	{
		SetRotation( Rotation );
		return TLMaths_TransformBitRotation;
	}
	else
	{
		if ( m_Rotation.GetData().HasDifferenceMin( Rotation.GetData(), MinChange ) )
		{
			SetRotation( Rotation );
			return TLMaths_TransformBitRotation;
		}
		else
		{
			//	very minor change - don't apply
			return 0x0;
		}
	}
}




//--------------------------------------------------
//	set rotation, return's the Transform bit that has changed
//--------------------------------------------------
FORCEINLINE u8 TLMaths::TTransform::SetRotationHasChanged(const TLMaths::TQuaternion& Rotation)
{
	if ( !HasRotation() || Rotation != m_Rotation )
	{
		SetRotation( Rotation );
		return TLMaths_TransformBitRotation;
	}
	else
		return 0x0;
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

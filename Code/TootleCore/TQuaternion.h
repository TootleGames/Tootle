/*
 *  TQuaternion.h
 *  TootleCore
 *
 *  Created by Duane Bradbury on 03/05/2010.
 *  Copyright 2010 Tootle. All rights reserved.
 *
 */

#pragma once

#include "TLTypes.h"
#include "TBinary.h"


namespace TLMaths
{
	class TQuaternion;
	
	class TAngle;
}


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
	
	// [28/07/09] DB - A quaternion is still valid with a zero w value. 
	//				   {1,0,0,0} for example is 180 degrees in the x-axis which
	//				   would fail when checking the w value first and it's likely to
	//				   be quicker simply checking the lengthsq of the float4 than
	//				   creating and testing a float3 lengthsq first
	//FORCEINLINE Bool				IsValid() const		{	return ((xyzw.w!=0.f) && (GetTempAxis().LengthSq() !=0.f));	};
	FORCEINLINE Bool		IsValid() const		{	return (xyzw.LengthSq() !=0.f);	}
	
	TLMaths::TAngle			GetAngle2D() const;	//	extract a eular angle in degrees from the quaternion. Is is based on an axis of 0,0,1. probably better ways to do it to get 3D angles...
	
	FORCEINLINE void				SetIdentity()								{	SetValues( 0,0,0,1 );	};
	
	
	FORCEINLINE void				Invert()									{	xyzw.Invert();	};
	void					LookAt(const float3& Dir,const float3& WorldUp=float3(0,1,0));
	void					RotateVector(float3& Vector) const;
	void					RotateVector(float2& Vector) const;
	void					UnrotateVector(float3& Vector) const;
	void					UnrotateVector(float2& Vector) const;
	
	// Interpolation
	void					Slerp(const TQuaternion& From, const TQuaternion& To, const float& t);					// Spherical linear interpolation
	void					Nlerp(const TQuaternion& From, const TQuaternion& To, const float& t);		// Normalised linear interpolation
	
	static float4			QuatMult(const float4& First,const float4& Second);	//	xyzw multiply for quaternions
	
public:
	float4					xyzw;
};




#define TLBinary_TypeRef_TQuaternion			TRef_Static4(Q,u,a,t)
TLBinary_DeclareDataTypeWithRef( TLMaths::TQuaternion,	TLBinary_TypeRef(TQuaternion) );


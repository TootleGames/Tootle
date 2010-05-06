/*
 *  TQuaternion.cpp
 *  TootleCore
 *
 *  Created by Duane Bradbury on 03/05/2010.
 *  Copyright 2010 Tootle. All rights reserved.
 *
 */

#include "TQuaternion.h"

#include "TMatrix.h"  // Needed for RotateVector & UnrotateVector
#include "TLMaths.h"


#include "TLDebug.h"
#include "TString.h"

void TLMaths::TQuaternion::Set(const float3& Axis,float RadAngle)
{
	float Temp1,Temp2;
	
	// [18 02 09] DB - LengthSq appears to return a NAN on the iPod in debug.  Not sure if it's an issue or not (?) but the same code gives the correct result
	Temp1 = (Axis.x * Axis.x) + (Axis.y * Axis.y) + (Axis.z * Axis.z); 
	//Temp1 = Axis.LengthSq();
	
	if ( Temp1 == 0.f )
	{
		//GDebug_Print("Quaternion Axis is zero!");
		//Axis = float3(0,0,1);
		//Temp1 = Axis.GetLength();
		Set( float3(0,0,1), RadAngle );
		return;
	}
	
	Temp2 = TLMaths::Sinf(RadAngle * 0.5f) / Temp1;
	SetValues(Axis.x * Temp2,Axis.y * Temp2,Axis.z * Temp2, TLMaths::Cosf(RadAngle * 0.5f) );
}


//---------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------
TLMaths::TQuaternion TLMaths::TQuaternion::operator * (const TLMaths::TQuaternion &Other) const
{
	TLDebug_Break("gr: this code doesn't seem to produce what I expect... *= operator works, but I can't seem to reuse the * code!? seems backwards");
	const float4& other = Other.xyzw;
	
	TLMaths::TQuaternion NewQuat;
	float4& temp = NewQuat.xyzw;
	
	temp.x = xyzw.w * other.x + xyzw.x * other.w + xyzw.y * other.z - xyzw.z * other.y;
	temp.y = xyzw.w * other.y + xyzw.y * other.w + xyzw.z * other.x - xyzw.x * other.z;
	temp.z = xyzw.w * other.z + xyzw.z * other.w + xyzw.x * other.y - xyzw.y * other.x;
	temp.w = xyzw.w * other.w - xyzw.x * other.x - xyzw.y * other.y - xyzw.z * other.z; 
	/*
	 return TLMaths::TQuaternion(
	 xyzw.w * Otherxyzw.x + xyzw.x * Otherxyzw.w + xyzw.y * Otherxyzw.z - xyzw.z * Otherxyzw.y,
	 xyzw.w * Otherxyzw.y + xyzw.y * Otherxyzw.w + xyzw.z * Otherxyzw.x - xyzw.x * Otherxyzw.z,
	 xyzw.w * Otherxyzw.z + xyzw.z * Otherxyzw.w + xyzw.x * Otherxyzw.y - xyzw.y * Otherxyzw.x,
	 xyzw.w * Otherxyzw.w - xyzw.x * Otherxyzw.x - xyzw.y * Otherxyzw.y - xyzw.z * Otherxyzw.z); 
	 */
	return NewQuat;
}

//---------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------
void TLMaths::TQuaternion::operator += (const TLMaths::TQuaternion &Other)
{
	xyzw += Other.xyzw;
}

//---------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------
void TLMaths::TQuaternion::operator -= (const TLMaths::TQuaternion &Other)
{
	xyzw -= Other.xyzw;
}

//---------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------
void TLMaths::TQuaternion::operator *= (const TLMaths::TQuaternion &Other)
{
	Bool OtherValid = Other.IsValid();
	if ( !OtherValid )
		return;
	
	Bool ThisValid = this->IsValid();
	
	if ( !ThisValid )
	{
		*this = Other;
		return;
	}
	
	const float4& other = Other.xyzw;
	
	float4 temp;
	
	temp.x = xyzw.w * other.x + xyzw.x * other.w + xyzw.y * other.z - xyzw.z * other.y;
	temp.y = xyzw.w * other.y + xyzw.y * other.w + xyzw.z * other.x - xyzw.x * other.z;
	temp.z = xyzw.w * other.z + xyzw.z * other.w + xyzw.x * other.y - xyzw.y * other.x;
	temp.w = xyzw.w * other.w - xyzw.x * other.x - xyzw.y * other.y - xyzw.z * other.z; 
	
	xyzw = temp;
}

//---------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------

void TLMaths::TQuaternion::operator /= (const float& Scalar)
{
	SetValues( xyzw/Scalar );
}

//---------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------

void TLMaths::TQuaternion::operator *= (const float &Scalar)
{
	SetValues( xyzw*Scalar );
}


//---------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------

void TLMaths::TQuaternion::SetEuler(const float& Pitch, const float& Yaw, const float& Roll)
{
	
	// original method
	TLMaths::TQuaternion Qx( float3(1, 0, 0), Pitch );	Qx.Normalise();
	TLMaths::TQuaternion Qy( float3(0, 1, 0), Yaw );	Qy.Normalise();
	TLMaths::TQuaternion Qz( float3(0, 0, 1), Roll );	Qz.Normalise();
	
	SetValues(0,0,0,1);
	*this *= Qx;
	*this *= Qy;
	*this *= Qz;
	
	/*
	 
	 // Method from the internet
	 
	 TLMaths::TQuaternion Qx( float3(1, 0, 0), -Pitch );
	 TLMaths::TQuaternion Qy( float3(0, 1, 0), -Yaw );
	 TLMaths::TQuaternion Qz( float3(0, 0, 1), -Roll );
	 
	 Qz = Qy * Qz;
	 *this = Qx * Qz;
	 */
}



// Quaternion to Euler conversion
// Source: http://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToEuler/index.htm
// Returns a float 3 such that x = pitch, y = yaw and z = roll
float3 TLMaths::TQuaternion::GetEuler() const
{
	float3 vector;
	float test = xyzw.x*xyzw.y + xyzw.z*xyzw.w;
	
	if (test > 0.499f) 
	{ 
		// singularity at north pole
		vector.x = 0;
		vector.y = 2 * TLMaths::Atan2f(xyzw.x, xyzw.w);
		vector.z = HALF_PI;
		return vector;
	}
	if (test < -0.499f) 
	{ 
		// singularity at south pole
		vector.x = 0;
		vector.y = -2 * TLMaths::Atan2f(xyzw.x,xyzw.w);
		vector.z = - HALF_PI;
		return vector;
	}
	
    float sqx = xyzw.x*xyzw.x;
    float sqy = xyzw.y*xyzw.y;
    float sqz = xyzw.z*xyzw.z;
	
	vector.x = TLMaths::Atan2f(2*xyzw.x*xyzw.w-2*xyzw.y*xyzw.z , 1 - 2*sqx - 2*sqz);
    vector.y = TLMaths::Atan2f(2*xyzw.y*xyzw.w-2*xyzw.x*xyzw.z , 1 - 2*sqy - 2*sqz);
	vector.z = TLMaths::Asinf(2*test);
	return vector;
}



//---------------------------------------------------------------------------
//	xyzw multiply for quaternions
//---------------------------------------------------------------------------
float4 TLMaths::TQuaternion::QuatMult(const float4& First,const float4& Second)		
{	
	return float4( First.w * Second.x + First.x * Second.w + First.y * Second.z - First.z * Second.y,
				  First.w * Second.y + First.y * Second.w + First.z * Second.x - First.x * Second.z,
				  First.w * Second.z + First.z * Second.w + First.x * Second.y - First.y * Second.x,
				  First.w * Second.w - First.x * Second.x - First.y * Second.y - First.z * Second.z); 
}


//---------------------------------------------------------------------------
// Operadores externos
//---------------------------------------------------------------------------

TLMaths::TQuaternion operator * (const TLMaths::TQuaternion &First, const TLMaths::TQuaternion &Second)
{
	return TLMaths::TQuaternion( TLMaths::TQuaternion::QuatMult( First.xyzw, Second.xyzw ) );
}



//---------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------
TLMaths::TQuaternion operator * (TLMaths::TQuaternion Quaternion,float Scalar)
{
	return TLMaths::TQuaternion( Quaternion.xyzw * Scalar );
}

//---------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------
TLMaths::TQuaternion operator * (float Scalar,TLMaths::TQuaternion Quaternion)
{
	return TLMaths::TQuaternion( Quaternion.xyzw * Scalar );
}

//---------------------------------------------------------------------------
//  
//---------------------------------------------------------------------------
TLMaths::TQuaternion RotationArc(const float3& V0,const float3& V1)
{
	TLMaths::TQuaternion Quaternion;
	// v0.normalize(); 
	// v1.normalize();  // If vector is already unit length then why do it again?
	float3 Temp = V0.CrossProduct(V1);
	
	float   d = V0.DotProduct(V1);
	float   s = (float)sqrt((1+d)*2);
	Quaternion.xyzw.x = Temp.x / s;
	Quaternion.xyzw.y = Temp.y / s;
	Quaternion.xyzw.z = Temp.z / s;
	Quaternion.xyzw.w = s /2.0f;
	return Quaternion;
}


/*
 //---------------------------------------------------------------------------
 // 
 //---------------------------------------------------------------------------
 void TLMaths::TQuaternion::Slerp(const TLMaths::TQuaternion &From, const TLMaths::TQuaternion &To, const float& t)
 {
 TLMaths::TQuaternion Temp;
 float omega, cosO, sinO;
 float scale0, scale1;
 
 cosO = From.DotProduct(To);
 
 if (cosO < 0.0f)
 {
 cosO = -cosO;
 Temp = -To;
 }
 else
 {
 Temp = -To;
 }
 
 if ((1.0f - cosO) > 0.0001f)
 {
 omega = (float)acos(cosO);
 sinO = TLMaths::Sinf(omega);
 scale0 = TLMaths::Sinf((1.0f - t) * omega) / sinO;
 scale1 = TLMaths::Sinf(t * omega) / sinO;
 }
 else
 {
 scale0 = 1.0f - t;
 scale1 = t;
 }
 *this = From*scale0 + Temp*scale1 ;
 }
 */

////////////////////////////////////////////////////////////////////////////////////////////////
// DB - We seem to have quite a few quaternion slerp routines
// I've adde dthe following two from http://www.3dkingdoms.com/weekly/quat.h
// Very likely these are all similar or the same so we could do with tidying these up and removing
// ones we aren't going to use.
////////////////////////////////////////////////////////////////////////////////////////////////
// Spherical linear interpolation
void TLMaths::TQuaternion::Slerp(const TQuaternion& From, const TQuaternion& To, const float& t)
{
	float w1, w2;
	
	TQuaternion FlipTo = To;
	
	float fCosTheta = From.DotProduct(FlipTo);
	float fTheta = TLMaths::Acosf(fCosTheta);
	float fSinTheta = TLMaths::Sinf(fTheta);
	
	if ( fCosTheta < 0)
	{   
		FlipTo.Invert();
	}
	
	if( fSinTheta > 0.0001f)
	{
		w1 = TLMaths::Sinf( (1.0f - t) * fTheta ) / fSinTheta;
		w2 = TLMaths::Sinf( t*fTheta) / fSinTheta;
	}
	else
	{
		w1 = 1.0f - t;
		w2 = t;
	}
	
	*this = (From*w1) + (FlipTo*w2);
}

/*
 // Normalised linear interpolation
 void TLMaths::TQuaternion::Nlerp(const TQuaternion& From, const TQuaternion& To, const float& t)
 {
 float w1 = 1.0f - t;
 *this = (From*w1) + (To*t);
 Normalise();
 }
 */


////////////////////////////////////////////////////////////////////////////////////////////////
void TLMaths::TQuaternion::Nlerp(const TLMaths::TQuaternion& From, const TLMaths::TQuaternion& To, const float& t)
{
	if ( From == To )
	{
		*this = From;
		return;
	}
	
	//represent the same rotation, but when doing interpolation they are in different places in the space used for interpolation, and produce different results.
	
	//The solution it to check how far apart the quaternions are, and if necessary flip one so they are closer. E.g. add the lines.
	
	TLMaths::TQuaternion FlipTo( To );
	
	//	flip to, if neccesary
	//if (Q1.w * Q2.w + Q1.x * Q2.x + Q1.y * Q2.y + Q1.z * Q2.z < 0)
	if ( From.DotProduct( FlipTo ) < 0)
	{   
		//Q2.w = -Q2.w;    Q2.x = -Q2.x;    Q2.y = -Q2.y;    Q2.z = -Q2.z;
		FlipTo.Invert();
	}
	
	float mint = 1.f - t;
	const TLMaths::TQuaternion& Q1 = From;
	const TLMaths::TQuaternion& Q2 = FlipTo;
	
	// Q = Q1*(1-Interpolation) + Q2*Interpolation;   
	
	//	interp
	float4 q1 = Q1.xyzw * mint;
	float4 q2 = Q2.xyzw * t;
	
	*this = ( q1 + q2 );
	Normalise();
}



/*
 TLMaths::TQuaternion Slerp(TLMaths::TQuaternion &From, TLMaths::TQuaternion &To, float Interpolation, Bool AllowFlip)
 { 
 float cosAngle = (From[0]*To[0]) + (From[1]*To[1]) + (From[2]*To[2]) + (From[3]*To[3]);
 
 float c1, c2; 
 
 //	Linear interpolation for close orientations 
 if ((1.0 - fabsf(cosAngle)) < 0.01)  
 { 
 c1 = 1.0f - Interpolation; 
 c2 = Interpolation; 
 } 
 else
 {  
 // Spherical interpolation 
 float angle    = TLMaths::Acosf(fabsf(cosAngle)); 
 float sinAngle = TLMaths::Sinf(angle); 
 c1 = TLMaths::Sinf(angle * (1.0f - Interpolation)) / sinAngle; 
 c2 = TLMaths::Sinf(angle * Interpolation) / sinAngle; 
 } 
 
 // Use the shortest path 
 if ((cosAngle < 0.0) && AllowFlip) 
 c1 = -c1; 
 
 return TLMaths::TQuaternion(c1*From[0] + c2*To[0], c1*From[1] + c2*To[1], c1*From[2] + c2*To[2], c1*From[3] + c2*To[3]); 
 } 
 */  


/*
 TLMaths::TQuaternion Slerp(TLMaths::TQuaternion& From, TLMaths::TQuaternion& To, float Interpolation, Bool AllowFlip)
 {
 TLMaths::TQuaternion Result;
 
 // Decide if one of the quaternions is backwards
 float a = 0, b = 0;
 
 a += ( From.xyzw.x-To.xyzw.x )*( From.xyzw.x-To.xyzw.x );
 a += ( From.xyzw.y-To.xyzw.y )*( From.xyzw.y-To.xyzw.y );
 a += ( From.xyzw.z-To.xyzw.z )*( From.xyzw.x-To.xyzw.z );
 a += ( From.xyzw.y-To.xyzw.w )*( From.xyzw.x-To.xyzw.w );
 
 b += ( From.xyzw.x+To.xyzw.x )*( From.xyzw.x+To.xyzw.x );
 b += ( From.xyzw.y+To.xyzw.y )*( From.xyzw.y+To.xyzw.y );
 b += ( From.xyzw.z+To.xyzw.z )*( From.xyzw.z+To.xyzw.z );
 b += ( From.xyzw.w+To.xyzw.w )*( From.xyzw.w+To.xyzw.w );
 
 if ( a > b )
 To.Invert();
 
 //	gr: this is dot product
 //	float cosom = From[0]*To[0]+From[1]*To[1]+From[2]*To[2]+From[3]*To[3];
 float cosom = From.xyzw.DotProduct( To.xyzw );
 float sclFrom, sclTo;
 
 if (( 1.0f+cosom ) > 0.00000001f )
 {
 if (( 1.0f-cosom ) > 0.00000001f )
 {
 float omega = TLMAths::Acosf( cosom );
 float sinom = TLMaths::Sinf( omega );
 sclFrom = TLMaths::Sinf(( 1.0f-Interpolation )*omega )/sinom;
 sclTo = TLMaths::Sinf( Interpolation*omega )/sinom;
 }
 else
 {
 sclFrom = 1.0f-Interpolation;
 sclTo = Interpolation;
 }
 
 TLDebug_CheckFloat(sclFrom);
 TLDebug_CheckFloat(sclTo);
 
 Result.xyzw.x = (sclFrom*From.xyzw.x) + (sclTo*To.xyzw.x);
 Result.xyzw.y = (sclFrom*From.xyzw.y) + (sclTo*To.xyzw.y);
 Result.xyzw.z = (sclFrom*From.xyzw.z) + (sclTo*To.xyzw.z);
 Result.xyzw.w = (sclFrom*From.xyzw.w) + (sclTo*To.xyzw.w);
 }
 else
 {
 Result.xyzw.x = -From.xyzw.y;
 Result.xyzw.y =  From.xyzw.x;
 Result.xyzw.z = -From.xyzw.w;
 Result.xyzw.w =  From.xyzw.z;
 
 sclFrom = TLMaths::Sinf(( 1.0f-Interpolation )*0.5f*PI );
 sclTo = TLMaths::Sinf( Interpolation*0.5f*PI );
 
 Result.xyzw.x = (sclFrom*From.xyzw.x) + (sclTo*Result.xyzw.x);
 Result.xyzw.y = (sclFrom*From.xyzw.y) + (sclTo*Result.xyzw.y);
 Result.xyzw.z = (sclFrom*From.xyzw.z) + (sclTo*Result.xyzw.z);
 //	gr: no w?
 }
 
 return Result;
 }
 */

/*
 
 #define kX	0
 #define kY	1
 #define kZ	2
 #define kW	3
 
 
 
 void QuaternionToMatrix(const TLMaths::TQuaternion &Quaternion,TLMaths::TMatrix &Matrix)
 {
 
 //If Quaternion is guaranteed to be a unit CQuaternion, s will always
 //be 1.  In that case, this calculation can be optimized out.
 
 float norm,s,xx,yy,zz,xy,xz,yz,wx,wy,wz;
 norm = Quaternion.x*Quaternion.x + Quaternion.y*Quaternion.y + Quaternion.z*Quaternion.z + Quaternion.w*Quaternion.w;
 s = (norm > 0) ? 2/norm : 0;
 
 //Precalculate coordinate products
 
 xx = Quaternion.x * Quaternion.x * s;
 yy = Quaternion.y * Quaternion.y * s;
 zz = Quaternion.z * Quaternion.z * s;
 xy = Quaternion.x * Quaternion.y * s;
 xz = Quaternion.x * Quaternion.z * s;
 yz = Quaternion.y * Quaternion.z * s;
 wx = Quaternion.w * Quaternion.x * s;
 wy = Quaternion.w * Quaternion.y * s;
 wz = Quaternion.w * Quaternion.z * s;
 
 //Calculate 3x3 matrix from orthonormal basis
 
 //x axis
 
 Matrix.GetCol(kX)[kX] = 1.0f - (yy + zz);
 Matrix.GetCol(kY)[kX] = xy + wz;
 Matrix.GetCol(kZ)[kX] = xz - wy;
 
 //y axis
 
 Matrix.GetCol(kX)[kY] = xy - wz;
 Matrix.GetCol(kY)[kY] = 1.0f - (xx + zz);
 Matrix.GetCol(kZ)[kY] = yz + wx;
 
 //z axis
 
 Matrix.GetCol(kX)[kZ] = xz + wy;
 Matrix.GetCol(kY)[kZ] = yz - wx;
 Matrix.GetCol(kZ)[kZ] = 1.0f - (xx + yy);
 
 
 //4th row and column of 4x4 matrix
 //Translation and scale are not stored in quaternions, so these
 //values are set to default (no scale, no translation).
 //For systems where Matrix comes pre-loaded with scale and translation
 //factors, this code can be excluded.
 
 Matrix.GetCol(kW)[kX] = 
 Matrix.GetCol(kW)[kY] = 
 Matrix.GetCol(kW)[kZ] = 
 Matrix.GetCol(kX)[kW] = 
 Matrix.GetCol(kY)[kW] = 
 Matrix.GetCol(kZ)[kW] = 0.0;
 Matrix.GetCol(kW)[kW] = 1.0f;
 }
 
 //---------------------------------------------------------------------------
 //  
 //---------------------------------------------------------------------------
 
 void MatrixToQuaternion(const GMatrix &Matrix,TLMaths::TQuaternion &Quaternion)
 {
 
 //This code can be optimized for Matrix[kW][kW] = 1, which 
 //should always be true.  This optimization is excluded
 //here for clarity.
 
 
 float Tr = Matrix.GetColConst(kX)[kX] + Matrix.GetColConst(kY)[kY] + Matrix.GetColConst(kZ)[kZ] + Matrix.GetColConst(kW)[kW],fourD;
 int i,j,k;
 
 //w >= 0.5 ?
 
 if (Tr >= 1.0f)
 {
 fourD = 2.0f * sqrtf(Tr);
 Quaternion.w = fourD/4.0f;
 Quaternion.x = (Matrix.GetColConst(kZ)[kY] - Matrix.GetColConst(kY)[kZ])/fourD;
 Quaternion.y = (Matrix.GetColConst(kX)[kZ] - Matrix.GetColConst(kZ)[kX])/fourD;
 Quaternion.z = (Matrix.GetColConst(kY)[kX] - Matrix.GetColConst(kX)[kY])/fourD;
 }
 else
 {
 //Find the largest component.  
 
 if (Matrix.GetColConst(kX)[kX] > Matrix.GetColConst(kY)[kY])
 {
 i = kX;
 }
 else
 {
 i = kY;
 }
 
 if (Matrix.GetColConst(kZ)[kZ] > Matrix.GetColConst(i)[i])
 {
 i = kZ;
 }
 
 //Set j and k to point to the next two components
 
 j = (i+1)%3;
 k = (j+1)%3;
 
 //fourD = 4 * largest component
 
 fourD = 2.0f * sqrtf(Matrix.GetColConst(i)[i] - Matrix.GetColConst(j)[j] - Matrix.GetColConst(k)[k] + 1.0 );
 
 //Set the largest component
 switch (i)
 {
 case 0:Quaternion.x=fourD/4.0f;
 case 1:Quaternion.y=fourD/4.0f;
 case 2:Quaternion.z=fourD/4.0f;
 case 3:Quaternion.w=fourD/4.0f;
 }
 
 //Calculate remaining components
 switch (j)
 {
 case 0:Quaternion.x=(Matrix.GetColConst(j)[i] + Matrix.GetColConst(i)[j])/fourD;
 case 1:Quaternion.y=(Matrix.GetColConst(j)[i] + Matrix.GetColConst(i)[j])/fourD;
 case 2:Quaternion.z=(Matrix.GetColConst(j)[i] + Matrix.GetColConst(i)[j])/fourD;
 case 3:Quaternion.w=(Matrix.GetColConst(j)[i] + Matrix.GetColConst(i)[j])/fourD;
 }
 
 switch (k)
 {
 case 0:Quaternion.x=(Matrix.GetColConst(j)[i] + Matrix.GetColConst(i)[k])/fourD;
 case 1:Quaternion.y=(Matrix.GetColConst(j)[i] + Matrix.GetColConst(i)[k])/fourD;
 case 2:Quaternion.z=(Matrix.GetColConst(j)[i] + Matrix.GetColConst(i)[k])/fourD;
 case 3:Quaternion.w=(Matrix.GetColConst(j)[i] + Matrix.GetColConst(i)[k])/fourD;
 }
 
 Quaternion.w = (Matrix.GetColConst(k)[j] - Matrix.GetColConst(j)[k])/fourD;
 }
 }
 
 
 
 #undef kX
 #undef kY
 #undef kZ
 #undef kW
 
 */


void TLMaths::TQuaternion::LookAt(const float3& Dir,const float3& WorldUp)
{
	/*
	 GMatrix m;
	 
	 float3 z(Dir);
	 z.Normalise();
	 
	 float3 x = WorldUp.CrossProduct(z); // x = y cross z
	 x.Normalise();
	 
	 float3 y = z.CrossProduct( x ); // y = z cross x
	 
	 float4 w( 0, 0, 0, 0 );
	 
	 m.SetCol( 0, x, 0.f );
	 m.SetCol( 1, y, 0.f );
	 m.SetCol( 2, z, 0.f );
	 m.SetCol( 3, w );
	 
	 MatrixToQuaternion( m, *this );
	 
	 
	 /*
	 float3 z(Dir); 
	 
	 //z.norm(); 
	 float3 x( WorldUp * z ); // x = y cross z 
	 
	 //x.norm(); 
	 float3 y( z * x ); // y = z cross x 
	 
	 // here vectors are perpendicular ich other. 
	 float tr = x.x + y.y + z.z; 
	 Set( float3( y.z - z.y , z.x - x.z, x.y - y.x ), tr + 1.0f ); 
	 
	 //if we could multiply our vectors that thay became the same length or multiplay the quaternion components that it all quat was scaled by some scale, than we have only one SQRT. The question is how to find that multipliers ? may be you can solve this ? Remember that we can easy get dot product between "up" and "z" 
	 Normalise(); 
	 
	 
	 */
	float3 DirNorm = Dir.Normal();
	float3 WorldUpNorm = WorldUp.Normal();
	
	if ( DirNorm == WorldUpNorm )
	{
		SetIdentity();
		return;
	}
	
	float3 crossZ( DirNorm );
	float3 crossX = WorldUpNorm.CrossProduct( crossZ ); // x = y cross z 
	crossX.Normalise();
	float3 crossY = crossZ.CrossProduct( crossX );	// y = z cross x 
	
	// here vectors are perpendicular ich other. 
	float tr = crossX.x + crossY.y + crossZ.z; 
	Set( float3( crossY.z - crossZ.y , crossZ.x - crossX.z, crossX.y - crossY.x ), tr + 1.0f ); 
	
	//if we could multiply our vectors that thay became the same length or multiplay the quaternion components that it all quat was scaled by some scale, than we have only one SQRT. The question is how to find that multipliers ? may be you can solve this ? Remember that we can easy get dot product between "up" and "z" 
	Normalise(); 
	
}

void TLMaths::TQuaternion::RotateVector(float3& Vector) const
{
	if ( !IsValid() )
		return;
	
	TLMaths::TMatrix RotationMatrix;
	RotationMatrix.SetRotation(*this);
	RotationMatrix.TransformVector( Vector );
	/*	
	 float3 This3 = float3( x, y, z );
	 //<0,w> = inv(q) * <0,v> * q
	 TLMaths::TQuaternion Inverse(*this);
	 Inverse.Invert();
	 
	 Vector = Inverse * Vector * This3;
	 */	
}

void TLMaths::TQuaternion::UnrotateVector(float3& Vector) const
{
	if ( !IsValid() )
		return;
	
	//	gr: slow here... speed up!
	TLMaths::TMatrix RotationMatrix;
	RotationMatrix.SetRotation(*this);
	RotationMatrix.Invert();
	RotationMatrix.TransformVector( Vector );
	/*	
	 float3 This3 = float3( x, y, z );
	 //<0,w> = inv(q) * <0,v> * q
	 TLMaths::TQuaternion Inverse(*this);
	 Inverse.Invert();
	 
	 Vector = Inverse * Vector * This3;
	 */	
}

void TLMaths::TQuaternion::RotateVector(float2& Vector) const
{
	if ( !IsValid() )
		return;
	
	//	gr: slow here... speed up!
	TLMaths::TMatrix RotationMatrix;
	RotationMatrix.SetRotation(*this);
	RotationMatrix.TransformVector( Vector );
	/*	
	 float3 This3 = float3( x, y, z );
	 //<0,w> = inv(q) * <0,v> * q
	 TLMaths::TQuaternion Inverse(*this);
	 Inverse.Invert();
	 
	 Vector = Inverse * Vector * This3;
	 */	
}

void TLMaths::TQuaternion::UnrotateVector(float2& Vector) const
{
	if ( !IsValid() )
		return;
	
	//	gr: slow here... speed up!
	TLMaths::TMatrix RotationMatrix;
	RotationMatrix.SetRotation(*this);
	RotationMatrix.Invert();
	RotationMatrix.TransformVector( Vector );
	/*	
	 float3 This3 = float3( x, y, z );
	 //<0,w> = inv(q) * <0,v> * q
	 TLMaths::TQuaternion Inverse(*this);
	 Inverse.Invert();
	 
	 Vector = Inverse * Vector * This3;
	 */	
}


//-----------------------------------------------------------
//	extract a eular angle in degrees from the quaternion. Is is based on an axis of 0,0,1. probably better ways to do it to get 3D angles...
//-----------------------------------------------------------
TLMaths::TAngle TLMaths::TQuaternion::GetAngle2D() const
{
	float2 Up( 0.f, -1.f );
	RotateVector( Up );
	
	TLMaths::TAngle Result;
	Result.SetAngle( Up );
	
	return Result;
}

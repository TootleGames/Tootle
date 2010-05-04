/*
 *  TMatrix.cpp
 *  TootleCore
 *
 *  Created by Duane Bradbury on 03/05/2010.
 *  Copyright 2010 Tootle. All rights reserved.
 *
 */

#include "TMatrix.h"

#include "TLMaths.h"
#include "TLMemory.h"

#include "TString.h"


#include "TQuaternion.h" // Needed for QuaternionToMatrix & MatrixToQuaternion

namespace TLMaths
{
	TMatrix		g_IdentityMatrix( float4(1,0,0,0), float4(0,1,0,0), float4(0,0,1,0), float4(0,0,0,1) );	//	identity matrix
}



void TLMaths::QuaternionToMatrix(const TLMaths::TQuaternion& Quaternion,TLMaths::TMatrix& Matrix)
{
#define kX	0
#define kY	1
#define kZ	2
#define kW	3
	/*
	 If Quaternion is guaranteed to be a unit CQuaternion, s will always
	 be 1.  In that case, this calculation can be optimized out.
	 */
	float norm,s,xx,yy,zz,xy,xz,yz,wx,wy,wz;
	const float4& Quat = Quaternion.xyzw;
	//	norm = Quaternion.x*Quaternion.x + Quaternion.y*Quaternion.y + Quaternion.z*Quaternion.z + Quaternion.w*Quaternion.w;
	norm = Quat.DotProduct( Quat );
	s = (norm > 0) ? 2/norm : 0;
	
	/*
	 Precalculate coordinate products
	 */
	xx = Quat.x * Quat.x * s;
	yy = Quat.y * Quat.y * s;
	zz = Quat.z * Quat.z * s;
	xy = Quat.x * Quat.y * s;
	xz = Quat.x * Quat.z * s;
	yz = Quat.y * Quat.z * s;
	wx = Quat.w * Quat.x * s;
	wy = Quat.w * Quat.y * s;
	wz = Quat.w * Quat.z * s;
	
	/*
	 Calculate 3x3 matrix from orthonormal basis
	 */
	
	/*
	 x axis
	 */
	Matrix.GetCol(kX)[kX] = 1.0f - (yy + zz);
	Matrix.GetCol(kY)[kX] = xy + wz;
	Matrix.GetCol(kZ)[kX] = xz - wy;
	
	/*
	 y axis
	 */
	Matrix.GetCol(kX)[kY] = xy - wz;
	Matrix.GetCol(kY)[kY] = 1.0f - (xx + zz);
	Matrix.GetCol(kZ)[kY] = yz + wx;
	
	/*
	 z axis
	 */
	Matrix.GetCol(kX)[kZ] = xz + wy;
	Matrix.GetCol(kY)[kZ] = yz - wx;
	Matrix.GetCol(kZ)[kZ] = 1.0f - (xx + yy);
	
	/*
	 4th row and column of 4x4 matrix
	 Translation and scale are not stored in quaternions, so these
	 values are set to default (no scale, no translation).
	 For systems where Matrix comes pre-loaded with scale and translation
	 factors, this code can be excluded.
	 */
	Matrix.GetCol(kW)[kX] = 
	Matrix.GetCol(kW)[kY] = 
	Matrix.GetCol(kW)[kZ] = 
	Matrix.GetCol(kX)[kW] = 
	Matrix.GetCol(kY)[kW] = 
	Matrix.GetCol(kZ)[kW] = 0.0f;
	Matrix.GetCol(kW)[kW] = 1.0f;
	
#undef kX
#undef kY
#undef kZ
#undef kW
	
}


void TLMaths::MatrixToQuaternion(const TLMaths::TMatrix& Matrix,TLMaths::TQuaternion& Quaternion)
{
#define kX	0
#define kY	1
#define kZ	2
#define kW	3
	/*
	 This code can be optimized for Matrix[kW][kW] = 1, which 
	 should always be true.  This optimization is excluded
	 here for clarity.
	 */
	
	float4& Quat = Quaternion.xyzw;
	
	float Tr = Matrix.GetCol(kX)[kX] + Matrix.GetCol(kY)[kY] + Matrix.GetCol(kZ)[kZ] + Matrix.GetCol(kW)[kW],fourD;
	int i,j,k;
	
	/*
	 w >= 0.5 ?
	 */
	if (Tr >= 1.0f)
	{
		fourD = 2.0f * sqrtf(Tr);
		Quat.w = fourD/4.0f;
		Quat.x = (Matrix.GetCol(kZ)[kY] - Matrix.GetCol(kY)[kZ])/fourD;
		Quat.y = (Matrix.GetCol(kX)[kZ] - Matrix.GetCol(kZ)[kX])/fourD;
		Quat.z = (Matrix.GetCol(kY)[kX] - Matrix.GetCol(kX)[kY])/fourD;
	}
	else
	{
		/*
		 Find the largest component.  
		 */
		if (Matrix.GetCol(kX)[kX] > Matrix.GetCol(kY)[kY])
		{
			i = kX;
		}
		else
		{
			i = kY;
		}
		
		if (Matrix.GetCol(kZ)[kZ] > Matrix.GetCol(i)[i])
		{
			i = kZ;
		}
		
		/*
		 Set j and k to point to the next two components
		 */
		j = (i+1)%3;
		k = (j+1)%3;
		
		/*
		 fourD = 4 * largest component
		 */
		fourD = 2.0f * sqrtf(Matrix.GetCol(i)[i] - Matrix.GetCol(j)[j] - Matrix.GetCol(k)[k] + 1.0f );
		
		/*
		 Set the largest component
		 */
		switch (i)
		{
			case 0:Quat.x=fourD/4.0f;
			case 1:Quat.y=fourD/4.0f;
			case 2:Quat.z=fourD/4.0f;
			case 3:Quat.w=fourD/4.0f;
		}
		
		/*
		 Calculate remaining components
		 */
		switch (j)
		{
			case 0:Quat.x=(Matrix.GetCol(j)[i] + Matrix.GetCol(i)[j])/fourD;
			case 1:Quat.y=(Matrix.GetCol(j)[i] + Matrix.GetCol(i)[j])/fourD;
			case 2:Quat.z=(Matrix.GetCol(j)[i] + Matrix.GetCol(i)[j])/fourD;
			case 3:Quat.w=(Matrix.GetCol(j)[i] + Matrix.GetCol(i)[j])/fourD;
		}
		
		switch (k)
		{
			case 0:Quat.x=(Matrix.GetCol(j)[i] + Matrix.GetCol(i)[k])/fourD;
			case 1:Quat.y=(Matrix.GetCol(j)[i] + Matrix.GetCol(i)[k])/fourD;
			case 2:Quat.z=(Matrix.GetCol(j)[i] + Matrix.GetCol(i)[k])/fourD;
			case 3:Quat.w=(Matrix.GetCol(j)[i] + Matrix.GetCol(i)[k])/fourD;
		}
		
		Quat.w = (Matrix.GetCol(k)[j] - Matrix.GetCol(j)[k])/fourD;
	}
	
#undef kX
#undef kY
#undef kZ
#undef kW
}




void TLMaths::TMatrix::Copy(const TMatrix& Matrix)			
{
	TLMemory::CopyData( GetData(), Matrix.GetData(), 16 );
}

void TLMaths::TMatrix::Copy(const float* pFloats)			
{
	TLMemory::CopyData( GetData(), pFloats, 16 );
}


Bool TLMaths::TMatrix::Compare(const TMatrix& Matrix) const
{
	return (GetCol(0) == Matrix.GetCol(0)) &&
	(GetCol(1) == Matrix.GetCol(1)) &&
	(GetCol(2) == Matrix.GetCol(2)) &&
	(GetCol(3) == Matrix.GetCol(3)) ;
}


void TLMaths::TMatrix::SetIdentity()
{
	Copy( g_IdentityMatrix );
}

Bool TLMaths::TMatrix::IsIdentity() const
{
	return Compare( g_IdentityMatrix );
}





//---------------------------------------------------------------------------
//  
//---------------------------------------------------------------------------
TLMaths::TMatrix::TMatrix(float Value)
{
    GetCol(0).Set(Value, Value, Value, Value);
    GetCol(1).Set(Value, Value, Value, Value);
    GetCol(2).Set(Value, Value, Value, Value);
    GetCol(3).Set(Value, Value, Value, Value);
	
#ifdef DEBUG_MATRIX
	m_Initialised = TRUE;
#endif
}


//---------------------------------------------------------------------------
//  
//---------------------------------------------------------------------------
TLMaths::TMatrix::TMatrix(const float4& col0, const float4& col1, const float4& col2, const float4& col3)
{
	m_Column[0] = col0;
	m_Column[1] = col1;
	m_Column[2] = col2;
	m_Column[3] = col3;
}


//---------------------------------------------------------------------------
//  Operadores internos
//---------------------------------------------------------------------------
/*float4 &TLMaths::TMatrix::operator [] (unsigned int Index)
 {
 GDebug_CheckIndex( Index, 0, 4 );
 return Column[Index];
 }
 
 //---------------------------------------------------------------------------
 //  
 //---------------------------------------------------------------------------
 const float4 &TLMaths::TMatrix::operator [] (unsigned int Index) const
 {
 GDebug_CheckIndex( Index, 0, 4 );
 return Column[Index];
 }*/


//---------------------------------------------------------------------------
//  
//---------------------------------------------------------------------------
void TLMaths::TMatrix::operator += (const TLMaths::TMatrix &Other)
{
	GetCol(0) += Other[0];
	GetCol(1) += Other[1];
	GetCol(2) += Other[2];
	GetCol(3) += Other[3];
}

//---------------------------------------------------------------------------
//  
//---------------------------------------------------------------------------
void TLMaths::TMatrix::operator -= (const TLMaths::TMatrix &Other)
{
	GetCol(0) -= Other[0];
	GetCol(1) -= Other[1];
	GetCol(2) -= Other[2];
	GetCol(3) -= Other[3];
}

//---------------------------------------------------------------------------
//  
//---------------------------------------------------------------------------
void TLMaths::TMatrix::operator *= (const TLMaths::TMatrix &Matrix)
{
#ifdef USE_OPENGL_MATRIX_MULT
	
	glPushMatrix();
	glLoadMatrixf(Float());
	glMultMatrixf(Matrix);
	glGetFloatv( GL_MODELVIEW_MATRIX, Float() );
	glPopMatrix();
	
#else
	
	TLMaths::TMatrix t;
	for ( int r = 0; r < 4; r++)
	{
		for ( int c = 0; c < 4; c++)
		{
			float f = 0;
			
			f += (GetCol(0)[r]) * (Matrix.GetCol(c)[0]);
			f += (GetCol(1)[r]) * (Matrix.GetCol(c)[1]);
			f += (GetCol(2)[r]) * (Matrix.GetCol(c)[2]);
			f += (GetCol(3)[r]) * (Matrix.GetCol(c)[3]);
			t.GetCol(c)[r] = f;
		}
	}
	
	*this = t;
	
#endif
}

//---------------------------------------------------------------------------
//  
//---------------------------------------------------------------------------
void TLMaths::TMatrix::operator *= (float Float)
{
	GetCol(0) *= Float;
	GetCol(1) *= Float;
	GetCol(2) *= Float;
	GetCol(3) *= Float;
}

//---------------------------------------------------------------------------
//  
//---------------------------------------------------------------------------
void TLMaths::TMatrix::operator /= (float Float)
{
	GetCol(0) /= Float;
	GetCol(1) /= Float;
	GetCol(2) /= Float;
	GetCol(3) /= Float;
}

//---------------------------------------------------------------------------
//  
//---------------------------------------------------------------------------
TLMaths::TMatrix operator + (const TLMaths::TMatrix &V1, const TLMaths::TMatrix &V2)
{
	TLMaths::TMatrix Return(V1);
	Return += V2;
	return Return;
}

//---------------------------------------------------------------------------
//  
//---------------------------------------------------------------------------
TLMaths::TMatrix operator - (const TLMaths::TMatrix &V1, const TLMaths::TMatrix &V2)
{
	TLMaths::TMatrix ret(V1);
	ret -= V2;
	return ret;
}

//---------------------------------------------------------------------------
//  
//---------------------------------------------------------------------------
TLMaths::TMatrix operator * (const TLMaths::TMatrix &V1, const TLMaths::TMatrix &V2)
{
#ifdef USE_OPENGL_MATRIX_MULT
	{
		glPushMatrix();
		glLoadMatrixf(V1);
		glMultMatrixf(V2);
		TLMaths::TMatrix Result;
		glGetFloatv( GL_MODELVIEW_MATRIX, Result );
		glPopMatrix();
		
		return Result;
	}
#else
	{
		TLMaths::TMatrix ret(V1);
		ret *= V2;
		return ret;
	}
#endif
}

/*
 //---------------------------------------------------------------------------
 //  
 //---------------------------------------------------------------------------
 float3 operator * (const TLMaths::TMatrix& Other, const float3& V1)
 {
 //	float4 ret( V1.x, V1.y, V1.z, 0.f );
 float4 ret( V1 );
 ret = Other * ret;
 return float3(ret.x, ret.y, ret.z);
 }
 
 //---------------------------------------------------------------------------
 //  
 //---------------------------------------------------------------------------
 float3 operator * (const float3 &V1, const TLMaths::TMatrix &Other)
 {
 float4 ret(V1);
 ret = ret * Other;
 return float3(ret.x, ret.y, ret.z);
 }
 
 //---------------------------------------------------------------------------
 //  
 //---------------------------------------------------------------------------
 
 float4 operator * (const TLMaths::TMatrix &Other, const float4 &V1)
 {
 float4 ret;
 ret.x = V1.x * Other.GetCol(0)[0] + V1.y * Other.GetCol(1)[0] + V1.z * Other.GetCol(2)[0] + V1.w * Other.GetCol(3)[0];
 ret.y = V1.x * Other.GetCol(0)[1] + V1.y * Other.GetCol(1)[1] + V1.z * Other.GetCol(2)[1] + V1.w * Other.GetCol(3)[1];
 ret.z = V1.x * Other.GetCol(0)[2] + V1.y * Other.GetCol(1)[2] + V1.z * Other.GetCol(2)[2] + V1.w * Other.GetCol(3)[2];
 ret.w = V1.x * Other.GetCol(0)[3] + V1.y * Other.GetCol(1)[3] + V1.z * Other.GetCol(2)[3] + V1.w * Other.GetCol(3)[3];
 return ret;
 }
 
 //---------------------------------------------------------------------------
 //  
 //---------------------------------------------------------------------------
 float4 operator * (const float4 &V1, const TLMaths::TMatrix &Other)
 {
 float4 ret;
 ret.x = Other.GetCol(0).DotProduct( V1 );
 ret.y = Other.GetCol(1).DotProduct( V1 );
 ret.z = Other.GetCol(2).DotProduct( V1 );
 ret.w = Other.GetCol(3).DotProduct( V1 );
 return ret;
 }
 
 //---------------------------------------------------------------------------
 //  
 //---------------------------------------------------------------------------
 
 TLMaths::TMatrix operator * (const TLMaths::TMatrix &Other, float f)
 {
 TLMaths::TMatrix ret(Other);
 ret *= f;
 return ret;
 }
 
 //---------------------------------------------------------------------------
 //  
 //---------------------------------------------------------------------------
 TLMaths::TMatrix operator * (float f, const TLMaths::TMatrix &Other)
 {
 TLMaths::TMatrix ret(Other);
 ret *= f;
 return ret;
 }
 
 */

//---------------------------------------------------------------------------
//  
//---------------------------------------------------------------------------
void TLMaths::TMatrix::Transpose()
{
	float t;
	
	for ( int c = 0; c < 4; c++)
	{
		for ( int r = c + 1; r < 4; r++)
		{
			t = GetCol(c)[r];
			GetCol(c)[r] = GetCol(r)[c];
			GetCol(r)[c] = t;
		} 
	} 
}

//---------------------------------------------------------------------------
//  
//---------------------------------------------------------------------------
void TLMaths::TMatrix::Invert()
{
	TLMaths::TMatrix V1( *this );
	TLMaths::TMatrix V2( g_IdentityMatrix );
	
	int r, c;
	int cc;
	int RowMax; // Points to max abs value row in this column
	int row;
	float tmp;
	
	// Go through columns
	for (c=0; c<4; c++)
	{
		// Find the row with max value in this column
		RowMax = c;
		for (r=c+1; r<4; r++)
		{
			if (fabs(V1.GetCol(c)[r]) > fabs(V1.GetCol(c)[RowMax]))
			{
				RowMax = r;
			}
		}
		
		// If the max value here is 0, we can't invert.  Return identity.
		if (V1.GetCol(RowMax)[c] == 0.0F)
		{
			SetIdentity();
			return;
		}
		
		// Swap row "RowMax" with row "c"
		for (cc=0; cc<4; cc++)
		{
			tmp = V1.GetCol(cc)[c];
			V1.GetCol(cc)[c] = V1.GetCol(cc)[RowMax];
			V1.GetCol(cc)[RowMax] = tmp;
			tmp = V2.GetCol(cc)[c];
			V2.GetCol(cc)[c] = V2.GetCol(cc)[RowMax];
			V2.GetCol(cc)[RowMax] = tmp;
		}
		
		// Now everything we do is on row "c".
		// Set the max cell to 1 by dividing the entire row by that value
		tmp = V1.GetCol(c)[c];
		for (cc=0; cc<4; cc++)
		{
			V1.GetCol(cc)[c] /= tmp;
			V2.GetCol(cc)[c] /= tmp;
		}
		
		// Now do the other rows, so that this column only has V1 1 and 0's
		for (row = 0; row < 4; row++)
		{
			if (row != c)
			{
				tmp = V1.GetCol(c)[row];
				for (cc=0; cc<4; cc++)
				{
					V1.GetCol(cc)[row] -= V1.GetCol(cc)[c] * tmp;
					V2.GetCol(cc)[row] -= V2.GetCol(cc)[c] * tmp;
				}
			}
		}
	}
	
	*this = V2;
}


void TLMaths::TMatrix::Rotate(const TLMaths::TQuaternion& q)
{
	//	make a matrix to multiply by
	TLMaths::TMatrix RotMatrix;
	QuaternionToMatrix( q, RotMatrix );
	
	//	have to do this twice...
	*this *= RotMatrix;
	*this *= RotMatrix;
}


void TLMaths::TMatrix::SetRotation(const TLMaths::TQuaternion& q)
{
	QuaternionToMatrix( q, *this );
}



void TLMaths::TMatrix::SetTranslate(const float3& xyz) 
{
	//SetIdentity();
	GetCol(3)[0] = xyz.x;
	GetCol(3)[1] = xyz.y;
	GetCol(3)[2] = xyz.z;
}



void TLMaths::TMatrix::InverseTranslateVect(float3& v)
{
	v[0] = v[0] - Get(12);
	v[1] = v[1] - Get(13);
	v[2] = v[2] - Get(14);
}


void TLMaths::TMatrix::InverseRotateVect(float3& v)
{
	float3 vec;
	
	vec[0] = v[0] * Get(0) + v[1] * Get(1) + v[2] * Get(2);
	vec[1] = v[0] * Get(4) + v[1] * Get(5) + v[2] * Get(6);
	vec[2] = v[0] * Get(8) + v[1] * Get(9) + v[2] * Get(10);
	
	v = vec;
	//TLMemory::CopyData( &v, &vec, 1 );
}


void TLMaths::TMatrix::TransformVector(float3& v) const
{
	float vector[4];
	
	vector[0] = v[0]*Get(0)+v[1]*Get(4)+v[2]*Get(8)+Get(12);
	vector[1] = v[0]*Get(1)+v[1]*Get(5)+v[2]*Get(9)+Get(13);
	vector[2] = v[0]*Get(2)+v[1]*Get(6)+v[2]*Get(10)+Get(14);
	vector[3] = v[0]*Get(3)+v[1]*Get(7)+v[2]*Get(11)+Get(15);
	
	v[0] = vector[0];
	v[1] = vector[1];
	v[2] = vector[2];
	//	v[3] = vector[3];
}


void TLMaths::TMatrix::TransformVector(float2& v) const
{
	float vector[4];
	
	//	TLDebug_Break("Not tested");
	//	vector[0] = v[0]*Get(0)+v[1]*Get(4)+v[2]*Get(8)+Get(12);
	//	vector[1] = v[0]*Get(1)+v[1]*Get(5)+v[2]*Get(9)+Get(13);
	vector[0] = v[0]*Get(0)+v[1]*Get(4)+Get(12);
	vector[1] = v[0]*Get(1)+v[1]*Get(5)+Get(13);
	
	v[0] = vector[0];
	v[1] = vector[1];
}


void TLMaths::TMatrix::SetRotationRadians(const float3& Angles)
{
	float cr = TLMaths::Cosf( Angles[0] );
	float sr = TLMaths::Sinf( Angles[0] );
	float cp = TLMaths::Cosf( Angles[1] );
	float sp = TLMaths::Sinf( Angles[1] );
	float cy = TLMaths::Cosf( Angles[2] );
	float sy = TLMaths::Sinf( Angles[2] );
	
	SetIdentity();
	
	Get(0) = ( float )( cp*cy );
	Get(1) = ( float )( cp*sy );
	Get(2) = ( float )( -sp );
	
	float srsp = sr*sp;
	float crsp = cr*sp;
	
	Get(4) = ( float )( srsp*cy-cr*sy );
	Get(5) = ( float )( srsp*sy+cr*cy );
	Get(6) = ( float )( sr*cp );
	
	Get(8) = ( float )( crsp*cy+sr*sy );
	Get(9) = ( float )( crsp*sy-sr*cy );
	Get(10) = ( float )( cr*cp );
}


//-------------------------------------------------------------------------
//	construct orientation matrix
//-------------------------------------------------------------------------
void TLMaths::TMatrix::SetOrientation(float3& Foward,float3& Up,float3& Left)
{
	TLDebug_Break("Unwritten");
}

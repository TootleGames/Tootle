#include "TLMaths.h"
#include "TLDebug.h"
#include "TString.h"
#include "TLMemory.h"
#include "TLTime.h"
#include "TColour.h"

namespace TLMaths
{
	TMatrix		g_IdentityMatrix( float4(1,0,0,0), float4(0,1,0,0), float4(0,0,1,0), float4(0,0,0,1) );	//	identity matrix
}

namespace TLColour
{
	TFixedArray<TColour,8>	g_Debug_Colours(0);	//	static list of debug colours
}



//	gr: do not use this on ipod!
//USE_OPENGL_MATRIX_MULT


//-------------------------------------------------------------
//	maths system init, just seeds random number
//-------------------------------------------------------------
void TLMaths::Init()
{
	//	init random seed
	TLTime::TTimestamp TimeNow(TRUE);
	srand( TimeNow.GetTotalMilliSeconds() );

	//	init debug colours
	TLColour::g_Debug_Colours.Add( TColour( 1.0f, 1.0f, 1.0f ) );	//	white
	TLColour::g_Debug_Colours.Add( TColour( 1.0f, 0.0f, 0.0f ) );	//	red
	TLColour::g_Debug_Colours.Add( TColour( 0.0f, 1.0f, 0.0f ) );	//	green
	TLColour::g_Debug_Colours.Add( TColour( 0.0f, 0.0f, 1.0f ) );	//	blue
	TLColour::g_Debug_Colours.Add( TColour( 1.0f, 1.0f, 0.0f ) );	//	yellow
	TLColour::g_Debug_Colours.Add( TColour( 1.0f, 0.0f, 1.0f ) );	//	pink
	TLColour::g_Debug_Colours.Add( TColour( 0.0f, 1.0f, 1.0f ) );	//	cyan
//	TLColour::g_Debug_Colours.Add( TColour( 0.0f, 0.0f, 0.0f ) );	//	black
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
	Matrix.GetCol(kZ)[kW] = 0.0;
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

	TLDebug_Break("Not tested");
//	vector[0] = v[0]*Get(0)+v[1]*Get(4)+v[2]*Get(8)+Get(12);
//	vector[1] = v[0]*Get(1)+v[1]*Get(5)+v[2]*Get(9)+Get(13);
	vector[0] = v[0]*Get(0)+v[1]*Get(4)+Get(12);
	vector[1] = v[0]*Get(1)+v[1]*Get(5)+Get(13);

	v[0] = vector[0];
	v[1] = vector[1];
}


void TLMaths::TMatrix::SetRotationRadians(const float3& Angles)
{
	float cr = cosf( Angles[0] );
	float sr = sinf( Angles[0] );
	float cp = cosf( Angles[1] );
	float sp = sinf( Angles[1] );
	float cy = cosf( Angles[2] );
	float sy = sinf( Angles[2] );

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
	const float4& Otherxyzw = Other.xyzw;

	return TLMaths::TQuaternion(
		xyzw.w * Otherxyzw.x + xyzw.x * Otherxyzw.w + xyzw.y * Otherxyzw.z - xyzw.z * Otherxyzw.y,
		xyzw.w * Otherxyzw.y + xyzw.y * Otherxyzw.w + xyzw.z * Otherxyzw.x - xyzw.x * Otherxyzw.z,
		xyzw.w * Otherxyzw.z + xyzw.z * Otherxyzw.w + xyzw.x * Otherxyzw.y - xyzw.y * Otherxyzw.x,
		xyzw.w * Otherxyzw.w - xyzw.x * Otherxyzw.x - xyzw.y * Otherxyzw.y - xyzw.z * Otherxyzw.z); 
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

	xyzw.x = xyzw.w * other.x + xyzw.x * other.w + xyzw.y * other.z - xyzw.z * other.y,
	xyzw.y = xyzw.w * other.y + xyzw.y * other.w + xyzw.z * other.x - xyzw.x * other.z,
	xyzw.z = xyzw.w * other.z + xyzw.z * other.w + xyzw.x * other.y - xyzw.y * other.x,
	xyzw.w = xyzw.w * other.w - xyzw.x * other.x - xyzw.y * other.y - xyzw.z * other.z; 
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

void TLMaths::TQuaternion::SetEuler(float Pitch, float Yaw, float Roll)
{
	/*
	float cosy = cosf(yaw / 2.0F);
	float siny = sinf(yaw / 2.0F);
	float cosP = cosf(Pitch / 2.0F);
	float sinP = sinf(Pitch / 2.0F);
	float cosR = cosf(Roll / 2.0F);
	float sinR = sinf(Roll / 2.0F);
	SetValues(
		cosR * sinP * cosy + sinR * cosP * siny,
		cosR * cosP * siny - sinR * sinP * cosy,
		sinR * cosP * cosy - cosR * sinP * siny,
		cosR * cosP * cosy + sinR * sinP * siny
		);
	return *this;

  */
	
	TLMaths::TQuaternion Qx( float3(0, 0, 1), Pitch );	Qx.Normalise();
	TLMaths::TQuaternion Qy( float3(0, 1, 0), Yaw );		Qx.Normalise();
	TLMaths::TQuaternion Qz( float3(1, 0, 0), Roll );	Qx.Normalise();

	SetValues(0,0,0,0);
	*this *= Qx;
	*this *= Qy;
	*this *= Qz;

	/*
	TLMaths::TQuaternion Qx( float3(sinf(a/2), 0, 0), cosf(a/2) );
	TLMaths::TQuaternion Qy( float3(0, sinf(b/2), 0), cosf(b/2) );
	TLMaths::TQuaternion Qz( float3(0, 0, sinf(c/2)), cosf(c/2) );

	*this = Qx;
	*this *= Qy;
	*this *= Qz;
	*/
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
TLMaths::TQuaternion Slerp(const TLMaths::TQuaternion &From, const TLMaths::TQuaternion &To, float Interpolation)
{
	TLMaths::TQuaternion Temp;
	float omega, cosO, sinO;
	float scale0, scale1;

	cosO = DotProduct(From, To);

	if (cosO < 0.0)
	{
		cosO = -cosO;
		Temp = -To;
	}
	else
	{
		Temp = -To;
	}

	if ((1.0 - cosO) > ERROR_TOLERANCE)
	{
		omega = (float)acos(cosO);
		sinO = sinf(omega);
		scale0 = sinf((1.0f - Interpolation) * omega) / sinO;
		scale1 = sinf(Interpolation * omega) / sinO;
	}
	else
	{
		scale0 = 1.0f - Interpolation;
		scale1 = Interpolation;
	}
	return From*scale0 + Temp*scale1 ;
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
		float angle    = acosf(fabsf(cosAngle)); 
		float sinAngle = sinf(angle); 
		c1 = sinf(angle * (1.0f - Interpolation)) / sinAngle; 
		c2 = sinf(angle * Interpolation) / sinAngle; 
	} 

	// Use the shortest path 
	if ((cosAngle < 0.0) && AllowFlip) 
		c1 = -c1; 

	return TLMaths::TQuaternion(c1*From[0] + c2*To[0], c1*From[1] + c2*To[1], c1*From[2] + c2*To[2], c1*From[3] + c2*To[3]); 
} 
  */  
  

TLMaths::TQuaternion InterpQ(const TLMaths::TQuaternion& From, const TLMaths::TQuaternion& To, float Interpolation)
{
	if ( From == To )
		return From;
	
	//represent the same rotation, but when doing interpolation they are in different places in the space used for interpolation, and produce different results.

	//The solution it to check how far apart the quaternions are, and if necessary flip one so they are closer. E.g. add the lines.

	TLMaths::TQuaternion FlipTo( To );

	//	flip to, if neccesary
	//if (Q1.w * Q2.w + Q1.x * Q2.x + Q1.y * Q2.y + Q1.z * Q2.z < 0)
	if ( From.xyzw.DotProduct( FlipTo.xyzw ) < 0)
	{   
		//Q2.w = -Q2.w;    Q2.x = -Q2.x;    Q2.y = -Q2.y;    Q2.z = -Q2.z;
		FlipTo.Invert();
	}

	float& t = Interpolation;
	float mint = 1.f - t;
	const TLMaths::TQuaternion& Q1 = From;
	const TLMaths::TQuaternion& Q2 = FlipTo;

	// Q = Q1*(1-Interpolation) + Q2*Interpolation;   

	//	interp
	float4 q1 = Q1.xyzw * mint;
	float4 q2 = Q2.xyzw * t;

	TLMaths::TQuaternion Result( q1 + q2 );
	Result.Normalise();
	
	return Result;	
}




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
			float omega = acosf( cosom );
			float sinom = sinf( omega );
			sclFrom = sinf(( 1.0f-Interpolation )*omega )/sinom;
			sclTo = sinf( Interpolation*omega )/sinom;
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

		sclFrom = sinf(( 1.0f-Interpolation )*0.5f*PI );
		sclTo = sinf( Interpolation*0.5f*PI );

		Result.xyzw.x = (sclFrom*From.xyzw.x) + (sclTo*Result.xyzw.x);
		Result.xyzw.y = (sclFrom*From.xyzw.y) + (sclTo*Result.xyzw.y);
		Result.xyzw.z = (sclFrom*From.xyzw.z) + (sclTo*Result.xyzw.z);
		//	gr: no w?
	}

	return Result;
}

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

void TLMaths::TQuaternion::UnRotateVector(float3& Vector) const
{
	if ( !IsValid() )
		return;

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



//-----------------------------------------------------------
//	transform this by Trans.
//	THIS is the parent, Trans is the "child" transform
//-----------------------------------------------------------
void TLMaths::TTransform::Transform(const TLMaths::TTransform& Trans)
{
	if ( Trans.HasTranslate() )
	{
		float3 Translate = Trans.m_Translate;
	
		//	gr: this is from the render code, we transform the translate of the child by ourselves(the parent)
		//		so the translate gets rotated, scaled etc... theres a possibility this might be wrong for other things, but Im not sure if it is or not
		//		it's done BEFORE the merge of the rotation and the matrix of the child
		//		the order of the other parts doesn't matter as they dont interact, where as this does
		TransformVector( Translate );

		SetTranslate( Translate );
		TLDebug_CheckFloat( Translate );
	}


	if ( Trans.HasScale() )
	{
		if ( HasScale() )
			m_Scale *= Trans.m_Scale;
		else
			SetScale( Trans.m_Scale );

		TLDebug_CheckFloat( m_Scale );
	}

	if ( Trans.HasRotation() )
	{
		//	gr: needs normalising?
		if ( HasRotation() )
			m_Rotation *= Trans.m_Rotation;
		else
			SetRotation( Trans.m_Rotation );
		
		TLDebug_CheckFloat( m_Rotation );
	}

	if ( Trans.HasMatrix() )
	{
		if ( HasMatrix() )
			m_Matrix *= Trans.m_Matrix;
		else
			SetMatrix( Trans.m_Matrix );

		TLDebug_CheckFloat( m_Matrix );
	}


}

//-----------------------------------------------------------
//	transform vector
//-----------------------------------------------------------
void TLMaths::TTransform::TransformVector(float3& Vector) const
{
	TLDebug_CheckFloat( Vector );

	if ( HasScale() )
	{
		Vector *= GetScale();
		TLDebug_CheckFloat( Vector );
	}

	if ( HasRotation() )
	{
		GetRotation().RotateVector( Vector );
		TLDebug_CheckFloat( Vector );
	}

	if ( HasMatrix() )
	{
		GetMatrix().TransformVector( Vector );
		TLDebug_CheckFloat( Vector );
	}

	if ( HasTranslate() )
	{
		Vector += GetTranslate();
		TLDebug_CheckFloat( Vector );
	}
}

//-----------------------------------------------------------
//	transform vector
//-----------------------------------------------------------
void TLMaths::TTransform::TransformVector(float2& Vector) const
{
	TLDebug_CheckFloat( Vector );

	if ( HasScale() )
	{
		Vector *= GetScale();
		TLDebug_CheckFloat( Vector );
	}

	if ( HasRotation() )
	{
		GetRotation().RotateVector( Vector );
		TLDebug_CheckFloat( Vector );
	}

	if ( HasMatrix() )
	{
		GetMatrix().TransformVector( Vector );
		TLDebug_CheckFloat( Vector );
	}

	if ( HasTranslate() )
	{
		Vector += GetTranslate();
		TLDebug_CheckFloat( Vector );
	}
}

//-----------------------------------------------------------
//	untransform vector
//-----------------------------------------------------------
void TLMaths::TTransform::UntransformVector(float3& Vector) const
{
	TLDebug_CheckFloat( Vector );

	if ( HasTranslate() )
	{
		Vector -= GetTranslate();
		TLDebug_CheckFloat( Vector );
	}

	if ( HasRotation() )
	{
		TLDebug_Break("todo: undo transform vector");
		//GetRotation().UnTransformVector( m_Pos );
		TLDebug_CheckFloat( Vector );
	}

	if ( HasMatrix() )
	{
		TLDebug_Break("todo: undo transform vector");
		//GetMatrix().UnTransformVector( m_Pos );
		TLDebug_CheckFloat( Vector );
	}

	if ( HasScale() )
	{
		Vector /= GetScale();
		TLDebug_CheckFloat( Vector );
	}
}

/*
void TLMaths::TTransform::GetMatrix(TMatrix& Matrix)
{
	QuaternionToMatrix( m_qRotation, Matrix );

	Matrix.SetTranslate( m_fPosition );
	
	//	gr: need to implement scale!
	if ( m_fScale.LengthSq() != 1.f )
	{
		//TLDebug_Print("Scale not yet implemented in matrix...");
		//Matrix.SetScale( m_Scale );
	}
}
*/


//-----------------------------------------------------------
//	get angle from a vector
//-----------------------------------------------------------
void TLMaths::TAngle::SetAngle(const float2& Direction)
{
	float AngRad = atan2f( Direction.x, Direction.y );
	SetRadians( AngRad );
}



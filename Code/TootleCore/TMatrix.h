/*
 *  TMatrix.h
 *  TootleCore
 *
 *  Created by Duane Bradbury on 03/05/2010.
 *  Copyright 2010 Tootle. All rights reserved.
 *
 */

#pragma once

#include "TLTypes.h"
#include "TLDebug.h"


namespace TLMaths
{
	class TMatrix;
	
	class TQuaternion;
	
	
	//	some conversions - should be members really
	void			QuaternionToMatrix(const TQuaternion& Quaternion,TMatrix& Matrix);
	void			MatrixToQuaternion(const TMatrix& Matrix,TQuaternion& Quaternion);
	
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

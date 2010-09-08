/*
 *  TAxisAngle.h
 *  TootleCore
 *
 *  Created by Duane Bradbury on 03/05/2010.
 *  Copyright 2010 Tootle. All rights reserved.
 *
 */

#pragma once
#include "TLMaths.h"

namespace TLMaths
{
	class TAxisAngle;
}


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


#define TLBinary_TypeRef_TAxisAngle				TRef_Static5(A,x,i,s,A)
TLBinary_DeclareDataTypeRef( TLMaths::TAxisAngle,	TLBinary_TypeRef(TAxisAngle) );

/*
 *  TEuler.h
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
	class TEuler;
}


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

#define TLBinary_TypeRef_TEuler					TRef_Static5(E,u,l,e,r)
TLBinary_DeclareDataTypeWithRef( TLMaths::TEuler,		TLBinary_TypeRef(TEuler) );


/*
 *  TAudioListener.h
 *  TootleAudio
 *
 *  Created by Duane Bradbury on 17/03/2010.
 *  Copyright 2010 Tootle. All rights reserved.
 *
 */

#pragma once

#include <TootleCore/TLTypes.h>

namespace TLAudio
{
	class TListenerProperties;
}

// Data for the audio system listener
class TLAudio::TListenerProperties
{
public:
	float3		m_vPosition;
	float3		m_vVelocity;
	float3		m_vLookAt;
	float3		m_vUp;
};

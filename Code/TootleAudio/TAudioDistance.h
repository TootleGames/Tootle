/*
 *  TAudioDistance.h
 *  TootleAudio
 *
 *  Created by Duane Bradbury on 27/04/2010.
 *  Copyright 2010 Tootle. All rights reserved.
 *
 */

namespace TLAudio
{
	// Distance model wrapper
	typedef enum DistanceModel
	{
		Type2D = 0,				// No 3D audio processing within the low-level system
		Type3DLinear,			// Linear 3D audio processing within the low level audio system
		Type3DLinearClamped,	// Clamped linear 3D audio processing within the low level audio system
		Type3DInverse,			// Inverse 3D audio processing within the low level audio system
		Type3DInverseClamped,	// Clamped inverse 3D audio processing within the low level audio system
	};
};
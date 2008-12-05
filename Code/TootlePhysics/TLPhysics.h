
#pragma once

#include "TPhysicsgraph.h"

#include <TootleCore/TLTypes.h>

/*

Friction Coefficient Table

											Fk		Fs
Aluminuim on Aluminium						1.4		1.1
Aluminium on Steel							0.47	0.61
Copper on Steel								0.36	0.53
Steel on Steel								0.57	0.74
Nickel on Nickel							0.53	1.1
Glass on Glass								0.4		0.94
Copper on Glass								0.53	0.68
Oak on Oak (parallel to grain)				0.48	0.62
Oak on Oak (perpendicular to grain)			0.32	0.54
Rubber on concrete (dry)					0.9		1.0
Rubber on Concrete (wet)					0.25	0.3

*/

namespace TLPhysics
{
	/*
	// Force
	inline float		Force(float fMass, float fAcceleration);

	// Friction 
	inline float		Friction(float fFrictionCoefficient, float fNormal);
	inline float3		Friction(float fFrictionCoefficient, float3 vNormal);

	inline float3		MediumResistence(float fMediumCoefficient, float3 fInitialVelocity, float3 fGravity, float fTime);
	inline float3		TerminalVelocity(float fMediumCoefficient, float3 fGravity);

	// Projectiles
	inline float3		Position(float3 fInitialPosition, float3 fInitialVelocity, float3 fAcceleration, float fTime);

	inline float		MaximumHeight(float fInitialY, float fVelocityY, float fGravity);
	inline float		MaximumRange(float fVelocityX, float fVelocityZ, float fGravity);

	// Rotational forces
	inline float3		AngularVelocity();
	inline float3		Centrifugal(float fMass, float3 fAngularVelocity, float3 fRelativePosition);
	inline float		Coriolis();

	inline float3		CentreOfMass();

	inline float3		AngularMomentum(float fMass, float3 fRelativePosition, float3 fMomentum);
	inline float		Torque();

	inline float3		SpringVelocity( float fRestorationConstant);
	*/
};


/*
//
//	Force = Mass x Acceleration
//
inline float TLPhysics::Force(float fMass, float fAcceleration)
{
	return (fMass * fAcceleration);
}


//
//	Calculate the friction of an object moving on another
//	If the object is rested it should use the static coefficient (Fs)
//	otherwise use the kinetic coefficient (Fk)
//	The normal is the the normal between the object and the surface in a particular axis
//	Returns the friction as a force in an axis
//
inline float TLPhysics::Friction(float fFrictionCoefficient, float fNormal)
{
	return -fFrictionCoefficient*fNormal;
}

//
//	Returns friction as a vector
//
inline float3 TLPhysics::Friction(float fFrictionCoefficient, float3 vNormal)
{
	float3 vFriction;
	vFriction.x = Friction(fFrictionCoefficient, vNormal.x);
	vFriction.y = Friction(fFrictionCoefficient, vNormal.y);
	vFriction.z = Friction(fFrictionCoefficient, vNormal.z);
	return vFriction;
}

//
//	returns the medium resistence - reduction of an objects velocity within a medium
//
inline float3 TLPhysics::MediumResistence(float fMediumCoefficient, float3 fInitialVelocity, float3 fGravity, float fTime)
{
	float3 fGK = fGravity / fMediumCoefficient;	// g/k
	float fPow = -fMediumCoefficient * fTime;	//-kt
	float3 fa = fInitialVelocity - fGK;			//(v0 - g/k)
	float fExponent = expf(fPow);				//e^(-kt)

	return (fGK + (fa * fExponent));			// g/k + (v0 -g/k)e^(-kt)
}

//
//Returns the terminal velocity 
//
inline float3 TLPhysics::TerminalVelocity(float fMediumCoefficient, float3 fGravity)
{
	return (fGravity / fMediumCoefficient);	// g/k
}


//
//	Returns a position vector for an objects position at a specified time 
//	from an initial position and velocity with a constant acceleration
//
inline float3 TLPhysics::Position(float3 fInitialPosition, float3 fInitialVelocity, float3 fAcceleration, float fTime)
{
	float fTimeSquared = fTime*fTime;							// t^2
	float3 fAccelTime = (fAcceleration * fTimeSquared) / 2.0f;	// 1/2(at^2)
	
	float3 fVelTime = fInitialVelocity*fTime;					// v0t

	return (fInitialPosition + fVelTime + fAccelTime);			// x0 + v0t + 1/2at^2
}


//
//	Returns the greatest height obtained given a velocity in the y-axis and gravity
//
inline float TLPhysics::MaximumHeight(float fInitialY, float fVelocityY, float fGravity)
{
	float fVelocityYSquared = fVelocityY * fVelocityY;
	return (fInitialY + (fVelocityYSquared / (2.0f * fGravity)));
}


//
//	Returns the furthest distance travelled given a velocity in the x and z axis and gravity
//
inline float TLPhysics::MaximumRange(float fVelocityX, float fVelocityZ, float fGravity)
{
	return (2.0f * fVelocityX * fVelocityZ) / fGravity;
}


inline float3 TLPhysics::Centrifugal(float fMass, float3 fAngularVelocity, float3 fRelativePosition)
{
	float3 fCross1 = fAngularVelocity.CrossProduct(fRelativePosition);

	float3 fCross2 = fAngularVelocity.CrossProduct(fCross1);

	return (fCross2 * -fMass);
}

inline float TLPhysics::Coriolis()
{
	return 0.0f;
}

inline float3 TLPhysics::CentreOfMass()
{
	return float3(0,0,0);
}

inline float3 TLPhysics::AngularMomentum(float fMass, float3 fRelativePosition, float3 fMomentum)
{
	return float3(0,0,0);
}

inline float TLPhysics::Torque()
{
	return 0.0f;
}

inline float3 TLPhysics::SpringVelocity( float fRestorationConstant)
{
	return float3(0,0,0);
}
*/


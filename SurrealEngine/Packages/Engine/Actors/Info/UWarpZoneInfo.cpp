
#include "Precomp.h"
#include "UWarpZoneInfo.h"

void UWarpZoneInfo::Warp(vec3& Loc, vec3& Vel, Rotator& R)
{
	vec3 origin = WarpCoords().Origin;
	mat3 rotate = WarpCoords().ToMatrix();
	mat3 invrotate = mat3::transpose(rotate);

	// Transform from warp space:
	Loc = (invrotate * Loc) + origin;
	Vel = invrotate * Vel;

	// Rotate the rotator
	Rotator newRotation = Rotator::FromVector(rotate * (Coords::Rotation(R).ToMatrix() * vec4(1.0f, 0.0f, 0.0f, 1.0f)).xyz());
	R.Yaw = newRotation.Yaw;
	R.Pitch = newRotation.Pitch;
}

void UWarpZoneInfo::UnWarp(vec3& Loc, vec3& Vel, Rotator& R)
{
	vec3 origin = WarpCoords().Origin;
	mat3 rotate = WarpCoords().ToMatrix();
	mat3 invrotate = mat3::transpose(rotate);

	// Transform to warp space:
	Loc = rotate * (Loc - origin);
	Vel = rotate * Vel;

	// Rotate the rotator
	Rotator newRotation = Rotator::FromVector(invrotate * (Coords::Rotation(R).ToMatrix() * vec4(1.0f, 0.0f, 0.0f, 1.0f)).xyz());
	R.Yaw = newRotation.Yaw;
	R.Pitch = newRotation.Pitch;
}

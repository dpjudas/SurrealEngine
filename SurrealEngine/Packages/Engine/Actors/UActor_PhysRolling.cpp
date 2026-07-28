
#include "Precomp.h"
#include "UActor.h"
#include "VM/ScriptCall.h"
#include "Packages/Engine/Actors/Decoration/UDecoration.h"
#include "Packages/Engine/Actors/Info/ULevelInfo.h"
#include "Packages/Engine/Actors/Brush/UMover.h"
#include "Packages/Engine/Actors/Pawn/UPlayerPawn.h"
#include "Packages/Engine/Resources/Level/ULevel.h"
#include "Packages/Engine/Resources/Level/UModel.h"
#include "Engine.h"

void UActor::TickRolling(float elapsed)
{
	if (Region().ZoneNumber == 0)
	{
		CallEvent(this, EventName::FellOutOfWorld);
		return;
	}

	// Save our starting point and state

	OldLocation() = Location();
	bJustTeleported() = false;

	// Update the actor velocity based on the acceleration and zone

	UZoneInfo* zone = Region().Zone;

	float speed = length(Velocity());
	Velocity() = Velocity() - speed * (normalize(Velocity()) - normalize(Acceleration())) * zone->ZoneGroundFriction() * elapsed;
	Velocity() = Velocity() * (1.0f - zone->ZoneFluidFriction() * elapsed) + Acceleration() * elapsed;


	vec3 moveDelta = (Velocity() + zone->ZoneVelocity() * elapsed * 25.0f) * elapsed;
	CollisionHit hit = TryMove(moveDelta);

	if (hit.Fraction < 1.0f && hit.Normal.z < 0.7071f)
	{
		CallEvent(this, EventName::HitWall, { ExpressionValue::VectorValue(hit.Normal), ExpressionValue::ObjectValue(hit.Actor ? hit.Actor : Level()) });

		vec3 alignedDelta = (moveDelta - hit.Normal * dot(moveDelta, hit.Normal)) * (1.0f - hit.Fraction);
		if (dot(moveDelta, alignedDelta) >= 0.0f) // Don't end up going backwards
			TryMove(alignedDelta);
	}

	if (Physics() != PHYS_Rolling)
		return;

	float gravityDirection = zone->ZoneGravity().z > 0.0f ? 1.0f : -1.0f;
	constexpr float stepHeightRatio = 25.0f / 47.5f; // not sure what this should be. Appears that humans have a step height of 25.0 and collision height of 47.5, so I guess I'll use that ratio
	vec3 stepDownDelta(0.0f, 0.0f, gravityDirection * stepHeightRatio * CollisionHeight() * stepDownDeltaFactor);

	// Can we reach the ground from here?
	CollisionHit floorHit = TryMove(stepDownDelta, true);
	if (floorHit.Fraction == 1.0f || floorHit.Normal.z < 0.7071f)
	{
		// No we couldn't. We are falling
		SetPhysics(PHYS_Falling);
		SetBase(nullptr, true);
	}
	else
	{
		// We could reach the ground. Step down there.
		floorHit = TryMove(stepDownDelta);
		if (floorHit.Fraction != 1.0f)
			SetBase(floorHit.Actor, true);
	}

	if (!bJustTeleported())
		Velocity() = (Location() - OldLocation()) / elapsed;
}

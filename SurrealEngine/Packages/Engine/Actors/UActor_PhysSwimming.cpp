
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

void UActor::TickSwimming(float elapsed)
{
	// Only pawns can swim!
	UPawn* pawn = PreparePawnMovementTick();
	if (!pawn)
		return;

	// Update the actor velocity based on the acceleration and zone

	UZoneInfo* zone = Region().Zone;
	// UDecoration* decor = UObject::TryCast<UDecoration>(this);
	UPlayerPawn* player = UObject::TryCast<UPlayerPawn>(this);

	float maxSpeed = player ? player->WaterSpeed() : pawn->WaterSpeed() * pawn->DesiredSpeed();
	ApplyMovementAcceleration(elapsed, pawn->AccelRate() * 0.3f, zone->ZoneFluidFriction(), maxSpeed);

	//float gravityDirection = zone->ZoneGravity().z > 0.0f ? 1.0f : -1.0f;

	float timeLeft = elapsed;
	vec3 vel = Velocity() + zone->ZoneVelocity() * elapsed * 25.0f;

	//same as tick walking, having any velosity at all should probably enable this branch
	if (length(vel))
	{
		for (int iteration = 0; timeLeft > 0.0f && iteration < 5; iteration++)
		{
			if (ShouldAbortMovementTick(PHYS_Swimming))
				break;

			vec3 moveDelta = vel * timeLeft;

			CollisionHit hit = TryMove(moveDelta);
			timeLeft -= timeLeft * hit.Fraction;
			moveDelta = vel * timeLeft;

			if (hit.Fraction < 1.0f)
			{
				if (player && UObject::IsType<UDecoration>(hit.Actor) && UObject::Cast<UDecoration>(hit.Actor)->bPushable() && dot(hit.Normal, moveDelta) < -0.9f)
				{
					// We hit a pushable decoration that is facing our movement direction

					//same question as with tick walk -> why set teleport flag on decoration?
					bJustTeleported() = true;
					Velocity() = Velocity() * Mass() / (Mass() + hit.Actor->Mass());
					FireHitWall(hit);
					timeLeft = 0.0f;
				}
				else
				{
					// We hit a wall
					FireHitWall(hit);

					//removed the second scaling, that caused the "exiting the water is difficult" bug
					//it appears to not fix it completely, but it helps
					vec3 alignedDelta = moveDelta - hit.Normal * dot(moveDelta, hit.Normal);

					if (dot(moveDelta, alignedDelta) >= 0.0f) // Don't end up going backwards
					{
						hit = TryMove(alignedDelta);
						timeLeft -= timeLeft * hit.Fraction;
						if (hit.Fraction < 1.0f)
						{
							FireHitWall(hit);
						}
					}
					else
					{
						timeLeft = 0.0f;
					}
				}
			}
		}
	}

	RecomputeVelocityFromDisplacement(elapsed);

	if (!Region().Zone->bWaterZone())
	{
		if (Velocity().z > 0.0f)
		{
			Velocity().z = std::max(Velocity().z, 100.0f);
		}
		if (Physics() == PHYS_Swimming)
			SetPhysics(PHYS_Falling);
	}
}
